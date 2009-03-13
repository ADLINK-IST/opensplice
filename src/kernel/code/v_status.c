
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
    break;
    case K_SUBSCRIBERSTATUS:
    break;
    case K_DOMAINSTATUS:
        v_domainStatus(s)->groupsChanged.totalCount = 0;
        v_domainStatus(s)->groupsChanged.totalChanged = 0;
    break;
    case K_WRITERSTATUS:
        v_writerStatus(s)->livelinessLost.totalCount = 0;
        v_writerStatus(s)->livelinessLost.totalChanged = 0;
        v_writerStatus(s)->deadlineMissed.totalCount = 0;
        v_writerStatus(s)->deadlineMissed.totalChanged = 0;
        v_gidSetNil(v_writerStatus(s)->deadlineMissed.instanceHandle);
        v_writerStatus(s)->incompatibleQos.totalCount = 0;
        v_writerStatus(s)->incompatibleQos.totalChanged = 0;
        v_writerStatus(s)->incompatibleQos.lastPolicyId = V_UNKNOWN_POLICY_ID;
        type = c_long_t(c_getBase(c_object(s)));
        v_writerStatus(s)->incompatibleQos.policyCount = c_arrayNew(type, V_POLICY_ID_COUNT);
        c_free(type);
        v_writerStatus(s)->publicationMatch.totalCount = 0;
        v_writerStatus(s)->publicationMatch.totalChanged = 0;
        v_writerStatus(s)->publicationMatch.instanceHandle = NULL;
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
        v_gidSetNil(v_readerStatus(s)->deadlineMissed.instanceHandle);
        v_readerStatus(s)->incompatibleQos.totalCount = 0;
        v_readerStatus(s)->incompatibleQos.totalChanged = 0;
        v_readerStatus(s)->incompatibleQos.lastPolicyId = V_UNKNOWN_POLICY_ID;
        type = c_long_t(c_getBase(c_object(s)));
        v_readerStatus(s)->incompatibleQos.policyCount = c_arrayNew(type, V_POLICY_ID_COUNT);
        c_free(type);
        v_readerStatus(s)->subscriptionMatch.totalCount = 0;
        v_readerStatus(s)->subscriptionMatch.totalChanged = 0;
        v_readerStatus(s)->subscriptionMatch.instanceHandle = NULL;
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
    v_status s)
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

    v_readerStatus(s)->sampleLost.totalCount++;
    v_readerStatus(s)->sampleLost.totalChanged++;

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
    c_object h)
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
        v_writerStatus(s)->deadlineMissed.instanceHandle = v_publicGid(v_public(h));
    break;
    case K_READERSTATUS:
        v_readerStatus(s)->deadlineMissed.totalCount++;
        v_readerStatus(s)->deadlineMissed.totalChanged++;
        v_readerStatus(s)->deadlineMissed.instanceHandle = v_publicGid(v_public(h));
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
