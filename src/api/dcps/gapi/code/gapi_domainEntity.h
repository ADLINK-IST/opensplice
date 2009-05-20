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
#ifndef GAPI_DOMAINENTITY_H
#define GAPI_DOMAINENTITY_H

#include "gapi_common.h"
#include "gapi_entity.h"
#include "gapi_domainParticipant.h"
#include "gapi_object.h"

#include "u_user.h"

#define _DomainEntity(o) ((_DomainEntity)(o))

#define gapi_domainEntityClaim(h,r) \
        (_DomainEntity(gapi_objectClaim(h,OBJECT_KIND_DOMAINENTITY,r)))

#define gapi_domainEntityClaimNB(h,r) \
        (_DomainEntity(gapi_objectClaimNB(h,OBJECT_KIND_DOMAINENTITY,r)))

C_STRUCT(_DomainEntity) {
    C_EXTENDS(_Entity);
    _DomainParticipant participant;
};

void
_DomainEntityInit (
    _DomainEntity _this,
    _DomainParticipant participant,
    _Entity factory,
    gapi_boolean enabled);

void
_DomainEntityDispose (
    _DomainEntity _this);

_DomainParticipant
_DomainEntityParticipant (
    _DomainEntity _this);

void
_DomainEntitySetUserData (
    _DomainEntity _this,
    c_voidp userData);

c_voidp
_DomainEntityGetUserData (
    _DomainEntity _this);

#endif
