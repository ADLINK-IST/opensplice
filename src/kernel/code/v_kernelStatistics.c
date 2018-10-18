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
#include "v_kernelStatistics.h"
#include "v_maxValue.h"

v_kernelStatistics v_kernelStatisticsNew(v_kernel k)
{
    v_kernelStatistics ks;
    c_type kernelStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k,v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    kernelStatisticsType = c_resolve(c_getBase((c_object)k), "kernelModuleI::v_kernelStatistics");

    ks = v_kernelStatistics(v_new(k, kernelStatisticsType));
    v_kernelStatisticsInit(ks);
    return ks;
}

void v_kernelStatisticsInit(v_kernelStatistics ks)
{
    assert(ks!=NULL);
    assert(C_TYPECHECK(ks, v_kernelStatistics));

    v_statisticsInit(v_statistics(ks));
    v_maxValueInit(&ks->maxShmUsed);
    v_maxValueInit(&ks->maxShmGarbage);
    v_maxValueInit(&ks->maxShmClaims);
    ks->shmUsed = 0;
    ks->shmClaims = 0;
    ks->shmClaimFails = 0;
    ks->shmGarbage = 0;
}

void v_kernelStatisticsDeinit(v_kernelStatistics ks)
{
    assert(ks!=NULL);
    assert(C_TYPECHECK(ks, v_kernelStatistics));
    OS_UNUSED_ARG(ks);
}

void v_kernelStatisticsFree(v_kernelStatistics ks)
{
    assert(ks != NULL);
    assert(C_TYPECHECK(ks, v_kernelStatistics));

    v_kernelStatisticsDeinit(ks);
    c_free(ks);
}


