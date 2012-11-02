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
#ifndef IN_CHANNEL_SDP_WRITER_H
#define IN_CHANNEL_SDP_WRITER_H

/* OS abstraction includes. */
#include "in__object.h"
#include "u_participant.h"
#include "u_networkReader.h"

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
#define in_channelSdpWriter(_this) ((in_channelSdpWriter)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_channelSdpWriterIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_channelSdpWriterKeep(_this) in_channelSdpWriter(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_channelSdpWriterFree(_this) in_objectFree(in_object(_this))

in_channelSdpWriter
in_channelSdpWriterNew(
    in_channelSdp sdp,
    in_plugKernel plug,
    in_streamWriter writer,
    in_endpointDiscoveryData discoveryData);

in_result
in_channelSdpWriterAddPeerEntity(
    in_channelSdpWriter _this,
    in_connectivityPeerEntity entity);

in_result
in_channelSdpWriterAddHeartbeatEvent(
    in_channelSdpWriter _this,
    in_ddsiHeartbeat event,
    in_ddsiReceiver receiver);

in_result
in_channelSdpWriterAddAckNack(
    in_channelSdpWriter _this,
    in_ddsiAckNack event,
    in_ddsiReceiver receiver);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_SDP_WRITER_H */

