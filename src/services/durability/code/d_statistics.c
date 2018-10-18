/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__statistics.h"
#include "d__types.h"
#include "d__admin.h"
#include "d__nameSpace.h"
#include "d__groupCreationQueue.h"
#include "v_maxValue.h"
#include "os_heap.h"

d_aligneeStatistics
d_aligneeStatisticsNew()
{
    d_aligneeStatistics s;

    /* Allocate aligneeStatistics object */
    s = d_aligneeStatistics(os_malloc(C_SIZEOF(d_aligneeStatistics)));
    if (s) {
        /* Call super-init */
        d_objectInit(d_object(s), D_ALIGNEE_STATS,
                     (d_objectDeinitFunc)d_aligneeStatisticsDeinit);
        /* Inititialize aligneeStatistics */
        s->aligneeRequestsWaiting           = 0;
        s->aligneeRequestsSentDif           = 0;
        s->aligneeRequestsOpenDif           = 0;
        s->aligneeSamplesTotalDif           = 0;
        s->aligneeSamplesRegisterDif        = 0;
        s->aligneeSamplesWriteDif           = 0;
        s->aligneeSamplesDisposeDif         = 0;
        s->aligneeSamplesWriteDisposeDif    = 0;
        s->aligneeSamplesUnregisterDif      = 0;
        s->aligneeTotalSizeDif              = 0;
    }
    return s;
}


void
d_aligneeStatisticsDeinit(
    d_aligneeStatistics stats)
{
    assert(d_aligneeStatisticsIsValid(stats));

    /* Nothing to deallocate */
    /* Call super-deinit */
    d_objectDeinit(d_object(stats));
}


void
d_aligneeStatisticsFree(
    d_aligneeStatistics stats)
{

    assert(d_aligneeStatisticsIsValid(stats));

    d_objectFree(d_object(stats));
}

void
d_statisticsUpdateAlignee(
    v_durabilityStatistics s,
    c_voidp args)
{
    d_aligneeStatistics as = d_aligneeStatistics(args);

    if(s && args){
        s->aligneeRequestsWaiting = as->aligneeRequestsWaiting;
        v_maxValueSetValue(&s->aligneeRequestsWaitingMax, s->aligneeRequestsWaiting);

        if(as->aligneeRequestsSentDif != 0){
            s->aligneeRequestsSent += as->aligneeRequestsSentDif;
        }
        if(as->aligneeRequestsOpenDif != 0){
            if (as->aligneeRequestsOpenDif >= 0) {
                s->aligneeRequestsOpen += (c_ulong) as->aligneeRequestsOpenDif;
            } else {
                s->aligneeRequestsOpen -= (c_ulong) -as->aligneeRequestsOpenDif;
            }
            v_maxValueSetValue(&s->aligneeRequestsOpenMax, s->aligneeRequestsOpen);
        }
        if(as->aligneeSamplesTotalDif != 0){
            s->aligneeSamplesTotal += as->aligneeSamplesTotalDif;
        }
        if(as->aligneeSamplesRegisterDif != 0){
            s->aligneeSamplesRegister += as->aligneeSamplesRegisterDif;
        }
        if(as->aligneeSamplesWriteDif != 0){
            s->aligneeSamplesWrite += as->aligneeSamplesWriteDif;
        }
        if(as->aligneeSamplesDisposeDif != 0){
            s->aligneeSamplesDispose += as->aligneeSamplesDisposeDif;
        }
        if(as->aligneeSamplesWriteDisposeDif != 0){
            s->aligneeSamplesWriteDispose += as->aligneeSamplesWriteDisposeDif;
        }
        if(as->aligneeSamplesUnregisterDif != 0){
            s->aligneeSamplesUnregister += as->aligneeSamplesUnregisterDif;
        }
        if(as->aligneeTotalSizeDif != 0){
            s->aligneeTotalSize += as->aligneeTotalSizeDif;
        }
    }
    return;
}


d_alignerStatistics
d_alignerStatisticsNew()
{
    d_alignerStatistics s;
    /* Allocate alignerStatistics object */
    s = os_malloc(C_SIZEOF(d_alignerStatistics));
    /* Call super-init */
    d_objectInit(d_object(s), D_ALIGNER_STATS,
                 (d_objectDeinitFunc)d_alignerStatisticsDeinit);
    /* Initialize alignerStatistics */
    s->alignerRequestsReceivedDif           = 0;
    s->alignerRequestsIgnoredDif            = 0;
    s->alignerRequestsAnsweredDif           = 0;
    s->alignerRequestsOpenDif               = 0;
    s->alignerRequestsCombinedDif           = 0;
    s->alignerRequestsCombinedOpenDif       = 0;
    s->alignerRequestsCombinedAnsweredDif   = 0;
    s->alignerSamplesTotalDif               = 0;
    s->alignerSamplesRegisterDif            = 0;
    s->alignerSamplesWriteDif               = 0;
    s->alignerSamplesDisposeDif             = 0;
    s->alignerSamplesWriteDisposeDif        = 0;
    s->alignerSamplesUnregisterDif          = 0;
    s->alignerTotalSizeDif                  = 0;
    return s;
}


void
d_alignerStatisticsDeinit(
    d_alignerStatistics stats)
{
    assert(d_alignerStatisticsIsValid(stats));

    /* Nothing to deallocate */
    /* Call super-deinit */
    d_objectDeinit(d_object(stats));
}


void
d_alignerStatisticsFree(
    d_alignerStatistics stats)
{
    assert(d_alignerStatisticsIsValid(stats));

    d_objectFree(d_object(stats));
}

static c_ulong add_dif_to_total (c_ulong total, c_long dif)
{
    if (dif >= 0) {
        return total + (c_ulong) dif;
    } else {
        return total - (c_ulong) -dif;
    }
}

void
d_statisticsUpdateAligner(
    v_durabilityStatistics ds,
    c_voidp args)
{
    d_alignerStatistics s = d_alignerStatistics(args);

    if(s->alignerRequestsReceivedDif != 0){
        ds->alignerRequestsReceived += s->alignerRequestsReceivedDif;
    }
    if(s->alignerRequestsIgnoredDif != 0){
        ds->alignerRequestsIgnored += s->alignerRequestsIgnoredDif;
    }
    if(s->alignerRequestsAnsweredDif != 0){
        ds->alignerRequestsAnswered += s->alignerRequestsAnsweredDif;
    }
    if(s->alignerRequestsOpenDif != 0){
        ds->alignerRequestsOpen = add_dif_to_total(ds->alignerRequestsOpen, s->alignerRequestsOpenDif);
        v_maxValueSetValue(&ds->alignerRequestsOpenMax, ds->alignerRequestsOpen);
    }
    if(s->alignerRequestsCombinedDif != 0){
        ds->alignerRequestsCombined += s->alignerRequestsCombinedDif;
    }
    if(s->alignerRequestsCombinedOpenDif != 0){
        ds->alignerRequestsCombinedOpen = add_dif_to_total(ds->alignerRequestsCombinedOpen, s->alignerRequestsCombinedOpenDif);
        v_maxValueSetValue(&ds->alignerRequestsCombinedOpenMax, ds->alignerRequestsCombinedOpen);
    }
    if(s->alignerRequestsCombinedAnsweredDif != 0){
        ds->alignerRequestsCombinedAnswered += s->alignerRequestsCombinedAnsweredDif;
    }
    if(s->alignerSamplesTotalDif != 0){
        ds->alignerSamplesTotal += s->alignerSamplesTotalDif;
    }
    if(s->alignerSamplesRegisterDif != 0){
        ds->alignerSamplesRegister += s->alignerSamplesRegisterDif;
    }
    if(s->alignerSamplesWriteDif != 0){
        ds->alignerSamplesWrite += s->alignerSamplesWriteDif;
    }
    if(s->alignerSamplesDisposeDif != 0){
        ds->alignerSamplesDispose += s->alignerSamplesDisposeDif;
    }
    if(s->alignerSamplesWriteDisposeDif != 0){
        ds->alignerSamplesWriteDispose += s->alignerSamplesWriteDisposeDif;
    }
    if(s->alignerSamplesUnregisterDif != 0){
        ds->alignerSamplesUnregister += s->alignerSamplesUnregisterDif;
    }
    if(s->alignerTotalSizeDif != 0){
        ds->alignerTotalSize += s->alignerTotalSizeDif;
    }
    return;
}

void
d_statisticsUpdateAdmin(
    v_durabilityStatistics statistics,
    c_voidp args)
{
    d_adminStatisticsInfo info = d_adminStatisticsInfo(args);

    if(info && statistics){
        if(info->kind == D_ADMIN_STATISTICS_FELLOW){
            if(info->fellowsKnownDif != 0){
                statistics->fellowsKnown = add_dif_to_total(statistics->fellowsKnown, info->fellowsKnownDif);
                v_maxValueSetValue(&statistics->fellowsKnownMax, statistics->fellowsKnown);
            }
            if(info->fellowsApprovedDif != 0){
                statistics->fellowsApproved = add_dif_to_total(statistics->fellowsApproved, info->fellowsApprovedDif);
            }
            if(info->fellowsIncompatibleStateDif != 0){
                statistics->fellowsIncompatibleState = add_dif_to_total(statistics->fellowsIncompatibleState, info->fellowsIncompatibleStateDif);
            }
            if(info->fellowsIncompatibleDataModelDif != 0){
                statistics->fellowsIncompatibleDataModel = add_dif_to_total(statistics->fellowsIncompatibleDataModel, info->fellowsIncompatibleDataModelDif);
            }
        } else if(info->kind == D_ADMIN_STATISTICS_GROUP){
            if(info->groupsKnownVolatileDif != 0){
                statistics->groupsKnownVolatile = add_dif_to_total(statistics->groupsKnownVolatile, info->groupsKnownVolatileDif);
                statistics->groupsKnownTotal = add_dif_to_total(statistics->groupsKnownTotal, info->groupsKnownVolatileDif);
            }
            if(info->groupsKnownTransientDif != 0){
                statistics->groupsKnownTransient = add_dif_to_total(statistics->groupsKnownTransient, info->groupsKnownTransientDif);
                statistics->groupsKnownTotal = add_dif_to_total(statistics->groupsKnownTotal, info->groupsKnownTransientDif);
            }
            if(info->groupsKnownPersistentDif != 0){
                statistics->groupsKnownPersistent = add_dif_to_total(statistics->groupsKnownPersistent, info->groupsKnownPersistentDif);
                statistics->groupsKnownTotal = add_dif_to_total(statistics->groupsKnownTotal, info->groupsKnownPersistentDif);
            }
            if(info->groupsCompleteVolatileDif != 0){
                statistics->groupsCompleteVolatile = add_dif_to_total(statistics->groupsCompleteVolatile, info->groupsCompleteVolatileDif);
                statistics->groupsCompleteTotal = add_dif_to_total(statistics->groupsCompleteTotal, info->groupsCompleteVolatileDif);
            }
            if(info->groupsCompleteTransientDif != 0){
                statistics->groupsCompleteTransient = add_dif_to_total(statistics->groupsCompleteTransient, info->groupsCompleteTransientDif);
                statistics->groupsCompleteTotal = add_dif_to_total(statistics->groupsCompleteTotal, info->groupsCompleteTransientDif);
            }
            if(info->groupsCompletePersistentDif != 0){
                statistics->groupsCompletePersistent = add_dif_to_total(statistics->groupsCompletePersistent, info->groupsCompletePersistentDif);
                statistics->groupsCompleteTotal = add_dif_to_total(statistics->groupsCompleteTotal, info->groupsCompletePersistentDif);
            }
            if(info->groupsIncompleteVolatileDif != 0){
                statistics->groupsIncompleteVolatile = add_dif_to_total(statistics->groupsIncompleteVolatile, info->groupsIncompleteVolatileDif);
                statistics->groupsIncompleteTotal = add_dif_to_total(statistics->groupsIncompleteTotal, info->groupsIncompleteVolatileDif);
            }
            if(info->groupsIncompleteTransientDif != 0){
                statistics->groupsIncompleteTransient = add_dif_to_total(statistics->groupsIncompleteTransient, info->groupsIncompleteTransientDif);
                statistics->groupsIncompleteTotal = add_dif_to_total(statistics->groupsIncompleteTotal, info->groupsIncompleteTransientDif);
            }
            if(info->groupsIncompletePersistentDif != 0){
                statistics->groupsIncompletePersistent = add_dif_to_total(statistics->groupsIncompletePersistent, info->groupsIncompletePersistentDif);
                statistics->groupsIncompleteTotal = add_dif_to_total(statistics->groupsIncompleteTotal, info->groupsIncompletePersistentDif);
            }
            if(info->groupsIgnoredVolatileDif != 0){
                statistics->groupsIgnoredVolatile = add_dif_to_total(statistics->groupsIgnoredVolatile, info->groupsIgnoredVolatileDif);
                statistics->groupsIgnoredTotal = add_dif_to_total(statistics->groupsIgnoredTotal, info->groupsIgnoredVolatileDif);
            }
            if(info->groupsIgnoredTransientDif != 0){
                statistics->groupsIgnoredTransient = add_dif_to_total(statistics->groupsIgnoredTransient, info->groupsIgnoredTransientDif);
                statistics->groupsIgnoredTotal = add_dif_to_total(statistics->groupsIgnoredTotal, info->groupsIgnoredTransientDif);
            }
            if(info->groupsIgnoredPersistentDif != 0){
                statistics->groupsIgnoredPersistent = add_dif_to_total(statistics->groupsIgnoredPersistent, info->groupsIgnoredPersistentDif);
                statistics->groupsIgnoredTotal = add_dif_to_total(statistics->groupsIgnoredTotal, info->groupsIgnoredPersistentDif);
            }
        }
    }
    return;
}


d_adminStatisticsInfo
d_adminStatisticsInfoNew()
{
    d_adminStatisticsInfo info;
    /* Allocate alignerStatistics object */
    info = os_malloc(C_SIZEOF(d_adminStatisticsInfo));
    /* Call super-init */
    d_objectInit(d_object(info), D_ADMIN_STATS_INFO,
                 (d_objectDeinitFunc)d_adminStatisticsInfoDeinit);
    /* Initialize adminStatisticsInfo */
    info->kind                              = D_ADMIN_STATISTICS_FELLOW;
    info->fellowsKnownDif                   = 0;
    info->fellowsApprovedDif                = 0;
    info->fellowsIncompatibleStateDif       = 0;
    info->fellowsIncompatibleDataModelDif   = 0;
    info->groupsKnownVolatileDif            = 0;
    info->groupsKnownTransientDif           = 0;
    info->groupsKnownPersistentDif          = 0;
    info->groupsCompleteVolatileDif         = 0;
    info->groupsCompleteTransientDif        = 0;
    info->groupsCompletePersistentDif       = 0;
    info->groupsIncompleteVolatileDif       = 0;
    info->groupsIncompleteTransientDif      = 0;
    info->groupsIncompletePersistentDif     = 0;
    info->groupsIgnoredVolatileDif          = 0;
    info->groupsIgnoredTransientDif         = 0;
    info->groupsIgnoredPersistentDif        = 0;
    return info;
}


void
d_adminStatisticsInfoDeinit(
    d_adminStatisticsInfo info)
{
    assert(d_adminStatisticsInfoIsValid(info));

    /* Nothing to deallocate */
    /* Call super-deinit */
    d_objectDeinit(d_object(info));
}


void
d_adminStatisticsInfoFree(
    d_adminStatisticsInfo info)
{
    assert(d_adminStatisticsInfoIsValid(info));

    d_objectFree(d_object(info));
}


void
d_statisticsUpdateConfiguration(
    v_durabilityStatistics ds,
    c_voidp args)
{
    c_ulong i;
    c_ulong master, slave;
    d_nameSpace ns;
    d_admin admin;
    c_iter nameSpaces;
    c_ulong nameSpaceCount;

    admin      = d_admin(args);
    master     = 0;
    slave      = 0;

    /* Collect namespaces from administration */
    nameSpaces = d_adminNameSpaceCollect(admin);
    nameSpaceCount = c_iterLength(nameSpaces);

    for(i=0; i<nameSpaceCount; i++){
        ns = d_nameSpace(c_iterObject(nameSpaces, i));

        if(d_nameSpaceMasterIsMe(ns, admin)){
            master++;
        } else {
            slave++;
        }
    }
    ds->nameSpacesKnown = nameSpaceCount;
    ds->nameSpacesMaster = master;
    ds->nameSpacesSlave  = slave;

    /* Free collected namespaces */
    d_adminNameSpaceCollectFree(admin, nameSpaces);

    return;
}

void
d_statisticsUpdateGroupsToCreate(
    v_durabilityStatistics ds,
    c_voidp args)
{
    c_ulong total;
    d_groupCreationQueue queue = d_groupCreationQueue(args);

    ds->groupsToCreateVolatile   = queue->groupsToCreateVolatile;
    total                        = queue->groupsToCreateVolatile;
    ds->groupsToCreateTransient  = queue->groupsToCreateTransient;
    total                       += queue->groupsToCreateTransient;
    ds->groupsToCreatePersistent = queue->groupsToCreatePersistent;
    total                       += queue->groupsToCreatePersistent;
    ds->groupsToCreateTotal      = total;

    return;
}
