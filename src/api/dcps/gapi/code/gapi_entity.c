/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include "gapi_domainEntity.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_status.h"
#include "gapi_kernel.h"
#include "gapi_builtin.h"

#include "u_types.h"

void
_EntityInit (
    _Entity entity,
    _DomainEntity domainEntity,
    _Entity factory,
    gapi_boolean enabled)
{
    
    entity->enabled  = enabled;
    entity->factory  = factory;
    entity->registry = _ObjectRegistryNew();

    entity->StatusCondition = NULL;
}

void
_EntityDispose (
    _Entity entity)
{
   _ObjectRegistryFree(entity->registry);
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
            if (!entity->enabled) {
                entity->enabled = TRUE;
                u_entityEnable(entity->uEntity);
            }
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
    }
    
    _EntityRelease(entity);   

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
    }
    _EntityRelease(entity);

    return result;
}

gapi_boolean
_EntitySetListenerInterest (
    _Entity               entity,
    _ListenerInterestInfo info)
{
    gapi_boolean result;

    _EntityClaim(entity);
    result = _StatusSetListenerInterest(entity->status, info);
    _EntityRelease(entity);

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
_EntityFreeStatusCondition (
    _Entity entity)
{
    if ( entity->StatusCondition ) {
        _EntityClaim(entity->StatusCondition);
        _StatusConditionFree (entity->StatusCondition);
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
    }
    _EntityRelease(entity);
    
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
    gapi_statusMask triggerMask;
    _Status status;
    c_ulong events;

    assert(_this);

    events = (c_ulong)kernelStatusGet(_this->uEntity);
    if ( events ) {       
        status = _this->status;
        assert(status);
        if ( status->enabled ) {
            triggerMask = _StatusGetMaskStatus(status, events);
            status->notify(_this, triggerMask);
        }
    }
}
    
void
gapi_entityNotifyEvent (
    gapi_entity _this,
    c_ulong events)
{
    gapi_statusMask triggerMask;
    _Entity entity;
    _Status status;
    
    entity = gapi_entityClaim(_this, NULL);
    if ( entity ) {
        status = entity->status;
        if ( status->enabled ) {
            triggerMask = _StatusGetMaskStatus(status, events);
            status->notify(entity, triggerMask);
        }
    }
    gapi_entityRelease(_this);
}

static void
get_name(
    v_entity e,
    c_voidp arg)
{
    gapi_string *name = (gapi_string *)arg;
    *name = gapi_string_dup(e->name);
}

/*
 *     String
 *     gapi_entity_get_name();
 */
gapi_string
gapi_entity_get_name (
    gapi_entity _this)
{
    _Entity entity;
    u_result uresult;
    gapi_returnCode_t result;
    gapi_string name;

    if (_this && name) {
        entity = gapi_entityClaim(_this, &result);
        if ( entity != NULL) {
            uresult = u_entityAction(entity->uEntity,get_name,&name);
            if (uresult == U_RESULT_OK) {
                result = GAPI_RETCODE_OK;
            } else {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
            _EntityRelease(entity);
        } else {
            result = GAPI_RETCODE_ALREADY_DELETED;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return name;
}

static void
set_name(
    v_entity e,
    c_voidp arg)
{
    c_free(e->name);
    e->name = c_stringNew(c_getBase(e),(c_char *)arg);
}

/*
 *     ReturnCode_t
 *     gapi_entity_set_name();
 */
gapi_returnCode_t
gapi_entity_set_name (
    gapi_entity _this,
    gapi_string name)
{
    _Entity entity;
    u_result uresult;
    gapi_returnCode_t result;

    if (_this) {
        entity = gapi_entityClaim(_this, &result);
        if ( entity != NULL ) {
            if ( !entity->enabled) {
                uresult = u_entityAction(entity->uEntity,set_name,name);
                if (uresult == U_RESULT_OK) {
                    result = GAPI_RETCODE_OK;
                } else {
                    result = GAPI_RETCODE_ALREADY_DELETED;
                }
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
            _EntityRelease(entity);
        } else {
            result = GAPI_RETCODE_ALREADY_DELETED;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}

