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

#include "v_statistics.h"
#include "v_networkReaderStatistics.h"
#include "v_dataReaderStatistics.h"
#include "v_maxValue.h"
#include "os_report.h"

v_networkReaderStatistics
v_networkReaderStatisticsNew(
    v_kernel k)
{
    v_networkReaderStatistics nrs;
    c_type type;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    type = c_resolve(c_getBase(k), "kernelModuleI::v_networkReaderStatistics");
    assert(type);
    nrs = v_networkReaderStatistics(v_new(k, type));
    c_free(type);
    v_networkReaderStatisticsInit(nrs);
    return nrs;
}

void
v_networkReaderStatisticsInit(
    v_networkReaderStatistics nrs)
{
    v_kernel kernel;
    assert(nrs != NULL);
    assert(C_TYPECHECK(nrs,v_networkReaderStatistics));
    kernel = v_objectKernel(nrs);
    v_statisticsInit(v_statistics(nrs));
    nrs->queuesCount = 0; /* better to get the actual value from networking */

    nrs->queues = c_arrayNew(c_resolve(c_getBase(kernel),
                             "kernelModuleI::v_networkQueueStatistics"),64);
}

void
v_networkReaderStatisticsDeinit(
    v_networkReaderStatistics nrs)
{
    OS_UNUSED_ARG(nrs);
    assert(nrs != NULL);
    assert(C_TYPECHECK(nrs, v_networkReaderStatistics));
}

void
v_networkReaderStatisticsFree(
    v_networkReaderStatistics nrs)
{
    assert(nrs != NULL);
    assert(C_TYPECHECK(nrs, v_networkReaderStatistics));

    v_networkReaderStatisticsDeinit(nrs);
    c_free(nrs);
}


