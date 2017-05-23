/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "v_statistics.h"
#include "v_dataReaderStatistics.h"
#include "v_maxValue.h"
#include "v_fullCounter.h"

v_dataReaderStatistics v_dataReaderStatisticsNew(v_kernel k)
{
    v_dataReaderStatistics rs;
    c_type readerStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    readerStatisticsType = v_kernelType(k,K_DATAREADERSTATISTICS);
    rs = v_dataReaderStatistics(v_new(k, readerStatisticsType));
    v_dataReaderStatisticsInit(rs);
    return rs;
}

void v_dataReaderStatisticsInit(v_dataReaderStatistics rs)
{
    assert(rs != NULL);
    assert(C_TYPECHECK(rs,v_dataReaderStatistics));

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

void v_dataReaderStatisticsDeinit(v_dataReaderStatistics rs)
{
    OS_UNUSED_ARG(rs);
    assert(rs!=NULL);
    assert(C_TYPECHECK(rs, v_dataReaderStatistics));
}

void v_readerStatisticsFree(v_dataReaderStatistics rs)
{
    assert(rs != NULL);
    assert(C_TYPECHECK(rs, v_dataReaderStatistics));

    v_dataReaderStatisticsDeinit(rs);
    c_free(rs);
}
