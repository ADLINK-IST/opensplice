/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#ifndef V__DATAREADERINSTANCE_H
#define V__DATAREADERINSTANCE_H

#include "v_dataReaderInstance.h"
#include "v_index.h"

#define v_dataReaderInstance_t(scope) \
        c_type(c_resolve(c_getBase(scope), \
                          "kernelModuleI::v_dataReaderInstance"))

#define v_dataReaderInstanceDataReader(_this) \
        v_indexDataReader(v_dataReaderInstance(_this)->index)

#define v_dataReaderInstanceSetOldest(_this,_sample) \
        (v_dataReaderInstanceTemplate(_this)->oldest = _sample)

#define v_dataReaderInstanceSetNewest(_this,_sample) \
        (v_dataReaderInstanceTemplate(_this)->sample = \
        v_dataReaderSampleTemplate(_sample))

#define v_dataReaderInstanceStateSet(instance, state)               \
    assert( ((instance) != NULL ) &&                                \
            ( ((state) == L_NEW) ||                                 \
              ((state) == L_EMPTY) ||                               \
              ((state) == L_DISPOSED) ||                            \
              ((state) == L_STATECHANGED) ||                        \
              ((state) == L_TRIGGER) ||                             \
              ((state) == L_REMOVED) ||                             \
              ((state) == L_REPLACED) ||                            \
              ((state) == L_MARK) ||                                \
              ((state) == L_NOWRITERS) ||                           \
              ((state) == L_INMINSEPTIME) ||                        \
              ((state) == L_LAZYNEW) ) );                           \
    v_stateSet(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceStateSetMask(instance, state)           \
    assert( ((instance) != NULL ) &&                                \
            ( ((state) & ~(L_NEW |                                  \
                           L_EMPTY |                                \
                           L_DISPOSED |                             \
                           L_STATECHANGED |                         \
                           L_TRIGGER |                              \
                           L_REPLACED |                             \
                           L_MARK |                                 \
                           L_NOWRITERS |                            \
                           L_LAZYNEW)) == 0 ) );                    \
    v_stateSet(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceStateClear(instance, state)             \
    assert( ((instance) != NULL ) &&                                \
            ( ((state) == L_NEW) ||                                 \
              ((state) == L_EMPTY) ||                               \
              ((state) == L_DISPOSED) ||                            \
              ((state) == L_STATECHANGED) ||                        \
              ((state) == L_TRIGGER) ||                             \
              ((state) == L_REPLACED) ||                            \
              ((state) == L_MARK) ||                                \
              ((state) == L_NOWRITERS) ||                           \
              ((state) == L_REMOVED) ||                             \
              ((state) == L_INMINSEPTIME) ||                        \
              ((state) == L_LAZYNEW) ) );                           \
    v_stateClear(v_dataReaderInstanceState(instance), (state))

#define v_dataReaderInstanceInNotEmptyList(_this) \
        (v_dataReaderInstance(_this)->inNotEmptyList)

/**
 * This macro determines if the reader contains VALID samples that are available for
 * consumption. Not all valid samples are available for consumption, since some of them
 * may belong to an unfinished transaction. The resourceSampleCount of the dataReaderInstance
 * is primarily used for determining whether there are enough resource limits to
 * accommodate any further samples, and so incorporates both transactional and
 * non-transactional samples.
 */

#define hasValidSampleAccessible(_this) \
        (_this->historySampleCount > 0)

v_dataReaderInstance
v_dataReaderInstanceNew(
    v_dataReader reader,
    v_message message);

void
v_dataReaderInstanceFree(
    v_dataReaderInstance _this);

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
    v_message message,
    v_messageContext context);

c_bool
v_dataReaderInstanceReadSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_state sampleMask,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_dataReaderInstanceTakeSamples(
    v_dataReaderInstance _this,
    c_query query,
    v_state sampleMask,
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
    v_dataReaderSample sample,
    c_bool pushedOutByNewer);

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
    v_state sampleMask,
    v_queryAction action,
    c_voidp args);

void
v_dataReaderInstanceSetEpoch (
    v_dataReaderInstance _this,
    os_timeW time);

v_dataReaderResult
v_dataReaderInstanceClaimResource(
    v_dataReaderInstance instance,
    v_message message,
    v_messageContext context);

void
v_dataReaderInstanceReleaseResource(
    v_dataReaderInstance _this);

void
v_dataReaderInstanceResetOwner(
    v_dataReaderInstance _this,
    v_gid wgid);

v_dataReaderResult
v_dataReaderInstanceUnregister (
    v_dataReaderInstance _this,
    v_registration unregistration,
    os_timeW timestamp);

void
v_dataReaderInstanceFlushPending(
    v_dataReaderInstance _this);

c_bool
v_dataReaderInstanceMatchesSampleMask(
    v_dataReaderInstance _this,
    v_sampleMask mask);

c_bool
v_dataReaderInstanceCheckMinimumSeparation(
    v_dataReaderInstance _this,
    os_timeE now);

#endif
