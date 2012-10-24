/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V_STATISTICSHELPERS_H
#define V_STATISTICSHELPERS_H

#include "v_entity.h"                  /* For the cast v_entity() */
#include "v_readerStatistics.h"        /* For the cast v_readerStatistics() */
#include "v_queryStatistics.h"         /* For the cast v_queryStatistics() */
#include "v_writerStatistics.h"        /* For the cast v_writerStatistics() */
#include "v_networkQueueStatistics.h"  /* For the cast v_networkQueueStatistics() */
#include "v_networkReaderStatistics.h" /* For the cast v_networkReaderStatistics() */
#include "v_networkingStatistics.h"    /* For the cast v_networkingStatistics() */
#include "v_cmsoapStatistics.h"        /* For the cast v_cmsoapStatistics() */
#include "v_durabilityStatistics.h"    /* For the cast v_durabilityStatistics()*/
#include "v_kernelStatistics.h"        /* For the cast v_kernelStatistics() */
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* Generic unit definitions */
#define unit_microSeconds        "microseconds"
#define unit_bytes               "bytes"
#define unit_samples             "samples"
#define unit_instances           "instances"
#define unit_samplesPerInstance  "samples/instance"
#define unit_calls               "calls"


/* READER */
#define unit_v_reader_maxSampleSize                        unit_bytes
#define unit_v_reader_maxSamplesPerInstance                unit_samplesPerInstance
#define unit_v_reader_maxNumberOfSamples                    unit_samples
#define unit_v_reader_maxNumberOfInstances                 unit_instances
#define unit_v_reader_numberOfSamples                      unit_samples
#define unit_v_reader_numberOfInstances                    unit_instances
#define unit_v_reader_readLatency                          unit_microSeconds
#define unit_v_reader_transportLatency                     unit_microSeconds
#define unit_v_reader_numberOfInstancesWithStatusNew       unit_instances
#define unit_v_reader_numberOfInstancesWithStatusAlive     unit_instances
#define unit_v_reader_numberOfInstancesWithStatusDisposed  unit_instances
#define unit_v_reader_numberOfInstancesWithStatusNoWriters unit_instances
#define unit_v_reader_numberOfSamplesWithStatusRead        unit_samples
#define unit_v_reader_numberOfSamplesExpired               unit_samples
#define unit_v_reader_numberOfSamplesPurgedByDispose       unit_samples
#define unit_v_reader_numberOfSamplesPurgedByNoWriters     unit_samples
#define unit_v_reader_numberOfSamplesDiscarded             unit_samples
#define unit_v_reader_numberOfSamplesRead                  unit_samples
#define unit_v_reader_numberOfSamplesTaken                 unit_samples
#define unit_v_reader_numberOfReads                        unit_calls
#define unit_v_reader_numberOfInstanceReads                unit_calls
#define unit_v_reader_numberOfNextInstanceReads            unit_calls
#define unit_v_reader_numberOfInstanceLookups              unit_calls
#define unit_v_reader_numberOfTakes                        unit_calls
#define unit_v_reader_numberOfInstanceTakes                unit_calls
#define unit_v_reader_numberOfNextInstanceTakes            unit_calls

#define resettable_v_reader_numberOfSamples                         (FALSE)
#define resettable_v_reader_numberOfInstances                       (FALSE)
#define resettable_v_reader_numberOfInstancesWithStatusNew          (FALSE)
#define resettable_v_reader_numberOfInstancesWithStatusAlive        (FALSE)
#define resettable_v_reader_numberOfInstancesWithStatusDisposed     (FALSE)
#define resettable_v_reader_numberOfInstancesWithStatusNoWriters    (FALSE)
#define resettable_v_reader_numberOfSamplesWithStatusRead           (FALSE)
#define resettable_v_reader_numberOfSamplesExpired                  (TRUE)
#define resettable_v_reader_numberOfSamplesPurgedByDispose          (TRUE)
#define resettable_v_reader_numberOfSamplesPurgedByNoWriters        (TRUE)
#define resettable_v_reader_numberOfSamplesArrived                  (TRUE)
#define resettable_v_reader_numberOfSamplesInserted                 (TRUE)
#define resettable_v_reader_numberOfSamplesDiscarded                (TRUE)
#define resettable_v_reader_numberOfSamplesRead                     (TRUE)
#define resettable_v_reader_numberOfSamplesTaken                    (TRUE)
#define resettable_v_reader_numberOfSamplesRejectedBySamplesLimit   (TRUE)
#define resettable_v_reader_numberOfSamplesRejectedByInstancesLimit (TRUE)
#define resettable_v_reader_numberOfReads                           (TRUE)
#define resettable_v_reader_numberOfInstanceReads                   (TRUE)
#define resettable_v_reader_numberOfNextInstanceReads               (TRUE)
#define resettable_v_reader_numberOfInstanceLookups                 (TRUE)
#define resettable_v_reader_numberOfTakes                           (TRUE)
#define resettable_v_reader_numberOfInstanceTakes                   (TRUE)
#define resettable_v_reader_numberOfNextInstanceTakes               (TRUE)

/* QUERY */
#define unit_v_query_numberOfReads                                  unit_calls
#define unit_v_query_numberOfInstanceReads                          unit_calls
#define unit_v_query_numberOfNextInstanceReads                      unit_calls
#define unit_v_query_numberOfTakes                                  unit_calls
#define unit_v_query_numberOfInstanceTakes                          unit_calls
#define unit_v_query_numberOfNextInstanceTakes                      unit_calls

#define resettable_v_query_numberOfReads                           (TRUE)
#define resettable_v_query_numberOfInstanceReads                   (TRUE)
#define resettable_v_query_numberOfNextInstanceReads               (TRUE)
#define resettable_v_query_numberOfTakes                           (TRUE)
#define resettable_v_query_numberOfInstanceTakes                   (TRUE)
#define resettable_v_query_numberOfNextInstanceTakes               (TRUE)

/* WRITER */
#define unit_v_writer_numberOfWrites                                unit_calls
#define unit_v_writer_numberOfDisposes                              unit_calls
#define unit_v_writer_numberOfRegisters                             unit_calls
#define unit_v_writer_numberOfImplicitRegisters                     unit_calls
#define unit_v_writer_numberOfUnregisters                           unit_calls
#define unit_v_writer_numberOfTimedOutWrites                        unit_calls
#define unit_v_writer_numberOfWritesBlockedBySamplesLimit           unit_calls
#define unit_v_writer_numberOfWritesBlockedByInstanceLimit          unit_calls
#define unit_v_writer_numberOfWritesBlockedBySamplesPerInstanceLimit  unit_calls
#define unit_v_writer_numberOfRetries                               unit_calls
#define unit_v_writer_numberOfInstancesWithStatusAlive              unit_instances
#define unit_v_writer_numberOfInstancesWithStatusDisposed           unit_instances
#define unit_v_writer_numberOfInstancesWithStatusUnregistered       unit_instances
#define unit_v_writer_numberOfSamples                               unit_samples
#define unit_v_writer_maxNumberOfSamplesPerInstance                 unit_samplesPerInstance


#define resettable_v_writer_numberOfWrites                          (TRUE)
#define resettable_v_writer_numberOfDisposes                        (TRUE)
#define resettable_v_writer_numberOfRegisters                       (TRUE)
#define resettable_v_writer_numberOfImplicitRegisters               (TRUE)
#define resettable_v_writer_numberOfUnregisters                     (TRUE)
#define resettable_v_writer_numberOfTimedOutWrites                  (TRUE)
#define resettable_v_writer_numberOfWritesBlockedBySamplesLimit     (TRUE)
#define resettable_v_writer_numberOfWritesBlockedByInstanceLimit    (TRUE)
#define resettable_v_writer_numberOfWritesBlockedBySamplesPerInstanceLimit (TRUE)
#define resettable_v_writer_numberOfRetries                         (TRUE)
#define resettable_v_writer_numberOfInstancesWithStatusAlive        (FALSE)
#define resettable_v_writer_numberOfInstancesWithStatusDisposed     (FALSE)
#define resettable_v_writer_numberOfInstancesWithStatusUnregistered (FALSE)
#define resettable_v_writer_numberOfSamples                         (FALSE)
#define resettable_v_writer_maxNumberOfSamplesPerInstance           (TRUE)

/* KERNEL */
#define resettable_v_kernel_shmUsed                                 (FALSE)
#define resettable_v_kernel_shmGarbage                              (FALSE)
#define resettable_v_kernel_shmClaims                               (TRUE)
#define resettable_v_kernel_shmClaimFails                           (TRUE)

/* DURABILITY */
#define resettable_v_durability_persistentSamplesWritten            (TRUE)
#define resettable_v_durability_fellowsKnownMax                     (TRUE)
#define resettable_v_durability_fellowsKnown                        (FALSE)
#define resettable_v_durability_fellowsApproved                     (FALSE)
#define resettable_v_durability_fellowsIncompatibleState            (TRUE)
#define resettable_v_durability_fellowsIncompatibleDataModel        (TRUE)

#define resettable_v_durability_nameSpacesKnown                     (FALSE)
#define resettable_v_durability_nameSpacesMaster                    (FALSE)
#define resettable_v_durability_nameSpacesSlave                     (FALSE)

#define resettable_v_durability_groupsToCreateTotal                 (FALSE)
#define resettable_v_durability_groupsToCreateVolatile              (FALSE)
#define resettable_v_durability_groupsToCreateTransient             (FALSE)
#define resettable_v_durability_groupsToCreatePersistent            (FALSE)

#define resettable_v_durability_groupsKnownTotal                    (FALSE)
#define resettable_v_durability_groupsKnownVolatile                 (FALSE)
#define resettable_v_durability_groupsKnownTransient                (FALSE)
#define resettable_v_durability_groupsKnownPersistent               (FALSE)

#define resettable_v_durability_groupsCompleteTotal                 (FALSE)
#define resettable_v_durability_groupsCompleteVolatile              (FALSE)
#define resettable_v_durability_groupsCompleteTransient             (FALSE)
#define resettable_v_durability_groupsCompletePersistent            (FALSE)

#define resettable_v_durability_groupsIncompleteTotal               (FALSE)
#define resettable_v_durability_groupsIncompleteVolatile            (FALSE)
#define resettable_v_durability_groupsIncompleteTransient           (FALSE)
#define resettable_v_durability_groupsIncompletePersistent          (FALSE)

#define resettable_v_durability_groupsIgnoredTotal                  (TRUE)
#define resettable_v_durability_groupsIgnoredVolatile               (TRUE)
#define resettable_v_durability_groupsIgnoredTransient              (TRUE)
#define resettable_v_durability_groupsIgnoredPersistent             (TRUE)

#define resettable_v_durability_alignerRequestsReceived             (TRUE)
#define resettable_v_durability_alignerRequestsIgnored              (TRUE)
#define resettable_v_durability_alignerRequestsAnswered             (TRUE)
#define resettable_v_durability_alignerRequestsOpen                 (FALSE)
#define resettable_v_durability_alignerRequestsOpenMax              (TRUE)
#define resettable_v_durability_alignerRequestsCombined             (TRUE)
#define resettable_v_durability_alignerRequestsCombinedOpen         (FALSE)
#define resettable_v_durability_alignerRequestsCombinedOpenMax      (TRUE)
#define resettable_v_durability_alignerRequestsCombinedAnswered     (TRUE)

#define resettable_v_durability_aligneeRequestsSent                 (TRUE)
#define resettable_v_durability_aligneeRequestsOpen                 (FALSE)
#define resettable_v_durability_aligneeRequestsOpenMax              (TRUE)
#define resettable_v_durability_aligneeRequestsWaiting              (FALSE)
#define resettable_v_durability_aligneeRequestsWaitingMax           (TRUE)

#define resettable_v_durability_aligneeSamplesTotal                 (TRUE)
#define resettable_v_durability_aligneeSamplesRegister              (TRUE)
#define resettable_v_durability_aligneeSamplesWrite                 (TRUE)
#define resettable_v_durability_aligneeSamplesDispose               (TRUE)
#define resettable_v_durability_aligneeSamplesWriteDispose          (TRUE)
#define resettable_v_durability_aligneeSamplesUnregister            (TRUE)

#define resettable_v_durability_alignerSamplesTotal                 (TRUE)
#define resettable_v_durability_alignerSamplesRegister              (TRUE)
#define resettable_v_durability_alignerSamplesWrite                 (TRUE)
#define resettable_v_durability_alignerSamplesDispose               (TRUE)
#define resettable_v_durability_alignerSamplesWriteDispose          (TRUE)
#define resettable_v_durability_alignerSamplesUnregister            (TRUE)

#define resettable_v_durability_aligneeTotalSize                    (TRUE)
#define resettable_v_durability_alignerTotalSize                    (TRUE)
/* v_entity helpermacro */

#define v_entityStatisticsGetRef(fieldName, entity) \
    &((v_entity(entity)->statistics)->fieldName)


/* v_reader helpermacro */
#define v_readerStatisticsGetRef(fieldName, entity) \
    &((v_readerStatistics(v_entity(entity)->statistics))->fieldName)

/* v_query helpermacro */
#define v_queryStatisticsGetRef(fieldName, entity) \
    &((v_queryStatistics(v_entity(entity)->statistics))->fieldName)

/* v_writer helpermacro */
#define v_writerStatisticsGetRef(fieldName, entity) \
    &((v_writerStatistics(v_entity(entity)->statistics))->fieldName)


/* v_networkReader helpermacro */
#define v_networkReaderStatisticsGetRef(fieldName, entity) \
    &((v_networkReaderStatistics(v_entity(entity)->statistics))->fieldName)

/* v_networkQueueStatistics should not appear in here */
#define v_networkReaderStatisticsGetIndexedRef(indexedName, index, fieldName, entity) \
    ((index > 0) && (index <= v_networkReaderStatistics(v_entity(entity)->statistics)->indexedName##Count) ? \
        &(((v_networkQueueStatistics *)(v_networkReaderStatistics(v_entity(entity)->statistics)->indexedName))[index-1]->fieldName) : NULL)


/* v_networking helpermacro */
#define v_networkingStatisticsGetRef(fieldName, entity) \
    &((v_networkingStatistics(v_entity(entity)->statistics))->fieldName)

/* v_networkChannelStatistics should not appear in here */
#define v_networkingStatisticsGetIndexedRef(indexedName, index, fieldName, entity) \
    ((index > 0) && (index <= v_networkingStatistics(v_entity(entity)->statistics)->indexedName##Count) ? \
        &(((v_networkChannelStatistics *)(v_networkingStatistics(v_entity(entity)->statistics)->indexedName))[index-1]->fieldName) : NULL)


/* v_cmsoap helpermacro */
#define v_cmsoapStatisticsGetRef(fieldName, entity) \
    &((v_cmsoapStatistics(v_entity(entity)->statistics))->fieldName)


/* v_durability helpermacro */
#define v_durabilityStatisticsGetRef(fieldName, entity) \
    &((v_durabilityStatistics(v_entity(entity)->statistics))->fieldName)


/* v_kernel helpermacro */
#define v_kernelStatisticsGetRef(fieldName, entity) \
    &((v_kernelStatistics(v_entity(entity)->statistics))->fieldName)


/* v_networkQueue helpermacro */
#define v_networkQueueStatisticsGetRef(fieldName, entity) \
    &(v_networkQueue(entity)->statistics->fieldName)


/* Validity macro */

#define v_statisticsValid(entity) (v_entity(entity)->statistics != NULL)

/* Resettability */

#define v_statisticsResettable(entityName, fieldName) \
    resettable_##entityName##_##fieldName

#undef OS_API

#endif /*V_STATISTICSHELPERS_H*/
