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
            v_statisticsULongValueDec(v_reader,                  \
              numberOfInstancesWithStatus##statSuffix, reader); \
        } else {                                                 \
            v_statisticsULongValueInc(v_reader,                 \
              numberOfInstancesWithStatus##statSuffix, reader); \
        }                                                        \
    }
/* Returns TRUE if both the L_DISPOSED and L_NOWRITERS flags are not set. */
#define __V_DATAREADER_ALIVE__(state) v_stateTestNot(state, (L_DISPOSED | L_NOWRITERS))
/* Returns TRUE if either the L_DISPOSED or L_NOWRITERS flags is set. */
#define __V_DATAREADER_LIVELINESS_CHANGED__(xoredState) v_stateTestOr(xoredState, (L_DISPOSED | L_NOWRITERS))

/* Updates the liveliness statistic for an instance. If oldState == 0, then
 * nothing is done. This is useful for initialization. It uses the newState
 * (oldState ^ xoredState) to determine whether counters have to be updated. */
#define __V_DATAREADER_UPDATE_ALIVE__(reader, oldState, xoredState) \
    if (oldState && __V_DATAREADER_LIVELINESS_CHANGED__(xoredState)) { \
        if (__V_DATAREADER_ALIVE__(oldState)){\
            v_statisticsULongValueDec(v_reader,                         \
                    numberOfInstancesWithStatusAlive, reader);          \
        } else if (__V_DATAREADER_ALIVE__(oldState ^ xoredState)){      \
            v_statisticsULongValueInc(v_reader,                         \
                    numberOfInstancesWithStatusAlive, reader);          \
        }\
    }

/* Updates the statistics for the instance-state flags that are enabled. Updates
 * are only performed when statistics are enabled for the specified reader. */
#define UPDATE_READER_STATISTICS(index, instance, oldState) \
    if (v_statisticsValid(index->reader)) { \
        v_state xoredState = oldState^instance->instanceState; \
                                                               \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NEW,      New,      index->reader,oldState,xoredState) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, index->reader,oldState,xoredState) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NOWRITERS,NoWriters,index->reader,oldState,xoredState) \
        __V_DATAREADER_UPDATE_ALIVE__(index->reader,oldState,xoredState) \
    }

/* Subtracts the currently still enabled instance-state flags from the
 * statistics. Updates are only performed when statistics are enabled for
 * the specified reader. */
#define UPDATE_READER_STATISTICS_REMOVE_INSTANCE(index, instance) \
    if (v_statisticsValid(index->reader)) { \
                                                                                   \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NEW,      New,      index->reader,instance->instanceState,instance->instanceState) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, index->reader,instance->instanceState,instance->instanceState) \
        __V_DATAREADER_UPDATE_FOR_FLAG__(L_NOWRITERS,NoWriters,index->reader,instance->instanceState,instance->instanceState) \
        if(__V_DATAREADER_ALIVE__(instance->instanceState)){        \
            v_statisticsULongValueDec(v_reader,                     \
                numberOfInstancesWithStatusAlive, index->reader);   \
        } \
    }

#define v_dataReaderLock(_this) \
        v_observerLock(v_dataReader(_this))

#define v_dataReaderUnLock(_this) \
        v_observerUnlock(v_dataReader(_this))

#define v_dataReaderQos(_this) \
        (v_reader(v_dataReader(_this))->qos)

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

c_bool
v_dataReaderSubscribe(
    v_dataReader reader,
    v_partition partition);

c_bool
v_dataReaderUnSubscribe(
    v_dataReader reader,
     v_partition partition);

c_bool
v_dataReaderSubscribeGroup(
    v_dataReader reader,
    v_group group);

c_bool
v_dataReaderUnSubscribeGroup(
    v_dataReader reader,
    v_group group);

#define v_dataReaderAddEntry(_this,entry) \
        v_dataReaderEntry(v_readerAddEntry(v_reader(_this),v_entry(entry)))

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
 * @return sample */
#define v_dataReaderTriggerValueKeep(sample) \
        (c_keep(v_readerSample(sample)->instance), \
         c_keep(sample))

/* The sample stored in the trigger-value has its (otherwise unreferenced)
 * instance-pointer explicitly kept, so it has to be freed. */
#define v_dataReaderTriggerValueFree(triggerValue)          \
    {                                                       \
        v_dataReaderInstance instance;                      \
                                                            \
        assert(C_TYPECHECK(triggerValue, v_readerSample));  \
        instance = v_readerSample(triggerValue)->instance;  \
        v_dataReaderSampleFree(triggerValue);               \
        c_free(instance);                                   \
    }

void
v_dataReaderUpdatePurgeLists(
    v_dataReader _this);

void
v_dataReaderUpdatePurgeListsLocked(
    v_dataReader _this);

typedef struct v_dataReaderConnectionChanges_s {
    /* the following fields are set when the partitionQosPolicy has changed. */
    c_iter addedPartitions;
    c_iter removedPartitions;
} v_dataReaderConnectionChanges;

void
v_dataReaderUpdateConnections(
    v_dataReader _this,
    v_dataReaderConnectionChanges *arg);

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
    c_bool       dispose);

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
    c_time now);

c_long
v_dataReaderInstanceCount(
    v_dataReader _this);

v_dataReaderInstance
v_dataReaderAllocInstance(
    v_dataReader _this);

OS_API v_topic
v_dataReaderGetTopic(
    v_dataReader _this);

#undef OS_API

#endif
