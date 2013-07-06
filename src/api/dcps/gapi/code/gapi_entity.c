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
#include "os_report.h"

#include "gapi.h"
#include "gapi_entity.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_status.h"
#include "gapi_kernel.h"
#include "gapi_builtin.h"

#include "u_types.h"

void
_EntityInit (
    _Entity entity,
    _Entity factory)
{

    entity->factory  = factory;
    entity->registry = _ObjectRegistryNew();
    entity->status = NULL;

    entity->StatusCondition = NULL;
}

void
_EntityDispose (
    _Entity entity)
{
    _ObjectRegistryFree(entity->registry);
    if ( entity->StatusCondition ) {
        _EntityClaim(entity->StatusCondition);
        _StatusConditionFree (entity->StatusCondition);
        entity->StatusCondition = NULL;
    }
    _EntityDelete(entity);
}


/*
 *     ReturnCode_t
 *     enable();
 */
gapi_returnCode_t
gapi_entity_enable (
    gapi_entity _this)
{
    _Entity entity;
    gapi_returnCode_t result;

    if (_this) {
        entity = gapi_entityClaim(_this, &result);
        if ( entity != NULL ) {
            u_entityEnable(entity->uEntity);
            _EntityRelease(entity);
            result = GAPI_RETCODE_OK;
        } else {
            result = GAPI_RETCODE_ALREADY_DELETED;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}

/*
 *     StatusCondition
 *     get_statuscondition();
 */
gapi_statusCondition
gapi_entity_get_statuscondition (
    gapi_entity _this)
{
    _Entity entity;
    _StatusCondition condition = NULL;

    entity = gapi_entityClaim(_this, NULL);

    if ( entity != NULL ) {
        if ( entity->StatusCondition == NULL ) {
            entity->StatusCondition = _StatusConditionNew(entity,
                                                          entity->uEntity);

            if ( entity->factory ) {
                _ENTITY_REGISTER_OBJECT(entity->factory,
                                        (_Object)entity->StatusCondition);
            } else {
                _ENTITY_REGISTER_OBJECT(entity,
                                        (_Object)entity->StatusCondition);
            }

            _EntityRelease(entity->StatusCondition);
        }

        condition = entity->StatusCondition;
        _EntityRelease(entity);
    }

    return (gapi_statusCondition)_EntityHandle(condition);
}

/*
 *     StatusMask
 *     get_status_changes();
 */
gapi_statusMask
gapi_entity_get_status_changes (
    gapi_entity _this)
{
    _Entity entity;
    gapi_statusMask result = GAPI_STATUS_KIND_NULL;

    entity = gapi_entityClaim(_this, NULL);
    if ( entity != NULL ) {
        result = _StatusGetCurrentStatus(entity->status);
        _EntityRelease(entity);
    }

    return result;
}

void
_EntitySetUserEntity (
    _Entity  entity,
    u_entity uEntity)
{
    entity->uEntity = uEntity;
    if ( uEntity ) {
        entity->kernelId = kernelGetKernelId(uEntity);
        u_entitySetUserData(uEntity, _EntityHandle(entity));
        entity->handle = u_entityGetInstanceHandle(uEntity);
    } else {
        entity->handle = GAPI_HANDLE_NIL;
    }
}

void
_EntityRegisterObject (
    _Entity entity,
    _Object object)
{
    assert(entity);
    assert(object);

    if ( entity->registry != NULL ) {
        _ObjectRegistryRegister(entity->registry, object);
    }
}

/*
 *     InstanceHandle_t
 *     get_instance_handle();
 */
gapi_instanceHandle_t
gapi_entity_get_instance_handle (
    gapi_entity _this)
{
    _Entity entity;
    gapi_instanceHandle_t handle = GAPI_HANDLE_NIL;

    entity = gapi_entityClaim(_this, NULL);
    if ( entity ) {
        handle = entity->handle;
        _EntityRelease(entity);
    }

    return handle;
}

gapi_boolean
_EntityHandleEqual (
    _Entity entity,
    gapi_instanceHandle_t handle)
{
    gapi_boolean result = FALSE;

    assert(entity);

    if ( handle == entity->handle ) {
        result = TRUE;
    }

    return result;
}

void
_EntityNotifyInitialEvents (
    _Entity _this)
{
    c_ulong events;

    assert(_this);

    events = (c_ulong)kernelStatusGet(_this->uEntity);
    if ( events ) {
        assert(_this->status);
        _StatusNotifyEvent(_this->status, events);
    }
}

void
gapi_entityNotifyEvent (
    gapi_entity _this,
    c_ulong events)
{
    _Entity entity;

    entity = gapi_entityClaim(_this, NULL);
    if ( entity ) {
        _StatusNotifyEvent(entity->status, events);
        gapi_entityRelease(_this);
    }
}

_DomainParticipant
_EntityParticipant (
    _Entity _this)
{
    u_participant uParticipant;
    _DomainParticipant participant;
    gapi_domainParticipant handle;

    uParticipant = u_entityParticipant(U_ENTITY_GET(_this));
    handle = u_entityGetUserData(u_entity(uParticipant));
    participant = _DomainParticipant(gapi_objectPeekUnchecked(handle));
    return participant;
}

