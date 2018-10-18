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
            writer->statistics->numberOfInstancesWithStatus##statSuffix--; \
        } else {                                                 \
            writer->statistics->numberOfInstancesWithStatus##statSuffix++; \
        }                                                        \
    }
/* Returns TRUE if both the L_DISPOSED and L_UNREGISTER flags are not set. */
#define __V_WRITER_ALIVE__(state) \
        v_stateTestNot(state, (L_DISPOSED | L_UNREGISTER))

/* Returns TRUE if either the L_DISPOSED or L_UNREGISTER flags is set. */
#define __V_WRITER_LIVELINESS_CHANGED__(state) \
        v_stateTestOr(state, (L_DISPOSED | L_UNREGISTER))

/* Updates the liveliness statistic for an instance. If oldState == 0, then
 * nothing the statistics are updated as if the instance just became ALIVE (so
 * the new state has to be ALIVE). This is useful for initialization. It uses
 * the newState (oldState ^ xoredState) to determine whether counters have to be
 * updated.
 */
#define __V_WRITER_UPDATE_ALIVE__(writer, oldState, xoredState) \
    if (oldState != 0 && __V_WRITER_LIVELINESS_CHANGED__(xoredState)) { \
        if (__V_WRITER_ALIVE__(oldState)){\
            writer->statistics->numberOfInstancesWithStatusAlive--; \
        } else if (__V_WRITER_ALIVE__(oldState ^ xoredState)){ \
            writer->statistics->numberOfInstancesWithStatusAlive++; \
        } \
    } else if (oldState == 0 && __V_WRITER_ALIVE__(oldState ^ xoredState)) { \
        writer->statistics->numberOfInstancesWithStatusAlive++; \
    }

/* Updates the statistics for the instance-state flags that are enabled. Updates
 * are only performed when statistics are enabled for the specified reader.
 */
#define UPDATE_WRITER_STATISTICS(writer, instance, oldState) \
    if (writer->statistics) { \
        v_state xoredState = oldState^v_instanceState(instance); \
                                                               \
        __V_WRITER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, writer,oldState,xoredState) \
        __V_WRITER_UPDATE_FOR_FLAG__(L_UNREGISTER,Unregistered, writer,oldState,xoredState) \
        __V_WRITER_UPDATE_ALIVE__(writer,oldState,xoredState) \
    }

/* Subtracts the currently still enabled instance-state flags from the
 * statistics. Updates are only performed when statistics are enabled for
 * the specified writer.
 */
#define UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(writer, instance) \
    if ((writer)->statistics) { \
        v_state state = v_instanceState(instance); \
        __V_WRITER_UPDATE_FOR_FLAG__(L_DISPOSED, Disposed, (writer), state, state) \
        __V_WRITER_UPDATE_FOR_FLAG__(L_UNREGISTER, Unregistered, (writer), state, state) \
        if(__V_WRITER_ALIVE__(state)) { \
            writer->statistics->numberOfInstancesWithStatusAlive--; \
        } \
    }

#define v_writerKeyList(_this) \
        c_tableKeyList(v_writer(_this)->instances)

#define v_writerTransactionID(_this) \
        v_writer(_this)->transactionId

#define v__writerResendInstances(w) ((w)->resend._u.instances)
#define v__writerInOrderAdmin(w) (&(w)->resend._u.admin)

/* Evaluates to TRUE in all cases when resends need to be done in order within
 * a writer. This is the case when the access_scope of the publisher is either
 * V_PRESENTATION_TOPIC or V_PRESENTATION_GROUP.
 */
#define v__writerNeedsInOrderResends(w) ((w)->resend._d != V_PRESENTATION_INSTANCE)

#define v__writerInOrderAdminOldest(w) (assert(v__writerNeedsInOrderResends(w)), v__writerInOrderAdmin(w)->resendOldest)
#define v__writerInOrderAdminNewest(w) (assert(v__writerNeedsInOrderResends(w)), v__writerInOrderAdmin(w)->resendNewest)

/* Evaluates to TRUE (c_bool) when the writer has a pending resend for any of its
 * instances.
 *
 * In case v__writerNeedsInOrderResends(...) evaluates to TRUE, this is
 * determined by means of checking the head of the in-order admin. Otherwise
 * the size of the resendInstances table is inspected.
 */
#define v__writerHasResendsPending(w) ((c_bool)(v__writerNeedsInOrderResends(w) ? (v__writerInOrderAdminOldest(w) != NULL) : (c_tableCount(v__writerResendInstances(w)) > 0)))

typedef struct v_writerNotifyChangedQosArg_s {
    /* the following fields are set when the partitionpolicy has changed. */
    c_iter addedPartitions;
    c_iter removedPartitions;
} v_writerNotifyChangedQosArg;

v_result
v__writerEnable(
    v_writer writer,
    os_boolean builtin);

c_bool
v_writerPublishGroup (
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

v_result
v_writerCheckDeadlineMissed (
    v_writer _this,
    os_timeE now);

void
v_writerResumePublication (
    v_writer _this,
    const os_timeW *resumeTime);

typedef c_bool (*v_writerGroupAction) (v_group group, c_voidp arg);

c_bool
v_writerGroupWalk(
    v_writer w,
    v_writerGroupAction action,
    c_voidp arg);

c_bool
v_writerGroupWalkUnlocked(
    v_writer w,
    v_writerGroupAction action,
    c_voidp arg);

void
v_writerResendItemRemove(
    v_writer writer,
    v_writerResendItem ri);

void
v_writerResendItemInsert(
    v_writer writer,
    v_writerResendItem ri);

char *
v__writerGetNameSpaceNames(
    v_writer writer,
    c_iter partitions,
    c_iter nameSpaces);

c_bool
v__writerInSingleNameSpace(
    v_writer writer,
    c_iter partitions,
    c_iter nameSpaces);

v_publisher
v_writerGetPublisher(
    v_writer _this);

/* This operation will return the actual builtin publication data for this writer.
 * The writer caches this last published publication data.
 */
v_message
v_writerPublication(
    v_writer _this);

#endif /* V__WRITER_H */
