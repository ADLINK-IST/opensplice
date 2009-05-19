#include "in_channelSdp.h"
#include "in__object.h"
#include "os_heap.h"
#include "in_channel.h"
#include "in_channelSdpWriter.h"
#include "in_channelSdpReader.h"
#include "in__configChannel.h"
#include "u_participant.h"
#include "in_streamWriter.h"
#include "u_participant.h"

OS_STRUCT(in_channelSdp)
{
    OS_EXTENDS(in_channel);
};

static void
in_channelSdpDeinit(
    in_object _this);

static os_boolean
in_channelSdpInit(
    in_channelSdp _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_configDiscoveryChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryDat);


in_channelSdp
in_channelSdpNew(
    in_configDiscoveryChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryData)
{
    in_channelSdp _this = NULL;

    assert(stream);

    _this = os_malloc(sizeof(OS_STRUCT(in_channelSdp)));
    if(_this != NULL)
    {
        os_boolean success;

        success = in_channelSdpInit(
            _this,
            IN_OBJECT_KIND_SDP_CHANNEL,
            in_channelSdpDeinit,
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
in_channelSdpInit(
    in_channelSdp _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_configDiscoveryChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryData)
{
    os_boolean success;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);
    assert(stream);

    success = in_channelInit(
        in_channel(_this),
        kind,
        deinit,
        stream,
        plug,
        in_configChannel(config));

    if(success)
    {
        in_channelSdpWriter writer;
        in_streamWriter streamWriter;

        streamWriter = in_streamGetWriter(stream);
        writer = in_channelSdpWriterNew(_this, plug, streamWriter,
            discoveryData);

		in_streamWriterFree(streamWriter);

        if(!writer)
        {
            success = OS_FALSE;
        } else
        {
            in_channelSetWriter(in_channel(_this), in_channelWriter(writer));
            in_channelSdpWriterFree(writer);
        }
    }
    if(success)
    {
        in_channelSdpReader reader;

        reader = in_channelSdpReaderNew(_this, config);
        if(!reader)
        {
            success = OS_FALSE;
        } else
        {
            in_channelSetReader(in_channel(_this), in_channelReader(reader));
            in_channelSdpReaderFree(reader);
        }
    }
    return success;
}

void
in_channelSdpDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_channelSdpIsValid(_this));

    in_channelDeinit(_this);
}
