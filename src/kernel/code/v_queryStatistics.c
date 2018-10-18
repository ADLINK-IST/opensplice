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
#include "v_queryStatistics.h"

v_queryStatistics v_queryStatisticsNew(v_kernel k)
{
    v_queryStatistics qs;
    c_type queryStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    queryStatisticsType = v_kernelType(k,K_QUERYSTATISTICS);
    qs = v_queryStatistics(v_new(k, queryStatisticsType));
    v_queryStatisticsInit(qs);
    return qs;
}

void v_queryStatisticsInit(v_queryStatistics qs)
{
    assert(qs != NULL);
    assert(C_TYPECHECK(qs,v_queryStatistics));

    v_statisticsInit(v_statistics(qs));

    qs->numberOfReads               = 0;
    qs->numberOfInstanceReads       = 0;
    qs->numberOfNextInstanceReads   = 0;
    qs->numberOfTakes               = 0;
    qs->numberOfInstanceTakes       = 0;
    qs->numberOfNextInstanceTakes   = 0;
}

void v_queryStatisticsDeinit(v_queryStatistics qs)
{
    OS_UNUSED_ARG(qs);
    assert(qs!=NULL);
    assert(C_TYPECHECK(qs, v_queryStatistics));
}

void v_queryStatisticsFree(v_queryStatistics qs)
{
    assert(qs != NULL);
    assert(C_TYPECHECK(qs, v_queryStatistics));

    v_queryStatisticsDeinit(qs);
    c_free(qs);
}
