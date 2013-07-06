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
#include "v_networkQueueStatistics.h"
#include "v_maxValue.h"
#include "os_report.h"

/* Legitimate use of static variable to hold type information in this case.
 * This type information may be accessed multiple times depending on how many
 * channels are set up, but this is only ever in the same single instance of
 * the networking service, so will not affect multi domain support.
 */
static c_type networkQueueStatisticsType = NULL;

v_networkQueueStatistics v_networkQueueStatisticsNew(v_kernel k, const c_char *name)
{
    v_networkQueueStatistics nqs;
    c_string channelName;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    if (networkQueueStatisticsType == NULL) {
        networkQueueStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_networkQueueStatistics");
    }
    nqs = v_networkQueueStatistics(v_new(k, networkQueueStatisticsType));
    channelName = c_stringNew(c_getBase(c_object(k)),name);
    v_networkQueueStatisticsInit(nqs,channelName);
    return nqs;
}

void v_networkQueueStatisticsInit(v_networkQueueStatistics nqs, c_string name)
{
    assert(nqs != NULL);
    assert(C_TYPECHECK(nqs,v_networkQueueStatistics));

    nqs->name = name;
    nqs->numberOfSamplesArrived =0;
    nqs->numberOfSamplesInserted =0;
    nqs->numberOfSamplesRejected =0;
    nqs->numberOfSamplesTaken =0;
    v_fullCounterInit(&(nqs->numberOfSamplesWaiting));
     /*
    v_statisticsULongInit(v_networkQueue,numberOfSamplesArrived,nqs);
    v_statisticsULongInit(v_networkQueue,numberOfSamplesInserted,nqs);
    v_statisticsULongInit(v_networkQueue,numberOfSamplesRejected,nqs);
    v_statisticsULongInit(v_networkQueue,numberOfSamplesTaken,nqs);
    v_statisticsFullCounterInit(v_networkQueue,numberOfSamplesWaiting,nqs);*/

}

c_bool v_networkQueueStatisticsReset(v_networkQueueStatistics nqs, const c_char* fieldName)
{
    c_bool result;

    OS_UNUSED_ARG(fieldName);
    assert(nqs!=NULL);
    assert(C_TYPECHECK(nqs, v_networkQueueStatistics));
    nqs->numberOfSamplesArrived =0;
	nqs->numberOfSamplesInserted =0;
	nqs->numberOfSamplesRejected =0;
	nqs->numberOfSamplesTaken =0;
	v_fullCounterInit(&(nqs->numberOfSamplesWaiting));
    result = TRUE;

    return result;
}

void v_networkQueueStatisticsDeinit(v_networkQueueStatistics nqs)
{
    assert(nqs != NULL);
    assert(C_TYPECHECK(nqs, v_networkQueueStatistics));
    c_free(nqs->name);
}

void v_networkQueueStatisticsFree(v_networkQueueStatistics nqs)
{
    assert(nqs != NULL);
    assert(C_TYPECHECK(nqs, v_networkQueueStatistics));

    v_networkQueueStatisticsDeinit(nqs);

    c_free(nqs);

}


