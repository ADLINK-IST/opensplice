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
#ifndef NW_PLUGSENDCHANNEL_H
#define NW_PLUGSENDCHANNEL_H

#include "nw_plugTypes.h"
#include "nw_plugChannel.h"
#include "v_fullCounter.h"

NW_CLASS(plugSendStatistics);

NW_STRUCT(plugSendStatistics){
    c_ulong numberOfPacketsSent;
    c_ulong numberOfBytesSent;
    c_ulong numberOfMessagesFragmented;
    c_ulong numberOfMessagesPacked;
    c_ulong numberOfKnownNodes;
	c_ulong numberOfBytesResent;
	c_ulong numberOfPacketsResent;
	c_ulong numberOfBytesInResendBuffer;
	c_ulong numberOfPacketsInResendBuffer;
	c_ulong numberOfAcksSent;
	v_fullCounter adminQueueAcks;
	v_fullCounter adminQueueData;
	int enabled;
    c_ulong nofBytesBeforeCompression;
    c_ulong nofBytesAfterCompression;
/* receive
	c_ulong numberOfBytesReceived;
	c_ulong numberOfPacketsReceived;
	c_ulong numberOfPacketsLost;
	c_ulong numberOfAcksSent;

	c_ulong numberOfMessagesDelivered;
	c_ulong numberOfBytesDelivered;
	c_ulong numberOfMessagesNotInterested;
	c_ulong numberOfBytesNotInterested;
    c_ulong nofBytesBeforeDecompression;
    c_ulong nofBytesAfterDecompression;
	*/
};



NW_CLASS(nw_plugSendChannel);
                         
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

/* Starting the writing of a message, returns a buffer of length to write into */
nw_bool        nw_plugSendChannelMessageStart(
                   nw_plugChannel channel,
                   nw_data *buffer,
                   nw_length *length,
                   nw_partitionId partitionId,
                   nw_signedLength *bytesLeft,
                   plugSendStatistics pss);

/* Starting the writing of a message (to a specific node), returns a buffer of length to write into */
nw_bool        nw_plugSendChannelWriteToMessageStart(
                    nw_plugChannel channel,
                    nw_data *buffer,
                    nw_length *length,
                    nw_partitionId partitionId,
                    nw_networkId destination,
                    nw_signedLength *bytesLeft, /* in/out */
                    plugSendStatistics pss);

                   
/* Retrieve a buffer for copying data into if the previous buffer was not
 * sufficiently long. NOTE: This function currently flushes the available
 * data to the network */
void           nw_plugSendChannelGetNextFragment(
                   nw_plugChannel channel,
                   nw_data *buffer,
                   nw_length *length);

/* Indicate the end of a message */
void           nw_plugSendChannelMessageEnd(
                   nw_plugChannel channel,
                   nw_data buffer,
                   plugSendStatistics pss);

/* Do a flush of data messages to the network.
 * bytesLeft is in/out indicating the number of bytes allowed to be flushed.
 * Flushes to the network go in chunks of complete fragments.
 * Function returns true if all packets were flushed, false otherwise */

nw_bool        nw_plugSendChannelMessagesFlush(
                   nw_plugChannel channel,
                   nw_bool all,
                   nw_signedLength *bytesLeft, /* in/out */
                   plugSendStatistics pss);
                   
/* Perform periodic actions:
  *  - send control messages 
  *  - resend data
  *  - process discovery messages
  *  - determine MaxBurstSize for the coming period
  */ 
void           nw_plugSendChannelPeriodicAction(
                   nw_plugChannel channel,
                   nw_signedLength *credits, /* in/out */
                   plugSendStatistics pss);
                   

#endif /* NW_PLUGSENDCHANNEL_H */
