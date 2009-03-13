#ifndef GAPI_ENTITY_H
#define GAPI_ENTITY_H

#include "gapi_common.h"
#include "gapi_object.h"
#include "gapi_entityValidity.h"
#include "gapi_condition.h"
#include "gapi_status.h"

#include "u_user.h"

#define _Entity(o) ((_Entity)(o))

#define gapi_entityClaim(h,r) \
        (_Entity(gapi_objectClaim(h,OBJECT_KIND_ENTITY,r)))

#define gapi_entityClaimNB(h,r) \
        (_Entity(gapi_objectClaimNB(h,OBJECT_KIND_ENTITY,r)))

#define U_ENTITY_GET(e)            (_Entity(e)->uEntity)
#define U_ENTITY_SET(e,u)           _Entity(e)->uEntity = u_entity(u)
#define ENTITY_STATUS_GET(e)       (_Entity(e)->status)
        
#define _EntityUEntity(e)           _Entity(e)->uEntity
#define _EntityStatus(e)            _Entity(e)->status
#define _EntityEnabled(e)           _Entity(e)->enabled

#define _EntityDelete(e)            _ObjectDelete((_Object)e)
#define _EntityClaim(e)             _ObjectClaim((_Object)e)
#define _EntityClaimNotBusy(e)      _ObjectClaimNotBusy((_Object)e)
#define _EntityRelease(e)           _ObjectRelease((_Object)e)
#define gapi_entityRelease(h) gapi_objectRelease(h)
#define _EntityHandle(e)            _ObjectToHandle((_Object)e)
#define _EntitySetBusy(e)           _ObjectSetBusy((_Object)e)

#define _EntityGetFactory(e) (_Entity(e)->factory)
#define _EntityGetStatus(e) (_Entity(e)->status)
#define _EntityGetHandle(e) (_Entity(e)->handle)

C_STRUCT(_Entity) {
    C_EXTENDS(_Object);
    gapi_boolean          enabled;
    _Entity               factory;
    _ObjectRegistry       registry;    
    _StatusCondition      StatusCondition;
    gapi_statusMask   StatusMask;
    _Status               status;
    u_entity              uEntity;
    v_kernel              kernelId;
    gapi_instanceHandle_t handle;
};

void
_EntityInit (
    _Entity _this,
    _DomainEntity domainEntity,
    _Entity       factory,
    gapi_boolean  enabled);

void
_EntityDispose (
    _Entity _this);

gapi_boolean
_EntitySetListenerInterest (
    _Entity _this,
    _ListenerInterestInfo info);

void
_EntitySetUserEntity (
    _Entity _this,
    u_entity uEntity);

void
_EntityFreeStatusCondition (
    _Entity entity);

void
_EntityRegisterObject (
    _Entity _this,
    _Object object);

gapi_boolean
_EntityHandleEqual (
    _Entity _this,
    gapi_instanceHandle_t handle);

void
_EntityNotifyInitialEvents (
    _Entity _this);

void
gapi_entityNotifyEvent (
    gapi_entity _this,
    c_ulong events);
   
#endif /* GAPI_ENTITY_H */
