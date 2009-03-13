#include "in_connectivityPeerWriter.h"
#include "os_defs.h"
#include "os_heap.h"

OS_STRUCT(in_connectivityPeerWriter)
{
    OS_EXTENDS(in_connectivityPeerEntity);
    struct v_publicationInfo info;
};

/* ------------- private  ------------------- */

static void
in_connectivityPeerWriterInit(
    in_connectivityPeerWriter _this,
    struct v_publicationInfo *info);

static void
in_connectivityPeerWriterDeinit(
    in_object _this);

/* ------------- public ----------------------- */
in_connectivityPeerWriter
in_connectivityPeerWriterNew(
    struct v_publicationInfo *info)
{
    in_connectivityPeerWriter _this;

    _this = in_connectivityPeerWriter(os_malloc(sizeof(OS_STRUCT(in_connectivityPeerWriter))));

    if(_this)
    {
        in_connectivityPeerWriterInit(_this,info);
    }

    return _this;
}

static void
in_connectivityPeerWriterInit(
    in_connectivityPeerWriter _this,
    struct v_publicationInfo *info)
{
    assert(_this);

    in_connectivityPeerEntityInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PEER_WRITER,
        in_connectivityPeerWriterDeinit);

    _this->info = *info;
}

static void
in_connectivityPeerWriterDeinit(
    in_object obj)
{
    in_connectivityPeerWriter _this;

    assert(in_connectivityPeerWriterIsValid(obj));
    _this = in_connectivityPeerWriter(obj);
    in_connectivityPeerEntityDeinit(in_connectivityPeerEntity(_this));
}

in_result
in_connectivityPeerWriterAddUnicastLocator(
    in_connectivityPeerWriter _this,
    in_locator locator)
{
    assert(in_connectivityPeerWriterIsValid(_this));
    return  in_connectivityPeerEntityAddUnicastLocator( in_connectivityPeerEntity(_this), locator);
}

in_result
in_connectivityPeerWriterAddMulticastLocator(
    in_connectivityPeerWriter _this,
    in_locator locator)
{
    assert(in_connectivityPeerWriterIsValid(_this));
    return in_connectivityPeerEntityAddMulticastLocator( in_connectivityPeerEntity(_this), locator);
}

Coll_List*
in_connectivityPeerWriterGetUnicastLocators(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return in_connectivityPeerEntityGetUnicastLocators( in_connectivityPeerEntity(_this));
}

Coll_List*
in_connectivityPeerWriterGetMulticastLocators(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return in_connectivityPeerEntityGetMulticastLocators( in_connectivityPeerEntity(_this));
}

struct v_publicationInfo *
in_connectivityPeerWriterGetInfo(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return &_this->info;
}

