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

#ifndef NW_PLUGRECEIVECHANNEL_H
#define NW_PLUGRECEIVECHANNEL_H

#include "nw_plugTypes.h"
#include "nw_plugChannel.h"

NW_CLASS(plugReceiveStatistics);

NW_STRUCT(plugReceiveStatistics){
/*
    c_ulong numberOfPacketsSent;
    c_ulong numberOfBytesSent;
    c_ulong numberOfMessagesFragmented;
    c_ulong numberOfMessagesPacked;
    c_ulong numberOfKnownNodes;
	c_ulong numberOfBytesResent;
	c_ulong numberOfPacketsResent;
	c_ulong numberOfBytesInResendBuffer;
	c_ulong numberOfPacketsInResendBuffer;
    c_ulong nofBytesBeforeCompression;
    c_ulong nofBytesAfterCompression;
 receive*/
	c_ulong numberOfMessagesReceived;
	c_ulong numberOfBytesReceived;
	c_ulong numberOfPacketsReceived;
	c_ulong numberOfPacketsLost;

	c_ulong numberOfMessagesDelivered;
	c_ulong numberOfBytesDelivered;
	c_ulong numberOfMessagesNotInterested;
	c_ulong numberOfBytesNotInterested;
	c_ulong nofUsedPacketBuffers;
	c_ulong nofFreePacketBuffers;
	int enabled;
    c_ulong nofBytesBeforeDecompression;
    c_ulong nofBytesAfterDecompression;
};

NW_CLASS(nw_plugReceiveChannel);

/* Note the symmetry between the read and write functions:
 *
 *
 *  |                          >------> ReadWait
 * \|/  WriteMessageStart      |      |   ReadMessageStart
 *  |     WriteGetNextFragment ^      |     ReadGetNextFragment
 *  |   WriteMessageEnd        |     \|/  ReadMessageEnd
 *  | WriteFlush               |      |
 *  >---->---->---->---->---->-^      |
 */
/* Read functions and corresponding buffer management functions */

/* Read data from socket into internal buffers and handle control messages
 */
void           nw_plugReceiveChannelProcessIncoming(
                   nw_plugChannel channel);


/* Blocking call returning a pointer to the next message. Function returns
 * {NULL, 0} if nw_plugReceiveChannelWakeup has been called */
c_bool         nw_plugReceiveChannelMessageStart(
                   nw_plugChannel channel,
                   nw_data *data,
                   nw_length *length,
				   nw_senderInfo sender,
                   plugReceiveStatistics prs);

/* Non-blocking call returning the remaining part of a message if fragmentation
 * has occurred over the previous buffer */
void           nw_plugReceiveChannelGetNextFragment(
                   nw_plugChannel channel,
                   nw_data *data,
                   nw_length *length);

/* Non-blocking call that throws away all buffers related to the current message */
void           nw_plugReceiveChannelMessageIgnore(
                   nw_plugChannel channel);

/* Non-blocking call freeing the resources used for the message retrieved
 * with the MessageStart function */
void           nw_plugReceiveChannelMessageEnd(
                   nw_plugChannel channel,
                   plugReceiveStatistics prs);

/* Wake up from the wait; this can be called for any reason. As a result,
 * the nw_plugReceiveChannelMessageStart will return with the value FALSE */
void           nw_plugReceiveChannelWakeUp(
                   nw_plugChannel channel);

#endif /*NW_PLUGRECEIVECHANNEL_H */
