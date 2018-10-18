/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "v_networkChannelStatistics.h"
#include "v_maxValue.h"
#include "v_fullCounter.h"
#include "v_avgValue.h"
#include "v_minValue.h"
#include "os_report.h"

v_networkChannelStatistics
v_networkChannelStatisticsNew(
    v_kernel k,
    const c_char *name)
{
    v_networkChannelStatistics ncs;
    c_string channelName;
    c_type type;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    type = c_resolve(c_getBase(k), "kernelModuleI::v_networkChannelStatistics");
    assert(type);
    ncs = v_networkChannelStatistics(v_new(k, type));
    c_free(type);
    channelName = c_stringNew(c_getBase(c_object(k)),name);
    v_networkChannelStatisticsInit(ncs,channelName);
    return ncs;
}

void
v_networkChannelStatisticsInit(
    v_networkChannelStatistics ncs,
    c_string name)
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
    ncs->numberOfPacketsReliabilityMismatch=0;
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

void
v_networkChannelStatisticsDeinit(
    v_networkChannelStatistics ncs)
{
    assert(ncs != NULL);
    assert(C_TYPECHECK(ncs, v_networkChannelStatistics));
    OS_UNUSED_ARG(ncs);
}

void
v_networkChannelStatisticsFree(
    v_networkChannelStatistics ncs)
{
    assert(ncs != NULL);
    assert(C_TYPECHECK(ncs, v_networkChannelStatistics));

    v_networkChannelStatisticsDeinit(ncs);

    c_free(ncs);
}


