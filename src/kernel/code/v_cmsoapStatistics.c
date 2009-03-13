
#include "v_statistics.h"
#include "v__statistics.h"
#include "v_cmsoapStatistics.h"
#include "v_maxValue.h"

static c_type cmsoapStatisticsType = NULL;

v_cmsoapStatistics v_cmsoapStatisticsNew(v_kernel k)
{
    v_cmsoapStatistics cs;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    if (cmsoapStatisticsType == NULL) {
        cmsoapStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_cmsoapStatistics");
    }
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


