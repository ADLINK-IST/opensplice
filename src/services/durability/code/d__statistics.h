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
#ifndef D__STATISTICS_H_
#define D__STATISTICS_H_

#include "d__types.h"
#include "v_durabilityStatistics.h"

#if defined (__cplusplus)
extern "C" {
#endif


C_CLASS(d_aligneeStatistics);

C_STRUCT(d_aligneeStatistics){
    c_ulong aligneeRequestsWaiting;
    c_ulong aligneeRequestsSentDif;
    c_long  aligneeRequestsOpenDif;
    c_ulong aligneeSamplesTotalDif;
    c_ulong aligneeSamplesRegisterDif;
    c_ulong aligneeSamplesWriteDif;
    c_ulong aligneeSamplesDisposeDif;
    c_ulong aligneeSamplesWriteDisposeDif;
    c_ulong aligneeSamplesUnregisterDif;
    c_ulong aligneeTotalSizeDif;
};
    
#define d_aligneeStatistics(s) ((d_aligneeStatistics)(s))

d_aligneeStatistics         d_aligneeStatisticsNew      ();

void                        d_aligneeStatisticsFree     (d_aligneeStatistics s);

C_CLASS(d_alignerStatistics);
    
C_STRUCT(d_alignerStatistics){
    c_ulong alignerRequestsReceivedDif;
    c_ulong alignerRequestsIgnoredDif;
    c_ulong alignerRequestsAnsweredDif;
    c_long  alignerRequestsOpenDif;
    c_ulong alignerRequestsCombinedDif;
    c_long  alignerRequestsCombinedOpenDif;
    c_ulong alignerRequestsCombinedAnsweredDif;
    c_ulong alignerSamplesTotalDif;
    c_ulong alignerSamplesRegisterDif;
    c_ulong alignerSamplesWriteDif;
    c_ulong alignerSamplesDisposeDif;
    c_ulong alignerSamplesWriteDisposeDif;
    c_ulong alignerSamplesUnregisterDif;
    c_ulong alignerTotalSizeDif;
};

#define d_alignerStatistics(s) ((d_alignerStatistics)(s))
    
d_alignerStatistics       d_alignerStatisticsNew      ();

void                      d_alignerStatisticsFree     (d_alignerStatistics s);

typedef enum d_adminStatisticsInfoKind {
    D_ADMIN_STATISTICS_FELLOW,
    D_ADMIN_STATISTICS_GROUP
}d_adminStatisticsInfoKind;
    
C_CLASS(d_adminStatisticsInfo);

C_STRUCT(d_adminStatisticsInfo){
    d_adminStatisticsInfoKind kind;
    c_long fellowsKnownDif;
    c_long fellowsApprovedDif;
    c_long fellowsIncompatibleStateDif;
    c_long fellowsIncompatibleDataModelDif;
    
    c_long groupsKnownVolatileDif;
    c_long groupsKnownTransientDif;
    c_long groupsKnownPersistentDif;

    c_long groupsCompleteVolatileDif;
    c_long groupsCompleteTransientDif;
    c_long groupsCompletePersistentDif;

    c_long groupsIncompleteVolatileDif;
    c_long groupsIncompleteTransientDif;
    c_long groupsIncompletePersistentDif;

    c_long groupsIgnoredVolatileDif;
    c_long groupsIgnoredTransientDif;
    c_long groupsIgnoredPersistentDif;
};    

#define d_adminStatisticsInfo(s) ((d_adminStatisticsInfo)(s))
    
d_adminStatisticsInfo   d_adminStatisticsInfoNew                ();

void                    d_adminStatisticsInfoFree               (d_adminStatisticsInfo info);

void                    d_statisticsUpdateAlignee               (v_durabilityStatistics s,
                                                                 c_voidp args);

void                    d_statisticsUpdateAligner               (v_durabilityStatistics ds,
                                                                 c_voidp args);

void                    d_statisticsUpdateAdmin                 (v_durabilityStatistics statistics,
                                                                 c_voidp args);    

void                    d_statisticsUpdateConfiguration         (v_durabilityStatistics statistics,
                                                                 c_voidp args);

void                    d_statisticsUpdateGroupsToCreate        (v_durabilityStatistics ds,
                                                                 c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /*D__STATISTICS_H_*/
