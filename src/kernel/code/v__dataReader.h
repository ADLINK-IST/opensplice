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

#ifndef V__DATAREADER_H
#define V__DATAREADER_H

#include "v_misc.h"
#include "v_event.h"
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v__status.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
/**************************************************************
 * Local Macro definitions
 **************************************************************/
#define __V_DATAREADER_UPDATE_FOR_FLAG__(flag, statSuffix, reader, oldState, xoredState) \
    if (v_stateTest(xoredState, flag)) {                         \
        if (v_stateTest(oldState, flag)) {                       \
            reader->statistics->numberOfInstancesWithStatusNew--; \
        } else {                                                 \
            reader->statistics->numberOfInstancesWithStatusNew++; \
        }                                                        \
    }
/* Returns TRUE if both the L_DISPOSED and L_NOWRITERS flags are not set. */
#define __V_DATAREADER_ALIVE__(state) v_stateTestNot(state, (L_DISPOSED | L_NOWRITERS))
/* Returns TRUE if either the L_DISPOSED or L_NOWRITERS flags is set. */
#define __V_DATAREADER_LIVELINESS_CHANGED__(xoredState) v_stateTestOr(xoredState, (L_DISPOSED | L_NOWRITERS))

/* Updates the liveliness statistic for an instance. If oldState == 0, then
 * nothing is done. This is useful for initialization. It uses the newState
 * (oldState ^ xoredState) to determine whether counters have to be updated.
 */
#define __V_DATAREADER_UPDATE_ALIVE__(reader, oldState, xoredState) \
    if (oldState && __V_DATAREADER_LIVELINESS_CHANGED__(xoredState)) { \
        if (__V_DATAREADER_ALIVE__(oldState)){\
            reader->statistics->numberOfInstancesWithStatusAlive--; \
        } else if (__V_DATAREADER_ALIVE__(oldState ^ xoredState)){      \
            reader->statistics->numberOfInstancesWithStatusAlive++; \
        }\
    }

/* Updates the statistics for the instance-state flags that are enabled. Updates
 * are only performed when statistics are enabled for the specified reader.
 */
#define UPDATE_READER_STATISTICS(index, instance, oldState) \
    if (reader->statistics) { \
        v_state xoredState = oldState^v_instanceState(instance); \
                                                               \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NEW,      New,      reader,oldState,xoredState) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, reader,oldState,xoredState) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NOWRITERS,NoWriters,reader,oldState,xoredState) \
        __V_DATAREADER_UPDATE_ALIVE__(reader,oldState,xoredState) \
    }

/* Subtracts the currently still enabled instance-state flags from the
 * statistics. Updates are only performed when statistics are enabled for
 * the specified reader.
 */
#define UPDATE_READER_STATISTICS_REMOVE_INSTANCE(reader, instance) \
    if (reader->statistics) { \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NEW, New, reader, \
                                         v_instanceState(instance), v_instanceState(instance)) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, reader, \
                                         v_instanceState(instance), v_instanceState(instance)) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NOWRITERS, NoWriters, reader, \
                                         v_instanceState(instance), v_instanceState(instance)) \
        if(__V_DATAREADER_ALIVE__(v_instanceState(instance))){        \
            reader->statistics->numberOfInstancesWithStatusAlive--; \
        } \
    }

#define v_dataReaderQos(_this) \
        (v_reader(v_dataReader(_this))->qos)

#define v_dataReaderDeadLineInstanceList(_this) \
        (v_deadLineInstanceList(v_dataReader(_this)->deadLineList))

/**
 * \brief Callback function definition for ordered read/take function.
 */
typedef v_actionResult(*v__dataReaderAction)(
    v_dataReaderSample , v_readerSampleAction, c_voidp);

#ifdef _MSG_STAMP_
void
v_dataReaderLogMessage(
    v_dataReader _this,
    v_message msg);
#endif

void
v_dataReaderDeinit(
    v_dataReader _this);

c_char *
v_dataReaderKeyExpr(
    v_dataReader reader);

/**
 * \brief This operation retrieves the dataReader instance keyList.
 *
 * The keyList is an array of c_field objects that specifies the instance
 * key fields.
 *
 * \param _this The dataReader this method operates on.
 *
 * \return A successful operation will return the dataReader instance keyList.
 *         otherwise the operation will return NULL.
 */
c_array
v_dataReaderKeyList(
    v_dataReader _this);

c_array
v_dataReaderSourceKeyList(
    v_dataReader _this);

c_field
v_dataReaderIndexField(
    v_dataReader reader,
    const c_char *name);

c_field
v_dataReaderField(
    v_dataReader reader,
    const c_char *name);

#define v_dataReaderAddEntry(_this,entry) \
        v_readerAddEntry(v_reader(_this),v_entry(entry))

#define v_dataReaderNextInstance(_this,_inst) \
        v_dataReaderInstance( \
            c_tableNext(v_dataReader(_this)->index->notEmptyList, \
                        v_dataReaderInstance(_inst)))

/* The trigger-value stores a sample but needs access to the unreferenced
 * instance-pointer of the sample, which thus needs to be explicitly kept and
 * freed.
 *
 * This macro returns the parameter sample
 *
 * @return sample
 */
#define v_dataReaderTriggerValueKeep(sample) \
        (c_keep(v_readerSample(sample)->instance), \
         c_keep(sample))

/* The sample stored in the trigger-value has its (otherwise unreferenced)
 * instance-pointer explicitly kept, so it has to be freed.
 */
#define v_dataReaderTriggerValueFree(triggerValue)          \
    {                                                       \
        v_dataReaderInstance _instance;                     \
                                                            \
        assert(C_TYPECHECK(triggerValue, v_readerSample));  \
        _instance = v_readerSample(triggerValue)->instance; \
        c_free(triggerValue);                               \
        c_free(_instance);                                  \
    }

void
v_dataReaderUpdatePurgeLists(
    v_dataReader _this);

void
v_dataReaderBeginAccess(
    v_dataReader _this);

void
v_dataReaderEndAccess(
    v_dataReader _this);

typedef struct v_dataReaderConnectionChanges_s {
    /* the following fields are set when the partitionQosPolicy has changed. */
    c_iter addedPartitions;
    c_iter removedPartitions;
} v_dataReaderConnectionChanges;

void
v_dataReaderNotify(
    v_dataReader _this,
    v_event event,
    c_voidp userData);

void
v_dataReaderNotifyDataAvailable(
    v_dataReader _this,
    v_dataReaderSample sample);

void
v_dataReaderTriggerDataAvailable(
    v_dataReader _this);

void
v_dataReaderNotifySampleLost_nl(
    v_dataReader _this,
    c_ulong nrSamplesLost);

void
v_dataReaderNotifySampleLost(
    v_dataReader _this,
    c_ulong nrSamplesLost);

void
v_dataReaderNotifySampleRejected(
    v_dataReader _this,
    v_sampleRejectedKind kind,
    v_gid instanceHandle);

void
v_dataReaderNotifyIncompatibleQos(
    v_dataReader _this,
    v_policyId id,
    v_gid writerGID);

void
v_dataReaderNotifySubscriptionMatched (
    v_dataReader r,
    v_gid        writerGID,
    c_bool       dispose,
    struct v_publicationInfo *publicationInfo,
    c_bool       isImplicit);

void
v_dataReaderNotifyChangedQos(
    v_dataReader _this);

void
v_dataReaderNotifyLivelinessChanged(
    v_dataReader _this,
    v_gid wGID,
    enum v_statusLiveliness oldLivState,
    enum v_statusLiveliness newLivState,
    v_message publicationInfo);

void
v_dataReaderNotifyOwnershipStrengthChanged(
    v_dataReader _this,
    struct v_owner *ownership);

void
v_dataReaderInsertView(
    v_dataReader _this,
    v_dataView view);

void
v_dataReaderRemoveView(
    v_dataReader _this,
    v_dataView view);

void
v_dataReaderRemoveViewUnsafe(
    v_dataReader _this,
    v_dataView view);

void
v_dataReaderRemoveExpiredSamples(
    v_dataReader _this,
    v_readerSampleAction action,
    c_voidp arg);

void
v_dataReaderRemoveInstanceNotAlive(
    v_dataReader _this,
    v_dataReaderInstance instance);

void
v_dataReaderRemoveInstance(
    v_dataReader _this,
    v_dataReaderInstance instance);

void
v_dataReaderCheckDeadlineMissed(
    v_dataReader _this,
    os_timeE now);

c_ulong
v_dataReaderInstanceCount_nl(
    v_dataReader _this);

v_dataReaderInstance
v_dataReaderAllocInstance(
    v_dataReader _this);

_Check_return_
_Ret_maybenull_
OS_API v_topic
v_dataReaderGetTopic(
    v_dataReader _this);

c_long
v_dataReaderHistoryCount(
    v_dataReader _this);

c_bool
v_dataReaderHasMatchingSamples(
    v_dataReader _this,
    v_sampleMask mask);

void
v_dataReaderCheckMinimumSeparationList(
    v_dataReader _this,
    os_timeE now);

void
v_dataReaderMinimumSeparationListRegister(
    v_dataReader _this,
    v_dataReaderSample sample);

void
v_dataReaderMinimumSeparationListRemove(
    v_dataReader _this,
    v_dataReaderInstance instance);

v_result
v_dataReaderAccessTest(
    v_dataReader _this);

/* This operation informs the reader to either add or remove the publication described by
 * the given publication info depending on the given ignore flag.
 * If ignore is TRUE then the publication will be added to the readers ignore list and if
 * ignore is FALSE then the publication will be removed from the ignore list if it exists.
 * Note that removing a publication from the ignore list should only be performed if no data
 * from the publication is to be expected in the future according to the DCPS specification.
 */
v_result
v_dataReaderIgnore(
    v_dataReader _this,
    struct v_publicationInfo *info,
    os_boolean ignore);

/* This operation will return the actual builtin subscription data for this dataReader.
 * The dataReader caches this last published subscription data.
 */
v_message
v_dataReaderSubscription(
    v_dataReader _this);

#undef OS_API

#endif
