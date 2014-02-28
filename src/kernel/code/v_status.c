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

#include "v_status.h"
#include "v__status.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_event.h"
#include "v_writerInstance.h"

#include "os_report.h"

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
    c_type type;

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
        type = c_long_t(c_getBase(c_object(s)));
        v_writerStatus(s)->incompatibleQos.policyCount = c_arrayNew(type, V_POLICY_ID_COUNT);
        c_free(type);
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
        type = c_long_t(c_getBase(c_object(s)));
        v_readerStatus(s)->incompatibleQos.policyCount = c_arrayNew(type, V_POLICY_ID_COUNT);
        c_free(type);
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
        OS_REPORT_1(OS_ERROR,
                    "v_statusInit", 0,
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

c_bool
v_statusNotifyInconsistentTopic(
    v_status s)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_topicStatus));

    if (s->state & V_EVENT_INCONSISTENT_TOPIC) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_INCONSISTENT_TOPIC;
        changed = TRUE;
    }

    v_topicStatus(s)->inconsistentTopic.totalCount++;
    v_topicStatus(s)->inconsistentTopic.totalChanged++;

    return changed;
}

c_bool
v_statusNotifyAllDataDisposed(
    v_status s)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_topicStatus));

    if (s->state & V_EVENT_ALL_DATA_DISPOSED) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_ALL_DATA_DISPOSED;
        changed = TRUE;
    }

    v_topicStatus(s)->allDataDisposed.totalCount++;
    v_topicStatus(s)->allDataDisposed.totalChanged++;

    return changed;
}

c_bool
v_statusNotifyDataAvailable(
    v_status s)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus) || C_TYPECHECK(s,v_subscriberStatus));

    if (s->state & V_EVENT_DATA_AVAILABLE) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_DATA_AVAILABLE;
        changed = TRUE;
    }

    return changed;
}

c_bool
v_statusNotifySampleLost(
    v_status s,
    c_ulong nrSamplesLost)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    if (s->state & V_EVENT_SAMPLE_LOST) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_SAMPLE_LOST;
        changed = TRUE;
    }

    v_readerStatus(s)->sampleLost.totalCount += nrSamplesLost;
    v_readerStatus(s)->sampleLost.totalChanged += nrSamplesLost;
    return changed;
}

c_bool
v_statusNotifyLivelinessLost(
    v_status s)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_writerStatus));

    if (s->state & V_EVENT_LIVELINESS_LOST) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_LIVELINESS_LOST;
        changed = TRUE;
    }

    v_writerStatus(s)->livelinessLost.totalCount++;
    v_writerStatus(s)->livelinessLost.totalChanged++;

    return changed;
}

c_bool
v_statusNotifyDeadlineMissed(
    v_status s,
    v_handle instanceHandle)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    if (s->state & V_EVENT_DEADLINE_MISSED) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_DEADLINE_MISSED;
        changed = TRUE;
    }

    switch(v_objectKind(s)) {
    case K_WRITERSTATUS:
        v_writerStatus(s)->deadlineMissed.totalCount++;
        v_writerStatus(s)->deadlineMissed.totalChanged++;
        v_writerStatus(s)->deadlineMissed.instanceHandle = instanceHandle;
    break;
    case K_READERSTATUS:
        v_readerStatus(s)->deadlineMissed.totalCount++;
        v_readerStatus(s)->deadlineMissed.totalChanged++;
        v_readerStatus(s)->deadlineMissed.instanceHandle = instanceHandle;
    break;
    default:
        assert(FALSE);
    break;
    }

    return changed;
}

c_bool
v_statusNotifyIncompatibleQos(
    v_status s,
    v_policyId id)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    if (s->state & V_EVENT_INCOMPATIBLE_QOS) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_INCOMPATIBLE_QOS;
        changed = TRUE;
    }

    switch(v_objectKind(s)) {
    case K_WRITERSTATUS:
        v_writerStatus(s)->incompatibleQos.totalCount++;
        v_writerStatus(s)->incompatibleQos.totalChanged++;
        v_writerStatus(s)->incompatibleQos.lastPolicyId = id;
        ((c_long *)(v_writerStatus(s)->incompatibleQos.policyCount))[id]++;
    break;
    case K_READERSTATUS:
        v_readerStatus(s)->incompatibleQos.totalCount++;
        v_readerStatus(s)->incompatibleQos.totalChanged++;
        v_readerStatus(s)->incompatibleQos.lastPolicyId = id;
        ((c_long *)(v_readerStatus(s)->incompatibleQos.policyCount))[id]++;
    break;
    default:
        assert(FALSE);
    break;
    }

    return changed;
}

c_bool
v_statusNotifyPublicationMatched(
    v_status s,
    v_gid instanceHandle,
    c_bool   dispose)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_writerStatus));

    if (s->state & V_EVENT_TOPIC_MATCHED) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_TOPIC_MATCHED;
        changed = TRUE;
    }

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

    return changed;
}


c_bool
v_statusNotifySubscriptionMatched(
    v_status s,
    v_gid    instanceHandle,
    c_bool   dispose)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    if (s->state & V_EVENT_TOPIC_MATCHED) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_TOPIC_MATCHED;
        changed = TRUE;
    }

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

    return changed;
}
c_bool
v_statusNotifyLivelinessChanged(
    v_status s,
    c_long activeInc,
    c_long inactiveInc,
    v_gid instanceHandle)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    if (s->state & V_EVENT_LIVELINESS_CHANGED) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_LIVELINESS_CHANGED;
        changed = TRUE;
    }

    v_readerStatus(s)->livelinessChanged.activeCount += activeInc;
    v_readerStatus(s)->livelinessChanged.activeChanged += abs(activeInc);
    v_readerStatus(s)->livelinessChanged.inactiveCount += inactiveInc;
    v_readerStatus(s)->livelinessChanged.inactiveChanged += abs(inactiveInc);
    v_readerStatus(s)->livelinessChanged.instanceHandle = instanceHandle;

    return changed;
}

c_bool
v_statusNotifySampleRejected(
    v_status s,
    v_sampleRejectedKind kind,
    v_gid instanceHandle)
{
    c_bool changed;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_readerStatus));

    if (s->state & V_EVENT_SAMPLE_REJECTED) {
        changed = FALSE;
    } else {
        s->state |= V_EVENT_SAMPLE_REJECTED;
        changed = TRUE;
    }

    v_readerStatus(s)->sampleRejected.totalCount++;
    v_readerStatus(s)->sampleRejected.totalChanged++;
    v_readerStatus(s)->sampleRejected.lastReason = kind;
    v_readerStatus(s)->sampleRejected.instanceHandle = instanceHandle;

    return changed;
}

v_statusResult
v_statusReset(
    v_status s,
    c_ulong mask)
{
    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    s->state &= ~mask;

    return STATUS_RESULT_SUCCESS;
}

c_ulong
v_statusGetMask(
    v_status s)
{
    c_ulong mask;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_status));

    mask = s->state;

    return mask;
}
