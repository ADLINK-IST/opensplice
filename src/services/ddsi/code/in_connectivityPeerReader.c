#include "in_connectivityPeerReader.h"
#include "os_defs.h"
#include "os_heap.h"

OS_STRUCT(in_connectivityPeerReader)
{
    OS_EXTENDS(in_connectivityPeerEntity);
    struct v_subscriptionInfo info;
    os_boolean requiresInlineQos;
    Coll_List uniLocators;
    Coll_List multiLocators;
};
/* ------------- private  ------------------- */

static void
in_connectivityPeerReaderInit(
    in_connectivityPeerReader _this,
    struct v_subscriptionInfo *info,
    os_boolean requiresInlineQos);

static void
in_connectivityPeerReaderDeinit(
    in_object _this);

/* ------------- public ----------------------- */
in_connectivityPeerReader
in_connectivityPeerReaderNew(
    struct v_subscriptionInfo *info,
    os_boolean requiresInlineQos)
{
    in_connectivityPeerReader _this;

    _this = in_connectivityPeerReader(os_malloc(sizeof(OS_STRUCT(in_connectivityPeerReader))));

    if(_this)
    {
        in_connectivityPeerReaderInit(_this, info, requiresInlineQos);
    }

    return _this;
}

static void
in_connectivityPeerReaderInit(
    in_connectivityPeerReader _this,
    struct v_subscriptionInfo *info,
    os_boolean requiresInlineQos)
{
    assert(_this);

    in_connectivityPeerEntityInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PEER_READER,
        in_connectivityPeerReaderDeinit);

    _this->requiresInlineQos = requiresInlineQos;
    _this->info = *info;
}

static void
in_connectivityPeerReaderDeinit(
    in_object _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    in_connectivityPeerEntityDeinit(in_connectivityPeerEntity(_this));
}

in_result
in_connectivityPeerReaderAddUnicastLocator(
    in_connectivityPeerReader _this,
    in_locator locator)
{
    os_uint32 collResult;
    in_result result;

    assert(in_connectivityPeerReaderIsValid(_this));
    assert(locator);

    collResult = Coll_List_pushBack(&(_this->uniLocators), in_locatorKeep(locator));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    return result;
}

in_result
in_connectivityPeerReaderAddMulticastLocator(
    in_connectivityPeerReader _this,
    in_locator locator)
{
    os_uint32 collResult;
    in_result result;

    assert(in_connectivityPeerReaderIsValid(_this));
    assert(locator);

    collResult = Coll_List_pushBack(&(_this->multiLocators), in_locatorKeep(locator));
    IN_COLLRESULT_TO_RESULT(collResult, result);

    return result;
}

Coll_List*
in_connectivityPeerReaderGetUnicastLocators(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return &(_this->uniLocators);
}

Coll_List*
in_connectivityPeerReaderGetMulticastLocators(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return &(_this->multiLocators);
}

struct v_subscriptionInfo *
in_connectivityPeerReaderGetInfo(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return &_this->info;
}

