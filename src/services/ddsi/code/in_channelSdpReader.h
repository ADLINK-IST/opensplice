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
/**
 * The in_channelSdpReader provides an implementation of read side of the
 * Simple Discovery Protocol. This component is responsible to keep the
 * information about remote RTPS Participants, Readers and Writers in the
 * connectivity administration up to date. Therefore it actively monitors
 * incoming discovery information from the in_readStream. This includes
 * PeerParticipant, PeerReader and PeerWriter information.
 *
 * Furthermore, the component is also responsible to monitor incoming
 * (negative) acknowledgments for the information that was sent by the
 * in_channelSdpWriter counterpart.
 */
#ifndef IN_CHANNEL_SDP_READER_H
#define IN_CHANNEL_SDP_READER_H

#include "in__object.h"
#include "in__configDiscoveryChannel.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

#define in_channelSdpReader(s) ((in_channelSdpReader)(s))

#define in_channelSdpReaderFree(s) in_objectFree(in_object(s))

#define in_channelSdpReaderKeep(s) in_objectKeep(in_object(s))

#define in_channelSdpReaderIsValid(s)\
    in_objectIsValidWithKind(in_object(s), IN_OBJECT_KIND_SDP_READER)

in_channelSdpReader
in_channelSdpReaderNew(
    in_channelSdp channel,
    in_configDiscoveryChannel config);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_SDP_READER_H */

