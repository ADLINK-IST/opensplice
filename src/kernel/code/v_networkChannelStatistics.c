/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "v_statistics.h"
#include "v__statistics.h"
#include "v_networkChannelStatistics.h"
#include "v_maxValue.h"
#include "os_report.h"

/* Legitimate use of static variable to hold type information in this case.
 * This type information may be accessed multiple times depending on how many
 * channels are set up, but this is only ever in the same single instance of
 * the networking service, so will not affect multi domain support.
 */
static c_type networkChannelStatisticsType = NULL;

v_networkChannelStatistics v_networkChannelStatisticsNew(v_kernel k, const c_char *name)
{
    v_networkChannelStatistics ncs;
    c_string channelName;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    if (networkChannelStatisticsType == NULL) {
        networkChannelStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_networkChannelStatistics");
    }
    ncs = v_networkChannelStatistics(v_new(k, networkChannelStatisticsType));
    channelName = c_stringNew(c_getBase(c_object(k)),name);
    v_networkChannelStatisticsInit(ncs,channelName);
    return ncs;
}

void v_networkChannelStatisticsInit(v_networkChannelStatistics ncs, c_string name)
{
    assert(ncs != NULL);
    assert(C_TYPECHECK(ncs,v_networkChannelStatistics));

    ncs->name = name;
    ncs->numberOfMessagesSent=0;
    ncs->numberOfBytesSent=0;
    ncs->numberOfPacketsSent=0;

    ncs->numberOfMessagesFragmented=0;
    ncs->numberOfMessagesPacked=0;

    ncs->numberOfKnownNodes=0;
    ncs->numberOfBytesResent=0;
    ncs->numberOfPacketsResent=0;
    ncs->numberOfBytesInResendBuffer=0;
    ncs->numberOfPacketsInResendBuffer=0;
    ncs->maxNumberOfBytesResentToOneNode=0;
    ncs->maxNumberOfPacketsResentToOneNode=0;

    ncs->numberOfMessagesReceived=0;
    ncs->numberOfBytesReceived=0;
    ncs->numberOfPacketsReceived=0;
    ncs->numberOfPacketsLost=0;
    ncs->numberOfPacketsOutOfOrder=0;
    ncs->numberOfAcksSent=0;

    ncs->numberOfMessagesDelivered=0;
    ncs->numberOfBytesDelivered=0;
    ncs->numberOfMessagesNotInterested=0;
    ncs->numberOfBytesNotInterested=0;
    ncs->numberOfPacketsNotConnectedPartition=0;
    ncs->numberOfPacketsUnknownAddress=0;
    ncs->numberOfPacketsInvalid=0;
    ncs->numberOfPacketsOutOfOrderDropped=0;
    ncs->nofFreePacketBuffers=0;
    ncs->nofUsedPacketBuffers=0;

    ncs->reorderAdminPacketsQueued=0;
    v_avgValueInit(&ncs->reorderAdminPacketsQueuedAvg);
    v_minValueInit(&ncs->reorderAdminPacketsQueuedMin);
    v_maxValueInit(&ncs->reorderAdminPacketsQueuedMax);

    ncs->reorderAdminBytesQueued=0;
    v_avgValueInit(&ncs->reorderAdminBytesQueuedAvg);
    v_minValueInit(&ncs->reorderAdminBytesQueuedMin);
    v_maxValueInit(&ncs->reorderAdminBytesQueuedMax);

    ncs->ringBufferMessagesQueued=0;
    v_avgValueInit(&ncs->ringBufferMessagesQueuedAvg);
    v_minValueInit(&ncs->ringBufferMessagesQueuedMin);
    v_maxValueInit(&ncs->ringBufferMessagesQueuedMax);

    ncs->resendAdminPacketsQueued=0;
    v_avgValueInit(&ncs->resendAdminPacketsQueuedAvg);
    v_minValueInit(&ncs->resendAdminPacketsQueuedMin);
    v_maxValueInit(&ncs->resendAdminPacketsQueuedMax);

    ncs->resendAdminBytesQueued=0;
    v_avgValueInit(&ncs->resendAdminBytesQueuedAvg);
    v_minValueInit(&ncs->resendAdminBytesQueuedMin);
    v_maxValueInit(&ncs->resendAdminBytesQueuedMax);

    v_fullCounterInit(&(ncs->adminQueueAcks));
    v_fullCounterInit(&(ncs->adminQueueData));

    ncs->nofBytesBeforeCompression=0;
    ncs->nofBytesAfterCompression=0;
    ncs->nofBytesBeforeDecompression=0;
    ncs->nofBytesAfterDecompression=0;
}

c_bool v_networkChannelStatisticsReset(v_networkChannelStatistics ncs)
{
    c_bool result;

    assert(ncs!=NULL);
    assert(C_TYPECHECK(ncs, v_networkChannelStatistics));
	ncs->numberOfMessagesSent=0;
	ncs->numberOfBytesSent=0;
	ncs->numberOfPacketsSent=0;

	ncs->numberOfMessagesFragmented=0;
	ncs->numberOfMessagesPacked=0;

	ncs->numberOfKnownNodes=0;
	ncs->numberOfBytesResent=0;
	ncs->numberOfPacketsResent=0;
	ncs->numberOfBytesInResendBuffer=0;
	ncs->numberOfPacketsInResendBuffer=0;
    ncs->maxNumberOfBytesResentToOneNode=0;
    ncs->maxNumberOfPacketsResentToOneNode=0;

	ncs->numberOfMessagesReceived=0;
	ncs->numberOfBytesReceived=0;
	ncs->numberOfPacketsReceived=0;
	ncs->numberOfPacketsLost=0;
	ncs->numberOfAcksSent=0;

	ncs->numberOfMessagesDelivered=0;
	ncs->numberOfBytesDelivered=0;
	ncs->numberOfMessagesNotInterested=0;
	ncs->numberOfBytesNotInterested=0;
	ncs->nofFreePacketBuffers=0;
	ncs->nofUsedPacketBuffers=0;
	v_fullCounterInit(&(ncs->adminQueueAcks));
	v_fullCounterInit(&(ncs->adminQueueData));

    ncs->nofBytesBeforeCompression=0;
    ncs->nofBytesAfterCompression=0;
    ncs->nofBytesBeforeDecompression=0;
    ncs->nofBytesAfterDecompression=0;

    result = TRUE;
    return result;
}

void v_networkChannelStatisticsDeinit(v_networkChannelStatistics ncs)
{
    assert(ncs != NULL);
    assert(C_TYPECHECK(ncs, v_networkChannelStatistics));
    OS_UNUSED_ARG(ncs);
}

void v_networkChannelStatisticsFree(v_networkChannelStatistics ncs)
{
    assert(ncs != NULL);
    assert(C_TYPECHECK(ncs, v_networkChannelStatistics));

    v_networkChannelStatisticsDeinit(ncs);

    c_free(ncs);

}


