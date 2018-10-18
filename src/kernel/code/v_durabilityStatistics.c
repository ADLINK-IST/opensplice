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

#include "v_statistics.h"
#include "v_durabilityStatistics.h"
#include "v_maxValue.h"

v_durabilityStatistics v_durabilityStatisticsNew(v_kernel k)
{
    v_durabilityStatistics ds;
    c_type durabilityStatisticsType;

    assert(k != NULL);
    assert(C_TYPECHECK(k, v_kernel));

    /* not necessary to cache this type since it is looked up only once per process */
    durabilityStatisticsType = c_resolve(c_getBase(k), "kernelModuleI::v_durabilityStatistics");

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

void v_durabilityStatisticsFree(v_durabilityStatistics ds)
{
    assert(ds != NULL);
    assert(C_TYPECHECK(ds, v_durabilityStatistics));

    v_durabilityStatisticsDeinit(ds);
    c_free(ds);
}


