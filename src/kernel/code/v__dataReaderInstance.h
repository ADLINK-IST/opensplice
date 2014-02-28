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

#ifndef V__DATAREADERINSTANCE_H
#define V__DATAREADERINSTANCE_H

#include "v_dataReaderInstance.h"
#include "v_index.h"

#define v_dataReaderInstance_t(scope) \
        c_type(c_resolve(c_getBase(scope), \
                          "kernelModule::v_dataReaderInstance"))

#define v_dataReaderInstanceDataReader(_this) \
        v_indexDataReader(v_dataReaderInstance(_this)->index)

#define v_dataReaderInstanceSetOldest(_this,_sample) \
        (v_dataReaderInstanceTemplate(_this)->oldest = \
        v_dataReaderSampleTemplate(_sample))

#define v_dataReaderInstanceSetNewest(_this,_sample) \
        (v_dataReaderInstanceTemplate(_this)->sample = \
        (c_voidp)_sample)

#define v_dataReaderInstanceStateSet(instance, state)               \
    assert( ((instance) != NULL ) &&                                \
            ( ((state) == L_NEW) ||                                 \
              ((state) == L_EMPTY) ||                               \
              ((state) == L_DISPOSED) ||                            \
              ((state) == L_STATECHANGED) ||                        \
              ((state) == L_TRIGGER) ||                             \
              ((state) == L_REMOVED) ||                             \
              ((state) == L_REPLACED) ||                            \
              ((state) == L_NOWRITERS) ) );                         \
    v_stateSet(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceStateSetMask(instance, state)           \
    assert( ((instance) != NULL ) &&                                \
            ( ((state) & ~(L_NEW |                                  \
                           L_EMPTY |                                \
                           L_DISPOSED |                             \
                           L_STATECHANGED |                         \
                           L_TRIGGER |                              \
                           L_REPLACED |                             \
                           L_NOWRITERS)) == 0 ) );                  \
    v_stateSet(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceStateClear(instance, state)             \
    assert( ((instance) != NULL ) &&                                \
            ( ((state) == L_NEW) ||                                 \
              ((state) == L_EMPTY) ||                               \
              ((state) == L_DISPOSED) ||                            \
              ((state) == L_STATECHANGED) ||                        \
              ((state) == L_TRIGGER) ||                             \
              ((state) == L_REPLACED) ||                            \
              ((state) == L_NOWRITERS) ||                           \
              ((state) == L_REMOVED) ) );                           \
    v_stateClear(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceInNotEmptyList(_this) \
        (v_dataReaderInstance(_this)->inNotEmptyList)

/**
 * This macro determines if the reader contains VALID samples that are available for
 * consumption. Not all valid samples are available for consumption, since some of them
 * may belong to an unfinished transaction. The sampleCount of the dataReaderInstance
 * is primarily used for determining whether there are enough resource limits to
 * accommodate any further samples, and so incorporates both transactional and
 * non-transactional samples.
 */

#define hasValidSampleAccessible(_this) \
        (_this->accessibleCount > 0)

v_dataReaderInstance
v_dataReaderInstanceNew(
    v_dataReader reader,
    v_message message);

void
v_dataReaderInstanceInit(
    v_dataReaderInstance _this,
    v_message message);

void
v_dataReaderInstanceDeinit(
    v_dataReaderInstance _this);

v_writeResult
v_dataReaderInstanceWrite (
    v_dataReaderInstance _this,
    v_message message);

v_dataReaderResult
v_dataReaderInstanceInsert(
    v_dataReaderInstance _this,
    v_message message);

c_bool
v_dataReaderInstanceReadSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_readerSampleAction action,
    c_voidp arg);

v_actionResult
v_dataReaderInstanceWalkSamples(
    v_dataReaderInstance _this,
    v_readerSampleAction action,
    c_voidp arg);

void
v_dataReaderInstanceSampleRemove(
    v_dataReaderInstance _this,
    v_dataReaderSample sample);

c_bool
v_dataReaderInstanceContainsSample(
    v_dataReaderInstance _this,
    v_dataReaderSample sample);

void
v_dataReaderInstancePurge(
    v_dataReaderInstance _this,
    c_long disposedCount,
    c_long noWritersCount);

c_bool
v_dataReaderInstanceTest(
    v_dataReaderInstance _this,
    c_query query,
    v_queryAction action,
    c_voidp args);

void
v_dataReaderInstanceSetEpoch (
    v_dataReaderInstance _this,
    c_time time);

void
v_dataReaderInstanceFlushTransaction(
    v_dataReaderInstance _this,
    c_ulong transactionId);

void
v_dataReaderInstanceAbortTransaction(
    v_dataReaderInstance _this,
    c_ulong transactionId);

void
v_dataReaderInstanceResetOwner(
    v_dataReaderInstance _this,
    v_gid wgid);

v_dataReaderResult
v_dataReaderInstanceUnregister (
    v_dataReaderInstance _this,
    v_registration unregistration,
    c_time timestamp);

#endif
