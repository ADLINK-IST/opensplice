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

#include "v_networkQueueStatistics.h"
#include "v_maxValue.h"
#include "v_fullCounter.h"
#include "os_report.h"

v_networkQueueStatistics
v_networkQueueStatisticsNew(
    v_kernel k,
    const c_char *name)
{
    v_networkQueueStatistics nqs;
    c_string channelName;
    c_type type;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    type = c_resolve(c_getBase(k), "kernelModuleI::v_networkQueueStatistics");
    assert(type);
    nqs = v_networkQueueStatistics(v_new(k, type));
    c_free(type);
    channelName = c_stringNew(c_getBase(c_object(k)),name);
    v_networkQueueStatisticsInit(nqs,channelName);
    return nqs;
}

void
v_networkQueueStatisticsInit(
    v_networkQueueStatistics nqs,
    c_string name)
{
    assert(nqs != NULL);
    assert(C_TYPECHECK(nqs,v_networkQueueStatistics));

    nqs->name = name;
    nqs->numberOfSamplesArrived =0;
    nqs->numberOfSamplesInserted =0;
    nqs->numberOfSamplesRejected =0;
    nqs->numberOfSamplesTaken =0;
    v_fullCounterInit(&(nqs->numberOfSamplesWaiting));
}

void
v_networkQueueStatisticsDeinit(
    v_networkQueueStatistics nqs)
{
    assert(nqs != NULL);
    assert(C_TYPECHECK(nqs, v_networkQueueStatistics));
    c_free(nqs->name);
}

void
v_networkQueueStatisticsFree(
    v_networkQueueStatistics nqs)
{
    assert(nqs != NULL);
    assert(C_TYPECHECK(nqs, v_networkQueueStatistics));

    v_networkQueueStatisticsDeinit(nqs);

    c_free(nqs);
}


