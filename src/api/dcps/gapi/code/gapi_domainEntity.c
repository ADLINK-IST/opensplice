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

#include "gapi_domainEntity.h"

void
_DomainEntityInit (
    _DomainEntity _this,
    _DomainParticipant participant,
    _Entity factory,
    gapi_boolean enabled)
{
    _EntityInit (_Entity(_this), _this, factory, enabled);
    _this->participant = participant;
}

void
_DomainEntityDispose (
    _DomainEntity _this)
{
    _EntityDispose (_Entity(_this));
}

_DomainParticipant
_DomainEntityParticipant (
    _DomainEntity _this)
{
    return _this->participant;
}

