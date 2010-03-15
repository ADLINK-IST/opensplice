/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "os_report.h"

#include "v__lease.h"
#include "v__leaseManager.h"
#include "v_time.h"
#include "v_event.h"
#include "v_entity.h"
#include "v_public.h"
#include "v__writer.h" /* for resending action */
#include "v__group.h" /* for resending action */
#include "v__dataReader.h" /* for deadline missed */
#include "v_serviceState.h"
#include "v__spliced.h" /* for heartbeat */


/* WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!
 *
 * The lease itself may never take its lock during the execution of
 * the action routine. The entity of the lease has the responsability
 * that the entity is not locked, while changing the lease during
 * the lease action.
 *
 * WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!
 */

/**************************************************************
 * Private functions
 **************************************************************/
static void
serviceStateExpired(
    v_lease lease)
{
    v_object o;
    v_handleResult r;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    r = v_handleClaim(lease->object, &o);
    if (r == V_HANDLE_OK) {
        if (o->kind == K_SERVICESTATE) {
            v_serviceStateChangeState(v_serviceState(o), STATE_DIED);
        } else {
            OS_REPORT_1(OS_WARNING, "v_lease", 0,
                        "Lease action on unexpected object type: %d", o->kind);
        }
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
    } /* else just skip, since entity is already gone */
}

static void
writerResend(
    v_lease lease)
{
    v_object writer;
    v_handleResult r;

    assert(v_lease(lease) != NULL);

    r = v_handleClaim(lease->object, &writer);
    if (r == V_HANDLE_OK) {
        v_writerResend(v_writer(writer));
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
    }
}

static void
readerDeadlineMissed(
    v_lease lease,
    c_time now)
{
    v_object o;
    v_handleResult r;

    assert(v_lease(lease) != NULL);

    r = v_handleClaim(lease->object, &o);
    if (r == V_HANDLE_OK) {
        v_dataReaderCheckDeadlineMissed(v_dataReader(o), now);
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
   }
}

static void
writerDeadlineMissed(
    v_lease lease,
    c_time now)
{
    v_object w;
    v_handleResult r;

    assert(v_lease(lease) != NULL);

    r = v_handleClaim(lease->object, &w);
    if (r == V_HANDLE_OK) {
        v_writerCheckDeadlineMissed(v_writer(w), now);
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
    }
}

static void
livelinessCheck(
    v_lease lease)
{
    v_object public;
    v_handleResult r;

    assert(v_lease(lease) != NULL);

    /* Liveliness lease expired, so the reader/writer must be notified! */
    r = v_handleClaim(lease->object, &public);
    if (r == V_HANDLE_OK) {
        v_writerNotifyLivelinessLost(v_writer(public));
        if (v_objectKind(public) != K_WRITER) {
            OS_REPORT_1(OS_WARNING, "v_lease", 0,
                        "entity %d has no liveliness policy",
                        v_objectKind(public));
        }
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
    } else {
        /* Corresponding reader/writer is already gone, so remove this lease
         * from its leasemanager.
         */
        v_leaseManagerDeregister(v_leaseManager(lease->leaseManager), lease);
    }
}

static void
heartbeatSend(
    v_lease lease)
{
    v_object sd;
    v_handleResult r;

    assert(v_lease(lease) != NULL);

    r = v_handleClaim(lease->object, &sd);
    if (r == V_HANDLE_OK) {
        v_splicedHeartbeat(v_spliced(sd));
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
    } else {
        OS_REPORT(OS_ERROR, "heartbeatSend", 0,
            "Could not claim the splicedaemon!");
    }
}

static void
heartbeatCheck(
    v_lease lease)
{
    v_object sd;
    v_handleResult r;

    r = v_handleClaim(lease->object, &sd);
    if (r == V_HANDLE_OK) {
        v_splicedCheckHeartbeats(v_spliced(sd));
        r = v_handleRelease(lease->object);
        assert(r == V_HANDLE_OK);
    } else {
        OS_REPORT(OS_ERROR, "heartbeatSend", 0,
            "Could not claim the splicedaemon!");
    }
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_lease
v_leaseNew(
    v_leaseManager  leaseManager,
    v_public        p,
    v_duration      leaseDuration,
    v_leaseActionId actionId,
    c_long          repeatCount)
{
    v_lease lease;

    assert(leaseManager != NULL);
    assert(C_TYPECHECK(leaseManager, v_leaseManager));
    assert(p != NULL);
    assert(C_TYPECHECK(p, v_public));

    lease = v_lease(v_objectNew(v_objectKernel(leaseManager), K_LEASE));
    if (lease != NULL) {
        c_mutexInit(&lease->mutex,SHARED_MUTEX);
        lease->expiryTime = c_timeAdd(v_timeGet(), leaseDuration);
        lease->duration = leaseDuration;
        lease->object = v_publicHandle(p);
        lease->leaseManager = leaseManager;
        lease->actionId = actionId;
        lease->repeatCount = repeatCount;
    }
    return lease;
}

void
v_leaseDeinit(
    v_lease lease)
{
    assert(lease != NULL);
    assert(C_TYPECHECK(lease,v_lease));

    if (lease != NULL) {
        lease->leaseManager = NULL;
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/
void
v_leaseRenew(
    v_lease lease,
    v_duration leaseDuration)
{
    if (lease != NULL) {
        assert(C_TYPECHECK(lease, v_lease));

        c_mutexLock(&lease->mutex);
        lease->expiryTime = c_timeAdd(v_timeGet(), leaseDuration);
        lease->duration = leaseDuration;
        c_mutexUnlock(&lease->mutex);

        v_leaseManagerNotify(v_leaseManager(lease->leaseManager),
                             lease, V_EVENT_LEASE_RENEWED);
    }
}

void
v_leaseUpdate(
    v_lease lease)
{
    if (lease != NULL) {
        assert(C_TYPECHECK(lease, v_lease));

        c_mutexLock(&lease->mutex);
        lease->expiryTime = c_timeAdd(v_timeGet(), lease->duration);
        c_mutexUnlock(&lease->mutex);
    }
}

void
v_leaseGetExpiryAndDuration(
    v_lease lease,
    c_time *expiryTime,
    v_duration *duration)
{
    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    if(expiryTime || duration){
        c_mutexLock(&lease->mutex);
        if(expiryTime){
            *expiryTime = lease->expiryTime;
        }
        if(duration){
            *duration = lease->duration;
        }
        c_mutexUnlock(&lease->mutex);
    }
}

c_time
v_leaseExpiryTime(
    v_lease lease)
{
    c_time expTime;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    c_mutexLock(&lease->mutex);
    expTime = lease->expiryTime;
    c_mutexUnlock(&lease->mutex);

    return expTime;
}

void
v_leaseAction(
    v_lease lease,
    c_time now)
{
    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    switch (lease->actionId) {
    case V_LEASEACTION_SERVICESTATE_EXPIRED:
        serviceStateExpired(lease);
    break;
    case V_LEASEACTION_WRITER_RESEND:
        writerResend(lease);
    break;
    case V_LEASEACTION_READER_DEADLINE_MISSED:
        readerDeadlineMissed(lease, now);
    break;
    case V_LEASEACTION_WRITER_DEADLINE_MISSED:
        writerDeadlineMissed(lease, now);
    break;
    case V_LEASEACTION_LIVELINESS_CHECK:
        livelinessCheck(lease);
    break;
    case V_LEASEACTION_HEARTBEAT_SEND:
        heartbeatSend(lease);
    break;
    case V_LEASEACTION_HEARTBEAT_CHECK:
        heartbeatCheck(lease);
    break;
    default:
        OS_REPORT_1(OS_WARNING, "v_leaseAction", 0,
                    "Unknown lease action %d", lease->actionId);
        /* lets remove lease from lease manager to prevent future wakeups of lease manager. */
        v_leaseManagerDeregister(v_leaseManager(lease->leaseManager), lease);
    break;
    }
}

c_long
v_leaseDecRepeatCount(
    v_lease lease)
{
    c_long rc;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    c_mutexLock(&lease->mutex);
    if (lease->repeatCount > 0) {
        lease->repeatCount--;
    }
    rc = lease->repeatCount;
    c_mutexUnlock(&lease->mutex);

    return rc;
}

c_long
v_leaseRepeatCount(
    v_lease lease)
{
    c_long rc;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    c_mutexLock(&lease->mutex);
    rc = lease->repeatCount;
    c_mutexUnlock(&lease->mutex);

    return rc;
}

/**************************************************************
 * Public functions
 **************************************************************/
