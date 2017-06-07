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
#include "v_cmsoapStatistics.h"
#include "v_maxValue.h"

v_cmsoapStatistics v_cmsoapStatisticsNew(v_kernel k)
{
    v_cmsoapStatistics cs;
    c_type cmsoapStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    cmsoapStatisticsType = c_resolve(c_getBase(k), "kernelModuleI::v_cmsoapStatistics");

    cs = v_cmsoapStatistics(v_new(k, cmsoapStatisticsType));
    v_cmsoapStatisticsInit(cs);
    return cs;
}

void v_cmsoapStatisticsInit(v_cmsoapStatistics cs)
{
    assert(cs != NULL);
    assert(C_TYPECHECK(cs, v_cmsoapStatistics));
    v_statisticsInit(v_statistics(cs));
    v_maxValueInit(&cs->maxConnectedClients);
    v_maxValueInit(&cs->maxClientThreads);
    cs->connectedClients = 0;
    cs->clientThreads = 0;
    cs->requestsHandled = 0;
}

void v_cmsoapStatisticsDeinit(v_cmsoapStatistics cs)
{
    assert(cs!=NULL);
    assert(C_TYPECHECK(cs, v_cmsoapStatistics));
    OS_UNUSED_ARG(cs);
}

void v_cmsoapStatisticsFree(v_cmsoapStatistics cs)
{
    assert(cs != NULL);
    assert(C_TYPECHECK(cs, v_cmsoapStatistics));

    v_cmsoapStatisticsDeinit(cs);
    c_free(cs);
}


