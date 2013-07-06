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
#ifndef V__WRITER_H
#define V__WRITER_H

#include "v_writer.h"
#include "v_writerInstance.h"
#include "v_entity.h"

/**************************************************************
 * Local Macro definitions
 **************************************************************/
#define __V_WRITER_UPDATE_FOR_FLAG__(flag, statSuffix, writer, oldState, xoredState) \
    if (v_stateTest(xoredState, flag)) {                         \
        if (v_stateTest(oldState, flag)) {                       \
            v_statisticsULongValueDec(v_writer,                  \
              numberOfInstancesWithStatus##statSuffix, writer); \
        } else {                                                 \
            v_statisticsULongValueInc(v_writer,                 \
              numberOfInstancesWithStatus##statSuffix, writer); \
        }                                                        \
    }
/* Returns TRUE if both the L_DISPOSED and L_UNREGISTER flags are not set. */
#define __V_WRITER_ALIVE__(state) v_stateTestNot(state, (L_DISPOSED | L_UNREGISTER))
/* Returns TRUE if either the L_DISPOSED or L_UNREGISTER flags is set. */
#define __V_WRITER_LIVELINESS_CHANGED__(xoredState) v_stateTestOr(xoredState, (L_DISPOSED | L_UNREGISTER))

/* Updates the liveliness statistic for an instance. If oldState == 0, then
 * nothing the statistics are updated as if the instance just became ALIVE (so
 * the new state has to be ALIVE). This is useful for initialization. It uses
 * the newState (oldState ^ xoredState) to determine whether counters have to be
 * updated. */
#define __V_WRITER_UPDATE_ALIVE__(writer, oldState, xoredState) \
    if (oldState != 0 && __V_WRITER_LIVELINESS_CHANGED__(xoredState)) { \
        if (__V_WRITER_ALIVE__(oldState)){\
            v_statisticsULongValueDec(v_writer,                         \
                    numberOfInstancesWithStatusAlive, writer);          \
        } else if (__V_WRITER_ALIVE__(oldState ^ xoredState)){          \
            v_statisticsULongValueInc(v_writer,                         \
                    numberOfInstancesWithStatusAlive, writer);          \
        }\
    } else if (oldState == 0 && __V_WRITER_ALIVE__(oldState ^ xoredState)) { \
        v_statisticsULongValueInc(v_writer,                         \
        numberOfInstancesWithStatusAlive, writer);          \
    }

/* Updates the statistics for the instance-state flags that are enabled. Updates
 * are only performed when statistics are enabled for the specified reader. */
#define UPDATE_WRITER_STATISTICS(writer, instance, oldState) \
    if (v_statisticsValid(writer)) { \
        v_state xoredState = oldState^instance->state; \
                                                               \
        __V_WRITER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, writer,oldState,xoredState) \
        __V_WRITER_UPDATE_FOR_FLAG__(L_UNREGISTER,Unregistered, writer,oldState,xoredState) \
        __V_WRITER_UPDATE_ALIVE__(writer,oldState,xoredState) \
    }

/* Subtracts the currently still enabled instance-state flags from the
 * statistics. Updates are only performed when statistics are enabled for
 * the specified writer. */
#define UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(writer, instance) \
    if (v_statisticsValid(writer)) { \
                                                                                   \
        __V_WRITER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, writer, instance->state, instance->state) \
        __V_WRITER_UPDATE_FOR_FLAG__(L_UNREGISTER, Unregistered, writer, instance->state, instance->state) \
        if(__V_WRITER_ALIVE__(instance->state)){             \
            v_statisticsULongValueDec(v_writer,              \
                numberOfInstancesWithStatusAlive, writer);   \
        } \
    }

#define v_writerKeyList(_this) \
        c_tableKeyList(v_writer(_this)->instances)

typedef struct v_writerNotifyChangedQosArg_s {
    /* the following fields are set when the partitionpolicy has changed. */
    c_iter addedPartitions;
    c_iter removedPartitions;
} v_writerNotifyChangedQosArg;

c_bool
v_writerPublishGroup (
    v_writer _this,
    v_group g);

c_bool
v_writerUnPublishGroup (
    v_writer _this,
    v_group g);

void
v_writerNotifyIncompatibleQos (
    v_writer _this,
    v_policyId id);

void
v_writerNotifyPublicationMatched (
    v_writer _this,
    v_gid    readerGID,
    c_bool   dispose);

void
v_writerNotifyChangedQos (
    v_writer _this,
    v_writerNotifyChangedQosArg *arg);

void
v_writerNotifyLivelinessLost (
    v_writer _this);

/* To be used by sendQueue */
void
v_writerGroupsWrite (
    v_writer _this,
    v_message message);

v_result
v_writerSetQos (
    v_writer _this,
    v_writerQos qos);

void
v_writerDeadLineListUpdateInstance (
    v_writer _this,
    v_writerInstance instance);

void
v_writerAssertByPublisher (
    v_writer _this);

void
v_writerNotifyTake (
    v_writer _this,
    v_writerInstance instance);

c_bool
v_writerCompareKeyValues (
    v_writer _this,
    v_message message,
    v_writerInstance instance);

v_message
v_writerKeepMessage (
    v_writer _this,
    c_ulong seqNr);

void
v_writerResendMessage (
    v_writer _this,
    c_ulong seqNr);

void
v_writerCheckDeadlineMissed (
    v_writer _this,
    c_time now);

void
v_writerResumePublication (
    v_writer _this,
    c_time *suspendTime);

typedef c_bool (*v_writerGroupAction) (v_group group, c_voidp arg);

c_bool
v_writerGroupWalk(
    v_writer w,
    v_writerGroupAction action,
    c_voidp arg);

v_result
v_writerCoherentBegin (
    v_writer _this,
    c_ulong id);

v_result
v_writerCoherentEnd (
    v_writer _this);

#endif
