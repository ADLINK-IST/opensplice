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
#include "v_durabilityStatistics.h"
#include "v_maxValue.h"

v_durabilityStatistics v_durabilityStatisticsNew(v_kernel k)
{
    v_durabilityStatistics ds;
    c_type durabilityStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    durabilityStatisticsType = c_resolve(c_getBase(k), "kernelModule::v_durabilityStatistics");

    ds = v_durabilityStatistics(v_new(k, durabilityStatisticsType));
    v_durabilityStatisticsInit(ds);
    return ds;
}

void v_durabilityStatisticsInit(v_durabilityStatistics ds)
{
    assert(ds != NULL);
    assert(C_TYPECHECK(ds, v_durabilityStatistics));

    v_statisticsInit(v_statistics(ds));
    
    ds->persistentSamplesWritten            = 0;
    
    v_maxValueInit(&ds->fellowsKnownMax);
    ds->fellowsKnown                        = 0;
    ds->fellowsApproved                     = 0;
    ds->fellowsIncompatibleState            = 0;
    ds->fellowsIncompatibleDataModel        = 0;
    
    ds->nameSpacesKnown                     = 0;
    ds->nameSpacesMaster                    = 0;
    ds->nameSpacesSlave                     = 0;

    ds->groupsToCreateTotal                 = 0;
    ds->groupsToCreateVolatile              = 0;
    ds->groupsToCreateTransient             = 0;
    ds->groupsToCreatePersistent            = 0;
    
    ds->groupsKnownTotal                    = 0;
    ds->groupsKnownVolatile                 = 0;
    ds->groupsKnownTransient                = 0;
    ds->groupsKnownPersistent               = 0;

    ds->groupsCompleteTotal                 = 0;
    ds->groupsCompleteVolatile              = 0;
    ds->groupsCompleteTransient             = 0;
    ds->groupsCompletePersistent            = 0;

    ds->groupsIncompleteTotal               = 0;
    ds->groupsIncompleteVolatile            = 0;
    ds->groupsIncompleteTransient           = 0;
    ds->groupsIncompletePersistent          = 0;

    ds->groupsIgnoredTotal                  = 0;
    ds->groupsIgnoredVolatile               = 0;
    ds->groupsIgnoredTransient              = 0;
    ds->groupsIgnoredPersistent             = 0;
    
    ds->alignerRequestsReceived             = 0;
    ds->alignerRequestsIgnored              = 0;
    ds->alignerRequestsAnswered             = 0;
    ds->alignerRequestsOpen                 = 0;
    v_maxValueInit(&ds->alignerRequestsOpenMax);
    ds->alignerRequestsCombined             = 0;
    ds->alignerRequestsCombinedOpen         = 0;
    v_maxValueInit(&ds->alignerRequestsCombinedOpenMax);
    ds->alignerRequestsCombinedAnswered     = 0;
    
    ds->aligneeRequestsSent                 = 0;
    v_maxValueInit(&ds->aligneeRequestsOpenMax);
    ds->aligneeRequestsOpen                 = 0;
    ds->aligneeRequestsWaiting              = 0;
    v_maxValueInit(&ds->aligneeRequestsWaitingMax);
    
    ds->aligneeSamplesTotal                 = 0;
    ds->aligneeSamplesRegister              = 0;
    ds->aligneeSamplesWrite                 = 0;
    ds->aligneeSamplesDispose               = 0;
    ds->aligneeSamplesWriteDispose          = 0;
    ds->aligneeSamplesUnregister            = 0;
    
    ds->alignerSamplesTotal                 = 0;
    ds->alignerSamplesRegister              = 0;
    ds->alignerSamplesWrite                 = 0;
    ds->alignerSamplesDispose               = 0;
    ds->alignerSamplesWriteDispose          = 0;
    ds->alignerSamplesUnregister            = 0;
    
    ds->aligneeTotalSize                    = 0;
    ds->alignerTotalSize                    = 0;
    
    return;
}

void v_durabilityStatisticsDeinit(v_durabilityStatistics ds)
{
    OS_UNUSED_ARG(ds);
    assert(ds!=NULL);
    assert(C_TYPECHECK(ds, v_durabilityStatistics));
}

c_bool v_durabilityStatisticsReset(v_durabilityStatistics ds, const c_char* fieldName)
{
    c_bool result;

    assert(ds != NULL);
    assert(C_TYPECHECK(ds, v_durabilityStatistics));

    result = FALSE;

    if (fieldName != NULL) {
        result  = v_statisticsResetField(v_statistics(ds), fieldName);
    } else {
        v_statisticsULongResetInternal(v_durability, persistentSamplesWritten, ds);
        
        v_maxValueReset(&ds->fellowsKnownMax);
        v_statisticsULongResetInternal(v_durability, fellowsKnown, ds);
        v_maxValueSetValue(&ds->fellowsKnownMax, ds->fellowsKnown);
        v_statisticsULongResetInternal(v_durability, fellowsApproved, ds);
        v_statisticsULongResetInternal(v_durability, fellowsIncompatibleState, ds);
        v_statisticsULongResetInternal(v_durability, fellowsIncompatibleDataModel, ds);
        
        v_statisticsULongResetInternal(v_durability, nameSpacesKnown, ds);
        v_statisticsULongResetInternal(v_durability, nameSpacesMaster, ds);
        v_statisticsULongResetInternal(v_durability, nameSpacesSlave, ds);
        
        v_statisticsULongResetInternal(v_durability, groupsToCreateTotal, ds);
        v_statisticsULongResetInternal(v_durability, groupsToCreateVolatile, ds);
        v_statisticsULongResetInternal(v_durability, groupsToCreateTransient, ds);
        v_statisticsULongResetInternal(v_durability, groupsToCreatePersistent, ds);
        
        v_statisticsULongResetInternal(v_durability, groupsKnownTotal, ds);
        v_statisticsULongResetInternal(v_durability, groupsKnownVolatile, ds);
        v_statisticsULongResetInternal(v_durability, groupsKnownTransient, ds);
        v_statisticsULongResetInternal(v_durability, groupsKnownPersistent, ds);

        v_statisticsULongResetInternal(v_durability, groupsCompleteTotal, ds);
        v_statisticsULongResetInternal(v_durability, groupsCompleteVolatile, ds);
        v_statisticsULongResetInternal(v_durability, groupsCompleteTransient, ds);
        v_statisticsULongResetInternal(v_durability, groupsCompletePersistent, ds);

        v_statisticsULongResetInternal(v_durability, groupsIncompleteTotal, ds);
        v_statisticsULongResetInternal(v_durability, groupsIncompleteVolatile, ds);
        v_statisticsULongResetInternal(v_durability, groupsIncompleteTransient, ds);
        v_statisticsULongResetInternal(v_durability, groupsIncompletePersistent, ds);

        v_statisticsULongResetInternal(v_durability, groupsIgnoredTotal, ds);
        v_statisticsULongResetInternal(v_durability, groupsIgnoredVolatile, ds);
        v_statisticsULongResetInternal(v_durability, groupsIgnoredTransient, ds);
        v_statisticsULongResetInternal(v_durability, groupsIgnoredPersistent, ds);
        
        v_statisticsULongResetInternal(v_durability, alignerRequestsReceived, ds);
        v_statisticsULongResetInternal(v_durability, alignerRequestsIgnored, ds);
        v_statisticsULongResetInternal(v_durability, alignerRequestsAnswered, ds);
        v_statisticsULongResetInternal(v_durability, alignerRequestsOpen, ds);
        v_statisticsULongResetInternal(v_durability, alignerRequestsCombined, ds);
        v_statisticsULongResetInternal(v_durability, alignerRequestsCombinedOpen, ds);
        v_statisticsULongResetInternal(v_durability, alignerRequestsCombinedAnswered, ds);
        v_maxValueReset(&ds->alignerRequestsOpenMax);
        v_maxValueSetValue(&ds->alignerRequestsOpenMax, ds->alignerRequestsOpen);
        
        v_maxValueReset(&ds->alignerRequestsCombinedOpenMax);
        v_maxValueSetValue(&ds->alignerRequestsCombinedOpenMax, ds->alignerRequestsCombinedOpen);
        
        v_statisticsULongResetInternal(v_durability, aligneeRequestsSent, ds);
        v_statisticsULongResetInternal(v_durability, aligneeRequestsOpen, ds);
        v_maxValueReset(&ds->aligneeRequestsOpenMax);
        v_maxValueSetValue(&ds->aligneeRequestsOpenMax, ds->aligneeRequestsOpen);
        v_statisticsULongResetInternal(v_durability, aligneeRequestsWaiting, ds);
        v_maxValueReset(&ds->aligneeRequestsWaitingMax);
        v_maxValueSetValue(&ds->aligneeRequestsWaitingMax, ds->aligneeRequestsWaiting);
        
        v_statisticsULongResetInternal(v_durability, aligneeSamplesTotal, ds);
        v_statisticsULongResetInternal(v_durability, aligneeSamplesRegister, ds);
        v_statisticsULongResetInternal(v_durability, aligneeSamplesWrite, ds);
        v_statisticsULongResetInternal(v_durability, aligneeSamplesDispose, ds);
        v_statisticsULongResetInternal(v_durability, aligneeSamplesWriteDispose, ds);
        v_statisticsULongResetInternal(v_durability, aligneeSamplesUnregister, ds);
        
        v_statisticsULongResetInternal(v_durability, alignerSamplesTotal, ds);
        v_statisticsULongResetInternal(v_durability, alignerSamplesRegister, ds);
        v_statisticsULongResetInternal(v_durability, alignerSamplesWrite, ds);
        v_statisticsULongResetInternal(v_durability, alignerSamplesDispose, ds);
        v_statisticsULongResetInternal(v_durability, alignerSamplesWriteDispose, ds);
        v_statisticsULongResetInternal(v_durability, alignerSamplesUnregister, ds);
        
        v_statisticsULongResetInternal(v_durability, aligneeTotalSize, ds);
        v_statisticsULongResetInternal(v_durability, alignerTotalSize, ds);
        
        result = TRUE;
    }
    return result;
}

void v_durabilityStatisticsFree(v_durabilityStatistics ds)
{
    assert(ds != NULL);
    assert(C_TYPECHECK(ds, v_durabilityStatistics));

    v_durabilityStatisticsDeinit(ds);
    c_free(ds);
}


