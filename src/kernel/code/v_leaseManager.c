/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "v__leaseManager.h"
#include "v_time.h"
#include "v__lease.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_observer.h"
#include "v_observable.h"
#include "v_event.h"
#include "v__writer.h" /* for resending action */
#include "v__group.h" /* for resending action */
#include "v__dataReader.h" /* for deadline missed */
#include "v_serviceState.h"
#include "v__spliced.h" /* for heartbeat */

#include "os_report.h"

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

struct findLeaseActionArg {
    v_leaseAction action;
    v_lease lease;
};

/**
 * \brief Walks over the set of leases being observed by this leaseManager
 * and returns the leaseAction object that is associated by the lease specified
 * in the arg ptr.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg The arg ptr which contains the 'findLeaseActionArg' struct with
 *            the lease object to search for.
 *
 * \return TRUE if the leaseAction object being searched for is not yet found,
 * FALSE otherwise.
 */
static c_bool
findLeaseAction(
    c_object o,
    c_voidp arg);

/**
 * \brief This function determines which of the leases in the set of observed
 * leases is the first lease to expire.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg The v_leaseManager object.
 *
 * \return TRUE as we want to check every element in the set
 */
static c_bool
determineFirstLeaseToExpire(
    c_object o,
    c_voidp arg);

struct collectExpiredArg {
    c_iter expiredLeases;
    v_leaseAction firstLeaseToExpire;
    c_time now;
};

/**
 * \brief This function determines which of the leases in the set of observed
 * leases has expired. This function also determines which of the leases will
 * expire first.
 *
 * \param o An element from the set, i.e., a v_leaseAction object.
 * \param arg The 'collectExpiredArg' struct containing relevant information
 * and to store the collected data in.
 *
 * \return TRUE as we want to check every element in the set
 */
static c_bool
collectExpired(
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
 * \brief This function will set a boolean in the v_kernel object indicating that
 * the spliced has died. This action routine is called when a lease with the
 * V_LEASEACTION_SPLICED_DEATH_DETECTED action expires.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
 */
static void
splicedDeathDetected(
    v_leaseAction leaseAction);

/**
 * \brief The action routine executed when a service state expires.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
 */
static void
serviceStateExpired(
    v_leaseAction leaseAction);

/**
 * \brief The action routine executed when a deadline is missed for a reader.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
 * \param now The current time.
 */
static void
readerDeadlineMissed(
    v_leaseAction leaseAction,
    c_time now);

/**
 * \brief The action routine executed when a deadline is missed for a writer.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
 * \param now The current time.
 */
static void
writerDeadlineMissed(
    v_leaseAction leaseAction,
    c_time now);

/**
 * \brief The action routine executed when a liveliness has been lost
 *
 * \param _this The leaseManager object to operate on, used when the action
 *              routine determines that the lease needs to be removed.
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
 */
static void
livelinessCheck(
    v_leaseManager _this,
    v_leaseAction leaseAction);

/**
 * \brief The action routine executed when the splice deamon needs to send a
 * heart beat.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
 */
static void
heartbeatSend(
    v_leaseAction leaseAction);

/**
 * \brief The action routine executed when the splice deamon needs to check
 * heart beats of services.
 *
 * \param leaseAction The leaseAction object containing the v_lease and
 *                    actionObject used within this action routine.
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
    if(_this)
    {
        v_leaseManagerInit(_this);
    } else
    {
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
    _this->firstLeaseToExpire = NULL;
    _this->leases = c_setNew(v_kernelType(k, K_LEASEACTION));
}

void
v_leaseManagerFree(
    v_leaseManager _this)
{
    if (_this != NULL)
    {
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

    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);
    c_free(_this->firstLeaseToExpire);
    _this->firstLeaseToExpire = NULL;
    lease = v_leaseAction(c_take(_this->leases));
    while (lease != NULL)
    {
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
    c_bool obsAdded;
    v_leaseAction leaseAction;
    v_leaseAction found;
    v_result result;
    v_kernel k;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));
    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));
    assert(actionObject != NULL);
    assert(C_TYPECHECK(actionObject, v_public));

    /* Step 1: Create a lease action object. This object will contain the relevant
     * information needed when reacting to an expired lease. This action information
     * can be different depending on which lease manager a lease is being registered
     * to, hence why the leaseAction object resides at leaseManager level and not
     * at lease level as it did in the past
     */
    k = v_objectKernel(_this);
    leaseAction = v_leaseAction(v_objectNew(k, K_LEASEACTION));
    if(!leaseAction)
    {
        OS_REPORT(OS_ERROR, "v_leaseManager", 0,
            "Failed to create a v_leaseManager object. "
            "Most likely not enough shared memory available to "
            "complete the operation.");
        result = V_RESULT_OUT_OF_MEMORY;
    } else
    {
        leaseAction->lease = v_lease(c_keep(lease));
        assert(leaseAction->lease);
        leaseAction->actionId = actionId;
        leaseAction->actionObject = v_publicHandle(actionObject);
        leaseAction->repeat = repeatLease;
        /* Step 2: insert the leaseAction object into the set of leases. */
        c_mutexLock(&_this->mutex);
        found = c_setInsert(_this->leases, leaseAction);
        if(!found)
        {
            /* Because the leaseAction object was just allocated we only have
             * to check if found is a NULL pointer. As it can never find the
             * action already being present in the set.
             */
            OS_REPORT(OS_ERROR, "v_leaseManager", 0,
                "Unable to register the lease to the list of "
                "leases of the leaseManager object! Most likely not enough shared "
                "memory available to complete the operation.");
            result = V_RESULT_OUT_OF_MEMORY;
            c_free(leaseAction);
            leaseAction = NULL;
        } else
        {
            assert(found == leaseAction);
            /* Step 3: Determine if the newly inserted leaseAction will become the
             * 'next lease to expire'. E.G., if the lease contained within the
             * leaseAction object has an expiry time that is the closest to the
             * present time compared to the other leases managed within this lease
             * manager. To prevent the lease time from changing while we evaluate the
             * lease we will lock the lease object.
             */
            v_leaseLock(lease);
            if(!_this->firstLeaseToExpire)
            {
                _this->firstLeaseToExpire = c_keep(leaseAction);
                /* head changed, so signal */
                c_condBroadcast(&_this->cond);
            } else if ((_this->firstLeaseToExpire->lease != lease) &&
                       (c_timeCompare(v_leaseExpiryTime(_this->firstLeaseToExpire->lease),
                                     v_leaseExpiryTimeNoLock(lease)) == C_GT))
            {
                c_free(_this->firstLeaseToExpire);
                _this->firstLeaseToExpire = c_keep(leaseAction);
                /* head changed, so signal */
                c_condBroadcast(&_this->cond);
            }/* else do nothing as the newly added lease expires after the firstLeaseToExpire */
            /* Step 4: Now that the lease was successfully inserted into the lease manager,
             * we need to register the leaseManager as an observer of the lease to ensure that the
             * lease manager is notified if the lease expiry time and/or duration changes.
             */
            obsAdded = v_leaseAddObserverNoLock(lease, _this);
            if(!obsAdded)
            {
                OS_REPORT(OS_ERROR, "v_leaseManager", 0,
                    "Unable to register the lease manager to the list of "
                    "observers of the lease object! Possibly not enough "
                    "shared memory available to complete the operation.");
                result = V_RESULT_INTERNAL_ERROR;
                v_leaseUnlock(lease);
                /* Remove the lease from the leaseManager */
                found = c_setRemove(_this->leases, leaseAction, NULL, NULL);
                if(found != leaseAction)
                {
                    OS_REPORT(OS_ERROR, "v_leaseManager", 0,
                        "Unable to unregister the lease to the list of "
                        "leases of the leaseManager object after previous internal error!");
                }
                c_free(leaseAction);
                leaseAction = NULL;
            } else
            {
                /* Now that the lease manager is in the observer list of the lease, we can unlock the lease
                 * as from now on we will be notified of any changes to the lease expiry time and/or duration
                 */
                v_leaseUnlock(lease);
                result = V_RESULT_OK;
            }

        }
        c_mutexUnlock(&_this->mutex);
    }
    if(leaseAction)
    {
        /* Done with the leaseAction object in this operation. If the object is not a NULL
         * pointer then everything went ok. The leases set of the leaseManager should be
         * the only one maintaining a ref count now (and possibly the 'firstLeaseToExpire'
         * attribute. But we do not need the leaseAction object in this operation anymore
         * and we are not returning it, so we need to lower the ref count for the new operation
         */
        c_free(leaseAction);
    }/* else do nothing */

    return result;
}

void
v_leaseManagerDeregister(
    v_leaseManager _this,
    v_lease lease)
{
    struct findLeaseActionArg arg;
    v_leaseAction found;
    c_bool removed;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    if (lease != NULL)
    {
        assert(C_TYPECHECK(lease, v_lease));

        /* Step 1: Locate the leaseAction object based on the lease object */
        c_mutexLock(&_this->mutex);
        arg.lease = lease;
        arg.action = NULL;
        c_setWalk(_this->leases, findLeaseAction, &arg);
        if(arg.action)
        {
            /* step 2a: If we found the action object, then remove it */
            found = c_setRemove(_this->leases, arg.action, NULL, NULL);
            assert(found == arg.action);
            /* Step 2b: Remove the leaseManager from the list of observers of the
             * lease object. Using explicit lock operations, to keep the
             * v_leaseRemoveObserverNoLock operation consistent with the
             * v_leaseAddObserverNoLock operation.
             */
            v_leaseLock(lease);
            removed = v_leaseRemoveObserverNoLock(lease, _this);
            v_leaseUnlock(lease);
            if(removed == FALSE)
            {
                OS_REPORT_2(OS_ERROR,
                    "v_leaseManagerDeregister",0,
                    "Failed to remove leaseManager %p from the list of "
                    "observers of lease %p, while the lease WAS contained in "
                    "the list of leases managed by the leaseManager. This means "
                    "the administration has become inconsistent internally. "
                    "This is not a fatal error in itself, but points towards "
                    "a bug that could affect behaviour of OpenSpliceDDS",
                    _this,
                    lease);
            } /* else everything is ok */
            /* step 3: If the removed action was the 'firstLeaseToExpire' then we need
             * to redetermine the new 'firstLeaseToExpire'. Take note that no broadcast is done
             * on the condition. Because the new 'firstLeaseToExpire' will always have a later
             * expiry time, otherwise the one removed would not have been the first to expire.
             * So as a result of not doing the broadcast the leaseManager will wake up and find no
             * lease to be expired and continue as normal
             */
            if (arg.action == _this->firstLeaseToExpire)
            {
                c_free(_this->firstLeaseToExpire);
                _this->firstLeaseToExpire = NULL;
                c_setWalk(_this->leases, determineFirstLeaseToExpire, _this);
            }
            c_free(found); /* delete local reference */
            c_free(arg.action);
        }/* else the lease was not contained, so do nothing */
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

    if(action && action->lease == a->lease)
    {
        a->action = c_keep(action);
        retVal = FALSE;
    } else
    {
        retVal = TRUE;
    }

    return retVal;
}

c_bool
determineFirstLeaseToExpire(
    c_object o,
    c_voidp arg)
{
    c_time headExpTime;
    c_time leaseExpTime;
    v_leaseAction leaseAction = v_leaseAction(o);
    v_leaseManager lm = v_leaseManager(arg);

    if (lm->firstLeaseToExpire == NULL)
    {
        lm->firstLeaseToExpire = c_keep(leaseAction);
    } else
    {
        headExpTime = v_leaseExpiryTime(lm->firstLeaseToExpire->lease);
        leaseExpTime = v_leaseExpiryTime(leaseAction->lease);
        if (c_timeCompare(headExpTime, leaseExpTime) == C_GT)
        {
            c_free(lm->firstLeaseToExpire);
            lm->firstLeaseToExpire = c_keep(leaseAction);
        }
    }
    return TRUE;
}

/**************************************************************
 * Main / notify fnctions
 **************************************************************/
void
v_leaseManagerMain(
    v_leaseManager _this)
{
    v_leaseAction leaseAction;
    c_time waitTime = C_TIME_ZERO;
    c_time expTime;
    v_duration duration;
    struct collectExpiredArg arg;
    c_syncResult waitResult = SYNC_RESULT_SUCCESS;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);
    /* initialize the current time once before the loop */
    arg.now = v_timeGet();
    while (_this->quit == FALSE) {
        if (_this->firstLeaseToExpire != NULL) {
            v_leaseGetExpiryAndDuration(_this->firstLeaseToExpire->lease, &expTime, &duration);
            if (c_timeCompare(expTime, C_TIME_INFINITE) != C_EQ) {
                waitTime = c_timeSub(expTime, arg.now);
                if (c_timeCompare(waitTime, C_TIME_ZERO) == C_GT) {
                    waitResult = c_condTimedWait(&_this->cond, &_this->mutex, waitTime);
                } else {
                    /* If the duration specified with the lease is C_TIME_ZERO,
                     * it is expected that the expiryTime lies in the past, so
                     * only warn if an actual duration was specified. */
                    if(c_timeCompare(duration, C_TIME_ZERO) != C_EQ){
                        OS_REPORT(OS_WARNING, "v_leaseManager", 0,
                            "The wait time has become negative! This means "
                            "that the leaseManager could not wake up in time to "
                            "evaluate the lease expiry statusses. This could be "
                            "due to scheduling problems or clock alignment issues on "
                            "multi core machines. The lease manager will continue to "
                            "function normal after this though.");
                    }
                }
            } else {
                /* The shortest expiry time is from a lease with an infinite duration. So
                 * wait indefinately
                 */
                waitResult = c_condWait(&_this->cond, &_this->mutex);
            }
        } else {
            /* no leases registered, so wait until the first one is registered */
            waitResult = c_condWait(&_this->cond, &_this->mutex);
        }

        if (waitResult == SYNC_RESULT_FAIL)
        {
            OS_REPORT(OS_CRITICAL, "v_leaseManagerMain", 0,
                      "c_condTimedWait / c_condWait failed - thread will terminate");
            break;
        }

        /**
         * First walk through the collection of leases and record all
         * expired leases in an iterator. We cannot remove expired leases
         * while walking through the set, since it interferes with the
         * walk.
         * Any lease with a repeat bool to TRUE  will automatically be renewed
         * while collecting all the leases.
         */
        arg.expiredLeases = NULL;
        arg.firstLeaseToExpire = NULL;
        arg.now = v_timeGet();
        c_setWalk(_this->leases, collectExpired, &arg);

        c_free(_this->firstLeaseToExpire);
        _this->firstLeaseToExpire = arg.firstLeaseToExpire;/* takes over ref count from arg object */
        c_mutexUnlock(&_this->mutex);

        leaseAction = v_leaseAction(c_iterTakeFirst(arg.expiredLeases));
        while (leaseAction != NULL) {
            if(!leaseAction->repeat)
            {
                v_leaseManagerDeregister(_this, leaseAction->lease);
            }
            v_leaseManagerProcessLeaseAction(_this, leaseAction, arg.now);
            c_free(leaseAction);
            leaseAction = v_leaseAction(c_iterTakeFirst(arg.expiredLeases));
        }
        c_iterFree(arg.expiredLeases);
        c_mutexLock(&_this->mutex);
    }
    _this->quit = FALSE; /* for a next time */
    c_mutexUnlock(&_this->mutex);
}

c_bool
collectExpired(
    c_object o,
    c_voidp arg)
{
    v_leaseAction leaseAction = v_leaseAction(o);
    struct collectExpiredArg *a = (struct collectExpiredArg *)arg;
    c_time headExpTime;
    c_time leaseExpTime;
    c_bool setHead;
    c_equality cmp;

    setHead = TRUE;
    leaseExpTime = v_leaseExpiryTime(leaseAction->lease);
    /*
     * A lease is expired if the expiry time is greater than or equal
     * to the current time!
     */
    cmp = c_timeCompare(a->now, leaseExpTime);
    if ((cmp ==  C_GT) || (cmp == C_EQ)) {
        a->expiredLeases = c_iterInsert(a->expiredLeases, c_keep(leaseAction));
        /* An expired lease can still become the next expirytime,
         * if it should be repeated
         */
        if (leaseAction->repeat)
        {
            v_leaseRenew(leaseAction->lease, NULL);
        } else
        {
            setHead = FALSE;
        }
    }
    if (setHead) {
        if (a->firstLeaseToExpire == NULL) {
            a->firstLeaseToExpire = c_keep(leaseAction);
        } else {
            headExpTime = v_leaseExpiryTime(a->firstLeaseToExpire->lease);
            leaseExpTime = v_leaseExpiryTime(leaseAction->lease);

            if (c_timeCompare(headExpTime, leaseExpTime) == C_GT) {
                c_free(a->firstLeaseToExpire);
                a->firstLeaseToExpire = c_keep(leaseAction);
            }
        }
    }

    return TRUE;
}

c_bool
v_leaseManagerNotify(
    v_leaseManager _this,
    v_lease lease,
    v_eventKind event)
{
    struct findLeaseActionArg arg;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));

    c_mutexLock(&_this->mutex);
    if (event & V_EVENT_LEASE_RENEWED) {
        if (_this->firstLeaseToExpire) {
            if (_this->firstLeaseToExpire->lease == lease) {
                /* If the lease is the head we are forced to wake-up the
                   thread, since we do not know the remaining sleeptime of the
                   thread.
                */
                c_condBroadcast(&_this->cond);
            } else {
                arg.lease = lease;
                arg.action = NULL;
                c_setWalk(_this->leases, findLeaseAction, &arg);
                if (arg.action) {
                    /* determine if this is the new head */
                    if (c_timeCompare(v_leaseExpiryTime(_this->firstLeaseToExpire->lease),
                                      v_leaseExpiryTime(lease)) == C_GT) {
                        c_free(_this->firstLeaseToExpire);
                        _this->firstLeaseToExpire = c_keep(arg.action);
                        c_condBroadcast(&_this->cond);
                    }
                    c_free(arg.action);
                } /* else lease is not registered, so no interest in this update! */
            }
        }
    } else {
        if (event & V_EVENT_TERMINATE) {
            _this->quit = TRUE;
            c_condBroadcast(&_this->cond);
        }
    }
    c_mutexUnlock(&_this->mutex);

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
v_leaseManagerProcessLeaseAction(
    v_leaseManager _this,
    v_leaseAction leaseAction,
    c_time now)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_leaseManager));
    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    switch (leaseAction->actionId)
    {
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
            livelinessCheck(_this, leaseAction);
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
                        "Unknown lease action %d for lease %p within leaseManager %p", leaseAction->actionId, leaseAction->lease, _this);
            /* lets remove lease from lease manager to prevent future wakeups of lease manager. */
            v_leaseManagerDeregister(_this, leaseAction->lease);
        break;
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
    if (r == V_HANDLE_OK)
    {
        kernel = v_kernel(o);
        kernel->splicedRunning = FALSE;
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } /* else just skip, since entity is already gone */

}

void
serviceStateExpired(
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));
    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK)
    {
        if (o->kind == K_SERVICESTATE)
        {
            v_serviceStateChangeState(v_serviceState(o), STATE_DIED);
        } else
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Lease action on unexpected object type: %d", o->kind);
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } /* else just skip, since entity is already gone */
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
    if (r == V_HANDLE_OK)
    {
        v_dataReaderCheckDeadlineMissed(v_dataReader(o), now);
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
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
    v_object w;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &w);
    if (r == V_HANDLE_OK)
    {
        v_writerCheckDeadlineMissed(v_writer(w), now);
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    }
}

void
livelinessCheck(
    v_leaseManager _this,
    v_leaseAction leaseAction)
{
    v_object o;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    /* Liveliness lease expired, so the reader/writer must be notified! */
    r = v_handleClaim(leaseAction->actionObject, &o);
    if (r == V_HANDLE_OK)
    {
        v_writerNotifyLivelinessLost(v_writer(o));
        if (v_objectKind(o) != K_WRITER)
        {
            OS_REPORT_1(OS_WARNING, "v_lease", 0,
                        "entity %d has no liveliness policy",
                        v_objectKind(o));
        }
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } else
    {
        /* Corresponding reader/writer is already gone, so remove this lease
         * from its leasemanager.
         */
        v_leaseManagerDeregister(_this, leaseAction->lease);
    }
}

void
heartbeatSend(
    v_leaseAction leaseAction)
{
    v_object sd;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &sd);
    if (r == V_HANDLE_OK)
    {
        v_splicedHeartbeat(v_spliced(sd));
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } else
    {
        OS_REPORT(OS_ERROR, "heartbeatSend", 0,
            "Could not claim the splicedaemon!");
    }
}

void
heartbeatCheck(
    v_leaseAction leaseAction)
{
    v_object sd;
    v_handleResult r;

    assert(leaseAction != NULL);
    assert(C_TYPECHECK(leaseAction, v_leaseAction));

    r = v_handleClaim(leaseAction->actionObject, &sd);
    if (r == V_HANDLE_OK)
    {
        v_splicedCheckHeartbeats(v_spliced(sd));
        r = v_handleRelease(leaseAction->actionObject);
        if(r != V_HANDLE_OK)
        {
            OS_REPORT_1(OS_WARNING, "v_leaseManager", 0,
                "Handle release failed with result code %d ", r);
        }
    } else
    {
        OS_REPORT(OS_ERROR, "heartbeatSend", 0,
            "Could not claim the splicedaemon!");
    }
}
