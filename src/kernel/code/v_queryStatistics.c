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
        v_statisticsULongResetInternal(v_query, numberOfInstanceReads, qs);
        v_statisticsULongResetInternal(v_query, numberOfNextInstanceReads, qs);
        v_statisticsULongResetInternal(v_query, numberOfTakes, qs);
        v_statisticsULongResetInternal(v_query, numberOfInstanceTakes, qs);
        v_statisticsULongResetInternal(v_query, numberOfNextInstanceTakes, qs);

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
