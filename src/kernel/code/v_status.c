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

#include "v_status.h"
#include "v__status.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_event.h"
#include "v_writerInstance.h"

#include "os_report.h"
#include "os_heap.h"

#if 0
#define _TRACE_EVENTS_ printf
#define _TRACE_STATE_ printf
#else
#define _TRACE_EVENTS_(...)
#define _TRACE_STATE_(...)
#endif

v_status
v_statusNew(
    v_entity e)
{
    v_kernel kernel;
    v_status s;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    kernel = v_objectKernel(e);

    switch (v_objectKind(e)) {
    case K_KERNEL:
        s = v_status(v_objectNew(kernel,K_KERNELSTATUS));
    break;
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        s = v_status(v_objectNew(kernel,K_TOPICSTATUS));
    break;
    case K_SUBSCRIBER:
        s = v_status(v_objectNew(kernel,K_SUBSCRIBERSTATUS));
    break;
    case K_PUBLISHER:
        s = v_status(v_objectNew(kernel,K_PUBLISHERSTATUS));
    break;
    case K_DOMAIN:
        s = v_status(v_objectNew(kernel,K_DOMAINSTATUS));
    break;
    case K_WRITER:
        s = v_status(v_objectNew(kernel,K_WRITERSTATUS));
    break;
    case K_READER:
    case K_GROUPQUEUE:
    case K_DATAREADER:
    case K_GROUPSTREAM:
    case K_DELIVERYSERVICE:
        s = v_status(v_objectNew(kernel,K_READERSTATUS));
    break;
    case K_PARTICIPANT:
        s = v_status(v_objectNew(kernel,K_PARTICIPANTSTATUS));
    break;
    default:
        return NULL;
    }
    v_statusInit(s, e->name);
    return s;
}

void
v_statusFree(
    v_status s)
{
    if (s != NULL) {
        assert(C_TYPECHECK(s,v_status));

        v_statusDeinit(s);
        c_free(s);
    }
}

void
v_statusInit(
    v_status s,
    const c_char *name)
{
    OS_UNUSED_ARG(name);
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    s->state = 0;

    switch (v_objectKind(s)) {
    case K_KERNELSTATUS:
        v_kernelStatus(s)->servicesInfo.totalCount = 0;
        v_kernelStatus(s)->servicesInfo.totalChanged = 0;
    break;
    case K_TOPICSTATUS:
        v_topicStatus(s)->inconsistentTopic.totalCount = 0;
        v_topicStatus(s)->inconsistentTopic.totalChanged = 0;
        v_topicStatus(s)->allDataDisposed.totalCount = 0;
        v_topicStatus(s)->allDataDisposed.totalChanged = 0;
    break;
    case K_SUBSCRIBERSTATUS:
    break;
    case K_DOMAINSTATUS:
        v_partitionStatus(s)->groupsChanged.totalCount = 0;
        v_partitionStatus(s)->groupsChanged.totalChanged = 0;
    break;
    case K_WRITERSTATUS:
        v_writerStatus(s)->livelinessLost.totalCount = 0;
        v_writerStatus(s)->livelinessLost.totalChanged = 0;
        v_writerStatus(s)->deadlineMissed.totalCount = 0;
        v_writerStatus(s)->deadlineMissed.totalChanged = 0;
        v_handleSetNil(v_writerStatus(s)->deadlineMissed.instanceHandle);
        v_writerStatus(s)->incompatibleQos.totalCount = 0;
        v_writerStatus(s)->incompatibleQos.totalChanged = 0;
        v_writerStatus(s)->incompatibleQos.lastPolicyId = V_UNKNOWN_POLICY_ID;
        v_writerStatus(s)->publicationMatch.totalCount = 0;
        v_writerStatus(s)->publicationMatch.totalChanged = 0;
        v_writerStatus(s)->publicationMatch.currentCount = 0;
        v_writerStatus(s)->publicationMatch.currentChanged = 0;
        v_writerStatus(s)->publicationMatch.instanceHandle = v_publicGid(NULL);
    break;
    case K_READERSTATUS:
        v_readerStatus(s)->livelinessChanged.activeCount = 0;
        v_readerStatus(s)->livelinessChanged.activeChanged = 0;
        v_readerStatus(s)->livelinessChanged.inactiveCount = 0;
        v_readerStatus(s)->livelinessChanged.inactiveChanged = 0;
        v_readerStatus(s)->livelinessChanged.instanceHandle = v_publicGid(NULL);
        v_readerStatus(s)->sampleRejected.totalCount = 0;
        v_readerStatus(s)->sampleRejected.totalChanged = 0;
        v_readerStatus(s)->sampleRejected.lastReason = S_NOT_REJECTED;
        v_readerStatus(s)->sampleRejected.instanceHandle = v_publicGid(NULL);
        v_readerStatus(s)->sampleLost.totalCount = 0;
        v_readerStatus(s)->sampleLost.totalChanged = 0;
        v_readerStatus(s)->deadlineMissed.totalCount = 0;
        v_readerStatus(s)->deadlineMissed.totalChanged = 0;
        v_handleSetNil(v_readerStatus(s)->deadlineMissed.instanceHandle);
        v_readerStatus(s)->incompatibleQos.totalCount = 0;
        v_readerStatus(s)->incompatibleQos.totalChanged = 0;
        v_readerStatus(s)->incompatibleQos.lastPolicyId = V_UNKNOWN_POLICY_ID;
        v_readerStatus(s)->subscriptionMatch.totalCount = 0;
        v_readerStatus(s)->subscriptionMatch.totalChanged = 0;
        v_readerStatus(s)->subscriptionMatch.currentCount = 0;
        v_readerStatus(s)->subscriptionMatch.currentChanged = 0;
        v_readerStatus(s)->subscriptionMatch.instanceHandle = v_publicGid(NULL);
    break;
    case K_PARTICIPANTSTATUS:
    case K_PUBLISHERSTATUS:
        /* These status are just instantations of v_status and have no
         * addition attributes! */
    break;
    default:
        OS_REPORT(OS_CRITICAL,
                    "v_statusInit", V_RESULT_ILL_PARAM,
                    "Unknown object kind %d",
                    v_objectKind(s));
    break;
    }
}

void
v_statusDeinit(
    v_status s)
{
    if (s != NULL) {
        assert(C_TYPECHECK(s,v_status));
    }
}

void
v_statusNotifyInconsistentTopic(
    v_status s)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_topicStatus));

    s->state |= V_EVENT_INCONSISTENT_TOPIC;

    v_topicStatus(s)->inconsistentTopic.totalCount++;
    v_topicStatus(s)->inconsistentTopic.totalChanged++;

    _TRACE_EVENTS_("v_statusNotifyInconsistentTopic: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_INCONSISTENT_TOPIC,
                   v_topicStatus(s)->inconsistentTopic.totalCount,
                   v_topicStatus(s)->inconsistentTopic.totalChanged);
}

void
v_statusNotifyAllDataDisposed(
    v_status s)
{

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_topicStatus));

    s->state |= V_EVENT_ALL_DATA_DISPOSED;

    v_topicStatus(s)->allDataDisposed.totalCount++;
    v_topicStatus(s)->allDataDisposed.totalChanged++;

    _TRACE_EVENTS_("v_statusNotifyAllDataDisposed: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_ALL_DATA_DISPOSED,
                   v_topicStatus(s)->allDataDisposed.totalCount,
                   v_topicStatus(s)->allDataDisposed.totalChanged);
}

void
v_statusNotifyDataOnReaders(
    v_status s)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_subscriberStatus));

    s->state |= V_EVENT_ON_DATA_ON_READERS;

    _TRACE_EVENTS_("v_statusNotifyDataOnReaders: "
                   "status = 0x%x, event = 0x%x\n",
                   s, V_EVENT_ON_DATA_ON_READERS);
}

void
v_statusNotifyDataAvailable(
    v_status s)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus) || C_TYPECHECK(s,v_subscriberStatus));

    s->state |= V_EVENT_DATA_AVAILABLE;

    _TRACE_EVENTS_("v_statusNotifyDataAvailable: "
                   "status = 0x%x, event = 0x%x\n",
                   s, V_EVENT_DATA_AVAILABLE);
}

void
v_statusNotifySampleLost(
    v_status s,
    c_ulong nrSamplesLost)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    s->state |= V_EVENT_SAMPLE_LOST;

    v_readerStatus(s)->sampleLost.totalCount += (c_long)nrSamplesLost;
    v_readerStatus(s)->sampleLost.totalChanged += (c_long)nrSamplesLost;
    _TRACE_EVENTS_("v_statusNotifySampleLost: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_SAMPLE_LOST,
                   v_readerStatus(s)->sampleLost.totalCount,
                   v_readerStatus(s)->sampleLost.totalChanged);
}

void
v_statusNotifyLivelinessLost(
    v_status s)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_writerStatus));

    s->state |= V_EVENT_LIVELINESS_LOST;

    v_writerStatus(s)->livelinessLost.totalCount++;
    v_writerStatus(s)->livelinessLost.totalChanged++;

    _TRACE_EVENTS_("v_statusNotifyLivelinessLost: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_LIVELINESS_LOST,
                   v_writerStatus(s)->livelinessLost.totalCount,
                   v_writerStatus(s)->livelinessLost.totalChanged);
}

void
v_statusNotifyOfferedDeadlineMissed(
    v_status s,
    v_handle instanceHandle)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));
    s->state |= V_EVENT_OFFERED_DEADLINE_MISSED;

    assert(v_objectKind(s) == K_WRITERSTATUS);
    v_writerStatus(s)->deadlineMissed.totalCount++;
    v_writerStatus(s)->deadlineMissed.totalChanged++;
    v_writerStatus(s)->deadlineMissed.instanceHandle = instanceHandle;

    _TRACE_EVENTS_("v_statusNotifyOfferedDeadlineMissed (WRITER): "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_OFFERED_DEADLINE_MISSED,
                   v_writerStatus(s)->deadlineMissed.totalCount,
                   v_writerStatus(s)->deadlineMissed.totalChanged);
}

void
v_statusNotifyRequestedDeadlineMissed(
    v_status s,
    v_handle instanceHandle)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    s->state |= V_EVENT_REQUESTED_DEADLINE_MISSED;

    assert(v_objectKind(s) == K_READERSTATUS);
    v_readerStatus(s)->deadlineMissed.totalCount++;
    v_readerStatus(s)->deadlineMissed.totalChanged++;
    v_readerStatus(s)->deadlineMissed.instanceHandle = instanceHandle;

    _TRACE_EVENTS_("v_statusNotifyRequetstedDeadlineMissed (READER): "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_REQUESTED_DEADLINE_MISSED,
                   v_readerStatus(s)->deadlineMissed.totalCount,
                   v_readerStatus(s)->deadlineMissed.totalChanged);
}

void
v_statusNotifyOfferedIncompatibleQos(
    v_status s,
    v_policyId id)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    s->state |= V_EVENT_OFFERED_INCOMPATIBLE_QOS;

    assert(v_objectKind(s) == K_WRITERSTATUS);
    v_writerStatus(s)->incompatibleQos.totalCount++;
    v_writerStatus(s)->incompatibleQos.totalChanged++;
    v_writerStatus(s)->incompatibleQos.lastPolicyId = id;
    v_writerStatus(s)->incompatibleQos.policyCount[id]++;

    _TRACE_EVENTS_("v_statusNotifyOfferedIncompatibleQos (WRITER): "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_OFFERED_INCOMPATIBLE_QOS,
                   v_writerStatus(s)->incompatibleQos.totalCount,
                   v_writerStatus(s)->incompatibleQos.totalChanged);
}

void
v_statusNotifyRequestedIncompatibleQos(
    v_status s,
    v_policyId id)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    s->state |= V_EVENT_REQUESTED_INCOMPATIBLE_QOS;

    assert(v_objectKind(s) == K_READERSTATUS);
    v_readerStatus(s)->incompatibleQos.totalCount++;
    v_readerStatus(s)->incompatibleQos.totalChanged++;
    v_readerStatus(s)->incompatibleQos.lastPolicyId = id;
    v_readerStatus(s)->incompatibleQos.policyCount[id]++;

    _TRACE_EVENTS_("v_statusNotifyRequestedIncompatibleQos (READER): "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_REQUESTED_INCOMPATIBLE_QOS,
                   v_readerStatus(s)->incompatibleQos.totalCount,
                   v_readerStatus(s)->incompatibleQos.totalChanged);
}

void
v_statusNotifyPublicationMatched(
    v_status s,
    v_gid instanceHandle,
    c_bool   dispose)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_writerStatus));

    s->state |= V_EVENT_PUBLICATION_MATCHED;

    if(dispose)
    {
        v_writerStatus(s)->publicationMatch.currentCount--;
        v_writerStatus(s)->publicationMatch.currentChanged++; /* NB: negative changes also increase this variable */
    }
    else
    {
        v_writerStatus(s)->publicationMatch.totalCount++;
        v_writerStatus(s)->publicationMatch.totalChanged++;
        v_writerStatus(s)->publicationMatch.currentCount++;
        v_writerStatus(s)->publicationMatch.currentChanged++;
    }
    v_writerStatus(s)->publicationMatch.instanceHandle = instanceHandle;

    _TRACE_EVENTS_("v_statusNotifyPublicationMatched: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_PUBLICATION_MATCHED,
                   v_writerStatus(s)->publicationMatch.totalCount,
                   v_writerStatus(s)->publicationMatch.totalChanged);
}


void
v_statusNotifySubscriptionMatched(
    v_status s,
    v_gid    instanceHandle,
    c_bool   dispose)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    s->state |= V_EVENT_SUBSCRIPTION_MATCHED;

    if(dispose)
    {
        v_readerStatus(s)->subscriptionMatch.currentCount--;
        v_readerStatus(s)->subscriptionMatch.currentChanged++; /* NB: negative changes also increase this variable */
    }
    else
    {
        v_readerStatus(s)->subscriptionMatch.currentCount++;
        v_readerStatus(s)->subscriptionMatch.currentChanged++;
        v_readerStatus(s)->subscriptionMatch.totalCount++;
        v_readerStatus(s)->subscriptionMatch.totalChanged++;
    }
    v_readerStatus(s)->subscriptionMatch.instanceHandle = instanceHandle;

    _TRACE_EVENTS_("v_statusNotifySubscriptionMatched: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_SUBSCRIPTION_MATCHED,
                   v_readerStatus(s)->subscriptionMatch.totalCount,
                   v_readerStatus(s)->subscriptionMatch.totalChanged);
}

void
v_statusNotifyLivelinessChanged(
    v_status s,
    c_long activeInc,
    c_long inactiveInc,
    v_gid instanceHandle)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    s->state |= V_EVENT_LIVELINESS_CHANGED;

    v_readerStatus(s)->livelinessChanged.activeCount += activeInc;
    v_readerStatus(s)->livelinessChanged.activeChanged += abs(activeInc);
    v_readerStatus(s)->livelinessChanged.inactiveCount += inactiveInc;
    v_readerStatus(s)->livelinessChanged.inactiveChanged += abs(inactiveInc);
    v_readerStatus(s)->livelinessChanged.instanceHandle = instanceHandle;

    _TRACE_EVENTS_("v_statusNotifyLivelinessChanged: "
                   "status = 0x%x, event = 0x%x, "
                   "activeCount = %d, activeChanged = %d\n",
                   s, V_EVENT_LIVELINESS_CHANGED,
                   v_readerStatus(s)->livelinessChanged.activeCount,
                   v_readerStatus(s)->livelinessChanged.activeChanged);
}

void
v_statusNotifySampleRejected(
    v_status s,
    v_sampleRejectedKind kind,
    v_gid instanceHandle)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    s->state |= V_EVENT_SAMPLE_REJECTED;

    v_readerStatus(s)->sampleRejected.totalCount++;
    v_readerStatus(s)->sampleRejected.totalChanged++;
    v_readerStatus(s)->sampleRejected.lastReason = kind;
    v_readerStatus(s)->sampleRejected.instanceHandle = instanceHandle;

    _TRACE_EVENTS_("v_statusNotifySampleRejected: "
                   "status = 0x%x, event = 0x%x, "
                   "totalCount = %d, totalChanged = %d\n",
                   s, V_EVENT_SAMPLE_REJECTED,
                   v_readerStatus(s)->sampleRejected.totalCount,
                   v_readerStatus(s)->sampleRejected.totalChanged);
}

void
v_statusReset(
    v_status s,
    c_ulong mask)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    _TRACE_STATE_("v_statusReset: status = 0x%x, "
                   "state = 0x%x, mask =0x%x\n",
                   s, s->state, mask);
    s->state &= ~mask;
}

void
v_statusResetCounters(
    v_status s,
    c_ulong mask)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    _TRACE_STATE_("v_statusResetCounters: status = 0x%x, "
                       "state = 0x%x, mask =0x%x\n",
                       s, s->state, mask);

    if (mask & V_EVENT_SAMPLE_REJECTED) {
        v_readerStatus(s)->sampleRejected.totalChanged = 0;
    }
    if (mask & V_EVENT_LIVELINESS_CHANGED) {
        v_readerStatus(s)->livelinessChanged.activeChanged = 0;
        v_readerStatus(s)->livelinessChanged.inactiveChanged = 0;
    }
    if (mask & V_EVENT_PUBLICATION_MATCHED) {
        v_writerStatus(s)->publicationMatch.totalChanged = 0;
        v_writerStatus(s)->publicationMatch.currentChanged = 0;
    }
    if (mask & V_EVENT_SUBSCRIPTION_MATCHED) {
        v_readerStatus(s)->subscriptionMatch.currentChanged = 0;
        v_readerStatus(s)->subscriptionMatch.totalChanged = 0;
    }
    if (mask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
        v_writerStatus(s)->incompatibleQos.totalChanged = 0;
    }
    if (mask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
        v_readerStatus(s)->incompatibleQos.totalChanged = 0;
    }
    if (mask & V_EVENT_OFFERED_DEADLINE_MISSED) {
        v_writerStatus(s)->deadlineMissed.totalChanged = 0;
    }
    if (mask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
        v_readerStatus(s)->deadlineMissed.totalChanged = 0;
    }
    if (mask & V_EVENT_LIVELINESS_LOST) {
        v_writerStatus(s)->livelinessLost.totalChanged = 0;
    }
    if (mask & V_EVENT_SAMPLE_LOST) {
        v_readerStatus(s)->sampleLost.totalChanged = 0;
    }
    if (mask & V_EVENT_INCONSISTENT_TOPIC) {
        v_topicStatus(s)->inconsistentTopic.totalChanged = 0;
    }
    if (mask & V_EVENT_ALL_DATA_DISPOSED) {
        v_topicStatus(s)->allDataDisposed.totalChanged = 0;
    }
}

c_ulong
v_statusGetMask(
    v_status s)
{
    c_ulong mask;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    mask = s->state;
    _TRACE_EVENTS_("v_statusGetMask: status = 0x%x, "
                   "state = 0x%x\n", s, s->state);

    return mask;
}

v_status
v_statusCopyOut(
    v_status s)
{
    v_status copy = NULL;
    v_kernel kernel = v_objectKernel(s);

    switch (v_objectKind(s)) {
    case K_KERNELSTATUS:
        copy = v_status(v_objectNew(kernel,K_KERNELSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_kernelStatus)));
    break;
    case K_TOPICSTATUS:
        copy = v_status(v_objectNew(kernel,K_TOPICSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_topicStatus)));
    break;
    case K_DOMAINSTATUS:
        copy = v_status(v_objectNew(kernel,K_DOMAINSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_partitionStatus)));
    break;
    case K_WRITERSTATUS:
        copy = v_status(v_objectNew(kernel,K_WRITERSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_writerStatus)));
    break;
    case K_READERSTATUS:
        copy = v_status(v_objectNew(kernel,K_READERSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_readerStatus)));
    break;
    case K_PARTICIPANTSTATUS:
        copy = v_status(v_objectNew(kernel,K_PARTICIPANTSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_status)));
    break;
    case K_SUBSCRIBERSTATUS:
        copy = v_status(v_objectNew(kernel,K_SUBSCRIBERSTATUS));
        memcpy(copy, s, sizeof(C_STRUCT(v_status)));
    break;
    case K_PUBLISHERSTATUS:
        copy = v_status(v_objectNew(kernel,K_PUBLISHERSTATUS));
        /* These status are just instantations of v_status and have no
         * addition attributes! */
        memcpy(copy, s, sizeof(C_STRUCT(v_status)));
    break;
    default:
        OS_REPORT(OS_CRITICAL,
                    "v_statusCopyOut", V_RESULT_ILL_PARAM,
                    "Unknown object kind %d",
                    v_objectKind(s));
    break;
    }
    return copy;
}
