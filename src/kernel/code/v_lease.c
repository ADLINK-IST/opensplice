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

#include "os_report.h"

#include "v__kernel.h"
#include "v__lease.h"
#include "v__leaseManager.h"
#include "v_time.h"
#include "v_event.h"
#include "v_entity.h"
#include "v_public.h"

/* For design information, see v_leaseManager.c */

/**************************************************************
 * Private functions
 **************************************************************/
static void
v_leaseInit(
    v_lease _this,
    v_kernel k,
    v_duration leaseDuration);

static c_bool
v_leaseCollectObservers(
    c_object o,
    c_voidp arg);

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_lease
v_leaseNew(
    v_kernel k,
    v_duration leaseDuration)
{
    v_lease _this;

    _this = v_lease(v_objectNew(k, K_LEASE));
    if(_this)
    {
        v_leaseInit(_this, k, leaseDuration);
    }
    return _this;
}

void
v_leaseInit(
    v_lease _this,
    v_kernel k,
    v_duration leaseDuration)
{
    if (_this != NULL)
    {
        assert(C_TYPECHECK(_this, v_lease));

        c_mutexInit(&_this->mutex,SHARED_MUTEX);
        _this->expiryTime = c_timeAdd(v_timeGet(), leaseDuration);
        _this->duration = leaseDuration;
        _this->observers = c_setNew(v_kernelType(k, K_LEASEMANAGER));
    }
}

void
v_leaseDeinit(
    v_lease lease)
{
    v_leaseManager lm;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease,v_lease));

    if (lease != NULL)
    {
        lm = v_leaseManager(c_take(lease->observers));
        while (lm != NULL)
        {
            c_free(lm);
            lm = v_leaseManager(c_take(lease->observers));
        }
        c_free(lease->observers);
        lease->observers = NULL;
    }
}
/**************************************************************
 * Protected functions
 **************************************************************/

void
v_leaseLock(
    v_lease _this)
{

    assert(_this);
    assert(C_TYPECHECK(_this, v_lease));
    c_mutexLock(&_this->mutex);
}

void
v_leaseUnlock(
    v_lease _this)
{

    assert(_this);
    assert(C_TYPECHECK(_this, v_lease));
    c_mutexUnlock(&_this->mutex);
}

void
v_leaseRenew(
    v_lease lease,
    v_duration* leaseDuration /* may be NULL */)
{
    c_iter observers = NULL;
    v_leaseManager observer;
    c_time newExpiryTime;
    c_equality cmp;

    if (lease != NULL) {
        assert(C_TYPECHECK(lease, v_lease));

        v_leaseLock(lease);
        /* Is a new lease duration provided, if so replace the current lease
         * duration with the new one
         */
        if(leaseDuration != NULL)
        {
            lease->duration = *leaseDuration;
        } /* else do nothing */
        /* Calculate the new expiry time */
        newExpiryTime = c_timeAdd(v_timeGet(), lease->duration);
        /* Is the new expiryTime earlier then the current expiryTime? */
        cmp = c_timeCompare(newExpiryTime, lease->expiryTime);
        /* Always replace the current expiry time with the new expiryTime */
        lease->expiryTime = newExpiryTime;
        /* If the new expiryTime is earlier then the previous expiryTime. Then
         * this means the observers must be notified so they can take the
         * earlier expiryTime into account
         */
        if (cmp == C_LT)
        {

            /* Collect all observers, so they can be notified of the lease change
             * Must do a seperate collect as the lease mutex is 'lower' then the
             * leaseManager mutex. So to prevent deadlock we can not directly notify
             * the lease manager as we walk the collection
             */
            if(lease->observers)
            {
                c_walk(lease->observers, v_leaseCollectObservers, &observers);
            }
            v_leaseUnlock(lease);
            if(observers)
            {
                observer = v_leaseManager(c_iterTakeFirst(observers));
                while (observer != NULL)
                {
                    v_leaseManagerNotify(
                        observer,
                        lease,
                        V_EVENT_LEASE_RENEWED);
                    c_free(observer);
                    observer = v_leaseManager(c_iterTakeFirst(observers));
                }
                c_iterFree(observers);
            }
        } else
        {
            /* No need to notify observers, the new expiryTime is not earlier
             * then what it was before.
             */
            v_leaseUnlock(lease);
        }
    }
}

c_bool
v_leaseCollectObservers(
    c_object o,
    c_voidp arg)
{
    c_iter *observers = (c_iter*)arg;
    *observers = c_iterInsert(*observers, c_keep(o));
    return TRUE;
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
        v_leaseLock(lease);
        if(expiryTime){
            *expiryTime = lease->expiryTime;
        }
        if(duration){
            *duration = lease->duration;
        }
        v_leaseUnlock(lease);
    }
}

c_time
v_leaseExpiryTime(
    v_lease lease)
{
    c_time expTime;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    v_leaseLock(lease);
    expTime = v_leaseExpiryTimeNoLock(lease);
    v_leaseUnlock(lease);

    return expTime;
}

c_time
v_leaseExpiryTimeNoLock(
    v_lease lease)
{
    c_time expTime;

    assert(lease != NULL);
    assert(C_TYPECHECK(lease, v_lease));

    expTime = lease->expiryTime;

    return expTime;
}

c_bool
v_leaseAddObserverNoLock(
    v_lease _this,
    v_leaseManager observer)
{
    c_bool added;
    v_leaseManager found;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));
    assert(observer != NULL);
    assert(C_TYPECHECK(observer, v_leaseManager));

    if(_this->observers)
    {
        found = c_setInsert(_this->observers, observer);
        if (found == observer)
        {
            added = TRUE;
        } else
        {
            added = FALSE;
        }
    } else
    {
        added = FALSE;
    }
    return added;
}

c_bool
v_leaseRemoveObserverNoLock(
    v_lease _this,
    v_leaseManager observer)
{
    c_bool removed;
    v_leaseManager found;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));
    assert(observer != NULL);
    assert(C_TYPECHECK(observer, v_leaseManager));

    if(_this->observers)
    {
        found = c_setRemove(_this->observers, observer, NULL, NULL);
        if (found == observer)
        {
            removed = TRUE;
            /* delete local reference */
            c_free(observer);
        } else {
            removed = FALSE;
        }
    } else
    {
        removed = FALSE;
    }
    return removed;
}

/**************************************************************
 * Public functions
 **************************************************************/
