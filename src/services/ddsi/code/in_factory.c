#include "in_factory.h"
#include "in_transport.h"
#include "in_stream.h"
#include "in_channel.h"
#include "in_streamPairIBasic.h"
#include "in_transportPairIBasic.h"
#include "in_channelSdp.h"
#include "in_channelData.h"

in_transport
in_factoryCreateTransport(
    in_configChannel channelConfig)
{
    in_transport transport;

    assert(channelConfig);

    transport = in_transport(in_transportPairIBasicNew(channelConfig));

    return transport;
}

in_stream
in_factoryCreateStream(
    in_configChannel config,
    in_plugKernel plug,
    in_transport transport)
{
    in_stream stream;

    assert(config);
    assert(plug);
    assert(transport);

    if(transport)
    {
        stream = in_stream(in_streamPairIBasicNew(config, transport,plug));
    } else
    {
        stream = NULL;
    }
	return stream;
}

in_channel
in_factoryCreateChannel(
    in_configChannel config,
    in_plugKernel plug,
    in_stream stream,
    in_endpointDiscoveryData discoveryData)
{
    in_channel channel;

    assert(config);
    assert(plug);
    assert(stream);

    if(stream && config)
    {
        channel = in_channel(in_channelDataNew(
                config, stream, plug, discoveryData));
    } else
    {
        channel = NULL;
    }
    return channel;
}

in_channel
in_factoryCreateDiscoveryChannel(
    in_configDiscoveryChannel config,
    in_plugKernel plug,
    in_stream stream,
    in_endpointDiscoveryData discoveryData)
{
    in_channel channel;

    assert(config);
    assert(stream);

    if(stream && config)
    {
        channel = in_channel(in_channelSdpNew(config, stream, plug, discoveryData));
    } else
    {
        channel = NULL;
    }
    return channel;
}


