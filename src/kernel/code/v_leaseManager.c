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

#include "v__leaseManager.h"
#include "v__lease.h"

#include "v_public.h"
#include "v_time.h"

#include "v_serviceState.h"
#include "v__dataReader.h"
#include "v__writer.h"
#include "v__spliced.h"

#include "os_report.h"
#include "os_abstract.h"

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
    c_iter leasesToRenew;
    c_time expiryTime;
    c_time shortestPeriod;
};

/**
 * \brief Walk function to determine which of the leases managed by a lease manager
 * expires the first.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg Pointer to a c_time struct to hold the earliest expiry time
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
    c_time now);

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
    c_time now);

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
    c_time now);

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
        OS_REPORT(OS_ERROR, "v_leaseManager", 0,
            "Failed to create a v_leaseManager object. "
            "Most likely not enough shared memory available "
            "to complete the operation.");
    }

    return _this;
}

void
v_leaseManagerInit(
    v_leaseManager _this)
{
    v_kernel k;

    assert(C_TYPECHECK(_this, v_leaseManager));

    k = v_objectKernel(_this);
    c_mutexInit(&_this->mutex, SHARED_MUTEX);
    c_condInit(&_this->cond, &_this->mutex, SHARED_COND);
    _this->quit = FALSE;
    _this->nextExpiryTime = C_TIME_INFINITE;
    _this->leases = c_setNew(v_kernelType(k, K_LEASEACTION));
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

void
v_leaseManagerDeinit(
    v_leaseManager _this)
{
    v_leaseAction lease;
    c_bool removed;

    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);
    lease = v_leaseAction(c_take(_this->leases));
    while (lease != NULL)
    {
        /* Unregister self from the lease, we are no longer an observer */
        v_leaseLock(lease->lease);
        removed = v_leaseRemoveObserverNoLock(lease->lease, _this);
        v_leaseUnlock(lease->lease);
        if(removed == FALSE)
        {
            OS_REPORT_2(OS_ERROR,
                "v_leaseManagerDeinit",0,
                "Failed to remove leaseManager %p from the list of "
                "observers of lease %p, while the lease WAS contained in "
                "the list of leases managed by the leaseManager. This means "
                "the administration has become inconsistent internally. "
                "This is not a fatal error in itself, but points towards "
                "a bug that could affect behaviour of OpenSpliceDDS",
                _this,
                lease);
        } /* else everything is ok */

        c_free(lease);
        lease = v_leaseAction(c_take(_this->leases));
    }
    c_free(_this->leases);
    _this->leases = NULL;
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
        OS_REPORT(OS_ERROR, "v_leaseManagerRegister", 0,
            "Failed to create a v_leaseAction object. "
            "Most likely not enough resources available to "
            "complete the operation.");
        result = V_RESULT_OUT_OF_MEMORY;
    } else {
        leaseAction->lease = v_lease(c_keep(lease));
        assert(leaseAction->lease);
        leaseAction->actionId = actionId;
        leaseAction->actionObject = v_publicHandle(actionObject);
        leaseAction->repeat = repeatLease;

        /* Step 2a: insert the leaseAction object into the set of leases. */
        c_mutexLock(&_this->mutex);
        foundLeaseAction = c_setInsert(_this->leases, leaseAction);
        if(foundLeaseAction != leaseAction) {
            OS_REPORT(OS_ERROR, "v_leaseManagerRegister", 0,
                "Failed to insert the lease in the lease manager. "
                "Most likely not enough resources available to "
                "complete the operation.");
            result = V_RESULT_INTERNAL_ERROR;
        } else {
            /* Step 2b: Now that the lease was successfully inserted into the lease manager,
             * we need to register the leaseManager as an observer of the lease to ensure that the
             * it is notified when the lease expiry time and/or duration is changed. To prevent the
             * lease time from changing while we evaluate the lease we will lock the lease object.
             */
            v_leaseLock(lease);
            observerAdded = v_leaseAddObserverNoLock(lease, _this);
            if (!observerAdded) {
                OS_REPORT(OS_ERROR, "v_leaseManagerRegister", 0,
                    "Failed to insert the lease manager as an observer of the lease. "
                    "Most likely not enough resources available to "
                    "complete the operation.");
                result = V_RESULT_INTERNAL_ERROR;
                /* Remove the lease from the leaseManager */
                foundLeaseAction = c_setRemove(_this->leases, leaseAction, NULL, NULL);
                if (foundLeaseAction != leaseAction) {
                    OS_REPORT(OS_ERROR, "v_leaseManagerRegister", 0,
                        "Failed to remove a lease from the lease manager");
                }
            }

            /* Step 3: If the newly registered lease expires before the
             * current nextExpiryTime of the lease manager, the nextExpiryTime
             * needs to be updated, to take into account the expiry of the
             * new leaseAction.
             */
            if((result == V_RESULT_OK) &&
               (c_timeCompare(lease->expiryTime, _this->nextExpiryTime) == C_LT)) {
                _this->nextExpiryTime = lease->expiryTime;
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
    c_bool removed;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    if (lease != NULL) {
        assert(C_TYPECHECK(lease, v_lease));

        /* Step 1: Get the leaseAction corresponding to the lease, from the set of leases */
        c_mutexLock(&_this->mutex);
        arg.lease = lease;
        arg.action = NULL;
        c_setWalk(_this->leases, findLeaseAction, &arg);
        if(arg.action) {
            /* step 2a: If the leaseAction object exists, remove it from the lease manager */
        	foundLeaseAction = c_setRemove(_this->leases, arg.action, NULL, NULL);
        	assert(foundLeaseAction == arg.action);

        	/* Step 2b: Unregister the lease manager as observer of the
			 * lease.
			 */
			 v_leaseLock(lease);
			 removed = v_leaseRemoveObserverNoLock(lease, _this);
			 v_leaseUnlock(lease);
			 if (removed == FALSE) {
				 OS_REPORT_2(OS_ERROR, "v_leaseManagerDeregister", 0,
				         "Failed to unregister lease manager %p as an observer of lease %p, "
				         "while the lease WAS contained in the set of leases managed by "
				         "this lease manager.",
				         _this, lease);
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

c_bool
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

c_bool
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

void
v_leaseManagerMain(
    v_leaseManager _this)
{
    v_leaseAction leaseAction;
    c_time waitTime = C_TIME_ZERO;
    struct collectExpiredArg arg;
    c_syncResult waitResult;
    c_time lag;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    arg.expiredLeases = NULL;
    arg.shortestPeriod = C_TIME_INFINITE;

    c_mutexLock(&_this->mutex);

    while (_this->quit == FALSE) {
        arg.expiryTime = v_timeGet();

        if (c_timeCompare(_this->nextExpiryTime, C_TIME_INFINITE) == C_EQ) {
            /* The next expiry time is from a lease with an infinite duration, so wait indefinitely */
            waitResult = c_condWait(&_this->cond, &_this->mutex);
        } else if (c_timeCompare(_this->nextExpiryTime, arg.expiryTime) >= C_EQ) {
            /* The next expiry time is in the future, wait until that moment */
            waitTime = c_timeSub(_this->nextExpiryTime, arg.expiryTime);
            waitResult = c_condTimedWait(&_this->cond, &_this->mutex, waitTime);
        } else {
            /* The next expiry time lies in the past, log a warning and proceed
             * as normal. The first cycle, the leaseManager may be late as the
             * thread is started asynchronously and leases may have been added
             * before the thread is really started.
             */

            /* Current time is apparently > next expiry-time, so calculate
             * lag by subtracting nextExpiryTime from current time.
             */
            lag = c_timeSub(arg.expiryTime, _this->nextExpiryTime);

            /* Only report warning if lag is more than the shortest period
             * of all registered leases.
             */
            if(c_timeCompare(lag, arg.shortestPeriod) == C_GT){
                OS_REPORT_6(OS_WARNING, "v_leaseManagerMain", 0,
                    "The lease manager did not wake up on time and missed the "
                    "next expiry-time (smallest lease-duration is %d.%09us). "
                    "This means there are likely one or more "
                    "leases that will not be processed in time. This could be due "
                    "to scheduling problems or clock alignment issues on multi-core "
                    "machines. The lease manager will continue to function normally. "
                    "Wake-up time should be %d.%09us, but current-time is %d.%09us.",
                    arg.shortestPeriod.seconds, arg.shortestPeriod.nanoseconds,
                    _this->nextExpiryTime.seconds, _this->nextExpiryTime.nanoseconds,
                    arg.expiryTime.seconds, arg.expiryTime.nanoseconds);
            }

            waitResult = SYNC_RESULT_SUCCESS;
        }

        if (waitResult == SYNC_RESULT_FAIL) {
            OS_REPORT(OS_CRITICAL, "v_leaseManagerMain", 0,
                "c_condTimedWait / c_condWait failed - memory sync no longer viable - probable cause is death of spliced."
                OS_REPORT_NL "Lease manager will terminate after performing any registered death-of-spliced actions.");
                c_setWalk(_this->leases, (c_action)splicedIsDead, NULL);
                break;
        }
        /* Collect expired leases */
        arg.expiredLeases = NULL;
        arg.leasesToRenew = NULL;
        arg.expiryTime = v_timeGet();
        arg.shortestPeriod = C_TIME_INFINITE;
        c_setWalk(_this->leases, collectExpired, &arg);

        /* Process expired leases */
        c_mutexUnlock(&_this->mutex);

        leaseAction = v_leaseAction(c_iterTakeFirst(arg.leasesToRenew));

        while(leaseAction != NULL){
            v_leaseRenewInternal(leaseAction->lease, NULL);
            leaseAction = v_leaseAction(c_iterTakeFirst(arg.leasesToRenew));
        }
        c_iterFree(arg.leasesToRenew);

        leaseAction = v_leaseAction(c_iterTakeFirst(arg.expiredLeases));
        while (leaseAction != NULL) {
            /* Either renew or unregister the lease */
            if (leaseAction->repeat) {
                v_leaseRenew(leaseAction->lease, NULL);
            } else {
                v_leaseManagerDeregister(_this, leaseAction->lease);
            }
            v_leaseManagerProcessLeaseAction(_this, leaseAction, arg.expiryTime);
            c_free(leaseAction);
            leaseAction = v_leaseAction(c_iterTakeFirst(arg.expiredLeases));
        }
        c_iterFree(arg.expiredLeases);

        /* Calculate next expiry time of lease manager */
        c_mutexLock(&_this->mutex);
        _this->nextExpiryTime = C_TIME_INFINITE;
        c_setWalk(_this->leases, calculateExpiryTime, &_this->nextExpiryTime);
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
    c_time expiryTime;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);

    if (_this->quit == FALSE) {
        if (v_eventTest(event, V_EVENT_LEASE_RENEWED)) {
            /* Check if the lease is registered by this lease manager */
            arg.lease = lease;
            arg.action = NULL;
            assert(_this->leases);
            c_setWalk(_this->leases, findLeaseAction, &arg);
            if (arg.action) {
                /* Check if the lease renewal resuls in an updated next expiry time */
                expiryTime = v_leaseExpiryTime(lease);
                if (c_timeCompare(expiryTime, _this->nextExpiryTime) == C_LT) {
                    _this->nextExpiryTime = expiryTime;
                    c_condBroadcast(&_this->cond);
                }
                c_free(arg.action);
            }

        } else if (v_eventTest(event, V_EVENT_TERMINATE)) {
            _this->quit = TRUE;
            c_condBroadcast(&_this->cond);
        } else {
            OS_REPORT_1(OS_WARNING, "v_leaseManagerNotify", 0,
                "Lease manager notified by unsupported event (%d)",
                event);
        }
    }
    c_mutexUnlock(&_this->mutex);

    return TRUE;
}

void
v_leaseManagerProcessLeaseAction(
    v_leaseManager _this,
    v_leaseAction leaseAction,
    c_time now)
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
            readerDeadlineMissed(leaseAction, now);
            break;
        case V_LEASEACTION_WRITER_DEADLINE_MISSED:
            writerDeadlineMissed(leaseAction, now);
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
        default:
            OS_REPORT_3(OS_WARNING, "v_leaseManager", 0,
                "Unknown lease action (%d) for lease %p within leaseManager %p. "
                "Lease will be removed from lease manager",
                leaseAction->actionId, leaseAction->lease, _this);
            /* Remove lease from lease manager to prevent unneeded future wakeups of lease manager. */
            v_leaseManagerDeregister(_this, leaseAction->lease);
            break;
    }
}

c_bool
collectExpired(
    c_object o,
    c_voidp arg)
{
    v_leaseAction leaseAction = v_leaseAction(o);
    struct collectExpiredArg *a = (struct collectExpiredArg *)arg;
    c_time leaseExpiryTime;
    v_duration leaseDuration;
    c_time lag, currentPeriod;
    c_equality expired;
    c_bool lastRenewInternal;
    c_time someRidiculousDelay = {10, 0};
    c_bool ignoreExpiry = FALSE;

    v_leaseLock(leaseAction->lease);
    leaseExpiryTime = v_leaseExpiryTimeNoLock(leaseAction->lease);
    leaseDuration = v_leaseDurationNoLock(leaseAction->lease);
    lastRenewInternal = v_leaseLastRenewInternalNoLock(leaseAction->lease);
    v_leaseUnlock(leaseAction->lease);

    /* Add to expiredLeases if the current expiry time is
     * equal or later than the lease expiry time */
    expired = c_timeCompare(a->expiryTime, leaseExpiryTime);

    if (expired >= C_EQ) {
        /* a->expiryTime is current-time in this case */
        lag = c_timeSub(a->expiryTime, leaseExpiryTime);

        /* Ignore long lags of over 2 minutes for some lease types
         * to allow resuming after hibernation. This is a temporary
         * workaround until we have real support for hibernation
         * in the product.
         *
         * In this case, the expired lease is renewed, but that renewal
         * is marked as an internal renewal. This ensures that the lease will
         * definitely expire in case the one that is actually responsible
         * for updating the lease does not renew it within the next
         * period.
         *
         * This is only done for service-state-expiry (including spliced)
         * and liveliness leases. This prevents configured failure actions
         * from being taken.
         */
        if(!lastRenewInternal){
            if( leaseAction->actionId == V_LEASEACTION_SERVICESTATE_EXPIRED ||
                leaseAction->actionId == V_LEASEACTION_SPLICED_DEATH_DETECTED ||
                leaseAction->actionId == V_LEASEACTION_LIVELINESS_CHECK)
            {

                if(c_timeCompare(lag, someRidiculousDelay) >= C_EQ){
                    ignoreExpiry = TRUE;

                    OS_REPORT_8(OS_WARNING, "v_leaseManager", 0,
                        "Processing of lease 0x" PA_ADDRFMT " is behind schedule; "
                        "expiry-time=%d.%09us and lag=%d.%09us "
                        "(actionId=%u and duration=%d.%09us). "
                        "Assuming resuming after hibernate...",
                        (PA_ADDRCAST)leaseAction->lease,
                        leaseExpiryTime.seconds, leaseExpiryTime.nanoseconds,
                        lag.seconds, lag.nanoseconds, leaseAction->actionId,
                        leaseDuration.seconds, leaseDuration.nanoseconds);
                }
            }
        }

        /* only print this warning for periodic leases as we cannot give information about a periodic (deadline) leases see OSPL-1681*/
        if (leaseAction->repeat && !ignoreExpiry) {
            /* If the difference is larger than the lease duration, the lease
             * was not renewed in time, thus lease processing is lagging behind
             */
            if (c_timeCompare(lag, leaseDuration) == C_GT){
                OS_REPORT_8(OS_WARNING, "v_leaseManager", 0,
                    "Processing of lease 0x" PA_ADDRFMT " is behind schedule; "
                    "expiry-time=%d.%09us and lag=%d.%09us "
                    "(actionId=%u and duration=%d.%09us). "
                    "This is often an indication that the machine is too busy. "
                    "The lease manager will continue to function normally.",
                    (PA_ADDRCAST)leaseAction->lease,
                    leaseExpiryTime.seconds, leaseExpiryTime.nanoseconds,
                    lag.seconds, lag.nanoseconds, leaseAction->actionId,
                    leaseDuration.seconds, leaseDuration.nanoseconds);
            }
            currentPeriod.seconds = leaseDuration.seconds;
            currentPeriod.nanoseconds = leaseDuration.nanoseconds;

            if(c_timeCompare(a->shortestPeriod, currentPeriod) == C_GT){
                a->shortestPeriod = currentPeriod;
            }
        }
        if(!ignoreExpiry){
            a->expiredLeases = c_iterInsert(a->expiredLeases, c_keep(leaseAction));
        } else {
            a->leasesToRenew = c_iterInsert(a->leasesToRenew, c_keep(leaseAction));
        }
    }
    /* Keep going */
    return TRUE;
}

c_bool
calculateExpiryTime(
    c_object o,
    c_voidp arg)
{
    c_time leaseExpiryTime, *nextExpiryTime;
    leaseExpiryTime = v_leaseExpiryTime(v_leaseAction(o)->lease);
    nextExpiryTime = (c_time*)arg;

    if (c_timeCompare(leaseExpiryTime, *nextExpiryTime) == C_LT) {
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

void
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
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'serviceStateExpired' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d", r);
        }
    }
}

void
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
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'splicedDeathDetected' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    }
}

void
readerDeadlineMissed(
    v_leaseAction leaseAction,
    c_time now)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_DATAREADER) {
            v_dataReaderCheckDeadlineMissed(v_dataReader(o), now);
        } else {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'readerDeadlineMissed' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
   }
}

void
writerDeadlineMissed(
    v_leaseAction leaseAction,
    c_time now)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK) {
        if (v_objectKind(o) == K_WRITER) {
            v_writerCheckDeadlineMissed(v_writer(o), now);
        } else {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'writerDeadlineMissed' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    }
}

void
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
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'livelinessCheck' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    }
}

void
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
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'heartbeatSend' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } else {
        OS_REPORT(OS_ERROR, "heartbeatSend", 0,
            "Could not claim the splicedaemon!");
    }
}

void
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
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Can't run lease action 'heartbeatCheck' on object kind %d", v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK) {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } else {
        OS_REPORT(OS_ERROR, "heartbeatCheck", 0,
            "Could not claim the splicedaemon!");
    }
}
