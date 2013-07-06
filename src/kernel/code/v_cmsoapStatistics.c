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
#include "v_cmsoapStatistics.h"
#include "v_maxValue.h"

v_cmsoapStatistics v_cmsoapStatisticsNew(v_kernel k)
{
    v_cmsoapStatistics cs;
    c_type cmsoapStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    cmsoapStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_cmsoapStatistics");

    cs = v_cmsoapStatistics(v_new(k, cmsoapStatisticsType));
    v_cmsoapStatisticsInit(cs);
    return cs;
}

void v_cmsoapStatisticsInit(v_cmsoapStatistics cs)
{
    assert(cs != NULL);
    assert(C_TYPECHECK(cs, v_cmsoapStatistics));
    v_statistics(cs)->lastReset = C_TIME_ZERO;
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

c_bool v_cmsoapStatisticsReset(v_cmsoapStatistics cs, const c_char* fieldName)
{
    c_bool result;

    assert(cs != NULL);
    assert(C_TYPECHECK(cs, v_cmsoapStatistics));

    result =  FALSE;

    if (fieldName != NULL) {
        result  = v_statisticsResetField(v_statistics(cs), fieldName);
    } else {
        v_maxValueReset(&(cs->maxConnectedClients));
        v_maxValueReset(&(cs->maxClientThreads));
        cs->requestsHandled = 0;
        cs->connectedClients = 0;
        cs->clientThreads = 0;
        result = TRUE;
    }
    return result;
}

void v_cmsoapStatisticsFree(v_cmsoapStatistics cs)
{
    assert(cs != NULL);
    assert(C_TYPECHECK(cs, v_cmsoapStatistics));

    v_cmsoapStatisticsDeinit(cs);
    c_free(cs);
}


