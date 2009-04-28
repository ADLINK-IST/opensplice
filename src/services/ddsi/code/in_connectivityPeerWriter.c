#include "os_defs.h"
#include "os_heap.h"

#include "in__ddsiPublication.h"

#include "in_connectivityPeerWriter.h"

OS_STRUCT(in_connectivityPeerWriter)
{
    OS_EXTENDS(in_connectivityPeerEntity);
    in_ddsiDiscoveredWriterData info;
};

/* ------------- private  ------------------- */

static void
in_connectivityPeerWriterInit(
    in_connectivityPeerWriter _this,
    in_ddsiDiscoveredWriterData info);

static void
in_connectivityPeerWriterDeinit(
    in_object _this);

/* ------------- public ----------------------- */
in_connectivityPeerWriter
in_connectivityPeerWriterNew(
    in_ddsiDiscoveredWriterData info)
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
    in_ddsiDiscoveredWriterData info)
{
    assert(_this);

    in_connectivityPeerEntityInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_PEER_WRITER,
        in_connectivityPeerWriterDeinit);

    _this->info = in_ddsiDiscoveredWriterDataKeep(info);
}

static void
in_connectivityPeerWriterDeinit(
    in_object obj)
{
    in_connectivityPeerWriter _this;

    assert(in_connectivityPeerWriterIsValid(obj));
    _this = in_connectivityPeerWriter(obj);

    in_ddsiDiscoveredWriterDataFree(_this->info);
    in_connectivityPeerEntityDeinit(in_connectivityPeerEntity(_this));
}


Coll_List*
in_connectivityPeerWriterGetUnicastLocators(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return &(_this->info->proxy.unicastLocatorList);
}

Coll_List*
in_connectivityPeerWriterGetMulticastLocators(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return &(_this->info->proxy.multicastLocatorList);
}

in_ddsiDiscoveredWriterData
in_connectivityPeerWriterGetInfo(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return _this->info;
}

c_char*
in_connectivityPeerWriterGetTopicName(
    in_connectivityPeerWriter _this)
{
    c_char *result = NULL;
    in_ddsiDiscoveredWriterData info;

    assert(in_connectivityPeerWriterIsValid(_this));

    info = _this->info;

    /* paranoid check, avoid any NULL-pointer-exception */
    if (info &&
            info->topicData.info.topic_name) {
        result =
            info->topicData.info.topic_name;
    }

    return result;
}

in_ddsiGuid
in_connectivityPeerWriterGetGuid(
    in_connectivityPeerWriter _this)
{
    assert(in_connectivityPeerWriterIsValid(_this));

    return &(_this->info->proxy.remoteWriterGuid);
}

v_gid
in_connectivityPeerWriterGetGid(
    in_connectivityPeerWriter _this)
{
    v_gid gid;
    in_octet* entityKey;

    assert(in_connectivityPeerWriterIsValid(_this));

    entityKey = _this->info->proxy.remoteWriterGuid.entityId.entityKey;

    gid.systemId  = *((os_uint32*) &(_this->info->proxy.remoteWriterGuid.guidPrefix));
    gid.localId =  (entityKey[0] << 16) & (entityKey[1] << 8) & (entityKey[2]);
    gid.serial = 0;

    return gid;
}

