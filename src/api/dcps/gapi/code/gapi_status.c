/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "gapi_entity.h"
#include "gapi_domainParticipant.h"
#include "gapi_topic.h"
#include "gapi_publisher.h"
#include "gapi_subscriber.h"
#include "gapi_dataReader.h"
#include "gapi_dataWriter.h"
#include "gapi_kernel.h"


#include "os_mutex.h"
#include "os_heap.h"
#include "os_report.h"

#include "u_entity.h"
#include "u_dispatcher.h"
#include "v_state.h"
#include "v_event.h"

#define TOPIC_STATUS_MASK           (GAPI_INCONSISTENT_TOPIC_STATUS|\
                                     GAPI_ALL_DATA_DISPOSED_STATUS)
#define READER_STATUS_MASK          (GAPI_SAMPLE_REJECTED_STATUS            |\
                                     GAPI_LIVELINESS_CHANGED_STATUS         |\
                                     GAPI_REQUESTED_DEADLINE_MISSED_STATUS  |\
                                     GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS |\
                                     GAPI_DATA_AVAILABLE_STATUS             |\
                                     GAPI_SAMPLE_LOST_STATUS                |\
                                     GAPI_SUBSCRIPTION_MATCH_STATUS)
#define WRITER_STATUS_MASK          (GAPI_LIVELINESS_LOST_STATUS            |\
                                     GAPI_OFFERED_DEADLINE_MISSED_STATUS    |\
                                     GAPI_OFFERED_INCOMPATIBLE_QOS_STATUS   |\
                                     GAPI_PUBLICATION_MATCH_STATUS)
#define SUBSCRIBER_STATUS_MASK      (GAPI_DATA_ON_READERS_STATUS)
#define PUBLISHER_STATUS_MASK       (GAPI_STATUS_KIND_NULL)
#define PARTICIPANT_STATUS_MASK     (GAPI_STATUS_KIND_NULL)


#define TOPIC_STATUS_INTEREST       (TOPIC_STATUS_MASK)
#define READER_STATUS_INTEREST      (READER_STATUS_MASK)
#define WRITER_STATUS_INTEREST      (WRITER_STATUS_MASK)
#define SUBSCRIBER_STATUS_INTEREST  (READER_STATUS_INTEREST    | SUBSCRIBER_STATUS_MASK)
#define PUBLISHER_STATUS_INTEREST   (WRITER_STATUS_INTEREST    | PUBLISHER_STATUS_MASK)
#define PARTICIPANT_STATUS_INTEREST (TOPIC_STATUS_INTEREST     | SUBSCRIBER_STATUS_INTEREST |\
                                     PUBLISHER_STATUS_INTEREST | PARTICIPANT_STATUS_MASK)

#define _STATUS_ENTITY(s)            (_Status(s)->entity)
#define _STATUS_PARENT(s)            (_Status(s)->parent)
#define _STATUS_USER_ENTITY(s)       (_Status(s)->userEntity)

C_STRUCT(_ListenerInterestInfo) {
    gapi_object handle;
    long depth;
    gapi_statusMask mask;
};

static _DomainParticipant
getParticipant (
    _Status status)
{
    _DomainParticipant participant;

    if ( _ObjectGetKind(_Object(status->entity)) == OBJECT_KIND_DOMAINPARTICIPANT ) {
        participant = (_DomainParticipant)status->entity;
    } else {
        participant = _EntityParticipant((_Entity)status->entity);
    }
    return participant;
}

static gapi_boolean
setEnabledMask(
    _Status status,
    _ListenerInterestInfo _info)
{
    long i;
    gapi_statusMask mask = GAPI_STATUS_KIND_NULL;

    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);

    for ( i = 0; i <= status->depth; i++ ) {
        mask |= status->listenerInfo[i].mask;
    }

    mask &= status->validMask;

    if ( mask != GAPI_STATUS_KIND_NULL ) {
        if ( status->enabled == GAPI_STATUS_KIND_NULL ) {
            _DomainParticipantAddListenerInterest(getParticipant(status),
                                                  status);
        }
    } else {
        if ( status->enabled != GAPI_STATUS_KIND_NULL ) {
            _DomainParticipantRemoveListenerInterest(getParticipant(status),
                                                     status);
        }
    }
    status->enabled = mask;

    return TRUE;
}

static gapi_boolean
setInterest (
    _Status status,
    _ListenerInterestInfo _info);

static c_bool
set_listener(
    u_entity entity,
    _ListenerInterestInfo info)
{
    gapi_dataWriter handle;
    _Entity e;
    gapi_boolean result;

    assert(entity);
    assert(info);

    handle = u_entityGetUserData(u_entity(entity));
    e = gapi_entityClaim(handle,NULL);
    if (e) {
        result = setInterest(e->status, info);
        _EntityRelease(e);
    }
    return TRUE;
}

static gapi_boolean
setInterest (
    _Status status,
    _ListenerInterestInfo _info)
{
    gapi_boolean result = TRUE;
    _Entity entity;

    if(status){
        assert(_info->depth >=0 && _info->depth < MAX_LISTENER_DEPTH);

        status->listenerInfo[_info->depth].handle = _info->handle;
        status->listenerInfo[_info->depth].mask   = _info->mask;

        setEnabledMask(status, _info);
        entity = status->entity;

        switch ( status->kind ) {
        case STATUS_KIND_PARTICIPANT:
            u_participantWalkTopics(U_PARTICIPANT_GET(entity),
                                    (u_topicAction)set_listener,
                                    (c_voidp)_info);

            u_participantWalkPublishers(U_PARTICIPANT_GET(entity),
                                        (u_publisherAction)set_listener,
                                        (c_voidp)_info);

            u_participantWalkSubscribers(U_PARTICIPANT_GET(entity),
                                         (u_subscriberAction)set_listener,
                                         (c_voidp)_info);
        break;
        case STATUS_KIND_PUBLISHER:
            u_publisherWalkWriters(U_PUBLISHER_GET(entity),
                                   (u_writerAction)set_listener,
                                   (c_voidp)_info);
        break;
        case STATUS_KIND_SUBSCRIBER:
            u_subscriberWalkReaders(U_SUBSCRIBER_GET(entity),
                                    (u_readerAction)set_listener,
                                    (c_voidp)_info);
        break;
        default:
        break;
        }
    }
    return result;
}

static void
findCommonInterest(
    _Status status,
    _Status requester)
{
    C_STRUCT(_ListenerInterestInfo) _info;

    _EntityClaim(status);

    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);

    _info.handle = _EntityHandle(status->entity);
    _info.depth  = status->depth;
    _info.mask   = status->listenerInfo[status->depth].mask;

    setInterest(requester, &_info);

    if ( status->parent ) {
        findCommonInterest(status->parent, requester);
    }

    _EntityRelease(status);
}

_Status
_StatusNew(
    _Entity entity,
    _StatusKind kind,
    const struct gapi_listener *info,
    gapi_statusMask mask)
{
    _Status status;

    assert(entity);

    status = (_Status)_ObjectAlloc(kind, C_SIZEOF(_Status), NULL);
    if (status) {
        _StatusInit(status, entity, kind, info, mask);
        _EntityRelease(status);
    }
    return status;
}

void
_StatusInit(
    _Status status,
    _Entity entity,
    _StatusKind kind,
    const struct gapi_listener *info,
    gapi_statusMask mask)
{
    _Entity factory;
    _Status parent;
    int i;

    assert(status);

    factory = _EntityGetFactory(entity);
    if (factory) {
        parent  = _EntityGetStatus(_Entity(factory));
    } else {
        parent = NULL;
    }

    status->entity = entity;
    status->parent = parent;
    status->kind = kind;
    status->depth = 0;
    status->validMask = 0;
    status->interestMask = 0;

    for ( i = 0; i < MAX_LISTENER_DEPTH; i++ ) {
        memset(&status->listenerInfo[i], 0, sizeof(_ListenerInfo));
    }

    status->enabled    = GAPI_STATUS_KIND_NULL;
    status->userEntity = _EntityUEntity(entity);

    _StatusSetListener(status, info, mask);

    if ( status->parent ) {
        findCommonInterest(status->parent, status);
    }
}

void
_StatusDeinit(
    _Status status)
{
    C_STRUCT(_ListenerInterestInfo) _info;

    assert(status);

    _info.handle = NULL;
    _info.depth  = status->depth;
    _info.mask   = GAPI_STATUS_KIND_NULL;

    setInterest(status, &_info);

    status->enabled = GAPI_STATUS_KIND_NULL;

    _EntityDelete(status);
}

static void
_StatusDefaultListener (
    _Entity _this,
    gapi_statusMask triggerMask)
{
}

gapi_boolean
_StatusSetListener(
    _Status status,
    const struct gapi_listener *info,
    gapi_statusMask mask)
{
    C_STRUCT(_ListenerInterestInfo) __info;

    status->notify = NULL;

    switch (status->kind) {
    case STATUS_KIND_DATAWRITER:
        status->depth = 2;
        status->validMask = WRITER_STATUS_MASK;
        status->interestMask = WRITER_STATUS_INTEREST;

        if (info) {
            const struct gapi_dataWriterListener *_info;

            _info = (const struct gapi_dataWriterListener *)info;
            status->callbackInfo.listenerData =
                   _info->listener_data;
            status->callbackInfo.on_offered_deadline_missed =
                   _info->on_offered_deadline_missed;
            status->callbackInfo.on_offered_incompatible_qos =
                   _info->on_offered_incompatible_qos;
            status->callbackInfo.on_liveliness_lost =
                   _info->on_liveliness_lost;
            status->callbackInfo.on_publication_match =
                   _info->on_publication_match;
        }
        status->notify = (ListenerAction)_DataWriterNotifyListener;
    break;
    case STATUS_KIND_DATAREADER:
        status->depth = 2;
        status->validMask = READER_STATUS_MASK;
        status->interestMask = READER_STATUS_INTEREST;

        if (info) {
            const struct gapi_dataReaderListener *_info;

            _info = (const struct gapi_dataReaderListener *)info;

            status->callbackInfo.listenerData =
                   _info->listener_data;
            status->callbackInfo.on_requested_deadline_missed =
                   _info->on_requested_deadline_missed;
            status->callbackInfo.on_requested_incompatible_qos =
                   _info->on_requested_incompatible_qos;
            status->callbackInfo.on_sample_rejected =
                   _info->on_sample_rejected;
            status->callbackInfo.on_liveliness_changed =
                   _info->on_liveliness_changed;
            status->callbackInfo.on_data_available =
                   _info->on_data_available;
            status->callbackInfo.on_subscription_match =
                   _info->on_subscription_match;
            status->callbackInfo.on_sample_lost =
                   _info->on_sample_lost;
        }
        status->notify = (ListenerAction)_DataReaderNotifyListener;
    break;
    case STATUS_KIND_PARTICIPANT:
        status->depth = 0;
        status->validMask = PARTICIPANT_STATUS_MASK;
        status->interestMask = PARTICIPANT_STATUS_INTEREST;

        if (info) {
            const struct gapi_domainParticipantListener *_info;

            _info = (const struct gapi_domainParticipantListener *)info;
            status->callbackInfo.listenerData =
                   _info->listener_data;
            status->callbackInfo.on_inconsistent_topic =
                   _info->on_inconsistent_topic;
            status->callbackInfo.on_all_data_disposed =
                   _info->on_all_data_disposed;
            status->callbackInfo.on_requested_deadline_missed =
                   _info->on_requested_deadline_missed;
            status->callbackInfo.on_requested_incompatible_qos =
                   _info->on_requested_incompatible_qos;
            status->callbackInfo.on_sample_rejected =
                   _info->on_sample_rejected;
            status->callbackInfo.on_liveliness_changed =
                   _info->on_liveliness_changed;
            status->callbackInfo.on_data_available =
                   _info->on_data_available;
            status->callbackInfo.on_subscription_match =
                   _info->on_subscription_match;
            status->callbackInfo.on_sample_lost =
                   _info->on_sample_lost;
            status->callbackInfo.on_data_on_readers =
                   _info->on_data_on_readers;
            status->callbackInfo.on_offered_deadline_missed =
                   _info->on_offered_deadline_missed;
            status->callbackInfo.on_offered_incompatible_qos =
                   _info->on_offered_incompatible_qos;
            status->callbackInfo.on_liveliness_lost =
                   _info->on_liveliness_lost;
            status->callbackInfo.on_publication_match =
                   _info->on_publication_match;
        }
        status->notify = _StatusDefaultListener;
    break;
    case STATUS_KIND_TOPIC:
        status->depth = 1;
        status->validMask = TOPIC_STATUS_MASK;
        status->interestMask = TOPIC_STATUS_INTEREST;

        if (info) {
            const struct gapi_topicListener *_info;

            _info = (const struct gapi_topicListener *)info;

            status->callbackInfo.listenerData =
                   _info->listener_data;
            status->callbackInfo.on_inconsistent_topic =
                   _info->on_inconsistent_topic;
            status->callbackInfo.on_all_data_disposed =
                   _info->on_all_data_disposed;
        }
        status->notify = (ListenerAction)_TopicNotifyListener;
    break;
    case STATUS_KIND_PUBLISHER:
        status->depth = 1;
        status->validMask = PUBLISHER_STATUS_MASK;
        status->interestMask = PUBLISHER_STATUS_INTEREST;

        if (info) {
            const struct gapi_publisherListener *_info;

            _info = (const struct gapi_publisherListener *)info;

            status->callbackInfo.listenerData =
                   _info->listener_data;
            status->callbackInfo.on_offered_deadline_missed =
                   _info->on_offered_deadline_missed;
            status->callbackInfo.on_offered_incompatible_qos =
                   _info->on_offered_incompatible_qos;
            status->callbackInfo.on_liveliness_lost =
                   _info->on_liveliness_lost;
            status->callbackInfo.on_publication_match =
                   _info->on_publication_match;
        }
        status->notify = _StatusDefaultListener;
    break;
    case STATUS_KIND_SUBSCRIBER:
        status->depth = 1;
        status->validMask = SUBSCRIBER_STATUS_MASK;
        status->interestMask = SUBSCRIBER_STATUS_INTEREST;

        if (info) {
            const struct gapi_subscriberListener *_info;

            _info = (const struct gapi_subscriberListener *)info;

            status->callbackInfo.listenerData =
                   _info->listener_data;
            status->callbackInfo.on_requested_deadline_missed =
                   _info->on_requested_deadline_missed;
            status->callbackInfo.on_requested_incompatible_qos =
                   _info->on_requested_incompatible_qos;
            status->callbackInfo.on_sample_rejected =
                   _info->on_sample_rejected;
            status->callbackInfo.on_liveliness_changed =
                   _info->on_liveliness_changed;
            status->callbackInfo.on_data_available =
                   _info->on_data_available;
            status->callbackInfo.on_subscription_match =
                   _info->on_subscription_match;
            status->callbackInfo.on_sample_lost =
                   _info->on_sample_lost;
            status->callbackInfo.on_data_on_readers =
                   _info->on_data_on_readers;
        }
        status->notify = (ListenerAction)_SubscriberNotifyListener;
    break;
    default:
        status->notify = NULL;
        status->depth = 0;
        status->validMask = 0;
        status->interestMask = 0;
        mask = GAPI_STATUS_KIND_NULL;
    break;
    }

    if (info == NULL) {
        mask = GAPI_STATUS_KIND_NULL;
    }

    if ( mask & GAPI_DATA_ON_READERS_STATUS ) {
        if ( (status->kind == STATUS_KIND_PARTICIPANT) ||
             (status->kind == STATUS_KIND_SUBSCRIBER) )
        {
            mask |= GAPI_DATA_AVAILABLE_STATUS;
        }
    }

    mask &= status->interestMask;

    __info.handle = _EntityHandle(status->entity);
    __info.depth  = status->depth;
    __info.mask   = mask;

    return setInterest(status, &__info);
}

/* Translates Event mask into Status mask */
gapi_statusMask
_StatusGetMaskStatus (
    _Status status,
    c_long  eventKindMask)
{
    gapi_statusMask StatusMask = GAPI_STATUS_KIND_NULL;

    switch ( status->kind ) {
    case STATUS_KIND_TOPIC:
        if ( eventKindMask & V_EVENT_INCONSISTENT_TOPIC ) {
            StatusMask |= GAPI_INCONSISTENT_TOPIC_STATUS;
        }
        if ( eventKindMask & V_EVENT_ALL_DATA_DISPOSED ) {
            StatusMask |= GAPI_ALL_DATA_DISPOSED_STATUS;
        }
    break;
    case STATUS_KIND_SUBSCRIBER:
        if ( eventKindMask & V_EVENT_DATA_AVAILABLE ) {
            StatusMask |= GAPI_DATA_ON_READERS_STATUS;
        }
    break;
    case STATUS_KIND_DATAWRITER:
        if ( eventKindMask & V_EVENT_LIVELINESS_LOST ) {
            StatusMask |= GAPI_LIVELINESS_LOST_STATUS;
        }
        if ( eventKindMask & V_EVENT_DEADLINE_MISSED ) {
            StatusMask |= GAPI_OFFERED_DEADLINE_MISSED_STATUS;
        }
        if ( eventKindMask & V_EVENT_INCOMPATIBLE_QOS ) {
            StatusMask |= GAPI_OFFERED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( eventKindMask & V_EVENT_TOPIC_MATCHED ) {
            StatusMask |= GAPI_PUBLICATION_MATCH_STATUS;
        }
    break;
    case STATUS_KIND_DATAREADER:
        if ( eventKindMask & V_EVENT_SAMPLE_REJECTED ) {
            StatusMask |= GAPI_SAMPLE_REJECTED_STATUS;
        }
        if ( eventKindMask & V_EVENT_LIVELINESS_CHANGED ) {
            StatusMask |= GAPI_LIVELINESS_CHANGED_STATUS;
        }
        if ( eventKindMask & V_EVENT_DEADLINE_MISSED ) {
            StatusMask |= GAPI_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if ( eventKindMask & V_EVENT_INCOMPATIBLE_QOS) {
            StatusMask |= GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( eventKindMask & V_EVENT_TOPIC_MATCHED ) {
            StatusMask |= GAPI_SUBSCRIPTION_MATCH_STATUS;
        }
        if ( eventKindMask & V_EVENT_DATA_AVAILABLE ) {
            StatusMask |= GAPI_DATA_AVAILABLE_STATUS;
        }
        if ( eventKindMask & V_EVENT_SAMPLE_LOST ) {
            StatusMask |= GAPI_SAMPLE_LOST_STATUS;
        }
    break;
    default:
    break;
    }

    return StatusMask;
}

gapi_statusMask
_StatusGetCurrentStatus(
    _Status status)
{
    long events = 0L;
    gapi_statusMask currentStatus;

    events = kernelStatusGet(status->userEntity);
    currentStatus = _StatusGetMaskStatus(status, events);
    if (status->kind == STATUS_KIND_SUBSCRIBER) {
        currentStatus &= GAPI_DATA_ON_READERS_STATUS;
    } else {
        currentStatus &= status->validMask;
    }
    return currentStatus;
}

gapi_object
_StatusFindTarget(
    _Status status,
    gapi_statusMask mask)
{
    gapi_object result = NULL;
    long i;

/* Searches bottom-up:
 * first check DataReader for listener and call it or,
 * in case of no listener, check the Subscriber and so on.
 */
    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);
    for ( i = status->depth; !result && i >= 0; i-- ) {
        if ( (status->listenerInfo[i].mask & mask) != GAPI_STATUS_KIND_NULL ) {
            result = status->listenerInfo[i].handle;
        }
    }
    return result;
}

void
_StatusNotifyEvent (
    _Status status,
    c_ulong events)
{
    gapi_statusMask triggerMask;
    if ( (status->enabled ) && (status->notify)) {
        triggerMask = _StatusGetMaskStatus(status, events);
        status->notify(status->entity, triggerMask);
    }
}

static void
resetDataAvailable(
   v_entity e,
   c_voidp arg)
{
    v_statusReset(e->status, V_EVENT_DATA_AVAILABLE);
}

void
_StatusNotifyDataAvailable (
    _Status status,
    gapi_object source)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_DataAvailableListener callback;
    u_result result;
    u_entity e = NULL;

    /* Check if interest for this event exists. */
    target = _StatusFindTarget(status, GAPI_DATA_AVAILABLE_STATUS);
    if (target) {
        /* A target is found so the data will be read thus reset the data available flag. */
        result = u_entityAction(U_ENTITY_GET(status->entity), resetDataAvailable, NULL);
        if (result == U_RESULT_OK) {
            /* get target listener and listener data. */
            if ( target != source ) {
                /* No listener set by this entity but an ancester does so get the
                 * listener with listener data from its ancester.
                 */
                entity = gapi_entityClaim(target, NULL);
                if (entity) {
                    callback = _EntityStatus(entity)->callbackInfo.on_data_available;
                    listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                    e = U_ENTITY_GET(entity);
                    if (e) {
                        result = u_entityAction(e, resetDataAvailable, NULL);
                    }
                    _EntityRelease(entity);
                } else {
                    OS_REPORT(OS_ERROR,
                              "_StatusNotifyDataAvailable", 0,
                              "Failed to claim target.");
                    callback = NULL;
                }
            } else {
                /* A listener is set for this entity, so get the listener and
                 * listener data from this entity.
                 */
                callback = status->callbackInfo.on_data_available;
                listenerData = status->callbackInfo.listenerData;
            }
            if (callback) {
                /* A listener will be called and thus reset the data on readers flag. */
                if (result == U_RESULT_OK) {
                    /* Temporary unlock status entity and call listener. */
                    _EntitySetBusy(status->entity);
                    _EntityRelease(status->entity);
                    callback(listenerData, source);
                    _EntityClaim(status->entity);
                    _EntityClearBusy(status->entity);
                }
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "_StatusNotifyDataAvailable", 0,
                      "Failed to reset data available flag.");
        }
    }
}

/* The status is from a DataReader */
c_bool
_StatusNotifyDataOnReaders (
    _Status status,
    gapi_object source)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_DataOnReadersListener callback;
    c_bool processed = FALSE;
    u_result result;
    u_entity e = NULL;

    /* Check if interest for this event exists. */
    target = _StatusFindTarget(status, GAPI_DATA_ON_READERS_STATUS);
    if (target) {
        /* A target is found so the data will be read thus reset the data available flag. */
        result = u_entityAction(U_ENTITY_GET(status->entity), resetDataAvailable, NULL);
        if (result == U_RESULT_OK) {
            /* get target listener and listener data. */
            if ( target != source ) {
                    /* No listener set by this entity but an ancester does so get the
                     * listener with listener data from its ancester.
                     */
                entity = gapi_entityClaim(target, NULL);
                if (entity) {
                    callback = _EntityStatus(entity)->callbackInfo.on_data_on_readers;
                    listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                    e = U_ENTITY_GET(entity);
                    _EntityRelease(entity);
                } else {
                    OS_REPORT(OS_ERROR,
                              "_StatusNotifyDataOnReaders", 0,
                              "Failed to claim target.");
                    callback = NULL;
                }
            } else {
                callback = status->callbackInfo.on_data_on_readers;
                listenerData = status->callbackInfo.listenerData;
                e = U_ENTITY_GET(status->entity);
            }
            if (callback) {
                /* A listener will be called and thus reset the data on readers flag. */
                if (e && u_entityKind(e) != U_READER) {
                    result = u_entityAction(e, resetDataAvailable, NULL);
                }
                if (result == U_RESULT_OK) {
                    /* Temporary unlock status entity and call listener. */
                    _EntitySetBusy(status->entity);
                    _EntityRelease(status->entity);
                    callback(listenerData, target);
                    processed = TRUE;
                    _EntityClaim(status->entity);
                    _EntityClearBusy(status->entity);
                } else {
                     OS_REPORT(OS_ERROR,
                               "_StatusNotifyDataOnReaders", 0,
                               "Failed to reset status flag, "
                               "listener is not called.");
                }
            }
        }
    }
    return processed;
}

/* Precondition: status must be locked when calling this method. */
void
_StatusNotifySubscriptionMatch (
    _Status status,
    gapi_object source,
    gapi_subscriptionMatchedStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_SubscriptionMatchedListener callback;

    target = _StatusFindTarget(status, GAPI_SUBSCRIPTION_MATCH_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_subscription_match;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifySubscriptionMatch", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_subscription_match;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyRequestedIncompatibleQos (
    _Status status,
    gapi_object source,
    gapi_requestedIncompatibleQosStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_RequestedIncompatibleQosListener callback;

    target = _StatusFindTarget(status, GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_requested_incompatible_qos;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyRequestedIncompatibleQos", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_requested_incompatible_qos;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyRequestedDeadlineMissed (
    _Status status,
    gapi_object source,
    gapi_requestedDeadlineMissedStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_RequestedDeadlineMissedListener callback;

    target = _StatusFindTarget(status, GAPI_REQUESTED_DEADLINE_MISSED_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_requested_deadline_missed;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyRequestedDeadlineMissed", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_requested_deadline_missed;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifySampleRejected (
    _Status status,
    gapi_object source,
    gapi_sampleRejectedStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_SampleRejectedListener callback;

    target = _StatusFindTarget(status, GAPI_SAMPLE_REJECTED_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_sample_rejected;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifySampleRejected", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_sample_rejected;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyLivelinessChanged (
    _Status status,
    gapi_object source,
    gapi_livelinessChangedStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_LivelinessChangedListener callback;

    target = _StatusFindTarget(status, GAPI_LIVELINESS_CHANGED_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_liveliness_changed;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyLivelinessChanged", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_liveliness_changed;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifySampleLost (
    _Status status,
    gapi_object source,
    gapi_sampleLostStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_SampleLostListener callback;

    target = _StatusFindTarget(status, GAPI_SAMPLE_LOST_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_sample_lost;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifySampleLost", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_sample_lost;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyPublicationMatch (
    _Status status,
    gapi_object source,
    gapi_publicationMatchedStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_PublicationMatchedListener callback;

    target = _StatusFindTarget(status, GAPI_PUBLICATION_MATCH_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_publication_match;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyPublicationMatch", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_publication_match;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyLivelinessLost (
    _Status status,
    gapi_object source,
    gapi_livelinessLostStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_LivelinessLostListener callback;

    target = _StatusFindTarget(status, GAPI_LIVELINESS_LOST_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_liveliness_lost;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyLivelinessLost", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_liveliness_lost;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyOfferedIncompatibleQos (
    _Status status,
    gapi_object source,
    gapi_offeredIncompatibleQosStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_OfferedIncompatibleQosListener callback;

    target = _StatusFindTarget(status, GAPI_OFFERED_INCOMPATIBLE_QOS_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_offered_incompatible_qos;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyOfferedIncompatibleQos", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_offered_incompatible_qos;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyOfferedDeadlineMissed (
    _Status status,
    gapi_object source,
    gapi_offeredDeadlineMissedStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_OfferedDeadlineMissedListener callback;

    target = _StatusFindTarget(status, GAPI_OFFERED_DEADLINE_MISSED_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_offered_deadline_missed;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyOfferedDeadlineMissed", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_offered_deadline_missed;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyAllDataDisposed (
    _Status status,
    gapi_object source)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_AllDataDisposedListener callback;

    target = _StatusFindTarget(status, GAPI_ALL_DATA_DISPOSED_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_all_data_disposed;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyAllDataDisposed", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_all_data_disposed;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

void
_StatusNotifyInconsistentTopic (
    _Status status,
    gapi_object source,
    gapi_inconsistentTopicStatus *info)
{
    gapi_object target;
    _Entity entity;
    c_voidp listenerData;
    gapi_listener_InconsistentTopicListener callback;

    target = _StatusFindTarget(status, GAPI_INCONSISTENT_TOPIC_STATUS);
    if (target) {
        /* get target listener and listener data. */
        if ( target != source ) {
            entity = gapi_entityClaim(target, NULL);
            if (entity) {
                callback = _EntityStatus(entity)->callbackInfo.on_inconsistent_topic;
                listenerData = _EntityStatus(entity)->callbackInfo.listenerData;
                _EntityRelease(entity);
            } else {
                OS_REPORT(OS_ERROR,
                          "_StatusNotifyInconsistentTopic", 0,
                          "Failed to claim target.");
                callback = NULL;
            }
        } else {
            callback = status->callbackInfo.on_inconsistent_topic;
            listenerData = status->callbackInfo.listenerData;
        }
        if (callback) {
            /* Temporary unlock status entity and call listener. */
            _EntitySetBusy(status->entity);
            _EntityRelease(status->entity);
            callback(listenerData, source, info);
            _EntityClaim(status->entity);
            _EntityClearBusy(status->entity);
        }
    }
}

