#include "v__leaseManager.h"
#include "v_time.h"
#include "v__lease.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_observer.h"
#include "v_observable.h"
#include "v_event.h"

#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/

static void
v_leaseManagerInit(
    v_leaseManager lm)
{
    v_kernel k;

    k = v_objectKernel(lm);

    c_mutexInit(&lm->mutex, SHARED_MUTEX);
    c_condInit(&lm->cond, &lm->mutex, SHARED_COND);
    lm->quit = FALSE;
    lm->head = NULL;
    lm->leases = c_setNew(v_kernelType(k,K_LEASE));
}

static void
v_leaseManagerDeinit(
    v_leaseManager lm)
{
    v_lease lease;

    c_mutexLock(&lm->mutex);

    c_free(lm->head);
    lease = v_lease(c_take(lm->leases));
    while (lease != NULL) {
        c_free(lease);
        lease = v_lease(c_take(lm->leases));
    }
    c_free(lm->leases);
    lm->leases = NULL;

    lm->quit = TRUE;
    c_condBroadcast(&lm->cond);

    c_mutexUnlock(&lm->mutex);

    /* Note the condition lm->cond is deinitalised via c_free */
}

static c_bool
leaseManagerAdd(
    v_leaseManager lm,
    v_lease lease)
{
    v_lease found;
    c_bool added;

    found = c_setInsert(lm->leases, lease);
    if (found == lease) {
        added = TRUE;
        if (lm->head == NULL) {
            lm->head = c_keep(lease);
            /* head changed, so signal */
            c_condBroadcast(&lm->cond);
        } else {
            if (c_timeCompare(v_leaseExpiryTime(lm->head),
                              v_leaseExpiryTime(lease)) == C_GT) {
                c_free(lm->head);
                lm->head = c_keep(lease);
                /* head changed, so signal */
                c_condBroadcast(&lm->cond);
            }
        }
    } else {
        added = FALSE;
    }

    return added;
}

static c_bool
determineNewHead(
    c_object o,
    c_voidp arg)
{
    c_time headExpTime;
    c_time leaseExpTime;
    v_lease lease = v_lease(o);
    v_leaseManager lm = v_leaseManager(arg);
    
    if (lm->head == NULL) {
        lm->head = c_keep(lease);
    } else {
        headExpTime = v_leaseExpiryTime(lm->head);
        leaseExpTime = v_leaseExpiryTime(lease);

        if (c_timeCompare(headExpTime, leaseExpTime) == C_GT) {
            c_free(lm->head);
            lm->head = c_keep(lease);
        }
    }
    return TRUE;
}


static c_bool
leaseManagerRemove(
    v_leaseManager lm,
    v_lease lease)
{
    v_lease found;
    c_bool removed;

    removed = FALSE;
    found = c_setRemove(lm->leases, lease,NULL,NULL);
    if (found == lease) {
        removed = TRUE;
        
        if (lease == lm->head) {
            c_free(lm->head);
            lm->head = NULL; 
            c_setWalk(lm->leases, determineNewHead, lm);
        }
        c_free(lease); /* delete local reference */
    } else {
        removed = FALSE;
    }
    return removed;
}


struct collectExpiredArg {
    c_iter  expiredLeases;
    v_lease head;
    c_time  now;
};

static c_bool
collectExpired(
    c_object o,
    c_voidp arg)
{
    v_lease lease = v_lease(o);
    struct collectExpiredArg *a = (struct collectExpiredArg *)arg;
    c_time headExpTime;
    c_time leaseExpTime;
    c_bool setHead;
    c_equality cmp;

    setHead = TRUE;
    leaseExpTime = v_leaseExpiryTime(lease);
    /**
     * A lease is expired if the expiry time is greater than or equal
     * to the current time!
     */
    cmp = c_timeCompare(a->now, leaseExpTime);
    if ((cmp ==  C_GT) || (cmp == C_EQ)) {
        a->expiredLeases = c_iterInsert(a->expiredLeases, c_keep(lease));
        /* An expired lease can still become the next expirytime, iff
           the repeatcount > 1
        */
        if (v_leaseRepeatCount(lease) != 1) {
            v_leaseUpdate(lease);
        } else {
            setHead = FALSE;
        }
    }
    if (setHead) {
        if (a->head == NULL) {
            a->head = c_keep(lease);
        } else {
            headExpTime = v_leaseExpiryTime(a->head);
            leaseExpTime = v_leaseExpiryTime(lease);

            if (c_timeCompare(headExpTime, leaseExpTime) == C_GT) {
                c_free(a->head);
                a->head = c_keep(lease);
            }
        }
    }

    return TRUE;
}

struct isElementArg {
    v_lease lease;
    c_bool  isElement;
};

static c_bool
isElement(
    c_object o,
    c_voidp arg)
{
    v_lease lease = v_lease(o);
    struct isElementArg *a = (struct isElementArg *)arg;
    c_bool cont;

    if (lease == a->lease) {
        a->isElement = TRUE;
        cont = FALSE;
    } else {
        cont = TRUE;
    }

    return cont;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_leaseManager
v_leaseManagerNew(
    v_kernel k)
{
    v_leaseManager lm;

    assert(C_TYPECHECK(k, v_kernel));

    lm = v_leaseManager(v_objectNew(k, K_LEASEMANAGER));
    v_leaseManagerInit(lm);

    return lm;
}

void
v_leaseManagerFree(
    v_leaseManager lm)
{
    if (lm != NULL) {
        assert(C_TYPECHECK(lm, v_leaseManager));

        v_leaseManagerDeinit(lm);
        c_free(lm);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_lease
v_leaseManagerRegister(
    v_leaseManager  lm,
    v_public        objectToLease,
    v_duration      leaseDuration,
    v_leaseActionId actionId,
    c_long          repeatCount)
{
    v_lease lease;
    c_bool added;

    assert(lm != NULL);
    assert(C_TYPECHECK(lm, v_leaseManager));
    assert(objectToLease != NULL);
    assert(C_TYPECHECK(objectToLease, v_public));

    lease = v_leaseNew(lm, objectToLease, leaseDuration,
                       actionId, repeatCount);
    if (lease != NULL) {
        c_mutexLock(&lm->mutex);
        added = leaseManagerAdd(lm, lease);
        c_mutexUnlock(&lm->mutex);
        if (added == FALSE) {
            c_free(lease);
            lease = NULL;
        } /* else refcount of lease is transferred to caller */
    }

    return lease;
}

void
v_leaseManagerDeregister(
    v_leaseManager lm,
    v_lease lease)
{
    assert(lm != NULL);
    assert(C_TYPECHECK(lm, v_leaseManager));

    if (lease != NULL) {
        assert(C_TYPECHECK(lease, v_lease));
        c_mutexLock(&lm->mutex);
        leaseManagerRemove(lm, lease);
        c_mutexUnlock(&lm->mutex);
    }
}

/**************************************************************
 * Public functions
 **************************************************************/
void
v_leaseManagerMain(
    v_leaseManager lm)
{
    v_lease lease;
    c_time waitTime = C_TIME_ZERO;
    c_time expTime;
    struct collectExpiredArg arg;

    assert(lm != NULL);
    assert(C_TYPECHECK(lm, v_leaseManager));

    c_mutexLock(&lm->mutex);
    arg.now = v_timeGet();
    while (lm->quit == FALSE) {
        if (lm->head != NULL) {
            expTime = v_leaseExpiryTime(lm->head);
            if (c_timeCompare(expTime, C_TIME_INFINITE) != C_EQ) {
                waitTime = c_timeSub(expTime, arg.now);
                if (c_timeCompare(waitTime, C_TIME_ZERO) == C_GT) {
                    c_condTimedWait(&lm->cond, &lm->mutex, waitTime);
                } else {
                    OS_REPORT(OS_WARNING, "v_leaseManager", 0,
                              "wait time has become negative!");
                }
            } else {
                c_condWait(&lm->cond, &lm->mutex);
            }
        } else {
            c_condWait(&lm->cond, &lm->mutex);
        }

        /**
         * First walk through the collection of leases and record all
         * expired leases in an iterator. We cannot remove expired leases
         * while walking through the set, since it interferes with the 
         * walk.
         */
        arg.expiredLeases = NULL; 
        arg.head = NULL;
        arg.now = v_timeGet();
        c_setWalk(lm->leases, collectExpired, &arg);

        c_free(lm->head);
        lm->head = arg.head;
        c_mutexUnlock(&lm->mutex);

        lease = v_lease(c_iterTakeFirst(arg.expiredLeases));
        while (lease != NULL) {
          if (v_leaseDecRepeatCount(lease) == 0) {
              v_leaseManagerDeregister(lm, lease);
          }
          v_leaseAction(lease, arg.now);
          c_free(lease);
          lease = c_iterTakeFirst(arg.expiredLeases);
        }
        c_iterFree(arg.expiredLeases);

        c_mutexLock(&lm->mutex);
    }
    lm->quit = FALSE; /* for a next time */
    c_mutexUnlock(&lm->mutex);
}

c_bool
v_leaseManagerNotify(
    v_leaseManager lm,
    v_lease lease,
    v_eventKind event)
{
    struct isElementArg isEl;

    assert(lm != NULL);
    assert(C_TYPECHECK(lm, v_leaseManager));

    c_mutexLock(&lm->mutex);
    if (event & V_EVENT_LEASE_RENEWED) {
        if ((lm->head == NULL) || (lm->head == lease)) {
            /* The head can be NULL, lm->leases is not empty, because
               the head lease can be de-registered (making the head NULL),
               but this does not wake-up the thread, because the new head
               can be determined on the next wake-up. But now we are forced
               to wake-up the thread, since this lease might be shorter than
               the remaining sleep time of the thread.
               If the lease is the head we are also forced to wake-up the
               thread, since we do not know the remaining sleeptime of the
               thread.
            */
            c_condBroadcast(&lm->cond);
        } else {
            isEl.lease = lease;
            isEl.isElement = FALSE;
            c_setWalk(lm->leases, isElement, &isEl);
            if (isEl.isElement == TRUE) {
                /* determine if this is the new head */
                if (c_timeCompare(v_leaseExpiryTime(lm->head),
                                  v_leaseExpiryTime(lease)) == C_GT) {
                    c_free(lm->head);
                    lm->head = c_keep(lease);
                    c_condBroadcast(&lm->cond);
                }
            } /* else lease is not registered, so no interest in this update! */
        }
    } else {
        if (event & V_EVENT_TERMINATE) {
            lm->quit = TRUE;
            c_condBroadcast(&lm->cond);
        }
    }
    c_mutexUnlock(&lm->mutex);

    return TRUE;
}
