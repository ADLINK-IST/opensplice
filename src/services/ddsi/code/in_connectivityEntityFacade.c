#include "in_connectivityEntityFacade.h"

os_boolean
in_connectivityEntityFacadeInit(
    in_connectivityEntityFacade _this,
    in_objectKind kind,
    in_objectDeinitFunc destroy,
    in_ddsiGuid id)
{
    os_boolean success;
    
    assert(_this);

    success =  in_objectInit(in_object(_this), kind, destroy);

    if ( success ) {
        _this->timestamp = os_timeGet();
        _this->id = *id;        
    }

    return success;
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

    return &(_this->id);
}

os_time 
in_connectivityEntityFacadeGetTimestamp(
    in_connectivityEntityFacade _this)
{
    assert(in_objectIsValid(in_object(_this)));

    return _this->timestamp;
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
