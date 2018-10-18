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
#ifndef V__GROUP_H
#define V__GROUP_H

#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define v_groupGIDKey(_this) \
        (v_group(_this)->gidkey)

#define V_GROUP_NAME_TEMPLATE "Group<%s,%s>"

#define v_groupKeyList(_this) \
        v_group(_this)->keyList

#define v_groupSampleCountIncrement(_this) \
        v_group(_this)->resourceSampleCount++;

#define v_groupSampleCountDecrement(_this) \
        v_group(_this)->resourceSampleCount--;

#define v_groupWriterAdministration(_this) \
        v_group(_this)->writerAdministration;

struct v_groupFlushTransactionArg {
    v_group group;
    v_transaction txn;
};

/* param 'id' is the sequence number of the group and is locally unique.
 * param 'gidkey' specifies the storage spectrum is defined by topic key or instance gid key.
 */
v_group
v_groupNew (
    v_partition partition,
    v_topic topic,
    c_long id,
    c_bool gidkey);

void
v_groupDeinit (
    v_group _this);

void
v_groupAddWriter (
    v_group _this,
    v_writer w);

void
v_groupRemoveWriter (
    v_group _this,
    v_writer w);

c_iter
v_groupGetRegisterMessages(
    v_group _this,
    c_ulong systemId);

c_iter
v_groupGetRegistrationsOfWriter(
    v_group _this,
    v_gid writerGid);

v_writeResult
v_groupResend(
    v_group _this,
    v_message o,
    v_groupInstance *instancePtr,
    v_resendScope *resendScope,
    v_networkId writingNetworkId);

void
v_groupDisconnectNode(
    v_group _this,
    c_ulong systemId,
    os_timeW cleanTime);

v_message
v_groupCreateInvalidMessage(
    v_kernel kernel,
    v_gid writerGID,
    c_array writerQos,
    os_timeW timestamp);

void
v_groupNotifyCoherentPublication(
    v_group _this,
    v_message msg);

v_writeResult
forwardMessageToStreams(
    v_group group,
    v_groupInstance instance,
    v_message message,
    os_timeE t,
    v_groupActionKind actionKind);

void
v__groupDataReaderEntriesWriteEOTNoLock(
    v_group _this,
    v_message msg);

void
v_groupFlushTransactionNoLock(
    v_instance instance,
    v_message message,
    c_voidp arg);  /* arg is internally cast to (struct v_groupFlushTransactionArg *) */

void
_dispose_purgeList_insert(
    v_groupInstance instance,
    os_timeE insertTime);

void
_empty_purgeList_insert(
    v_groupInstance instance,
    os_timeE insertTime);

typedef c_bool (*v_groupEntrySetWalkAction)(v_groupEntry entry, c_voidp arg);

c_bool
v_groupEntrySetWalk(
    struct v_groupEntrySet *s,
    v_groupEntrySetWalkAction action,
    c_voidp arg);

v_groupSample
v_groupSampleNew (
    v_group _this,
    v_message message);

void
v_groupSampleFree (
    v_groupSample sample);

os_boolean
v_groupIsDurable(
    v_group _this);

c_bool
v_groupIsOnRequest(
    v_group _this);

/* This operation is called by a writer upon connecting to the group.
 * This operation generates a V_EVENT_CONNECT_WRITER event to notify the Durability Service that
 * the group has writers.
 */
void
v_groupNotifyWriter(
    v_group _this,
    v_writer w);

v_result
v_groupSetFilter(
    v_group _this,
    q_expr condition,
    const c_value params[],
    os_uint32 nrOfParams);

_Requires_lock_held_(g->mutex)
v_alignState
v__groupCompleteGet_nl(
    _In_ v_group g);

void
v_groupGetOpenTransactions(
    v_group g,
    v_entry e,
    c_bool groupAdmin);

void
v_groupInsertTransactionMessage(
    v_group _this,
    v_message msg,
    v_groupInstance instance);

#if defined (__cplusplus)
}
#endif

#endif
