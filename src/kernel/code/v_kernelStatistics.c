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
#include "v_kernelStatistics.h"
#include "v_maxValue.h"

v_kernelStatistics v_kernelStatisticsNew(v_kernel k)
{
    v_kernelStatistics ks;
    c_type kernelStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k,v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    kernelStatisticsType = c_resolve(c_getBase((c_object)k), "kernelModule::v_kernelStatistics");

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

c_bool v_kernelStatisticsReset(v_kernelStatistics ks, const c_char* fieldName)
{
    c_bool result;

    assert(ks!=NULL);
    assert(C_TYPECHECK(ks, v_kernelStatistics));

    result = FALSE;

    if (fieldName != NULL) {
        result = v_statisticsResetField(v_statistics(ks), fieldName);
    } else {
        v_maxValueReset(&ks->maxShmUsed);
        v_maxValueReset(&ks->maxShmGarbage);
        v_maxValueReset(&ks->maxShmClaims);
        v_statisticsULongResetInternal(v_kernel, shmUsed, ks); 
        v_statisticsULongResetInternal(v_kernel, shmClaims, ks); 
        v_statisticsULongResetInternal(v_kernel, shmClaimFails, ks); 
        v_statisticsULongResetInternal(v_kernel, shmGarbage, ks); 
        result = TRUE;
    }
    return result;
}

void v_kernelStatisticsFree(v_kernelStatistics ks)
{
    assert(ks != NULL);
    assert(C_TYPECHECK(ks, v_kernelStatistics));

    v_kernelStatisticsDeinit(ks);
    c_free(ks);
}


