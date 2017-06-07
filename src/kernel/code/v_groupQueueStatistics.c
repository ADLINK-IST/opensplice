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
#include "v__statistics.h"
#include "v_groupQueueStatistics.h"
#include "v_maxValue.h"
#include "v_fullCounter.h"

v_groupQueueStatistics v_groupQueueStatisticsNew(v_kernel k)
{
    v_groupQueueStatistics gqs;
    c_type groupQueueStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    groupQueueStatisticsType = c_resolve(c_getBase(k),
                                         "kernelModuleI::v_groupQueueStatistics");
    gqs = v_groupQueueStatistics(v_new(k, groupQueueStatisticsType));
    v_groupQueueStatisticsInit(gqs);
    return gqs;
}

void v_groupQueueStatisticsInit(v_groupQueueStatistics gqs)
{
    assert(gqs != NULL);
    assert(C_TYPECHECK(gqs,v_groupQueueStatistics));

    v_statisticsInit(v_statistics(gqs));

    gqs->numberOfWrites = 0;
    gqs->numberOfReads  = 0;
    gqs->numberOfTakes  = 0;
    v_fullCounterInit(&gqs->numberOfSamples);
}

void v_groupQueueStatisticsDeinit(v_groupQueueStatistics gqs)
{
    OS_UNUSED_ARG(gqs);
    assert(gqs!=NULL);
    assert(C_TYPECHECK(gqs, v_groupQueueStatistics));
}

c_bool v_groupQueueStatisticsReset(v_groupQueueStatistics gqs, const c_char* fieldName)
{
    c_bool result;

    assert(gqs!=NULL);
    assert(C_TYPECHECK(gqs, v_groupQueueStatistics));

    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(gqs), fieldName);
    } else {
        gqs->numberOfWrites = 0;
        gqs->numberOfReads = 0;
        gqs->numberOfTakes = 0;
        v_fullCounterReset(&gqs->numberOfSamples);
        result = TRUE;
    }
    return result;
}

void v_groupQueueStatisticsFree(v_groupQueueStatistics gqs)
{
    assert(gqs != NULL);
    assert(C_TYPECHECK(gqs, v_groupQueueStatistics));

    v_groupQueueStatisticsDeinit(gqs);
    c_free(gqs);
}
