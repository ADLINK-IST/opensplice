/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_FACTORY_H
#define IN_FACTORY_H

#include "in__object.h"
#include "in_endpointDiscoveryData.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

in_transport
in_factoryCreateTransport(
    in_configChannel channelConfig);

in_stream
in_factoryCreateStream(
    in_configChannel config,
    in_plugKernel plug,
	in_transport transport);

in_channel
in_factoryCreateChannel(
    in_configChannel channelConfig,
    in_plugKernel plug,
    in_stream stream,
    in_endpointDiscoveryData discoveryData);

in_channel
in_factoryCreateDiscoveryChannel(
    in_configDiscoveryChannel channelConfig,
    in_plugKernel plug,
    in_stream stream,
    in_endpointDiscoveryData discoveryData);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_FACTORY_H */

