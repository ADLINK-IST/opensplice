/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_statusCondition.h"
#include "sac_domainParticipant.h"
#include "sac_topic.h"
#include "sac_publisher.h"
#include "sac_subscriber.h"
#include "sac_dataWriter.h"
#include "sac_dataReader.h"
#include "sac_objManag.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_listenerDispatcher.h"

#include "u_observable.h"
#include "v_entity.h"
#include "v_status.h"
#include "os_atomics.h"
#include "sac_report.h"

#define DDS_EntityClaim(_this, entity) \
        DDS_Object_claim(_this, DDS_ENTITY, (_Object *)entity)
#define DDS_EntityClaimRead(_this, entity) \
        DDS_Object_claim(_this, DDS_ENTITY, (_Object *)entity)
#define DDS_EntityRelease(_this) \
        DDS_Object_release(_this)
#define DDS_EntityCheck(_this, entity) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_ENTITY, (_Object *)entity)

DDS_ReturnCode_t
_Entity_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;
    u_result uResult;
    _Entity entity;

    entity = _Entity(_this);
    if (entity != NULL) {
        result = DDS__free(entity->statusCondition);
        assert(result == DDS_RETCODE_OK);
        if (entity->uEntity != NULL) {
            uResult = u_objectFree_s(u_object(entity->uEntity));
            result = DDS_ReturnCode_get(uResult);
            assert(result == DDS_RETCODE_OK);
        } else {
            result = DDS_RETCODE_OK;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Entity = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        if (entity->userData) {
            DDS_Entity_release_user_data(entity->userData);
        }
        entity->listenerDispatcher = NULL;
        entity->interest = 0;
        entity->listenerEnabled = FALSE;
        entity->uEntity = NULL;
    }
    return result;
}

DDS_ReturnCode_t
DDS_Entity_init (
    DDS_Entity _this,
    u_entity uEntity)
{
    DDS_ReturnCode_t result;
    _Entity entity;

    entity = _Entity(_this);
    if (entity != NULL) {
        /* TODO: This 'if (uEntity != NULL)' is added in case a contentFilteredTopic
         * is being created, because than the uEntity is NULL. This should be taken
         * care of properly in the future.
         * Previously it wasn't a big problem, but now the user layer is more strict
         * and asserts on the NULL pointer.
         */
        if (uEntity != NULL) {
            entity->uEntity = uEntity;
            u_observableSetUserData(u_observable(uEntity), entity);
            entity->handle = u_entityGetInstanceHandle(uEntity);
            DDS_Object_set_domain_id(_Object(_this), u_observableGetDomainId(u_observable(uEntity)));

            if (u_observableGetY2038Ready(u_observable(uEntity))) {
                entity->maxSupportedSeconds = OS_TIME_MAX_VALID_SECONDS;
            } else {
                entity->maxSupportedSeconds = INT32_MAX;
            }
            entity->userData = NULL;
        }
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Entity = NULL");
    }
    return result;
}

DDS_boolean
_Entity_is_enabled (
    _Entity _this)
{
    DDS_boolean result;

    if (_this) {
        result = u_entityEnabled(_this->uEntity);
    } else {
        result = FALSE;
    }
    return result;
}

u_entity
_Entity_get_user_entity (
    _Entity _this)
{
    return _this->uEntity;
}

u_entity
DDS_Entity_get_user_entity_for_test (
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    _Entity entity;
    u_entity uEntity = NULL;

    result = DDS_EntityCheck(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        uEntity = entity->uEntity;
    }
    return uEntity;
}

DDS_ReturnCode_t
DDS_Entity_get_user_entity(
    DDS_Entity _this,
    DDS_ObjectKind kind,
    u_entity *uEntity)
{
    DDS_ReturnCode_t result;
    _Entity entity;

    result = DDS_Object_check(_this, kind);
    if (result == DDS_RETCODE_OK) {
        entity = _Entity(_this);
        if (entity->uEntity == NULL) {
            result = DDS_RETCODE_ALREADY_DELETED;
            SAC_REPORT(result, "Entity is already deleted");
        } else {
            *uEntity = entity->uEntity;
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_Entity_set_listenerDispatcher (
    DDS_Entity _this,
    cmn_listenerDispatcher listenerDispatcher)
{
    DDS_ReturnCode_t result;
    _Entity entity;

    SAC_REPORT_STACK();

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        if (entity->listenerDispatcher == NULL) {
            entity->listenerDispatcher = listenerDispatcher;
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "ListenerDispatcher already set");
        }
        DDS_EntityRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

cmn_listenerDispatcher
DDS_Entity_get_listenerDispatcher (
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    _Entity entity;
    cmn_listenerDispatcher listenerDispatcher = NULL;

    SAC_REPORT_STACK();

    result = DDS_EntityCheck(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        listenerDispatcher = entity->listenerDispatcher;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return listenerDispatcher;
}

/*
 *     ReturnCode_t
 *     enable();
 */
DDS_ReturnCode_t
DDS_Entity_enable (
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    _Entity entity;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        uResult = u_entityEnable(entity->uEntity);
        result = DDS_ReturnCode_get(uResult);
        DDS_EntityRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*
 *     StatusCondition
 *     get_statuscondition();
 */
DDS_StatusCondition
DDS_Entity_get_statuscondition (
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    DDS_StatusCondition sc = NULL;
    _Entity entity;

    SAC_REPORT_STACK();

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        if (entity->statusCondition == NULL) {
            entity->statusCondition = DDS_StatusConditionNew(_this);
        }
        sc = entity->statusCondition;
        DDS_EntityRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return sc;
}

static void
getStatusMask(
    v_public p,
    c_voidp arg)
{
    DDS_StatusMask *StatusMask = (DDS_StatusMask *)arg;
    c_ulong events;

    events = v_statusGetMask(v_entity(p)->status);
    switch(v_objectKind(v_object(p))) {
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        if ( events & V_EVENT_INCONSISTENT_TOPIC ) {
            *StatusMask |= DDS_INCONSISTENT_TOPIC_STATUS;
        }
        if ( events & V_EVENT_ALL_DATA_DISPOSED ) {
            *StatusMask |= DDS_ALL_DATA_DISPOSED_TOPIC_STATUS;
        }
    break;
    case K_SUBSCRIBER:
        if ( events & V_EVENT_ON_DATA_ON_READERS ) {
            *StatusMask |= DDS_DATA_ON_READERS_STATUS;
        }
    break;
    case K_WRITER:
        if ( events & V_EVENT_LIVELINESS_LOST ) {
            *StatusMask |= DDS_LIVELINESS_LOST_STATUS;
        }
        if ( events & V_EVENT_OFFERED_DEADLINE_MISSED ) {
            *StatusMask |= DDS_OFFERED_DEADLINE_MISSED_STATUS;
        }
        if ( events & V_EVENT_OFFERED_INCOMPATIBLE_QOS ) {
            *StatusMask |= DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( events & V_EVENT_PUBLICATION_MATCHED ) {
            *StatusMask |= DDS_PUBLICATION_MATCHED_STATUS;
        }
    break;
    case K_READER:
    case K_GROUPQUEUE:
    case K_DATAREADER:
    case K_GROUPSTREAM:
        if ( events & V_EVENT_SAMPLE_REJECTED ) {
            *StatusMask |= DDS_SAMPLE_REJECTED_STATUS;
        }
        if ( events & V_EVENT_LIVELINESS_CHANGED ) {
            *StatusMask |= DDS_LIVELINESS_CHANGED_STATUS;
        }
        if ( events & V_EVENT_REQUESTED_DEADLINE_MISSED ) {
            *StatusMask |= DDS_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if ( events & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            *StatusMask |= DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( events & V_EVENT_SUBSCRIPTION_MATCHED ) {
            *StatusMask |= DDS_SUBSCRIPTION_MATCHED_STATUS;
        }
        if ( events & V_EVENT_DATA_AVAILABLE ) {
            *StatusMask |= DDS_DATA_AVAILABLE_STATUS;
        }
        if ( events & V_EVENT_SAMPLE_LOST ) {
            *StatusMask |= DDS_SAMPLE_LOST_STATUS;
        }
    break;
    case K_PARTICIPANT:
    case K_PUBLISHER:
    case K_DOMAIN:
    case K_KERNEL:
    break;
    default:
        assert(FALSE);
    }
}

/*
 *     StatusKindMask
 *     get_status_changes();
 */
DDS_StatusMask
DDS_Entity_get_status_changes (
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    _Entity entity;
    DDS_StatusMask StatusMask = 0;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_EntityCheck(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        uResult = u_observableAction(u_observable(entity->uEntity), getStatusMask, &StatusMask);
        if (uResult != U_RESULT_OK) {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return StatusMask;
}

/*
 *     InstanceHandle_t
 *     get_instance_handle();
 */
DDS_InstanceHandle_t
DDS_Entity_get_instance_handle (
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    DDS_InstanceHandle_t handle;
    _Entity entity;

    SAC_REPORT_STACK();

    result = DDS_EntityCheck(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        handle = entity->handle;
    } else {
        handle = DDS_HANDLE_NIL_NATIVE;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return handle;
}

DDS_boolean
DDS_Entity_check_handle (
    DDS_Entity _this,
    DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    DDS_boolean equal = FALSE;
    _Entity entity;

    SAC_REPORT_STACK();

    result = DDS_EntityCheck(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        if ( handle == entity->handle ) {
            equal = TRUE;
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return equal;
}

DDS_ReturnCode_t
DDS_Entity_set_listener_interest (
    DDS_Entity _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Entity entity;
    u_result uResult;

    entity = _Entity(_this);
    assert (entity != NULL);

    if (mask != 0) {
        result = (DDS_ReturnCode_t)cmn_listenerDispatcher_add (
            entity->listenerDispatcher,
            entity->uEntity,
            NULL, NULL,
            DDS_StatusMask_get_eventMask (mask));
        if (result == DDS_RETCODE_OK) {
            entity->listenerEnabled = TRUE;
        }
    } else {
        uResult = u_entitySetListener (entity->uEntity, NULL, NULL, 0);
        result = DDS_ReturnCode_get (uResult);
        if (result == DDS_RETCODE_OK) {
            DDS_Entity_wait_listener_removed_wlReq (entity);
            result = (DDS_ReturnCode_t)cmn_listenerDispatcher_remove (
                entity->listenerDispatcher, entity->uEntity);
        }
    }

    if (result == DDS_RETCODE_OK) {
        entity->interest = mask;
    }

    return result;
}

DDS_StatusMask
DDS_Entity_get_listener_interest (
    DDS_Entity _this)
{
    _Entity entity = _Entity(_this);

    assert (entity != NULL);

    return entity->interest;
}

static DDS_ReturnCode_t
_Entity_set_listener_mask(
    _Entity _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    switch (_ObjectKind(_this)) {
    case DDS_DOMAINPARTICIPANT:
        result = DDS_DomainParticipant_set_listener_mask(_DomainParticipant(_this), mask);
        break;
    case DDS_PUBLISHER:
        result = DDS_Publisher_set_listener_mask(_Publisher(_this), mask);
        break;
    case DDS_SUBSCRIBER:
        result = DDS_Subscriber_set_listener_mask(_Subscriber(_this), mask);
        break;
    case DDS_DATAWRITER:
        result = DDS_DataWriter_set_listener_mask(_DataWriter(_this), mask);
        break;
    case DDS_DATAREADER:
        result = DDS_DataReader_set_listener_mask(_DataReader(_this), mask);
        break;
    case DDS_TOPIC:
        result = DDS_Topic_set_listener_mask(_Topic(_this), mask);
        break;
    default:
        result = DDS_RETCODE_BAD_PARAMETER;
        break;
    }

    return result;
}


DDS_ReturnCode_t
DDS_Entity_set_listener_mask(
    DDS_Entity _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Entity entity;

    SAC_REPORT_STACK();

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        if (entity->listenerEnabled) {
            _Entity_set_listener_mask(entity, mask);
        }
        DDS_EntityRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return result;
}


struct GetStatusMaskArgs {
    DDS_StatusMask mask;
    DDS_StatusMask enabled;
    DDS_boolean    clear;
};

static void
getStatusMaskAndClear(
    v_public p,
    c_voidp arg)
{
    struct GetStatusMaskArgs *info = arg;
    c_ulong events;
    c_ulong mask = 0;

    events = v_statusGetMask(v_entity(p)->status);
    switch(v_objectKind(v_object(p))) {
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        if ( (events & V_EVENT_INCONSISTENT_TOPIC) && (info->enabled & DDS_INCONSISTENT_TOPIC_STATUS) ) {
            info->mask |= DDS_INCONSISTENT_TOPIC_STATUS;
            mask |= V_EVENT_INCONSISTENT_TOPIC;
        }
        if ( (events & V_EVENT_ALL_DATA_DISPOSED) && (info->enabled & DDS_ALL_DATA_DISPOSED_TOPIC_STATUS) ) {
            info->mask |= DDS_ALL_DATA_DISPOSED_TOPIC_STATUS;
            mask |= V_EVENT_ALL_DATA_DISPOSED;
        }

    break;
    case K_SUBSCRIBER:
        if ( (events & V_EVENT_ON_DATA_ON_READERS) && (info->enabled & DDS_DATA_ON_READERS_STATUS) ) {
            info->mask |= DDS_DATA_ON_READERS_STATUS;
            mask |= V_EVENT_ON_DATA_ON_READERS;
        }
    break;
    case K_WRITER:
        if ( (events & V_EVENT_LIVELINESS_LOST) && (info->enabled & DDS_LIVELINESS_LOST_STATUS) ) {
            info->mask |= DDS_LIVELINESS_LOST_STATUS;
            mask |= V_EVENT_LIVELINESS_LOST;
        }
        if ( (events & V_EVENT_OFFERED_DEADLINE_MISSED) && (info->enabled & DDS_LIVELINESS_LOST_STATUS) ) {
            info->mask |= DDS_OFFERED_DEADLINE_MISSED_STATUS;
            mask |= V_EVENT_OFFERED_DEADLINE_MISSED;
        }
        if ( (events & V_EVENT_OFFERED_INCOMPATIBLE_QOS) && (info->enabled & DDS_LIVELINESS_LOST_STATUS) ) {
            info->mask |= DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
            mask |= V_EVENT_OFFERED_INCOMPATIBLE_QOS;
        }
        if ( (events & V_EVENT_PUBLICATION_MATCHED) && (info->enabled & DDS_PUBLICATION_MATCHED_STATUS) ) {
            info->mask |= DDS_PUBLICATION_MATCHED_STATUS;
            mask |= V_EVENT_PUBLICATION_MATCHED;
        }
    break;
    case K_READER:
    case K_GROUPQUEUE:
    case K_DATAREADER:
    case K_GROUPSTREAM:
        if ( (events & V_EVENT_SAMPLE_REJECTED) && (info->enabled & DDS_SAMPLE_REJECTED_STATUS) ) {
            info->mask |= DDS_SAMPLE_REJECTED_STATUS;
            mask |= V_EVENT_SAMPLE_REJECTED;
        }
        if ( (events & V_EVENT_LIVELINESS_CHANGED) && (info->enabled & DDS_LIVELINESS_CHANGED_STATUS) ) {
            info->mask |= DDS_LIVELINESS_CHANGED_STATUS;
            mask |= V_EVENT_LIVELINESS_CHANGED;
        }
        if ( (events & V_EVENT_REQUESTED_DEADLINE_MISSED) && (info->enabled & DDS_REQUESTED_DEADLINE_MISSED_STATUS) ) {
            info->mask |= DDS_REQUESTED_DEADLINE_MISSED_STATUS;
            mask |= V_EVENT_REQUESTED_DEADLINE_MISSED;
        }
        if ( (events & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) && (info->enabled & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS) ) {
            info->mask |= DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
            mask |= V_EVENT_REQUESTED_INCOMPATIBLE_QOS;
        }
        if ( (events & V_EVENT_SUBSCRIPTION_MATCHED) && (info->enabled & DDS_SUBSCRIPTION_MATCHED_STATUS) ) {
            info->mask |= DDS_SUBSCRIPTION_MATCHED_STATUS;
            mask |= V_EVENT_SUBSCRIPTION_MATCHED;
        }
        if ( (events & V_EVENT_DATA_AVAILABLE) && (info->enabled & DDS_DATA_AVAILABLE_STATUS) ) {
            info->mask |= DDS_DATA_AVAILABLE_STATUS;
            mask |= V_EVENT_DATA_AVAILABLE;
        }
        if ( (events & V_EVENT_SAMPLE_LOST) && (info->enabled & DDS_SAMPLE_LOST_STATUS) ) {
            info->mask |= DDS_SAMPLE_LOST_STATUS;
            mask |= V_EVENT_SAMPLE_LOST;
        }
    break;
    case K_PARTICIPANT:
    case K_PUBLISHER:
    case K_DOMAIN:
    case K_KERNEL:
    break;
    default:
        assert(FALSE);
    }

    if (info->clear && mask) {
        v_statusReset(v_entity(p)->status, mask);
    }
}

DDS_ReturnCode_t
DDS_Entity_read_status(
    DDS_Entity _this,
    DDS_StatusMask *status,
    DDS_StatusMask mask,
    DDS_boolean clear)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    struct GetStatusMaskArgs args;
    _Entity entity;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        if (entity->statusCondition) {
            args.mask = 0;
            args.enabled = DDS_StatusCondition_get_enabled_statuses(entity->statusCondition) & mask;
            args.clear = clear;

            uResult = u_observableAction(u_observable(entity->uEntity), getStatusMaskAndClear, &args);
            if (uResult == U_RESULT_OK) {
                *status = args.mask;
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        } else {
            *status = 0;
        }
        DDS_EntityRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return result;
}



static void
resetDataAvailable(
   v_public p,
   c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    v_statusReset(v_entity(p)->status, V_EVENT_DATA_AVAILABLE);
}

DDS_ReturnCode_t
DDS_Entity_reset_dataAvailable_status (
    DDS_Entity _this)
{
    u_entity uEntity;
    u_result uResult;

/* TODO OSPL-8179: This is a bit tricky, the entity may already been deleted and in that case
 * this operation will perform a dirty memory read.
 * It may be better to wipe all pending events belonging to an entity when it is deleted or
 * if that is too intrusive find another way to safely detect/avoid deletion.
 */
    uEntity = _Entity_get_user_entity(_Entity(_this));
    uResult = u_observableAction(u_observable(uEntity), resetDataAvailable, NULL);
    return DDS_ReturnCode_get(uResult);
}

static void
resetOnDataOnReaders(
   v_public p,
   c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    v_statusReset(v_entity(p)->status, V_EVENT_ON_DATA_ON_READERS);
}

DDS_ReturnCode_t
DDS_Entity_reset_on_data_on_readers_status (
    DDS_Entity _this)
{
    u_entity uEntity;
    u_result uResult;

/* TODO OSPL-8179: This is a bit tricky, the entity may already been deleted and in that case
 * this operation will perform a dirty memory read.
 * It may be better to wipe all pending events belonging to an entity when it is deleted or
 * if that is too intrusive find another way to safely detect/avoid deletion.
 */
    uEntity = _Entity_get_user_entity(_Entity(_this));
    uResult = u_observableAction(u_observable(uEntity), resetOnDataOnReaders, NULL);
    return DDS_ReturnCode_get(uResult);
}


void
DDS_Entity_wait_listener_removed_wlReq(
    DDS_Entity _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (_this != NULL) {
        while ((result == DDS_RETCODE_OK) && _Entity(_this)->listenerEnabled) {
            result = DDS_Object_wait_wlReq(_this);
        }
    }
}

void
DDS_Entity_notify_listener_removed(
    DDS_Entity _this)
{
    DDS_ReturnCode_t result;
    _Entity entity;

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        if (entity->listenerEnabled) {
            entity->listenerEnabled = FALSE;
            (void)_Object_trigger_claimed(_Object(entity));
        }
        DDS_EntityRelease(_this);
    }
}

/* This operation disables the entity's triggering of listeners and notifies all
 * observing listeners by means of an OBJECT_DESTROYED event.
 * In case the event was delivered to a listener this operation will wait until
 * the listener has processed all events and triggers this entity about it.
 */
DDS_ReturnCode_t
DDS_Entity_disable_callbacks (
    DDS_Entity _this)
{
    u_entity uEntity;

    uEntity = _Entity_get_user_entity(_Entity(_this));
    if (u_entityDisableCallbacks(uEntity)) {
        _Entity(_this)->listenerEnabled = TRUE;
        DDS_Entity_wait_listener_removed_wlReq(_this);
    }
    return DDS_RETCODE_OK;
}

DDS_EntityKind_t
DDS_Entity_get_kind(
    DDS_Entity _this)
{
    DDS_EntityKind_t kind;

    switch(DDS_Object_get_kind(_this)) {
    case DDS_DOMAINPARTICIPANT:
        kind = DDS_ENTITY_KIND_DOMAINPARTICIPANT;
        break;
    case DDS_TOPIC:
        kind = DDS_ENTITY_KIND_TOPIC;
        break;
    case DDS_PUBLISHER:
        kind = DDS_ENTITY_KIND_PUBLISHER;
        break;
    case DDS_SUBSCRIBER:
        kind = DDS_ENTITY_KIND_SUBSCRIBER;
        break;
    case DDS_DATAWRITER:
        kind = DDS_ENTITY_KIND_DATAWRITER;
        break;
    case DDS_DATAREADER:
        kind = DDS_ENTITY_KIND_DATAREADER;
        break;
    default:
        kind = DDS_ENTITY_KIND_UNDEFINED;
        break;
    }
    return kind;
}

void
DDS_Entity_user_data_init(
   DDS_EntityUserData userData,
   DDS_Entity_user_data_destructor destructor)
{
    if (userData) {
        pa_st32(&userData->refCount, 1);
        userData->destructor = destructor;
    }
}

DDS_ReturnCode_t
DDS_Entity_set_user_data(
    DDS_Entity _this,
    DDS_EntityUserData userData)
{
    DDS_ReturnCode_t result;
    _Entity entity;

    result = DDS_EntityClaim(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        entity->userData = userData;
        pa_inc32(&entity->userData->refCount);
        DDS_EntityRelease(_this);
    }

    return result;
}

DDS_ReturnCode_t
DDS_Entity_claim_user_data(
    DDS_Entity _this,
    DDS_EntityUserData *userData)
{
    DDS_ReturnCode_t result;
    _Entity entity;

    if (userData) {
        result = DDS_EntityClaim(_this, &entity);
        if (result == DDS_RETCODE_OK) {
            *userData = entity->userData;
            pa_inc32(&entity->userData->refCount);
            DDS_EntityRelease(_this);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

void
DDS_Entity_release_user_data(
    DDS_EntityUserData userData)
{
    os_uint32 count;
    if (userData) {
        count = pa_dec32_nv(&userData->refCount);
        if (count == 0) {
            if (userData->destructor) {
                userData->destructor(userData);
            }
        }
    }
}

DDS_string
DDS_Entity_get_name(
    DDS_Entity _this)
{
    DDS_string name;
    os_char *uName;
    DDS_ReturnCode_t result;
    _Entity entity;

    result = DDS_EntityCheck(_this, &entity);
    if (result == DDS_RETCODE_OK) {
        uName = u_entityName(entity->uEntity);
        name = DDS_string_dup(uName);
        os_free(uName);
    } else {
        name = DDS_string_dup("<unknown>");
    }
    return name;
}
