/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "c_base.h"
#include "v__objectLoan.h"
#include "v__entity.h"
#include "v_handle.h"
#include "v__participant.h"
#include "v__publisher.h"
#include "v__subscriber.h"
#include "v__writer.h"
#include "v_dataReader.h"
#include "v_dataReaderEntry.h"
#include "v__dataView.h"
#include "v__groupQueue.h"
#include "v__reader.h"
#include "v__deliveryService.h"
#include "v_partition.h"
#include "v__topic.h"
#include "v__topicImpl.h"
#include "v__topicAdapter.h"
#include "v__topicQos.h"
#include "v_query.h"
#include "v_status.h"
#include "v_serviceManager.h"
#include "v_service.h"
#include "v_serviceState.h"
#include "v_qos.h"
#include "v__spliced.h"
#include "v_waitset.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__collection.h"
#include "v__observable.h"
#include "v__listener.h"
#include "sd_serializerXML.h"

#include "vortex_os.h"
#include "os_report.h"

#if 0
#define _TRACE_EVENTS_ printf("PID<%d> : ", os_procIdSelf()); printf
#else
#define _TRACE_EVENTS_(...)
#endif

void
v_entityInit(
    _Inout_ v_entity e,
    _In_opt_z_ const c_char *name)
{
    assert(C_TYPECHECK(e,v_entity));

    e->name = c_stringNew(c_getBase(e),name);
    e->status = v_statusNew(e);
    e->listener = NULL;
    e->listenerInterest = 0;
    e->state = V_ENTITYSTATE_DISABLED;
    e->loan = NULL;
    v_observerInit(v_observer(e));
}

void
v_entityFree(
    v_entity e)
{
    assert(C_TYPECHECK(e,v_entity));

    v_observerFree(v_observer(e));
}

void
v_entityDeinit(
    v_entity e)
{
    assert(C_TYPECHECK(e,v_entity));

    if (e == NULL) {
        return;
    }
    v_observerDeinit(v_observer(e));
}

v_objectLoan
v_entityLoan(
    v_entity _this,
    c_bool subLoan)
{
    v_objectLoan loan = NULL;

    if (!_this->loan) {
        _this->loan = v_objectLoanNew(v_objectKernel(_this));
        if (!_this->loan) {
            OS_REPORT(OS_ERROR, "kernel::v_entityLoan", V_RESULT_INTERNAL_ERROR, "v_objectLoanNew failed");
            return NULL;
        }
    }

    /* An entity can either have a regular loan, or a loan that contains sub-loans.
     * A sub-loan is used to maintain multiple individual loans on a single v_entity,
     * i.e. for shared readers)
     */
    if (subLoan) {
        loan = v_objectLoanSubLoan(_this->loan);
    } else {
        loan = _this->loan;
    }

    return loan;
}

void
v_entityReleaseLoan(
    v_entity _this)
{
    if (_this->loan) {
        v_objectLoanRelease(_this->loan);
    }
}

v_status
v_entityStatus(
    v_entity e)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    return c_keep(e->status);
}

v_state
v_entityGetStatusFlags(
    v_entity e)
{
    v_state state;
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    OSPL_BLOCK_EVENTS(e);
    state = e->status->state;
    OSPL_UNBLOCK_EVENTS(e);
    return state;
}

void
v_entityStatusReset(
    v_entity e,
    c_ulong mask)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    if (e->status) {
        v_statusReset(e->status, mask);
    }
}

c_ulong
v_entityStatusGetMask(
    v_entity e)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    return v_statusGetMask(e->status);
}

c_ulong
v_entityGetTriggerValue(
    _In_ v_entity _this)
{
    return v_statusGetMask(_this->status);
}

v_entity
v_entityOwner (
    v_entity _this)
{
    v_entity parent;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entity));

    switch (v_objectKind(_this)) {
    case K_WRITER:
        parent = v_entity(v_writerPublisher(v_writer(_this)));
    break;
    case K_DATAREADER:
        parent = v_entity(v_readerSubscriber(v_reader(_this)));
    break;
    case K_PUBLISHER:
        parent = v_entity(v_publisherParticipant(v_publisher(_this)));
    break;
    case K_SUBSCRIBER:
        parent = v_entity(v_subscriberParticipant(v_subscriber(_this)));
    break;
    case K_TOPIC_ADAPTER:
        parent = v_entity(v_topicAdapterParticipant(v_topicAdapter(_this)));
    break;
    default:
        parent = NULL;
    break;
    }
    return parent;
}

c_longlong
v_entityGetProcessId(
    v_entity _this)
{
    v_entity e = _this;
    c_longlong processId = 0;

    while (e && v_objectKind(e) != K_PARTICIPANT) {
        e = v_entityOwner(e);
    }
    if (e) {
        processId = v_participantProcessId(v_participant(e));
    }
    return processId;
}

static v_result
v__entityEnable (
    _Inout_ v_entity e)
{
    v_result result;

    switch (v_objectKind(e)) {
        case K_TOPIC:
        case K_TOPIC_ADAPTER:
            result = v__topicEnable(v_topic(e));
        break;
        case K_WRITER:
            result = v_writerEnable(v_writer(e));
        break;
        case K_DATAREADER:
            result = v_dataReaderEnable(v_dataReader(e));
        break;
        case K_GROUPQUEUE:
            result = v_groupQueueEnable(v_groupQueue(e));
        break;
        case K_PUBLISHER:
            result = v_publisherEnable(v_publisher(e));
        break;
        case K_SUBSCRIBER:
            result = v__subscriberEnable(v_subscriber(e));
        break;
        case K_PARTICIPANT:
        case K_SERVICE:
        case K_SPLICED:
        case K_NETWORKING:
        case K_DURABILITY:
        case K_NWBRIDGE:
        case K_CMSOAP:
        case K_RNR:
        case K_DBMSCONNECT:
            result = v_participantEnable(v_participant(e));
            break;
        case K_DELIVERYSERVICE:
            result = v__deliveryServiceEnable(v_deliveryService(e));
            break;
        case K_SERVICEMANAGER:
        case K_NETWORKREADER:
        case K_DATAVIEW:
        case K_DOMAIN:
            result = V_RESULT_OK;
            break;
        case K_KERNEL:
            v_kernelEnable(v_kernel(e));
            result = V_RESULT_OK;
            break;
        default:
            result = V_RESULT_CLASS_MISMATCH;
            OS_REPORT(OS_ERROR,
                        "v_entityEnable", result,
                        "Supplied entity %s can not be enabled",
                        v_objectKindImage(v_object(e)));
            break;
    }
    return result;
}

v_result
v_entityEnable (
    _Inout_ v_entity e)
{
    v_result result;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    do {
        OSPL_LOCK(e);
        _TRACE_EVENTS_("Enabling entity '%s' (%s(%d)) (%d) %p\n", v_entityName(e), v_objectKindImage(v_object(e)), v_objectKind(e), e->state, e);
        switch (e->state) {
            case V_ENTITYSTATE_ENABLED:
                OSPL_UNLOCK(e);
                result = V_RESULT_OK;
            break;
            case V_ENTITYSTATE_ENABLING:
                OSPL_UNLOCK(e);
                {
                    /* TODO: Replace polling? */
                    os_time poll = {0, 100000000};
                    os_nanoSleep(poll);
                }
                result = V_RESULT_NOT_ENABLED;
            break;
            case V_ENTITYSTATE_DISABLED:
                e->state = V_ENTITYSTATE_ENABLING;
                OSPL_UNLOCK(e);
                result = v__entityEnable(e);
                OSPL_LOCK(e);
                e->state = (result == V_RESULT_OK) ? V_ENTITYSTATE_ENABLED : V_ENTITYSTATE_DISABLED;
                OSPL_UNLOCK(e);
                break;
            default:
                result = V_RESULT_NOT_ENABLED;
                break;
        }
    } while (result == V_RESULT_NOT_ENABLED);

    _TRACE_EVENTS_("Enabling entity %s (%d) %p returned %s\n", v_objectKindImage(v_object(e)), e->state, e, v_resultImage(result));

    return result;
}

c_bool
v_entityEnabled (
    _In_ v_entity _this)
{
    c_bool enabled;

    OSPL_LOCK(_this);
    enabled = v__entityEnabled_nl(_this);
    OSPL_UNLOCK(_this);

    return enabled;
}

c_bool
v__entityEnabled_nl (
    _In_ v_entity _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entity));

    return _this->state == V_ENTITYSTATE_ENABLED;
}

c_bool
v_entityDisabled (
    _In_ v_entity _this)
{
    c_bool disabled;

    OSPL_LOCK(_this);
    disabled = v__entityDisabled_nl(_this);
    OSPL_UNLOCK(_this);

    return disabled;
}

c_bool
v__entityDisabled_nl (
    _In_ v_entity _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entity));

    return _this->state == V_ENTITYSTATE_DISABLED;
}

c_voidp
v_entityGetUserData (
    v_entity e)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    return v_public(e)->userDataPublic;
}

c_voidp
v_entitySetUserData (
    v_entity e,
    c_voidp userDataPublic)
{
    c_voidp old;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    old = v_public(e)->userDataPublic;
    v_public(e)->userDataPublic = userDataPublic;
    return old;
}

typedef struct entryActionArg {
    c_action action;
    c_voidp arg;
} *entryActionArg_t;

static c_bool
entryAction (
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry = v_dataReaderEntry(o);
    entryActionArg_t a = (entryActionArg_t)arg;

    return a->action(v_entity(entry->topic),a->arg);
}

c_bool
v_entityWalkEntities(
    v_entity e,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    c_bool r = TRUE;

    OSPL_LOCK(e);
    switch(v_objectKind(e)){
    case K_DATAREADER:
        r = c_setWalk(v_collection(e)->queries,(c_action)action,arg);
        if (r == TRUE) {
            struct entryActionArg a;
            a.action = (c_action)action;
            a.arg = arg;
            v_readerWalkEntries_nl(v_reader(e),entryAction,&a);
        }
    break;
    case K_QUERY:
    case K_DATAREADERQUERY:
        r = c_setWalk(v_collection(e)->queries,(c_action)action,arg);
    break;
    case K_PARTICIPANT:
    case K_SERVICE:
    case K_SPLICED:
    case K_DURABILITY:
    case K_NWBRIDGE:
    case K_NETWORKING:
    case K_CMSOAP:
    case K_RNR:
    case K_DBMSCONNECT:
        r = c_setWalk(v_participant(e)->entities,(c_action)action,arg);
    break;
    case K_PUBLISHER:
        r = c_setWalk(v_publisher(e)->writers,(c_action)action,arg);
        if (r == TRUE) {
            r = c_tableWalk(v_publisher(e)->partitions->partitions,(c_action)action,arg);
        }
    break;
    case K_SUBSCRIBER:
        r = c_setWalk(v_subscriber(e)->readers,(c_action)action,arg);
        if (r == TRUE) {
            r = c_tableWalk(v_subscriber(e)->partitions->partitions,(c_action)action,arg);
        }
    break;
    case K_WRITER:
        r = action(v_entity(v_writer(e)->topic),arg);
    break;
    case K_KERNEL:
        r = c_tableWalk(v_kernel(e)->topics,(c_action)action,arg);
        if (r == TRUE) {
            r = c_tableWalk(v_kernel(e)->partitions,(c_action)action,arg);
        }
        if (r == TRUE) {
            r = c_setWalk(v_kernel(e)->participants,(c_action)action,arg);
        }
    break;
    default:
    break;
    }
    OSPL_UNLOCK(e);
    return r;
}

static c_bool
getTopic (
    c_object o,
    c_voidp arg)
{
    c_iter *iter = (c_iter *)arg;
    c_bool result = TRUE;

    if ((iter != NULL) &&
        (v_objectKind(o) == K_DATAREADERENTRY))
    {
        *iter = c_iterInsert(*iter,c_keep(v_dataReaderEntryTopic(o)));
    } else {
        result = FALSE;
    }
    return result;
}

#define DDS_268
#define DDS_269

c_bool
v_entityWalkDependantEntities(
    v_entity e,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    c_bool r;
    v_entity entity;
    c_iter iter;
#ifdef DDS_268
    c_iter iter2, iter3;
    v_entity entity2;
    v_topic topic;
#else
#ifdef DDS_269
    v_entity entity2;
#endif
#endif

    r = TRUE;

    switch(v_objectKind(e)){
    case K_DATAREADER:
        if (v_reader(e)->subscriber != NULL) {
            r = action(v_entity(v_reader(e)->subscriber), arg);
        } else {
            r = FALSE;
        }
    break;
    case K_PUBLISHER:
        if (v_publisher(e)->participant != NULL) {
            r = action(v_publisher(e)->participant, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_QUERY:
    case K_DATAREADERQUERY:
        if (v_query(e)->source != NULL) {
            r = action(v_query(e)->source, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_SUBSCRIBER:
        if (v_subscriber(e)->participant != NULL) {
            r = action(v_subscriber(e)->participant, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_WRITER:
        if (v_writer(e)->publisher != NULL) {
            r = action(v_writer(e)->publisher, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_TOPIC_ADAPTER:
    case K_TOPIC:
        if (v_objectKind(e) == K_TOPIC_ADAPTER) {
            topic = v_topic(v_topicAdapter(e)->topic);
        } else {
            topic = v_topic(e);
        }

        /**@todo lookup v_join*/

        /*lookup readers.*/
        iter = v_topicLookupReaders(topic);
        entity = v_entity(c_iterTakeFirst(iter));

        while (entity != NULL) {
            if (r == TRUE) {
                v_subscriber s;
                (void) action(entity, arg);
#ifdef DDS_269
                s = v_readerGetSubscriber(v_reader(entity));
                OSPL_LOCK(s);
                r = c_tableWalk(s->partitions->partitions,(c_action)action,arg);
                OSPL_UNLOCK(s);
                c_free(s);
#endif
            }
            c_free(entity);
            entity = v_entity(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

         /*lookup writers.*/
        if (r == TRUE) {
            iter = v_topicLookupWriters(topic);
            entity = v_entity(c_iterTakeFirst(iter));

            while(entity != NULL){
               if (r == TRUE) {
                   v_publisher p;
                   (void) action(entity, arg);
#ifdef DDS_269
                   p = v_writerGetPublisher(v_writer(entity));
                   OSPL_LOCK(p);
                   r = c_tableWalk(p->partitions->partitions,(c_action)action,arg);
                   OSPL_UNLOCK(p);
                   c_free(p);
#endif
               }
               c_free(entity);
               entity = v_entity(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);
        }
    break;
    case K_DOMAIN:
        iter = v_partitionLookupPublishers(v_partition(e));
        entity = v_entity(c_iterTakeFirst(iter));
#ifdef DDS_268
        iter3 = c_iterNew(NULL);
#endif
        while (entity != NULL) {
            if (r == TRUE) {
                r = action(entity, arg);
#ifdef DDS_268
                if(r == TRUE){
                    iter2 = v_publisherLookupWriters(v_publisher(entity), "*");
                    entity2 = v_entity(c_iterTakeFirst(iter2));
                    while (entity2 != NULL) {
                        topic = c_keep(v_topic(v_writer(entity2)->topic));

                        if (c_iterContains(iter3, topic) == FALSE) {
                            iter3 = c_iterInsert(iter3, topic);
                        } else {
                            c_free(topic);
                        }
                        c_free(entity2);
                        entity2 = v_entity(c_iterTakeFirst(iter2));
                    }
                    c_iterFree(iter2);
                }
#endif
            }
            c_free(entity);
            entity = v_entity(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

        if (r == TRUE) {
            iter = v_partitionLookupSubscribers(v_partition(e));
            entity = v_entity(c_iterTakeFirst(iter));

            while (entity != NULL) {
                if(r == TRUE){
                    r = action(entity, arg);
#ifdef DDS_268
                    if(r == TRUE){
                        iter2 = v_subscriberLookupReadersByTopic(v_subscriber(entity), "*");
                        entity2 = v_entity(c_iterTakeFirst(iter2));
                        while (entity2 != NULL) {
                            if (c_iterContains(iter3, entity2) == FALSE) {
                                v_readerWalkEntries(v_reader(entity2),
                                                    getTopic,
                                                    &iter3);
                            }
                            c_free(entity2);
                            entity2 = v_entity(c_iterTakeFirst(iter2));
                        }
                        c_iterFree(iter2);
                    }
#endif
                }
                c_free(entity);
                entity = v_entity(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);
        }

#ifdef DDS_268
        entity = v_entity(c_iterTakeFirst(iter3));

        while (entity != NULL) {
           if (r == TRUE) {
               r = action(entity, arg);
           }
           c_free(entity);
           entity = v_entity(c_iterTakeFirst(iter3));
        }
        c_iterFree(iter3);
#endif
    break;
    default:
    break;
    }
    return r;
}

#undef DDS_268
#undef DDS_269

v_result
v_entitySetListener(
    v_entity _this,
    v_listener listener,
    void *listenerData,
    v_eventMask interest)
{
    C_STRUCT(v_event) event;
    v_eventMask obsolete_interest;
    v_eventMask additional_interest;
    c_voidp userData;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entity));
    assert(C_TYPECHECK(listener,v_listener));

    OSPL_LOCK(_this);

    additional_interest = interest & ~_this->listenerInterest;
    obsolete_interest = _this->listenerInterest & ~interest & ~V_EVENT_OBJECT_DESTROYED;

    /* When a the listener on an entity is removed or replaced the old listener may
     * still contain events with this entity as source. This may cause problems when
     * the entity is deleted. To synchronize the removal or the change in listener
     * a marker event has to be set in the old listener. Currently the PREPARE_DELETE
     * event is used for this purpose.
     */
    if (_this->listener && (_this->listener != listener)) {
        event.kind = V_EVENT_PREPARE_DELETE;
        event.source = v_observable(_this);
        event.data = NULL;
        if (!v_entityNotifyListener(_this, &event)) {
            OS_REPORT(OS_WARNING, "v_entitySetListener", V_RESULT_INTERNAL_ERROR,
                      "Failed to notify listener that listener has been removed from entity");
        }
    }

    c_free(_this->listener);
    _this->listener = c_keep(listener);
    _this->listenerInterest = interest;
    _this->listenerData = listenerData;

    OSPL_UNLOCK(_this);

    /* Now reset status flags for new interest so that the listener will be triggered.
     * Note that triggers are blocked once the flag is set.
     * And flush all events for obsolete interest.
     */
    userData = v_publicGetUserData(v_public(_this));
    v_entityStatusReset(_this, additional_interest);
    v_listenerFlush(_this->listener, obsolete_interest, userData);

    _TRACE_EVENTS_("v_entitySetListener: %s(0x%x), listener = 0x%x, interest = 0x%x\n",
                    v_objectKindImage(_this), _this, listener, interest);

    return V_RESULT_OK;
}

c_bool
v_entityNotifyListener(
    v_entity _this,
    v_event event)
{
    v_entity entity, subscriber;
    c_voidp source;
    c_bool notified = FALSE;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entity));

    _TRACE_EVENTS_("v_entityNotifyListener: Received event(0x%x) kind(0x%x) on %s(0x%x)\n",
                    event, event->kind, v_objectKindImage(_this),_this);

    /* If the OBJECT_DESTROYED is set then this entity is in process of being
     * destroyed, see v_entityDisableCallbacks where this flag is set.
     * In this case the event is ignored.
     * If there was listener interest then this operation should return TRUE as if
     * the event was handled by a listener as this will avoid triggering waitsets.
     * If there was no listener interest then this operation should return FALSE so
     * that waitsets (or any other kind of observer will be triggered.
     */
    entity = _this;
    if ((entity->listenerInterest & V_EVENT_OBJECT_DESTROYED) && !(event->kind & V_EVENT_OBJECT_DESTROYED)) {
        if (entity->listenerInterest & event->kind & ~V_EVENT_OBJECT_DESTROYED) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    /* first this operation verifies for data availability events
     * if any data on readers listeners are set.
     */
    if (v_eventTest(event->kind, V_EVENT_DATA_AVAILABLE)) {
        switch(v_objectKind(v_object(_this))) {
        case K_DATAREADER:
            _TRACE_EVENTS_("v_entityNotifyListener: -- The event is DATA_AVAILABLE on DataReader(0x%x)\n", _this);

            entity = v_entityOwner(_this);
            subscriber = entity;
            while (entity && !notified) {
                _TRACE_EVENTS_("v_entityNotifyListener: -- Check %s(0x%x) listener interest for event(DATA_AVAILABLE)\n",
                               v_objectKindImage(entity), entity);
                if (entity->listenerInterest & V_EVENT_ON_DATA_ON_READERS) {
                    _TRACE_EVENTS_("v_entityNotifyListener: -- The DATA_AVAILABLE event matches %s(0x%x) interest(0x%x)\n",
                                   v_objectKindImage(entity), entity, entity->listenerInterest);
                    if (entity->listener) {
                        /* TODO: replace v_publicGetUserData */
                        event->kind = V_EVENT_ON_DATA_ON_READERS;
                        source = event->source;
                        event->source = v_observable(subscriber);
                        _TRACE_EVENTS_("v_entityNotifyListener (DataReader) : -- %s(0x%x) calls listener(0x%x) source = 0x%x\n",
                                        v_objectKindImage(entity), entity, entity->listener, event->source);
                        /* Do not reset the DATA_AVAILABLE status prior to invoking the on_data_on_readers on the SubscriberListener.
                         * This status needs to remain set, so that notify_datareaders can see which readers it will need to invoke.
                         * The DATA_AVAILABLE flag will not notify the StatusCondition in this case, and will be reset by the first
                         * read/take action on this particular reader.
                         */
                        v_listenerNotify(entity->listener, event, entity);
                        event->source = source;
                        notified = TRUE;
                    }
                } else {
                    entity = v_entityOwner(entity);
                }
            }
        break;
        case K_DATAREADERQUERY:
        case K_DATAVIEWQUERY:
            if (_this->listener) {
                _TRACE_EVENTS_("v_entityNotifyListener (Query): -- %s(0x%x) calls listener(0x%x) source = 0x%x\n",
                               v_objectKindImage(entity), entity, entity->listener, event->source);
                if (v_queryTestSample(v_query(_this), event->data)) {
                    v_listenerNotify(_this->listener, event, _this);
                }
                notified = TRUE;
            }
        break;
        default:
        break;
        }
    }

    /* If no data on readers is triggered then find the first available
     * listener going up the hierarchy.
     * Destroyed events are non maskable and must always be passed to listeners.
     */
    entity = _this;
    while ((entity != NULL) && (!notified)) {
        if ((entity->listener) && (event->kind & (entity->listenerInterest | V_EVENT_OBJECT_DESTROYED | V_EVENT_PREPARE_DELETE)))
        {
            _TRACE_EVENTS_("v_entityNotifyListener: -- Event(0x%x) matches %s(0x%x) interest(0x%x) so notify listener\n",
                           event->kind, v_objectKindImage(entity), entity, entity->listenerInterest);
            /* workaround to reset the topic listener when set on a participant */
            if (_this != v_entity(event->source) && v_objectKind(v_object(v_entity(event->source))) == K_TOPIC) {
                v_entityStatusReset(v_entity(event->source), event->kind);
            } else {
                v_entityStatusReset(_this, event->kind);
            }
            v_listenerNotify(entity->listener, event, entity);
            notified = TRUE;
        } else {
            /* No listener is triggered so find the first available
             * listener going up the hierarchy.
             */
            entity = v_entityOwner(entity);
            if (entity == NULL) {
                _TRACE_EVENTS_("v_entityNotifyListener: -- No matching listener found for event(0x%x) on %s(0x%x)\n",
                                event, v_objectKindImage(_this), _this);
            }
        }
    }
    return notified;
}

/* This operation should be called by the deinit of each entity to disable
 * the execution of listener callbacks on this entity.
 * This does not affect normal observer-observable notify mechanism.
 * This operation will send an OBJECT_DESTROYED event to notify listeners threads
 * to finish outstanding events and then trigger the entity that it can continue
 * the deinit call.
 * The OBJECT_DESTROYED flag in the entity's listenerInterest is set to discard
 * all events that occur after this call, the v_entityNotifyListener call will
 * check for this flag before notifying listener threads.
 */
c_bool
v_entityDisableCallbacks (
    v_entity e)
{
    c_bool triggered;
    C_STRUCT(v_event) event;

    OSPL_LOCK(e);

    event.kind = V_EVENT_OBJECT_DESTROYED;
    event.source = v_observable(e);
    event.data = NULL;
    e->listenerInterest |= V_EVENT_OBJECT_DESTROYED;
    triggered = v_entityNotifyListener(e, &event);
    OSPL_UNLOCK(e);

    return triggered;
}

c_char *
v_entityGetXMLQos(
    v_entity e)
{
    c_type type;
    sd_serializer ser;
    sd_serializedData data;
    c_char *xml = NULL;
    v_qos qos;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    switch(v_objectKind(e)){
    case K_PARTICIPANT:
    case K_SERVICE:
    case K_SPLICED:
    case K_NETWORKING:
    case K_DURABILITY:
    case K_NWBRIDGE:
    case K_CMSOAP:
    case K_RNR:
    case K_DBMSCONNECT:
        qos = v_qos(c_keep(v_participant(e)->qos));
    break;
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        qos = v_qos(v_topicQosRef(v_topic(e)));
    break;
    case K_WRITER:
        qos = v_qos(v_writer(e)->qos);
    break;
    case K_DATAREADER:
    case K_GROUPQUEUE:
    case K_NETWORKREADER:
        qos = v_qos(v_reader(e)->qos);
    break;
    case K_PUBLISHER:
        qos = v_qos(v_publisher(e)->qos);
    break;
    case K_SUBSCRIBER:
        qos = v_qos(v_subscriber(e)->qos);
    break;
    case K_DATAVIEW:
        qos = v_qos(v_dataView(e)->qos);
    break;
    default:
        qos = NULL;
        OS_REPORT(OS_ERROR,
                    "v_entityGetXMLQos", V_RESULT_ILL_PARAM,
                    "Supplied entity (%d) has no QoS",
                    v_objectKind(e));
    break;
    }
    if (qos != NULL) {
        type = c_getType(qos);
        assert(type != NULL);
        ser = sd_serializerXMLNewTyped(type);
        assert(ser != NULL);
        data = sd_serializerSerialize(ser, qos);
        assert(data != NULL);
        xml = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
    }
    return xml;
}

static v_qos
createQosFromXML(
    c_type qosType,
    const c_char *xml)
{
    sd_serializer ser;
    sd_serializedData data;
    v_qos vqos;

    assert(qosType != NULL);
    assert(xml != NULL);

    ser = sd_serializerXMLNewTyped(qosType);
    data = sd_serializerFromString(ser, xml);
    vqos = (v_qos)(sd_serializerDeserialize(ser, data));
    if (!vqos) {
        OS_REPORT(OS_ERROR, "kernel::v_entity::createQosFromXML", V_RESULT_INTERNAL_ERROR,
                    "Creation of qos failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));
    }
    sd_serializedDataFree(data);
    sd_serializerFree(ser);

    return vqos;
}

v_result
v_entitySetXMLQos(
    v_entity e,
    const c_char *xml)
{
    v_result result;
    v_qos qos;
    c_type type;
    c_base base;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));
    assert(xml != NULL);

    result = V_RESULT_INTERNAL_ERROR; /* i.e. xml serialization failure */

    base = c_getBase(e);
    switch (v_objectKind(e)) {
    case K_PARTICIPANT: /* intentionally no break */
    case K_SERVICE:     /* intentionally no break */
    case K_SPLICED:
    case K_NETWORKING:
    case K_DURABILITY:
    case K_NWBRIDGE:
    case K_CMSOAP:
    case K_RNR:
    case K_DBMSCONNECT:
        type = c_resolve(base, "kernelModuleI::v_participantQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_participantSetQos(v_participant(e), (v_participantQos)qos);
        }
    break;
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        type = c_resolve(base, "kernelModuleI::v_topicQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_topicSetQos(v_topic(e), (v_topicQos)qos);
        }
    break;
    case K_WRITER:
        type = c_resolve(base, "kernelModuleI::v_writerQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_writerSetQos(v_writer(e), (v_writerQos)qos);
        }
    break;
    case K_DATAREADER:
        type = c_resolve(base, "kernelModuleI::v_readerQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_dataReaderSetQos(v_dataReader(e), (v_readerQos)qos);
        }
    break;
    case K_NETWORKREADER:    /* intentionally no break */
    case K_GROUPQUEUE:
    break;
    case K_PUBLISHER:
        type = c_resolve(base, "kernelModuleI::v_publisherQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_publisherSetQos(v_publisher(e), (v_publisherQos)qos);
        }
    break;
    case K_SUBSCRIBER:
        type = c_resolve(base, "kernelModuleI::v_subscriberQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_subscriberSetQos(v_subscriber(e), (v_subscriberQos)qos);
        }
    break;
    case K_DATAVIEW:
        type = c_resolve(base, "kernelModuleI::v_dataViewQos");
        qos = createQosFromXML(type, xml);
        if (qos) {
            result = v_dataViewSetQos(v_dataView(e), (v_dataViewQos)qos);
        }
    break;
    default:
        result = V_RESULT_CLASS_MISMATCH;
        OS_REPORT(OS_ERROR,
                    "v_entitySetXMLQos", result,
                    "Supplied entity (%d) has no QoS",
                    v_objectKind(e));
    break;
    }

    return result;
}
v_result
v_entityGetProperty(
    const v_entity _this,
    const os_char * property,
    os_char ** value)
{
    v_result result = V_RESULT_UNSUPPORTED;
    *value = NULL;
    if (!os_strcasecmp(property, "isolatenode")) {
        os_uint32 isolate;
        result = V_RESULT_OK;

        v_kernelGetIsolate(v_objectKernel(_this), &isolate);
        switch (isolate) {
            case V_ISOLATE_DEAF:
                *value = os_strdup("deaf");
                break;
            case V_ISOLATE_MUTE:
                *value = os_strdup("mute");
                break;
            case V_ISOLATE_ALL:
                *value = os_strdup("true");
                break;
            case V_ISOLATE_NONE:
                *value = os_strdup("false");
                break;
            default:
                *value = NULL;
                result = V_RESULT_INTERNAL_ERROR;
                break;
        }
    } else {
        OS_REPORT(OS_ERROR,
                    "v_entityGetProperty", result,
                    "Supplied property %s is invalid", property);
    }
    return result;
}

v_result
v_entitySetProperty(
    const v_entity _this,
    const os_char * property,
    const os_char * value)
{
    v_result result = V_RESULT_UNSUPPORTED;
    if (!os_strcasecmp(property, "isolatenode")) {
        result = V_RESULT_OK;
        if (!os_strcasecmp(value, "false")) {
            v_kernelSetIsolate(v_objectKernel(_this), V_ISOLATE_NONE);
        } else if (!os_strcasecmp(value, "true")) {
            v_kernelSetIsolate(v_objectKernel(_this), V_ISOLATE_ALL);
        } else if (!os_strcasecmp(value, "deaf")) {
            v_kernelSetIsolate(v_objectKernel(_this), V_ISOLATE_DEAF);
        } else if (!os_strcasecmp(value, "mute")) {
            v_kernelSetIsolate(v_objectKernel(_this), V_ISOLATE_MUTE);
        } else {
            result = V_RESULT_ILL_PARAM;
        }
    } else {
        OS_REPORT(OS_ERROR,
                    "v_entitySetProperty", result,
                    "Supplied property %s is invalid", property);
    }
    return result;
}
