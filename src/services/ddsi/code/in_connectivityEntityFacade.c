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
#include "in_connectivityEntityFacade.h"

os_boolean
in_connectivityEntityFacadeInit(
    in_connectivityEntityFacade _this,
    in_objectKind kind,
    in_objectDeinitFunc destroy)
{
    assert(_this);

    return in_objectInit(in_object(_this), kind, destroy);
}

void
in_connectivityEntityFacadeDeinit(
    in_object _this)
{

    assert(_this);

    in_objectDeinit(_this);
}

in_ddsiGuid
in_connectivityEntityFacadeGetGuid(
    in_connectivityEntityFacade _this)
{
    assert(in_objectIsValid(in_object(_this)));

    return _this->id;
}

void
in_connectivityEntityFacadeAddMatchedEntity(
    in_connectivityEntityFacade _this)
{
    assert(in_objectIsValid(in_object(_this)));
    /* no-op */
}

void
in_connectivityEntityFacadeRemoveMatchedEntity(
    in_connectivityEntityFacade _this)
{
    assert(in_objectIsValid(in_object(_this)));
    /* no-op */
}
