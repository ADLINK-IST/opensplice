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
#include "v_writerStatistics.h"
#include "v_maxValue.h"

v_writerStatistics
v_writerStatisticsNew(
    v_kernel k)
{
    v_writerStatistics ws;
    c_type writerStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    writerStatisticsType = v_kernelType(k,K_WRITERSTATISTICS);
    ws = v_writerStatistics(v_new(k, writerStatisticsType));
    v_writerStatisticsInit(ws);
    return ws;
}

void
v_writerStatisticsInit(
    v_writerStatistics ws)
{
    assert(ws != NULL);
    assert(C_TYPECHECK(ws, v_writerStatistics));

    v_statisticsInit(v_statistics(ws));
    ws->numberOfWrites = 0;
    ws->numberOfDisposes = 0;
    ws->numberOfRegisters = 0;
    ws->numberOfImplicitRegisters = 0;
    ws->numberOfUnregisters = 0;
    ws->numberOfTimedOutWrites = 0;
    ws->numberOfWritesBlockedBySamplesLimit = 0;
    ws->numberOfWritesBlockedByInstanceLimit = 0;
    ws->numberOfWritesBlockedBySamplesPerInstanceLimit = 0;
    ws->numberOfRetries = 0;

    ws->numberOfInstancesWithStatusAlive = 0;
    ws->numberOfInstancesWithStatusDisposed = 0;
    ws->numberOfInstancesWithStatusUnregistered = 0;
    ws->numberOfSamples = 0;
    v_maxValueInit(&ws->maxNumberOfSamplesPerInstance);
}

void
v_writerStatisticsDeinit(
    v_writerStatistics ws)
{
    assert(ws!=NULL);
    assert(C_TYPECHECK(ws, v_writerStatistics));
    OS_UNUSED_ARG(ws);
}

void
v_writerStatisticsFree(
    v_writerStatistics ws)
{
    assert(ws != NULL);
    assert(C_TYPECHECK(ws, v_writerStatistics));

    v_writerStatisticsDeinit(ws);
    c_free(ws);
}
