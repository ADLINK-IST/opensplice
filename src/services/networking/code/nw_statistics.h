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
#ifndef NW_STATISTICS_H_
#define NW_STATISTICS_H_

#include "v_networkingStatistics.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define NW_STRUCT(name)  struct name##_s
#define NW_EXTENDS(type) NW_STRUCT(type) _parent
#define NW_CLASS(name)   typedef NW_STRUCT(name) *name

NW_CLASS(nw_SendChannelStatistics);

NW_STRUCT(nw_SendChannelStatistics){

    c_ulong            numberOfMessagesSent;
	c_ulong            numberOfBytesSent;
	c_ulong            numberOfPacketsSent;

	c_ulong            numberOfMessagesFragmented;
	c_ulong            numberOfMessagesPacked;

	c_ulong            numberOfKnownNodes;
	c_ulong            numberOfBytesResent;
	c_ulong            numberOfPacketsResent;
	c_ulong            numberOfBytesInResendBuffer;
	c_ulong            numberOfPacketsInResendBuffer;
	v_maxValue         maxNumberOfBytesResentToOneNode;
	v_maxValue         maxNumberOfPacketsResentToOneNode;

	c_ulong            numberOfMessagesReceived;
	c_ulong            numberOfBytesReceived;
	c_ulong            numberOfPacketsReceived;
	c_ulong            numberOfPacketsLost;
	c_ulong            numberOfAcksSent;

	c_ulong            numberOfMessagesDelivered;
	c_ulong            numberOfBytesDelivered;
	c_ulong            numberOfMessagesNotInterested;
	c_ulong            numberOfBytesNotInterested;
	v_fullCounter	   adminQueueData;
	v_fullCounter	   adminQueueAcks;

    c_ulong            nofBytesBeforeCompression;
    c_ulong            nofBytesAfterCompression;
};

#define nw_SendChannelStatistics(s) ((nw_SendChannelStatistics)(s))

nw_SendChannelStatistics        nw_SendChannelStatisticsNew      ();

void                       nw_SendChannelStatisticsFree     (nw_SendChannelStatistics s);

NW_CLASS(nw_ReceiveChannelStatistics);

NW_STRUCT(nw_ReceiveChannelStatistics){
    c_ulong            numberOfMessagesSent;
	c_ulong            numberOfBytesSent;
	c_ulong            numberOfPacketsSent;

	c_ulong            numberOfMessagesFragmented;
	c_ulong            numberOfMessagesPacked;

	c_ulong            numberOfKnownNodes;
	c_ulong            numberOfBytesResent;
	c_ulong            numberOfPacketsResent;
	c_ulong            numberOfBytesInResendBuffer;
	c_ulong            numberOfPacketsInResendBuffer;
	v_maxValue         maxNumberOfBytesResentToOneNode;
	v_maxValue         maxNumberOfPacketsResentToOneNode;

	c_ulong            numberOfMessagesReceived;
	c_ulong            numberOfBytesReceived;
	c_ulong            numberOfPacketsReceived;
	c_ulong            numberOfPacketsLost;
	c_ulong            numberOfAcksSent;

	c_ulong            numberOfMessagesDelivered;
	c_ulong            numberOfBytesDelivered;
	c_ulong            numberOfMessagesNotInterested;
	c_ulong            numberOfBytesNotInterested;
	c_ulong			   nofUsedPacketBuffers;
	c_ulong			   nofFreePacketBuffers;

    c_ulong            nofBytesBeforeDecompression;
    c_ulong            nofBytesAfterDecompression;
};

NW_CLASS(nw_SendChannelStatisticsArgs);

NW_STRUCT(nw_SendChannelStatisticsArgs){

	nw_SendChannelStatistics            scs;
	c_ulong            					channel_id;
};

NW_CLASS(nw_ReceiveChannelStatisticsArgs);

NW_STRUCT(nw_ReceiveChannelStatisticsArgs){

	nw_ReceiveChannelStatisticsArgs     rcs;
	c_ulong            					channel_id;
};

void nw_ReceiveChannelStatisticsReset(nw_ReceiveChannelStatistics s);
void nw_SendChannelStatisticsReset(nw_SendChannelStatistics s);


#define nw_ReceiveChannelStatistics(s) ((nw_ReceiveChannelStatistics)(s))

nw_ReceiveChannelStatistics        nw_ReceiveChannelStatisticsNew      ();

void                       nw_ReceiveChannelStatisticsFree     (nw_ReceiveChannelStatistics s);

void					   nw_SendChannelUpdate(v_networkChannelStatistics s,nw_SendChannelStatistics nws);
void					   nw_ReceiveChannelUpdate(v_networkChannelStatistics s, nw_ReceiveChannelStatistics nws);

#if defined (__cplusplus)
}
#endif

#endif /*NW_STATISTICS_H_*/
