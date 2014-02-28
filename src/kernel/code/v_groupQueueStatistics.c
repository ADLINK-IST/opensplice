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
                                         "kernelModule::v_groupQueueStatistics");
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

    result = FALSE;

    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(gqs), fieldName);
    } else {
        v_fullCounterReset(&gqs->numberOfSamples);
        v_statisticsULongResetInternal(v_groupQueue, numberOfWrites, gqs);
        v_statisticsULongResetInternal(v_groupQueue, numberOfReads,  gqs);
        v_statisticsULongResetInternal(v_groupQueue, numberOfTakes,  gqs);

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
