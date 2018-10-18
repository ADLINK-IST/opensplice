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
#ifndef D__STATISTICS_H
#define D__STATISTICS_H

#include "d__types.h"
#include "v_durabilityStatistics.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_aligneeStatistics validity.
 * Because d_aligneeStatistics is a concrete class typechecking is required.
 */
#define             d_aligneeStatisticsIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ALIGNEE_STATS)

/**
 * \brief The d_aligneeStatistics cast method.
 *
 * This method casts an object to a d_aligneeStatistics object.
 */
#define d_aligneeStatistics(_this) ((d_aligneeStatistics)(_this))

C_STRUCT(d_aligneeStatistics){
    C_EXTENDS(d_object);
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

d_aligneeStatistics         d_aligneeStatisticsNew      ();

void                        d_aligneeStatisticsDeinit   (d_aligneeStatistics stats);

void                        d_aligneeStatisticsFree     (d_aligneeStatistics stats);

/**
 * Macro that checks the d_alignerStatistics validity.
 * Because d_alignerStatistics is a concrete class typechecking is required.
 */
#define             d_alignerStatisticsIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ALIGNER_STATS)

/**
 * \brief The d_alignerStatistics cast macro.
 *
 * This macro casts an object to a d_alignerStatistics object.
 */
#define d_alignerStatistics(_this) ((d_alignerStatistics)(_this))

C_STRUCT(d_alignerStatistics){
    C_EXTENDS(d_object);
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


d_alignerStatistics       d_alignerStatisticsNew      ();

void                      d_alignerStatisticsDeinit   (d_alignerStatistics stats);

void                      d_alignerStatisticsFree     (d_alignerStatistics stats);


typedef enum d_adminStatisticsInfoKind {
    D_ADMIN_STATISTICS_FELLOW,
    D_ADMIN_STATISTICS_GROUP
} d_adminStatisticsInfoKind;

/**
 * Macro that checks the d_adminStatisticsInfo validity.
 * Because d_adminStatisticsInfo is a concrete class typechecking is required.
 */
#define             d_adminStatisticsInfoIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ADMIN_STATS_INFO)

/**
 * \brief The d_adminStatisticsInfo cast macro.
 *
 * This macro casts an object to a d_adminStatisticsInfo object.
 */
#define d_adminStatisticsInfo(_this) ((d_adminStatisticsInfo)(_this))

C_STRUCT(d_adminStatisticsInfo) {
    C_EXTENDS(d_object);
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

d_adminStatisticsInfo   d_adminStatisticsInfoNew                ();

void                    d_adminStatisticsInfoDeinit             (d_adminStatisticsInfo info);

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

#endif /* D__STATISTICS_H */
