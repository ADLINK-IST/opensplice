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
#include "v__topic.h"
#include "v__topicQos.h"
#include "v__kernel.h"
#include "v__entity.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_public.h"
#include "v_participant.h"
#include "v__status.h"
#include "os_heap.h"
#include "v_topic.h"
#include "v__topicImpl.h"
#include "v__participant.h"
#include "v_topicAdapter.h"

#include "vortex_os.h"
#include "os_report.h"


void
v_topicAdapterInit(
    v_topicAdapter adapter,
    v_topic        topic,
    v_participant  p,
    const c_char  *name)
{
    assert(adapter != NULL);
    assert(p != NULL);
    assert(C_TYPECHECK(adapter, v_topicAdapter));
    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(topic,v_topic));

    v_entityInit(v_entity(adapter), name, TRUE);

    adapter->topic = c_keep(topic);

    (void)v_observerSetEvent(v_observer(adapter), V_EVENT_ALL_DATA_DISPOSED | V_EVENT_INCONSISTENT_TOPIC);

    v_observableAddObserver(v_observable(topic), v_observer(adapter), NULL);
    v_observableAddObserver(v_observable(adapter), v_observer(p), NULL);
    v_participantAdd(p, v_object(adapter));
    v_topic(adapter)->owner = p;
}


v_topicAdapter
v_topicAdapterNew(
    v_participant p,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyExpr,
    v_topicQos qos)
{
    v_topicAdapter adapter = NULL;
    v_topicImpl topic;
    v_kernel kernel;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));

    kernel = v_objectKernel(p);

    topic = v_topicImplNew(kernel, name, typeName, keyExpr, qos, TRUE);
    if (topic) {
        adapter = v_topicAdapterWrap(p, v_topic(topic));
    }

    return adapter;
}

v_topicAdapter
v_topicAdapterNewFromTopicInfo(
    v_participant p,
    const struct v_topicInfo *info,
    c_bool announce)
{
    v_topicAdapter adapter = NULL;
    v_topicImpl topic;
    v_kernel kernel;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));

    kernel = v_objectKernel(p);

    topic = v_topicImplNewFromTopicInfo(kernel, info, announce);
    if (topic) {
        adapter = v_topicAdapterWrap(p, v_topic(topic));
    }

    return adapter;
}

v_topicAdapter
v_topicAdapterWrap(
    v_participant p,
    v_topic topic)
{
    v_topicAdapter adapter = NULL;
    v_kernel kernel;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(p);

    adapter = v_topicAdapter(v_objectNew(kernel,K_TOPIC_ADAPTER));
    if (adapter != NULL) {
        v_topicAdapterInit(adapter, topic, p, v_topicName(topic));
    } else {
        OS_REPORT(OS_ERROR, "v_topicAdapterWrap", V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate TopicAdapter for topic '%s'.",
                   v_topicName(topic));
    }

    return adapter;
}


void
v_topicAdapterFree(
    v_topicAdapter adapter)
{
    v_participant p;

    assert(C_TYPECHECK(adapter,v_topicAdapter));

    p = v_topicAdapterParticipant(adapter);

    v_observableRemoveObserver(v_observable(adapter->topic), v_observer(adapter), NULL);
    v_observableRemoveObserver(v_observable(adapter), v_observer(p), NULL);

    if (p != NULL) {
        v_participantRemove(p,v_object(adapter));
        v_topic(adapter)->owner = NULL;
    }

    v_entityFree(v_entity(adapter));
}

void
v_topicAdapterDeinit(
    v_topicAdapter adapter)
{
    assert(C_TYPECHECK(adapter,v_topicAdapter));

    v_entityDeinit(v_entity(adapter));
}

void
v_topicAdapterNotify(
    v_topicAdapter adapter,
    v_event event,
    c_voidp userData)
{
    C_STRUCT(v_event) e;
    c_bool forward = TRUE;
    c_bool notified;

    OS_UNUSED_ARG(userData);
    assert(C_TYPECHECK(adapter,v_topicAdapter));

    if (event != NULL) {
        switch (event->kind) {
        case V_EVENT_ALL_DATA_DISPOSED:
            v_statusNotifyAllDataDisposed(v_entity(adapter)->status);
            break;
        case V_EVENT_INCONSISTENT_TOPIC:
            v_statusNotifyInconsistentTopic(v_entity(adapter)->status);
            break;
        default:
            forward = FALSE;
            break;
        }
        if (forward) {
            e.kind = event->kind;
            e.source = v_observable(adapter);
            e.data = NULL;
            notified = v_entityNotifyListener(v_entity(adapter), &e);
            if (!notified) {
                v_observableNotify(v_observable(adapter), &e);
            }
        }
    }
}

v_result
v_topicAdapterGetInconsistentTopicStatus(
    v_topicAdapter _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(C_TYPECHECK(_this,v_topicAdapter));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_topicStatus(status)->inconsistentTopic, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_INCONSISTENT_TOPIC);
        }
        v_topicStatus(status)->inconsistentTopic.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
    }

    return result;
}

v_result
v_topicAdapterGetAllDataDisposedStatus(
    v_topicAdapter _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(C_TYPECHECK(_this,v_topicAdapter));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_topicStatus(status)->allDataDisposed, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_ALL_DATA_DISPOSED);
        }
        v_topicStatus(status)->allDataDisposed.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
    }

    return result;
}
