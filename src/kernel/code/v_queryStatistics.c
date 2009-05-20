/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include "v_queryStatistics.h"

static c_type queryStatisticsType = NULL;

v_queryStatistics v_queryStatisticsNew(v_kernel k)
{
    v_queryStatistics qs;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    if (queryStatisticsType == NULL) {
        queryStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_queryStatistics");
    }
    qs = v_queryStatistics(v_new(k, queryStatisticsType));
    v_queryStatisticsInit(qs);
    return qs;
}

void v_queryStatisticsInit(v_queryStatistics qs)
{
    assert(qs != NULL);
    assert(C_TYPECHECK(qs,v_queryStatistics));

    v_statisticsInit(v_statistics(qs));

    qs->numberOfReads = 0;
    qs->numberOfTakes = 0;
}

void v_queryStatisticsDeinit(v_queryStatistics qs)
{
    assert(qs!=NULL);
    assert(C_TYPECHECK(qs, v_queryStatistics));
}

c_bool v_queryStatisticsReset(v_queryStatistics qs, const c_char* fieldName)
{
    c_bool result;

    assert(qs!=NULL);
    assert(C_TYPECHECK(qs, v_queryStatistics));

    result = FALSE;

    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(qs), fieldName);
    } else {
        v_statisticsULongResetInternal(v_query, numberOfReads, qs);
        v_statisticsULongResetInternal(v_query, numberOfTakes, qs);

        result = TRUE;
    }
    return result;
}

void v_queryStatisticsFree(v_queryStatistics qs)
{
    assert(qs != NULL);
    assert(C_TYPECHECK(qs, v_queryStatistics));

    v_queryStatisticsDeinit(qs);
    c_free(qs);
}
