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

#include "v__leaseManager.h"
#include "v__lease.h"

#include "v_public.h"

#include "v_serviceState.h"
#include "v__dataReader.h"
#include "v__writer.h"
#include "v__spliced.h"
#include "v__processInfo.h"
#include "v__kernel.h"

#include "vortex_os.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_process.h"

/**
 * \brief The <code>v_leaseAction</code> cast method.
 *
 * This method casts an object to a <code>v_leaseAction</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_leaseAction</code> or
 * one of its subclasses.
 */
#define v_leaseAction(o) (C_CAST(o,v_leaseAction))

/**************************************************************
 * Private function/struct declarations
 **************************************************************/

/**
 * \brief Initializes the v_leaseManager object.
 *
 * \param _this The leaseManager to use within this function.
 */
static void
v_leaseManagerInit(
    v_leaseManager _this);

/**
 * \brief Deinitializes the v_leaseManager object.
 *
 * \param _this The leaseManager to use within this function.
 */
static void
v_leaseManagerDeinit(
    v_leaseManager _this);

/**
 * \brief Walk function to find a leaseAction object containing
 * a lease that matches the lease passed in arg, in the set of
 * leases managed by a lease manager.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg A pointer to a 'findLeaseActionArg' struct with the lease object to search for.
 *
 * \return FALSE if a matching leaseAction object is found, TRUE if not
 */
static c_bool
findLeaseAction(
    c_object o,
    c_voidp arg);

struct findLeaseActionArg {
    v_leaseAction action;
    v_lease lease;
};


/**
 * \brief Walk function to find all expired leases in the set of leases
 * managed by a lease manager.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg A pointer to a 'collectExpiredArg' struct containing relevant information
 * and to store the collected data in.
 *
 * \return TRUE in all cases, such that all leases in the set are considered
 */
static c_bool
collectExpired(
    c_object o,
    c_voidp arg);


struct collectExpiredArg {
    c_iter expiredLeases;
    v_leaseTime expiryTime;
    os_duration shortestPeriod;
};

/**
 * \brief Walk function to determine which of the leases managed by a lease manager
 * expires the first.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg Pointer to a os_timeM struct to hold the earliest expiry time
 */
static c_bool
calculateExpiryTime(
    c_object o,
    c_voidp arg);

/**
 * \brief Called when a lease has expired so that the appropiate action can be
 * executed for the expired lease.
 *
 * \param _this The leaseManager object to operate on.
 * \param leaseAction The leaseAction object representing the expired lease.
 * \param the current time, used within some of the action routines.
 */
static void
v_leaseManagerProcessLeaseAction(
    v_leaseManager _this,
    v_leaseAction leaseAction,
    v_leaseTime now_el);

/**
 * \brief Walk function to find all leases that should be immediately
 * processed in case of unexpected spliced termination. This is the
 * case for leases with a SPLICED_DEATH_DETECTED or SERVICESTATE_EXPIRED
 * lease action ID.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg NULL
 *
 * \return TRUE in all cases, such that all leases in the set are considered
 */
static c_bool
splicedIsDead(
    c_object o,
    c_voidp arg);

/* Lease expiry action routines */

/**
 * \brief Lease action routine executed when a service state expires.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 */
static void
serviceStateExpired(
    v_leaseAction leaseAction);

/**
 * \brief Lease action routine executed when a lease with the
 * SPICED_DEATH_DETECTED action expires.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 */
static void
splicedDeathDetected(
    v_leaseAction leaseAction);

static void
minSepTimeWindowExpiry(
    v_leaseAction leaseAction,
    v_leaseTime now_el);

/**
 * \brief Lease action routine executed when a deadline is missed by a reader.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 * \param now The current time.
 */
static void
readerDeadlineMissed(
    v_leaseAction leaseAction,
    v_leaseTime now_el);

/**
 * \brief Lease action routine executed when a deadline is missed by a writer.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 * \param now The current time.
 */
static void
writerDeadlineMissed(
    v_leaseAction leaseAction,
    v_leaseTime now_el);

/**
 * \brief Lease action routine executed when a writer loses liveliness.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 */
static void
writerLivelinessLost(
    v_leaseAction leaseAction);

/**
 * \brief Lease action routine executed when the splicedeamon needs to send a
 * heartbeat.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 */
static void
heartbeatSend(
    v_leaseAction leaseAction);

/**
 * \brief Lease action routine executed when the splicedeamon needs to receive
 * a heartbeat from a remote splicedaemon.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 * actionObject used within this action routine.
 */
static void
heartbeatCheck(
    v_leaseAction leaseAction);



/**************************************************************
 * constructor/destructor
 **************************************************************/

v_leaseManager
v_leaseManagerNew(
    v_kernel k)
{
    v_leaseManager _this;

    assert(C_TYPECHECK(k, v_kernel));

    _this = v_leaseManager(v_objectNew(k, K_LEASEMANAGER));
    if(_this) {
        v_leaseManagerInit(_this);
    } else {
        OS_REPORT(OS_ERROR, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
            "Failed to create a v_leaseManager object. "
            "Most likely not enough shared memory available "
            "to complete the operation.");
    }

    return _this;
}

static void
v_leaseManagerInit(
    v_leaseManager _this)
{
    v_kernel k;

    assert(C_TYPECHECK(_this, v_leaseManager));

    k = v_objectKernel(_this);
    (void)c_mutexInit(c_getBase(_this), &_this->mutex);
    c_condInit(c_getBase(_this), &_this->cond, &_this->mutex);
    _this->quit = FALSE;
    _this->monotonic.nextExpiryTime = v_leaseTimeInfinite(V_LEASE_KIND_MONOTONIC);
    _this->monotonic.leases = c_setNew(v_kernelType(k, K_LEASEACTION));
    _this->elapsed.nextExpiryTime = v_leaseTimeInfinite(V_LEASE_KIND_ELAPSED);
    _this->elapsed.leases = c_setNew(v_kernelType(k, K_LEASEACTION));
}

void
v_leaseManagerFree(
    v_leaseManager _this)
{
    if (_this != NULL) {
        assert(C_TYPECHECK(_this, v_leaseManager));
        v_leaseManagerDeinit(_this);
        c_free(_this);
    }
}

static void
v_leaseManagerLeaseAdminDeinit(
    v_leaseManager _this,
    v_leaseAdmin *set)
{
    v_leaseAction lease;
    c_bool removed;

    lease = v_leaseAction(c_take(set->leases));
    while (lease != NULL) {
        /* Unregister self from the lease, we are no longer an observer */
        v_leaseLock(lease->lease);
        removed = v_leaseRemoveObserverNoLock(lease->lease, _this);
        v_leaseUnlock(lease->lease);
        if (!removed) {
            OS_REPORT(OS_CRITICAL,
                "v_leaseManagerDeinit",V_RESULT_INTERNAL_ERROR,
                "Failed to remove leaseManager %p from the list of "
                "observers of lease %p, while the lease WAS contained in "
                "the list of leases managed by the leaseManager. This means "
                "the administration has become inconsistent internally. "
                "This is not a fatal error in itself, but points towards "
                "a bug that could affect behaviour of OpenSpliceDDS",
                (void*)_this,
                (void*)lease);
        } /* else everything is ok */

        c_free(lease);
        lease = v_leaseAction(c_take(set->leases));
    }
    c_free(set->leases);
    set->leases = NULL;
}


static void
v_leaseManagerDeinit(
    v_leaseManager _this)
{
    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);
    v_leaseManagerLeaseAdminDeinit(_this, &_this->monotonic);
    v_leaseManagerLeaseAdminDeinit(_this, &_this->elapsed);
    _this->quit = TRUE;
    c_condBroadcast(&_this->cond);
    c_mutexUnlock(&_this->mutex);

    /* Note the condition _this->cond is deinitalised via c_free */
}

/**************************************************************
 * register/deregister of leases
 **************************************************************/

v_result
v_leaseManagerRegister(
    v_leaseManager  _this,
    v_lease         lease,
    v_leaseActionId actionId,
    v_public        actionObject,
    c_bool          repeatLease)
{
    v_leaseAction leaseAction, foundLeaseAction;
    v_result result;
    v_kernel k;
    c_bool observerAdded;
    v_leaseAdmin *leaseAdmin = NULL;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));
    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));
    assert(actionObject != NULL);
    assert(C_TYPECHECK(actionObject, v_public));

    result = V_RESULT_OK;

    /* Step 1: Create a lease action object. This object will contain the relevant
     * information needed when reacting to an expired lease. This action information
     * can be different depending on which lease manager a lease is being registered
     * to, hence why the leaseAction object resides at leaseManager level and not
     * at lease level as it did in the past
     */
    k = v_objectKernel(_this);
    leaseAction = v_leaseAction(v_objectNew(k, K_LEASEACTION));
    if(!leaseAction) {
        result = V_RESULT_OUT_OF_MEMORY;
        OS_REPORT(OS_ERROR, "v_leaseManagerRegister", result,
            "Failed to create a v_leaseAction object. "
            "Most likely not enough resources available to "
            "complete the operation.");
    } else {
        leaseAction->lease = v_lease(c_keep(lease));
        assert(leaseAction->lease);
        leaseAction->actionId = actionId;
        leaseAction->actionObject = v_publicHandle(actionObject);
        leaseAction->repeat = repeatLease;

        /* Step 2a: insert the leaseAction object into the set of leases. */
        c_mutexLock(&_this->mutex);
        if (v_leaseGetKind(lease) == V_LEASE_KIND_MONOTONIC) {
            leaseAdmin = &_this->monotonic;
        } else {
            leaseAdmin = &_this->elapsed;
        }
        foundLeaseAction = c_setInsert(leaseAdmin->leases, leaseAction);
        if(foundLeaseAction != leaseAction) {
            result = V_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR, "v_leaseManagerRegister", result,
                "Failed to insert the lease in the lease manager. "
                "Most likely not enough resources available to "
                "complete the operation.");
        } else {
            /* Step 2b: Now that the lease was successfully inserted into the lease manager,
             * we need to register the leaseManager as an observer of the lease to ensure that the
             * it is notified when the lease expiry time and/or duration is changed. To prevent the
             * lease time from changing while we evaluate the lease we will lock the lease object.
             */
            v_leaseLock(lease);
            observerAdded = v_leaseAddObserverNoLock(lease, _this);
            if (!observerAdded) {
                result = V_RESULT_INTERNAL_ERROR;
                OS_REPORT(OS_CRITICAL, "v_leaseManagerRegister", result,
                    "Failed to insert the lease manager as an observer of the lease. "
                    "Most likely not enough resources available to "
                    "complete the operation.");
                /* Remove the lease from the leaseManager */
                foundLeaseAction = c_setRemove(leaseAdmin->leases, leaseAction, NULL, NULL);
                if (foundLeaseAction != leaseAction) {
                    OS_REPORT(OS_ERROR, "v_leaseManagerRegister", V_RESULT_INTERNAL_ERROR,
                        "Failed to remove a lease from the lease manager");
                }
            }

            /* Step 3: If the newly registered lease expires before the
             * current nextExpiryTime of the lease manager, the nextExpiryTime
             * needs to be updated, to take into account the expiry of the
             * new leaseAction.
             */
            if((result == V_RESULT_OK) &&
               (v_leaseTimeCompare(lease->expiryTime, leaseAdmin->nextExpiryTime) == OS_LESS)) {
                leaseAdmin->nextExpiryTime = lease->expiryTime;
                c_condBroadcast(&_this->cond);
            }
            v_leaseUnlock(lease);
        }
        c_mutexUnlock(&_this->mutex);
    }
    c_free(leaseAction);
    return result;
}

void
v_leaseManagerDeregister(
    v_leaseManager _this,
    v_lease lease)
{
    struct findLeaseActionArg arg;
    v_leaseAction foundLeaseAction;
    v_leaseAdmin *leaseAdmin;
    c_bool removed;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    if (lease != NULL) {
        assert(C_TYPECHECK(lease, v_lease));
        /* Step 1: Get the leaseAction corresponding to the lease, from the set of leases */
        c_mutexLock(&_this->mutex);
        if (v_leaseGetKind(lease) == V_LEASE_KIND_MONOTONIC) {
            leaseAdmin = &_this->monotonic;
        } else {
            leaseAdmin = &_this->elapsed;
        }
        arg.lease = lease;
        arg.action = NULL;
        if (leaseAdmin) {
            (void)c_setWalk(leaseAdmin->leases, findLeaseAction, &arg);
        }
        if(arg.action) {
            /* step 2a: If the leaseAction object exists, remove it from the lease manager */
            foundLeaseAction = c_setRemove(leaseAdmin->leases, arg.action, NULL, NULL);
            assert(foundLeaseAction == arg.action);

            /* Step 2b: Unregister the lease manager as observer of the
             * lease.
             */
             v_leaseLock(lease);
             removed = v_leaseRemoveObserverNoLock(lease, _this);
             v_leaseUnlock(lease);
             if (!removed) {
                 OS_REPORT(OS_CRITICAL, "v_leaseManagerDeregister", V_RESULT_INTERNAL_ERROR,
                         "Failed to unregister lease manager %p as an observer of lease %p, "
                         "while the lease WAS contained in the set of leases managed by "
                         "this lease manager.",
                         (void*)_this, (void*)lease);
             }

             /* Note: There is no need to update the nextExpiryTime of the lease manager here.
              * Possibly the lease manager wakes too early (because the deregistered lease was the
              * one responsible for setting the nextExpiryTime). But if we do determine a new
              * 'nextExpiryTime' here, we would also need to wake the lease manager to have it
              * taken into account.
              */
             c_free(foundLeaseAction);
             c_free(arg.action);
        }
        c_mutexUnlock(&_this->mutex);
    }
}

static c_bool
findLeaseAction(
    c_object o,
    c_voidp arg)
{
    c_bool retVal;
    v_leaseAction action = v_leaseAction(o);
    struct findLeaseActionArg *a = (struct findLeaseActionArg *)arg;

    if (action && (action->lease == a->lease)) {
        a->action = c_keep(action);
        retVal = FALSE;
    } else {
        retVal = TRUE; /* not found, continue walk */
    }

    return retVal;
}

static c_bool
splicedIsDead(
    c_object o,
    c_voidp arg)
{
    v_leaseAction action = v_leaseAction(o);
    OS_UNUSED_ARG(arg);

    switch (action->actionId)
    {
        case V_LEASEACTION_SERVICESTATE_EXPIRED:
            serviceStateExpired(action);
        break;
        /* Found an action registered for the death of spliced. Perform it. */
        case V_LEASEACTION_SPLICED_DEATH_DETECTED:
            splicedDeathDetected(action);
        break;
        default:
        /* Do nothing for all other ids */
        break;
    }
    /* Keep going */
    return TRUE;
}

/**************************************************************
 * Main / notify functions
 **************************************************************/

static os_duration
v_leaseManagerEvaluateLeases(
    v_leaseManager _this,
    v_leaseAdmin *leaseAdmin,
    v_leaseKind kind,
    os_duration *shortestPeriod,
    os_duration *lag)
{
    v_leaseAction leaseAction;
    struct collectExpiredArg arg;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));
    assert(shortestPeriod);
    assert(lag);

    /* Collect expired leases */
    arg.expiredLeases = NULL;
    arg.expiryTime = v_leaseTimeGet(kind);
    arg.shortestPeriod = *shortestPeriod;

    if (leaseAdmin->leases) {
        (void)c_setWalk(leaseAdmin->leases, collectExpired, &arg);

        /* Process expired leases */
        c_mutexUnlock(&_this->mutex);

        leaseAction = v_leaseAction(c_iterTakeFirst(arg.expiredLeases));
        if(leaseAction){
            const v_leaseTime now = v_leaseTimeGet(kind);

            while (leaseAction != NULL) {
                /* Either renew or unregister the lease */

                if (leaseAction->repeat) {
                    v_leaseRenew(leaseAction->lease, OS_DURATION_INVALID);
                } else {
                    v_leaseManagerDeregister(_this, leaseAction->lease);
                }
                v_leaseManagerProcessLeaseAction(_this, leaseAction, now);
                c_free(leaseAction);
                leaseAction = v_leaseAction(c_iterTakeFirst(arg.expiredLeases));
            }
        }
        c_iterFree(arg.expiredLeases);

        /* Calculate next expiry time of lease manager */
        c_mutexLock(&_this->mutex);
        leaseAdmin->nextExpiryTime = v_leaseTimeInfinite(kind);
        (void)c_setWalk(leaseAdmin->leases, calculateExpiryTime, &leaseAdmin->nextExpiryTime);
    }

    if (arg.shortestPeriod != 0) {
         /* Current time is apparently > next expiry-time, so calculate
          * lag by subtracting nextExpiryTime from current time.
          */
         *lag = v_leaseTimeDiff(arg.expiryTime, leaseAdmin->nextExpiryTime);
    }

    *shortestPeriod = arg.shortestPeriod;

    return v_leaseTimeDiff(leaseAdmin->nextExpiryTime, v_leaseTimeGet(kind));
}


void
v_leaseManagerMain(
    v_leaseManager _this)
{
    os_duration tm, te, timeout;
    os_duration shortestPeriod, lag;
    v_result waitResult = V_RESULT_OK;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    shortestPeriod = 0;
    lag = OS_DURATION_INFINITE;

    c_mutexLock(&_this->mutex);

    while (_this->quit == FALSE) {
        te = v_leaseManagerEvaluateLeases(_this, &_this->elapsed, V_LEASE_KIND_ELAPSED, &shortestPeriod, &lag);
        tm = v_leaseManagerEvaluateLeases(_this, &_this->monotonic, V_LEASE_KIND_MONOTONIC, &shortestPeriod, &lag);

        timeout = (tm < te) ? tm : te;

        if (shortestPeriod != 0) {
             /* The next expiry-time lies in the past, log a warning and proceed
              * as normal. The first cycle, the leaseManager may be late as the
              * thread is started asynchronously and leases may have been added
              * before the thread is really started.
              */

             /* Only report warning if lag is more than the shortest period
              * of all registered leases. */
             if (lag > shortestPeriod) {
                 OS_REPORT(OS_WARNING, "v_leaseManagerMain", V_RESULT_OK,
                         "The lease manager did not wake up on time and missed the "
                         "expiry-time by %.09fs (smallest lease-duration is %.09fs). "
                         "This means there are likely one or more "
                         "leases that will not be processed in time. This could be due "
                         "to scheduling problems or clock alignment issues on multi-core "
                         "machines. The lease manager will continue to function normally.",
                         os_durationToReal(lag), os_durationToReal(shortestPeriod));
             }
        }

        shortestPeriod = OS_DURATION_INFINITE;

        if (timeout > 0) {
            waitResult = v_condWait(&_this->cond, &_this->mutex, timeout);
        }

        if (waitResult == V_RESULT_INTERNAL_ERROR) {
            OS_REPORT(OS_CRITICAL, "v_leaseManagerMain", V_RESULT_INTERNAL_ERROR,
                "v_condWait failed - memory sync no longer viable - "
                "probable cause is death of spliced.");
            (void)c_setWalk(_this->monotonic.leases, (c_action)splicedIsDead, NULL);
            break;
        }
    }
    _this->quit = FALSE;
    c_mutexUnlock(&_this->mutex);
}


c_bool
v_leaseManagerNotify(
    v_leaseManager _this,
    v_lease lease,
    v_eventKind event)
{
    struct findLeaseActionArg arg;
    v_leaseAdmin *leaseAdmin;
    v_leaseTime expiryTime;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);

    if (_this->quit == FALSE) {
        if (v_eventTest(event, V_EVENT_LEASE_RENEWED)) {
            /* Check if the lease is registered by this lease manager */
            if (v_leaseGetKind(lease) == V_LEASE_KIND_MONOTONIC) {
                leaseAdmin = &_this->monotonic;
            } else {
                leaseAdmin = &_this->elapsed;
            }
            arg.lease = lease;
            arg.action = NULL;
            assert(leaseAdmin->leases);
            (void)c_setWalk(leaseAdmin->leases, findLeaseAction, &arg);
            if (arg.action) {
                /* Check if the lease renewal results in an updated next expiry time */
                expiryTime = v_leaseExpiryTime(lease);
                if (v_leaseTimeCompare(expiryTime, leaseAdmin->nextExpiryTime) == OS_LESS) {
                    leaseAdmin->nextExpiryTime = expiryTime;
                    c_condBroadcast(&_this->cond);
                }
                c_free(arg.action);
            }
        } else if (v_eventTest(event, V_EVENT_TERMINATE)) {
            _this->quit = TRUE;
            c_condBroadcast(&_this->cond);
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManagerNotify", V_RESULT_ILL_PARAM,
                "Lease manager notified by unsupported event (%d)",
                event);
        }
    }
    c_mutexUnlock(&_this->mutex);

    return TRUE;
}

static void
disposeTransaction(
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        switch (v_objectKind(o)) {
        case K_GROUP:
            /* At this point call the group transactionAdmin to dispose the writer */
        break;
        case K_DATAREADER:
            /* At this point call the DataReader transactionAdmin to dispose the writer */
        break;
        default:
            assert(FALSE);
        break;
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
    }
}

static void
v_leaseManagerProcessLeaseAction(
    v_leaseManager _this,
    v_leaseAction leaseAction,
    v_leaseTime now_el)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));
    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    switch (leaseAction->actionId) {
        case V_LEASEACTION_SERVICESTATE_EXPIRED:
            serviceStateExpired(leaseAction);
            break;
        case V_LEASEACTION_READER_DEADLINE_MISSED:
            readerDeadlineMissed(leaseAction, now_el);
            break;
        case V_LEASEACTION_WRITER_DEADLINE_MISSED:
            writerDeadlineMissed(leaseAction, now_el);
            break;
        case V_LEASEACTION_LIVELINESS_CHECK:
            writerLivelinessLost(leaseAction);
            break;
        case V_LEASEACTION_HEARTBEAT_SEND:
            heartbeatSend(leaseAction);
            break;
        case V_LEASEACTION_HEARTBEAT_CHECK:
            heartbeatCheck(leaseAction);
            break;
        case V_LEASEACTION_SPLICED_DEATH_DETECTED:
            splicedDeathDetected(leaseAction);
            break;
        case V_LEASEACTION_DISPOSE_TRANSACTION:
            disposeTransaction(leaseAction);
            break;
        case V_LEASEACTION_MINIMUM_SEPARATION_EXPIRY:
            minSepTimeWindowExpiry(leaseAction, now_el);
            break;
        default:
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Unknown lease action (%d) for lease %p within leaseManager %p. "
                "Lease will be removed from lease manager",
                leaseAction->actionId, (void*)leaseAction->lease, (void*)_this);
            /* Remove lease from lease manager to prevent unneeded future wakeups of lease manager. */
            v_leaseManagerDeregister(_this, leaseAction->lease);
            break;
    }
}

static c_bool
collectExpired(
    c_object o,
    c_voidp arg)
{
    v_leaseAction leaseAction = v_leaseAction(o);
    struct collectExpiredArg *a = (struct collectExpiredArg *)arg;
    v_leaseTime leaseExpiryTime;
    os_duration leaseDuration;
    os_duration lag, currentPeriod;
    os_compare expired;

    v_leaseLock(leaseAction->lease);
    leaseExpiryTime = v_leaseExpiryTimeNoLock(leaseAction->lease);
    leaseDuration = v_leaseDurationNoLock(leaseAction->lease);
    v_leaseUnlock(leaseAction->lease);

    /* Add to expiredLeases if the current expiry time is
     * equal or later than the lease expiry time
     */
    expired = v_leaseTimeCompare(a->expiryTime, leaseExpiryTime);

    if (expired == OS_MORE) {
        c_bool logWarning;
        a->expiredLeases = c_iterAppend(a->expiredLeases, c_keep(leaseAction));

        /* Warn if the lease expiry processing is very late (twice the duration),
         * but not for the DEADLINE_MISSED cases as those tend to have very short
         * duration and cause too many warnings on highly loaded machines. For
         * aperiodic leases, we do not have enough information available.
         */
        if (leaseAction->actionId == V_LEASEACTION_READER_DEADLINE_MISSED ||
            leaseAction->actionId == V_LEASEACTION_WRITER_DEADLINE_MISSED) {
            logWarning = FALSE;
        } else {
            logWarning = TRUE;
        }
        if (logWarning && leaseAction->repeat) {
            /* a->expiryTime is current-time in this case */
            lag = v_leaseTimeDiff(a->expiryTime, leaseExpiryTime);
            if ((lag > leaseDuration) && (leaseDuration > 0)) {
                OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_OK,
                    "Processing of lease 0x" PA_ADDRFMT " is behind schedule; "
                    "expiry-time=%" PA_PRItime " and lag=%.09fs "
                    "(actionId=%u and duration=%.09fs). "
                    "This is often an indication that the machine is too busy. "
                    "The lease manager will continue to function normally.",
                    (PA_ADDRCAST)leaseAction->lease,
                    V_LEASE_TIME_PRINT(leaseExpiryTime),
                    os_durationToReal(lag), leaseAction->actionId,
                    os_durationToReal(leaseDuration));
            }
            currentPeriod = leaseDuration;
            if (a->shortestPeriod > currentPeriod) {
                a->shortestPeriod = currentPeriod;
            }
        }
    }
    /* Keep going */
    return TRUE;
}

static c_bool
calculateExpiryTime(
    c_object o,
    c_voidp arg)
{
    v_leaseTime leaseExpiryTime, *nextExpiryTime;
    leaseExpiryTime = v_leaseExpiryTime(v_leaseAction(o)->lease);
    nextExpiryTime = (v_leaseTime*)arg;

    if (v_leaseTimeCompare(leaseExpiryTime, *nextExpiryTime) == OS_LESS) {
        *nextExpiryTime = leaseExpiryTime;
    }

    /* Keep going */
    return TRUE;
}

/**************************************************************
 * Lease expiry action functions
 **************************************************************/

/* WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!
 *
 * The lease itself may never take its lock during the execution of
 * the action routine. The entity of the lease has the responsibility
 * that the entity is not locked, while changing the lease during
 * the lease action. Not adhering to this warning will result in a
 * deadlock
 *
 * WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!
 */

static void
serviceStateExpired(
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_SERVICESTATE) {
            v_serviceStateChangeState(v_serviceState(o), STATE_DIED);
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'serviceStateExpired' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d", r);
        }
    }
}

static void
splicedDeathDetected(
    v_leaseAction leaseAction)
{
    v_object o;
    v_kernel kernel;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_KERNEL) {
            kernel = v_kernel(o);
            kernel->splicedRunning = FALSE;
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'splicedDeathDetected' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
    }
}

static void
minSepTimeWindowExpiry(
    v_leaseAction leaseAction,
    v_leaseTime now_el)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_DATAREADER) {
            v_dataReaderCheckMinimumSeparationList(v_dataReader(o), v_leaseTimeToTimeE(now_el));
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'minSepTimeWindowExpiry' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
   }
}

static void
readerDeadlineMissed(
    v_leaseAction leaseAction,
    v_leaseTime now_el)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_DATAREADER) {
            v_dataReaderCheckDeadlineMissed(v_dataReader(o), v_leaseTimeToTimeE(now_el));
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'readerDeadlineMissed' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
   }
}

static void
writerDeadlineMissed(
    v_leaseAction leaseAction,
    v_leaseTime now_el)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_WRITER) {
            if (v_writerCheckDeadlineMissed(v_writer(o), v_leaseTimeToTimeE(now_el)) != V_RESULT_OK) {
                v_kernel kernel = v_objectKernel(o);
                kernel->splicedRunning = FALSE;
                OS_REPORT(OS_CRITICAL, "v_leaseManager", V_RESULT_OUT_OF_MEMORY,
                          "Out of memory detected, can't guarantee correct behaviour");
            }
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'writerDeadlineMissed' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
    }
}

static void
writerLivelinessLost(
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_WRITER) {
            v_writerNotifyLivelinessLost(v_writer(o));
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'livelinessCheck' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
    }
}

static void
heartbeatSend(
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_SPLICED) {
            v_splicedHeartbeat(v_spliced(o));
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'heartbeatSend' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
    } else {
        OS_REPORT(OS_ERROR, "heartbeatSend", V_RESULT_INTERNAL_ERROR,
            "Could not claim the splicedaemon!");
    }
}

static void
heartbeatCheck(
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_SPLICED) {
            v_splicedCheckHeartbeats(v_spliced(o));
        } else {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_ILL_PARAM,
                "Can't run lease action 'heartbeatCheck' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT(OS_WARNING, "v_leaseManager", V_RESULT_INTERNAL_ERROR,
                "Handle release failed with result code %d ", r);
        }
    } else {
        OS_REPORT(OS_ERROR, "heartbeatCheck", V_RESULT_INTERNAL_ERROR,
            "Could not claim the splicedaemon!");
    }
}
