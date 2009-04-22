#include "os_defs.h"
#include "os_heap.h"

#include "in__ddsiSubscription.h"
#include "in_connectivityPeerReader.h"
#include "in_connectivityPeerParticipant.h"

OS_STRUCT(in_connectivityPeerReader)
{
    OS_EXTENDS(in_connectivityPeerEntity);
    in_ddsiDiscoveredReaderData info;
};
/* ------------- private  ------------------- */

static void
in_connectivityPeerReaderInit(
    in_connectivityPeerReader _this,
    in_ddsiDiscoveredReaderData info);

static void
in_connectivityPeerReaderDeinit(
    in_object _this);

/* ------------- public ----------------------- */
in_connectivityPeerReader
in_connectivityPeerReaderNew(
    in_ddsiDiscoveredReaderData info)
{
    in_connectivityPeerReader _this;

    _this = in_connectivityPeerReader(os_malloc(sizeof(OS_STRUCT(in_connectivityPeerReader))));

    if(_this)
    {
        in_connectivityPeerReaderInit(_this, info );
    }

    return _this;
}

static void
in_connectivityPeerReaderInit(
    in_connectivityPeerReader _this,
    in_ddsiDiscoveredReaderData info)
{
    assert(_this);

    in_connectivityPeerEntityInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PEER_READER,
        in_connectivityPeerReaderDeinit);

    _this->info = in_ddsiDiscoveredReaderDataKeep(info);
}

static void
in_connectivityPeerReaderDeinit(
    in_object _this)
{
    in_connectivityPeerReader peer;
    assert(in_connectivityPeerReaderIsValid(_this));
    peer = in_connectivityPeerReader(_this);

    in_ddsiDiscoveredReaderDataFree(peer->info);
    in_connectivityPeerEntityDeinit(in_connectivityPeerEntity(_this));
}

Coll_List*
in_connectivityPeerReaderGetUnicastLocators(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return &(_this->info->proxy.unicastLocatorList);
}

Coll_List*
in_connectivityPeerReaderGetMulticastLocators(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return &(_this->info->proxy.multicastLocatorList);
}

in_ddsiDiscoveredReaderData
in_connectivityPeerReaderGetInfo(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return _this->info;
}

in_ddsiGuid
in_connectivityPeerReaderGetGuid(
    in_connectivityPeerReader _this)
{
    assert(in_connectivityPeerReaderIsValid(_this));

    return &(_this->info->proxy.remoteReaderGuid);
}


