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
#ifndef IN_CHANNEL_SDP_H
#define IN_CHANNEL_SDP_H

/* OS abstraction includes. */
#include "in__object.h"
#include "in_stream.h"
#include "u_participant.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_channelSdp(_this) ((in_channelSdp)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_channelSdpIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_channelSdpKeep(_this) in_channelSdp(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_channelSdpFree(_this) in_objectFree(in_object(_this))

in_channelSdp
in_channelSdpNew(
    in_configDiscoveryChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryDat);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_SDP_H */

