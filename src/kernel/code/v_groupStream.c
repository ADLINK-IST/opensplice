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

#include "v__groupStream.h"
#include "v_groupStream.h"
#include "v__groupQueue.h"
#include "v__reader.h"
#include "v_subscriber.h"
#include "v_event.h"
#include "v_entity.h"
#include "v_topic.h"
#include "v_public.h"
#include "v_observer.h"
#include "v__status.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v__observable.h"
#include "c_iterator.h"
#include "c_collection.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_stdlib.h"

/**
 * To be able to slave mechanisms to the mechanisms already implemented in the
 * v_group, it is necessary to monitor all actions that are taken in the group
 * (write, dispose, register, unregister, lifespan expiry, service_cleanup_delay
 * expiry, etc.). The v_groupStream  provides an abstract base reader
 * class for implementations that need to monitor these actions. The
 * implementation of this base class allows the creation of multiple
 * implementations for handling group actions. When an instance of the
 * v_groupStream is attached to a group, all actions in the group will be
 * forwarded to the stream using the v_groupStreamWrite.
 *
 * The v_groupStream provides facilities for:
 * - Autoconnecting to new appearing groups.
 * - Notification on new data available (usage i.c.w. waitset).
 */

struct groupConnected{
    c_bool connected;
    v_group group;
};
struct groupMatched{
    c_bool matched;
    v_group group;
};

static c_bool
isGroupConnected(
    v_group group,
    c_voidp args)
{
    struct groupConnected* data;

    assert(C_TYPECHECK(group, v_group));

    data = (struct groupConnected*)args;

    if(strcmp(v_partitionName(group->partition),
              v_partitionName(data->group->partition)) == 0){
        if(strcmp(v_topicName(group->topic),
                  v_topicName(data->group->topic)) == 0){
            data->connected = TRUE;
        }
    }
    return (!data->connected);
}

static c_bool
isGroupMatched(
    c_string expr,
    c_voidp args)
{
    c_value strv, exprv, matchv;
    os_size_t partitionLength, topicLength;
    c_string partition, topic;
    struct groupMatched* data;

    data = (struct groupMatched*)args;

    /* Retrieve partition- & topicname and their lengths */
    partition = v_entity(data->group->partition)->name;
    topic = v_entity(data->group->topic)->name;
    partitionLength = strlen(partition);
    topicLength = strlen(topic);

    strv.kind = exprv.kind = V_STRING;

    exprv.is.String = expr;

    strv.is.String = os_alloca(partitionLength + topicLength + 1 + 1); /* include '.' */
    os_strcpy(strv.is.String, partition);
    strv.is.String[partitionLength] = '.';
    os_strcpy(strv.is.String + partitionLength + 1, topic);

    matchv = c_valueStringMatch (exprv, strv);
    data->matched = matchv.is.Boolean;

    os_freea(strv.is.String);
    return (!data->matched);
}

void
v_groupStreamConnectNewGroups(
    v_groupStream stream,
    v_group group)
{
    struct groupConnected data;

    assert(stream != NULL);
    assert(C_TYPECHECK(stream,v_groupStream));
    v_observerLock(v_observer(stream));

    /*
     * This means the group is interesting for this
     * groupActionStream. Now I have to check if the stream is already
     * connected to this group, because we wouldn't want to connect
     * multiple times to one single group.
     */
    data.connected = FALSE;
    data.group     = group;

    c_walk(stream->groups, (c_action)isGroupConnected, &data);

    if(data.connected == FALSE){
        /*
         * The stream is not connected to the group yet, so connect now.
         */
        v_groupStreamSubscribeGroup(stream, group);
    }
    v_observerUnlock(v_observer(stream));

    if(data.connected == FALSE){
        v_groupStreamHistoricalData(group, stream);
    }

    return;
}

/**
 * PRE: observer must be locked.
 */
void
v_groupStreamNotifyDataAvailable(
    v_groupStream stream)
{
    /* This Notify method is part of the observer-observable pattern.
     * It is designed to be invoked when _this object as observer receives
     * an event from an observable object.
     * It must be possible to pass the event to the subclass of itself by
     * calling <subclass>Notify(_this, event, userData).
     * This implies that _this cannot be locked within any Notify method
     * to avoid deadlocks.
     * For consistency _this must be locked by v_observerLock(_this) before
     * calling this method.
     */
    C_STRUCT(v_event) event;

    assert(stream != NULL);
    assert(C_TYPECHECK(stream,v_groupStream));

    v_statusNotifyDataAvailable(v_entity(stream)->status);

    event.kind = V_EVENT_DATA_AVAILABLE;
    event.source = v_observable(stream);
    event.data = NULL;
    v_observableNotify(v_observable(stream), &event);

    return;
}

static void fillExprList(
    void* o,
    void* udata) {

    c_list list = udata;
    char* str = o;

    /* Insert expressions into list */
    c_listInsert(list, c_stringNew(c_getBase(list), str));
}

void
v_groupStreamInit(
    v_groupStream stream,
    const c_char *name,
    v_subscriber subscriber,
    v_readerQos qos,
    c_iter expr)
{
    v_kernel kernel;

    assert(C_TYPECHECK(stream, v_groupStream));
    assert(C_TYPECHECK(subscriber, v_subscriber));

    kernel = v_objectKernel(subscriber);

    stream->groups = c_setNew(v_kernelType(kernel,K_GROUP));
    stream->expr = c_listNew(c_resolve(c_getBase(stream), "::c_string"));
    c_iterWalk(expr, fillExprList, stream->expr);

    v_readerInit(v_reader(stream), name, subscriber, qos, TRUE);
    (void)v_subscriberAddReader(subscriber,v_reader(stream));
}

void
v_groupStreamDeinit(
    v_groupStream stream)
{
    c_iter groups;
    v_group group;

    assert(C_TYPECHECK(stream, v_groupStream));

    v_readerDeinit(v_reader(stream));

    groups = ospl_c_select(stream->groups, 0);
    group = v_group(c_iterTakeFirst(groups));

    while(group){
        v_groupRemoveStream(group, stream);
        c_free(group);
        group = v_group(c_iterTakeFirst(groups));
    }
    c_iterFree(groups);
    c_free(stream->groups);
    stream->groups = NULL;
}

void
v_groupStreamFree(
    v_groupStream stream)
{
    assert(C_TYPECHECK(stream, v_groupStream));

    v_readerFree(v_reader(stream));
}

c_bool
v_groupStreamSubscribe(
    v_groupStream stream,
    v_partition partition)
{
    c_iter list;
    v_kernel kernel;
    c_value params[1];
    v_group group;

    assert(C_TYPECHECK(stream,v_groupStream));

    kernel = v_objectKernel(v_entity(partition));
    params[0] = c_objectValue(partition);
    list = v_groupSetSelect(kernel->groupSet,"partition = %0 ",params);
    group = c_iterTakeFirst(list);

    while (group != NULL) {
        v_groupStreamSubscribeGroup(stream, group);
        c_free(group);
        group = c_iterTakeFirst(list);
    }
    c_iterFree(list);

    return TRUE;
}

c_bool
v_groupStreamSubscribeGroup(
    v_groupStream stream,
    v_group group)
{
    c_bool inserted;

    assert(C_TYPECHECK(stream, v_groupStream));
    assert(C_TYPECHECK(group, v_group));

    if (v_reader(stream)->qos->durability.v.kind == v_topicQosRef(group->topic)->durability.v.kind) {
        struct groupMatched data;
        /*
         * OSPL-1073: Check if the new group matches with the group-expression list. This
         * is a collection of partition.topic strings that allows users to connect to specific
         * topics rather than connecting to all topics within a partition.
         */
        if(stream->expr) {
            data.matched = FALSE;
            data.group = group;
            c_walk(stream->expr, (c_action)isGroupMatched, &data);
        }else {
            data.matched = TRUE;
        }

        if(data.matched) {
            inserted = v_groupAddStream(group, stream);
            if(inserted == TRUE){
                ospl_c_insert(stream->groups, group);
            }
        }
    }
    return TRUE;
}

c_bool
v_groupStreamUnSubscribe(
    v_groupStream stream,
    v_partition partition)
{
    c_iter list;
    v_group group;
    c_bool result;

    assert(C_TYPECHECK(stream, v_groupStream));
    assert(C_TYPECHECK(partition, v_partition));

    list = ospl_c_select(stream->groups, 0);
    group = c_iterTakeFirst(list);
    result = FALSE;

    while (group != NULL) {
        if(strcmp(v_partitionName(partition),
                  v_partitionName(group->partition)) == 0){
            result = v_groupStreamUnSubscribeGroup(stream, group);
        }
        c_free(group);
        group = c_iterTakeFirst(list);
    }
    c_iterFree(list);

    return result;
}

c_bool
v_groupStreamUnSubscribeGroup(
    v_groupStream stream,
    v_group group)
{
    v_group found;
    c_bool result;

    assert(C_TYPECHECK(stream, v_groupStream));
    assert(C_TYPECHECK(group, v_group));

    found = c_remove(stream->groups, group, NULL, NULL);

    if(found == group){
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}

v_writeResult
v_groupStreamWrite(
    v_groupStream stream,
    v_groupAction action)
{
    v_writeResult result;

    assert(C_TYPECHECK(stream,v_groupStream));
    assert(C_TYPECHECK(action, v_groupAction));

    result = V_WRITE_ERROR;

    switch(v_objectKind(stream)){
        case K_GROUPQUEUE:
            result = v_groupQueueWrite(v_groupQueue(stream), action);
            break;
        default:
            OS_REPORT(OS_CRITICAL,"v_groupStreamWrite",result,
                        "illegal entity kind (%d) specified",
                        v_objectKind(stream));
            assert(FALSE);
            break;
    }
    return result;
}
