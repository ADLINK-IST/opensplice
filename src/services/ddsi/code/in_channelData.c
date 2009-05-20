#include "in_channel.h"
#include "in_channelData.h"
#include "os_heap.h"
#include "in_channelDataWriter.h"
#include "in_channelDataReader.h"

static void
in_channelDataDeinit(
    in_object _this);

static os_boolean
in_channelDataInit(
    in_channelData _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_configChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryData);

OS_STRUCT(in_channelData)
{
    OS_EXTENDS(in_channel);
};

in_channelData
in_channelDataNew(
    in_configChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryData)
{
    in_channelData _this = NULL;

    assert(config);
    assert(stream);
    assert(plug);
    assert(in_plugKernelIsValid(plug));
    assert(discoveryData);

    _this = os_malloc(sizeof(OS_STRUCT(in_channelData)));
    if(_this != NULL)
    {
        os_boolean success;

        success = in_channelDataInit(
            _this,
            IN_OBJECT_KIND_DATA_CHANNEL,
            in_channelDataDeinit,
            config,
            stream,
            plug,
            discoveryData);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}

os_boolean
in_channelDataInit(
    in_channelData _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_configChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryData)
{
    os_boolean success;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);
    assert(config);
    assert(stream);
    assert(plug);
    assert(in_plugKernelIsValid(plug));
    assert(discoveryData);

    success = in_channelInit(
        in_channel(_this),
        kind,
        deinit,
        stream,
        plug,
        config);

    if(success)
    {
        in_channelDataWriter writer;

        writer = in_channelDataWriterNew(_this, config, in_plugKernelGetNetworkReader(plug), discoveryData);

        if(!writer)
        {
            success = OS_FALSE;
        } else
        {
            in_channelSetWriter(in_channel(_this), in_channelWriter(writer));
            in_channelDataWriterFree(writer);
        }
    }
    if(success)
    {
        in_channelDataReader reader;

        reader = in_channelDataReaderNew(_this, config, in_plugKernelGetNetworkReader(plug));

        if(!reader)
        {
            success = OS_FALSE;
        } else
        {
            in_channelSetReader(in_channel(_this), in_channelReader(reader));
            in_channelDataReaderFree(reader);
        }
    }
    return success;
}

void
in_channelDataDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_channelDataIsValid(_this));

    in_channelDeinit(_this);
}
