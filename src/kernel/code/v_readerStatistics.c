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
#include "v__statisticsInterface.h"
#include "v_readerStatistics.h"
#include "v_maxValue.h"
#include "v_fullCounter.h"

v_readerStatistics v_readerStatisticsNew(v_kernel k)
{
    v_readerStatistics rs;
    c_type readerStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    readerStatisticsType = v_kernelType(k,K_READERSTATISTICS);
    rs = v_readerStatistics(v_new(k, readerStatisticsType));
    v_readerStatisticsInit(rs);
    return rs;
}

void v_readerStatisticsInit(v_readerStatistics rs)
{
    assert(rs != NULL);
    assert(C_TYPECHECK(rs,v_readerStatistics));

    v_statisticsInit(v_statistics(rs));
    v_maxValueInit(&rs->maxSampleSize);
    v_maxValueInit(&rs->maxSamplesPerInstance);
    v_maxValueInit(&rs->maxNumberOfSamples);
    v_maxValueInit(&rs->maxNumberOfInstances);

    v_fullCounterInit(&rs->readLatency);
    v_fullCounterInit(&rs->transportLatency);

    rs->numberOfSamples = 0;
    rs->numberOfInstances = 0;

    rs->numberOfInstancesWithStatusNew = 0;
    rs->numberOfInstancesWithStatusAlive = 0;
    rs->numberOfInstancesWithStatusDisposed = 0;
    rs->numberOfInstancesWithStatusNoWriters = 0;

    rs->numberOfSamplesWithStatusRead = 0;
    rs->numberOfSamplesExpired = 0;
    rs->numberOfSamplesPurgedByDispose = 0;
    rs->numberOfSamplesPurgedByNoWriters = 0;
    rs->numberOfSamplesArrived = 0;
    rs->numberOfSamplesInserted = 0;
    rs->numberOfSamplesDiscarded = 0;
    rs->numberOfSamplesRead = 0;
    rs->numberOfSamplesTaken = 0;
    rs->numberOfSamplesLost = 0;

    rs->numberOfSamplesRejectedBySamplesLimit = 0;
    rs->numberOfSamplesRejectedByInstancesLimit = 0;
    rs->numberOfReads = 0;
    rs->numberOfInstanceReads = 0;
    rs->numberOfNextInstanceReads = 0;
    rs->numberOfInstanceLookups = 0;
    rs->numberOfTakes = 0;
    rs->numberOfInstanceTakes = 0;
    rs->numberOfNextInstanceTakes = 0;
}

void v_readerStatisticsDeinit(v_readerStatistics rs)
{
    OS_UNUSED_ARG(rs);
    assert(rs!=NULL);
    assert(C_TYPECHECK(rs, v_readerStatistics));
}

c_bool v_readerStatisticsReset(v_readerStatistics rs, const c_char* fieldName)
{
    c_bool result;

    assert(rs!=NULL);
    assert(C_TYPECHECK(rs, v_readerStatistics));

    result = FALSE;

    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(rs), fieldName);
    } else {
        v_maxValueReset(&rs->maxSampleSize);
        v_maxValueReset(&rs->maxSamplesPerInstance);
        v_maxValueReset(&rs->maxNumberOfSamples);
        v_maxValueReset(&rs->maxNumberOfInstances);

        v_fullCounterReset(&rs->readLatency);
        v_fullCounterReset(&rs->transportLatency);

        v_statisticsULongResetInternal(v_reader, numberOfSamples, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstances, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstancesWithStatusNew, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstancesWithStatusAlive, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstancesWithStatusDisposed, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstancesWithStatusNoWriters, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesWithStatusRead, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesExpired, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesPurgedByDispose, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesPurgedByNoWriters, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesArrived, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesInserted, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesDiscarded, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesRead, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesTaken, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesRejectedBySamplesLimit, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesRejectedByInstancesLimit, rs);
        v_statisticsULongResetInternal(v_reader, numberOfSamplesLost, rs);
        v_statisticsULongResetInternal(v_reader, numberOfReads, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstanceReads, rs);
        v_statisticsULongResetInternal(v_reader, numberOfNextInstanceReads, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstanceLookups, rs);
        v_statisticsULongResetInternal(v_reader, numberOfTakes, rs);
        v_statisticsULongResetInternal(v_reader, numberOfInstanceTakes, rs);
        v_statisticsULongResetInternal(v_reader, numberOfNextInstanceTakes, rs);

        result = TRUE;
    }
    return result;
}

void v_readerStatisticsFree(v_readerStatistics rs)
{
    assert(rs != NULL);
    assert(C_TYPECHECK(rs, v_readerStatistics));

    v_readerStatisticsDeinit(rs);
    c_free(rs);
}
