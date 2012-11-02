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
#include "nw_statistics.h"
#include "v_fullCounter.h"
#include "v_maxValue.h"
#include "os_heap.h"

nw_ReceiveChannelStatistics
nw_ReceiveChannelStatisticsNew()
{
	nw_ReceiveChannelStatistics s;

    s = nw_ReceiveChannelStatistics(os_malloc(C_SIZEOF(nw_ReceiveChannelStatistics)));

    if(s){
    	s->numberOfMessagesSent=0;
		s->numberOfBytesSent=0;
		s->numberOfPacketsSent=0;

		s->numberOfMessagesFragmented=0;
		s->numberOfMessagesPacked=0;

		s->numberOfKnownNodes=0;
		s->numberOfBytesResent=0;
		s->numberOfPacketsResent=0;
		s->numberOfBytesInResendBuffer=0;
		s->numberOfPacketsInResendBuffer=0;

		s->numberOfMessagesReceived=0;
		s->numberOfBytesReceived=0;
		s->numberOfPacketsReceived=0;
		s->numberOfPacketsLost=0;
		s->numberOfAcksSent=0;

		s->numberOfMessagesDelivered=0;
		s->numberOfBytesDelivered=0;
		s->numberOfMessagesNotInterested=0;
		s->numberOfBytesNotInterested=0;
		s->nofFreePacketBuffers=0;
		s->nofUsedPacketBuffers=0;

        s->nofBytesBeforeDecompression=0;
        s->nofBytesAfterDecompression=0;
    }
    return s;
}

nw_SendChannelStatistics
nw_SendChannelStatisticsNew()
{
	nw_SendChannelStatistics s;
    
    s = nw_SendChannelStatistics(os_malloc(C_SIZEOF(nw_SendChannelStatistics)));
    
    if(s){
    	s->numberOfMessagesSent=0;
		s->numberOfBytesSent=0;
		s->numberOfPacketsSent=0;

		s->numberOfMessagesFragmented=0;
		s->numberOfMessagesPacked=0;

		s->numberOfKnownNodes=0;
		s->numberOfBytesResent=0;
		s->numberOfPacketsResent=0;
		s->numberOfBytesInResendBuffer=0;
		s->numberOfPacketsInResendBuffer=0;

		s->numberOfMessagesReceived=0;
		s->numberOfBytesReceived=0;
		s->numberOfPacketsReceived=0;
		s->numberOfPacketsLost=0;
		s->numberOfAcksSent=0;

		s->numberOfMessagesDelivered=0;
		s->numberOfBytesDelivered=0;
		s->numberOfMessagesNotInterested=0;
		s->numberOfBytesNotInterested=0;
		v_fullCounterInit(&(s->adminQueueAcks));
		v_fullCounterInit(&(s->adminQueueData));

        s->nofBytesBeforeCompression=0;
        s->nofBytesAfterCompression=0;
    }
    return s;
}

void
nw_SendChannelStatisticsReset(nw_SendChannelStatistics s)
{
    if(s){
    	s->numberOfMessagesSent=0;
		s->numberOfBytesSent=0;
		s->numberOfPacketsSent=0;

		s->numberOfMessagesFragmented=0;
		s->numberOfMessagesPacked=0;

		s->numberOfKnownNodes=0;
		s->numberOfBytesResent=0;
		s->numberOfPacketsResent=0;
		s->numberOfBytesInResendBuffer=0;
		s->numberOfPacketsInResendBuffer=0;

		s->numberOfMessagesReceived=0;
		s->numberOfBytesReceived=0;
		s->numberOfPacketsReceived=0;
		s->numberOfPacketsLost=0;
		s->numberOfAcksSent=0;

		s->numberOfMessagesDelivered=0;
		s->numberOfBytesDelivered=0;
		s->numberOfMessagesNotInterested=0;
		s->numberOfBytesNotInterested=0;
		v_fullCounterInit(&(s->adminQueueAcks));
		v_fullCounterInit(&(s->adminQueueData));

        s->nofBytesBeforeCompression=0;
        s->nofBytesAfterCompression=0;
    }
}

void
nw_ReceiveChannelStatisticsReset(nw_ReceiveChannelStatistics s)
{
    if(s){
    	s->numberOfMessagesSent=0;
		s->numberOfBytesSent=0;
		s->numberOfPacketsSent=0;

		s->numberOfMessagesFragmented=0;
		s->numberOfMessagesPacked=0;

		s->numberOfKnownNodes=0;
		s->numberOfBytesResent=0;
		s->numberOfPacketsResent=0;
		s->numberOfBytesInResendBuffer=0;
		s->numberOfPacketsInResendBuffer=0;

		s->numberOfMessagesReceived=0;
		s->numberOfBytesReceived=0;
		s->numberOfPacketsReceived=0;
		s->numberOfPacketsLost=0;
		s->numberOfAcksSent=0;

		s->numberOfMessagesDelivered=0;
		s->numberOfBytesDelivered=0;
		s->numberOfMessagesNotInterested=0;
		s->numberOfBytesNotInterested=0;
		s->nofFreePacketBuffers=0;
		s->nofUsedPacketBuffers=0;

        s->nofBytesBeforeDecompression=0;
        s->nofBytesAfterDecompression=0;
    }
}
void
nw_SendChannelStatisticsFree(
		nw_SendChannelStatistics s)
{
    if(s){
        os_free(s);
    }
}

void
nw_ReceiveChannelStatisticsFree(
		nw_ReceiveChannelStatistics s)
{
    if(s){
        os_free(s);
    }
}


void
nw_ReceiveChannelUpdate(
	v_networkChannelStatistics s,
    nw_ReceiveChannelStatistics nws)
{

    if(s && nws){

    	if(nws->numberOfMessagesSent != 0){
    		s->numberOfMessagesSent = nws->numberOfMessagesSent;
    	}
        if(nws->numberOfBytesSent != 0){
            s->numberOfBytesSent = nws->numberOfBytesSent;
        }
        if(nws->numberOfPacketsSent != 0){
            s->numberOfPacketsSent = nws->numberOfPacketsSent;
        }

        if(nws->numberOfMessagesFragmented != 0){
            s->numberOfMessagesFragmented = nws->numberOfMessagesFragmented;
        }
        if(nws->numberOfMessagesPacked != 0){
            s->numberOfMessagesPacked = nws->numberOfMessagesPacked;
        }

        if(nws->numberOfKnownNodes != 0){
            s->numberOfKnownNodes = nws->numberOfKnownNodes;
        }
        if(nws->numberOfBytesResent != 0){
            s->numberOfBytesResent = nws->numberOfBytesResent;
            v_maxValueSetValue(&s->maxNumberOfBytesResentToOneNode, s->numberOfBytesResent);
        }
        if(nws->numberOfPacketsResent != 0){
            s->numberOfPacketsResent = nws->numberOfPacketsResent;
            v_maxValueSetValue(&s->maxNumberOfPacketsResentToOneNode, s->numberOfPacketsResent);
        }
        if(nws->numberOfBytesInResendBuffer != 0){
            s->numberOfBytesInResendBuffer = nws->numberOfBytesInResendBuffer;
        }
        if(nws->numberOfPacketsInResendBuffer != 0){
            s->numberOfPacketsInResendBuffer = nws->numberOfPacketsInResendBuffer;
        }

        if(nws->numberOfMessagesReceived != 0){
            s->numberOfMessagesReceived = nws->numberOfMessagesReceived;
        }
        if(nws->numberOfBytesReceived != 0){
            s->numberOfBytesReceived = nws->numberOfBytesReceived;
        }
        if(nws->numberOfPacketsReceived != 0){
            s->numberOfPacketsReceived = nws->numberOfPacketsReceived;
        }
        if(nws->numberOfPacketsLost != 0){
            s->numberOfPacketsLost = nws->numberOfPacketsLost;
        }
        if(nws->numberOfAcksSent != 0){
            s->numberOfAcksSent = nws->numberOfAcksSent;
        }

        if(nws->numberOfMessagesDelivered != 0){
            s->numberOfMessagesDelivered = nws->numberOfMessagesDelivered;
        }
        if(nws->numberOfBytesDelivered != 0){
            s->numberOfBytesDelivered = nws->numberOfBytesDelivered;
        }
        if(nws->numberOfMessagesNotInterested != 0){
            s->numberOfMessagesNotInterested = nws->numberOfMessagesNotInterested;
        }
        if(nws->numberOfBytesNotInterested != 0){
            s->numberOfBytesNotInterested = nws->numberOfBytesNotInterested;
        }
        if(nws->nofFreePacketBuffers != 0){
            s->nofFreePacketBuffers = nws->nofFreePacketBuffers;
        }
        if(nws->nofUsedPacketBuffers != 0){
            s->nofUsedPacketBuffers = nws->nofUsedPacketBuffers;
        }
        if(nws->nofBytesBeforeDecompression != 0){
            s->nofBytesBeforeDecompression = nws->nofBytesBeforeDecompression;
        }
        if(nws->nofBytesAfterDecompression != 0){
            s->nofBytesAfterDecompression = nws->nofBytesAfterDecompression;
        }
    }
    return;
}

void
nw_SendChannelUpdate(
    v_networkChannelStatistics s,
    nw_SendChannelStatistics nws)
{

    if(s && nws){
        
    	if(nws->numberOfMessagesSent != 0){
    		s->numberOfMessagesSent = nws->numberOfMessagesSent;
    	}
        if(nws->numberOfBytesSent != 0){
            s->numberOfBytesSent = nws->numberOfBytesSent;
        }
        if(nws->numberOfPacketsSent != 0){
            s->numberOfPacketsSent = nws->numberOfPacketsSent;
        }

        if(nws->numberOfMessagesFragmented != 0){
            s->numberOfMessagesFragmented = nws->numberOfMessagesFragmented;
        }
        if(nws->numberOfMessagesPacked != 0){
            s->numberOfMessagesPacked = nws->numberOfMessagesPacked;
        }

        if(nws->numberOfKnownNodes != 0){
            s->numberOfKnownNodes = nws->numberOfKnownNodes;
        }
        if(nws->numberOfBytesResent != 0){
            s->numberOfBytesResent = nws->numberOfBytesResent;
            v_maxValueSetValue(&s->maxNumberOfBytesResentToOneNode, s->numberOfBytesResent);
        }
        if(nws->numberOfPacketsResent != 0){
            s->numberOfPacketsResent = nws->numberOfPacketsResent;
            v_maxValueSetValue(&s->maxNumberOfPacketsResentToOneNode, s->numberOfPacketsResent);
        }
        if(nws->numberOfBytesInResendBuffer != 0){
            s->numberOfBytesInResendBuffer = nws->numberOfBytesInResendBuffer;
        }
        if(nws->numberOfPacketsInResendBuffer != 0){
            s->numberOfPacketsInResendBuffer = nws->numberOfPacketsInResendBuffer;
        }

        if(nws->numberOfMessagesReceived != 0){
            s->numberOfMessagesReceived = nws->numberOfMessagesReceived;
        }
        if(nws->numberOfBytesReceived != 0){
            s->numberOfBytesReceived = nws->numberOfBytesReceived;
        }
        if(nws->numberOfPacketsReceived != 0){
            s->numberOfPacketsReceived = nws->numberOfPacketsReceived;
        }
        if(nws->numberOfPacketsLost != 0){
            s->numberOfPacketsLost = nws->numberOfPacketsLost;
        }
        if(nws->numberOfAcksSent != 0){
            s->numberOfAcksSent = nws->numberOfAcksSent;
        }

        if(nws->numberOfMessagesDelivered != 0){
            s->numberOfMessagesDelivered = nws->numberOfMessagesDelivered;
        }
        if(nws->numberOfBytesDelivered != 0){
            s->numberOfBytesDelivered = nws->numberOfBytesDelivered;
        }
        if(nws->numberOfMessagesNotInterested != 0){
            s->numberOfMessagesNotInterested = nws->numberOfMessagesNotInterested;
        }
        if(nws->numberOfBytesNotInterested != 0){
            s->numberOfBytesNotInterested = nws->numberOfBytesNotInterested;
        }

        s->adminQueueAcks = nws->adminQueueAcks;


        s->adminQueueData = nws->adminQueueData;

        if(nws->nofBytesBeforeCompression != 0){
            s->nofBytesBeforeCompression = nws->nofBytesBeforeCompression;
        }
        if(nws->nofBytesAfterCompression != 0){
            s->nofBytesAfterCompression = nws->nofBytesAfterCompression;
        }

    }
    return;
}
