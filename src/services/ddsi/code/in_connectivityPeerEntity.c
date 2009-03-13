#include "in_connectivityPeerEntity.h"
#include "os_defs.h"
#include "os_heap.h"


os_boolean
in_connectivityPeerEntityInit(
    in_connectivityPeerEntity _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit)
{
    os_boolean success;

    assert(_this);

    success = in_objectInit(
        in_object(_this),
        kind,
        deinit);

    if(success){
        Coll_List_init(&_this->uniLocators);
        Coll_List_init(&_this->multiLocators);
    }
    return success;
}

void
in_connectivityPeerEntityDeinit(
    in_connectivityPeerEntity _this)
{
    os_uint32 i;

    assert(_this);

    for(i = Coll_List_getNrOfElements(&_this->uniLocators); i > 0; i--)
    {
        in_locatorFree(in_locator(Coll_List_popBack(&_this->uniLocators)));
    }
    for(i = Coll_List_getNrOfElements(&_this->multiLocators); i > 0; i--)
    {
        in_locatorFree(in_locator(Coll_List_popBack(&_this->multiLocators)));
    }

    in_objectDeinit(_this);
}

in_result
in_connectivityPeerEntitSetGuid(
    in_connectivityPeerEntity _this,
    in_ddsiGuid guid)
{
    assert(_this);

    _this->guid = *guid;

    return IN_RESULT_OK;
}

in_result
in_connectivityPeerEntityAddUnicastLocator(
    in_connectivityPeerEntity _this,
    in_locator locator)
{
    os_uint32 collResult;
    in_result result;

    assert(_this);
    assert(locator);

    collResult = Coll_List_pushBack(&(_this->uniLocators), in_locatorKeep(locator));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    return result;
}

in_result
in_connectivityPeerEntityAddMulticastLocator(
    in_connectivityPeerEntity _this,
    in_locator locator)
{
    os_uint32 collResult;
    in_result result;

    assert(_this);
    assert(locator);

    collResult = Coll_List_pushBack(&(_this->multiLocators), in_locatorKeep(locator));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    return result;
}

in_ddsiGuid
in_connectivityPeerEntityGetGuid(
    in_connectivityPeerEntity _this)
{
    assert(_this);

    return &(_this->guid);
}


Coll_List*
in_connectivityPeerEntityGetUnicastLocators(
    in_connectivityPeerEntity _this)
{
    assert(_this);

    return &(_this->uniLocators);
}

Coll_List*
in_connectivityPeerEntityGetMulticastLocators(
    in_connectivityPeerEntity _this)
{
    assert(_this);

    return &(_this->multiLocators);
}
