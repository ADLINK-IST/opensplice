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

#include "v__lease.h"
#include "v__leaseManager.h"

#include "v_time.h"
#include "v_event.h"

#include "os_report.h"

/* For design information, see v_leaseManager.c */

/**************************************************************
 * Private function/struct declarations
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

static void
v_leaseRenewCommon(
    v_lease _this,
    v_duration* leaseDuration,
    c_bool internal);

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
    if(_this) {
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
    if (_this != NULL) {
        assert(C_TYPECHECK(_this, v_lease));

        c_mutexInit(&_this->mutex, SHARED_MUTEX);
        _this->expiryTime = c_timeAdd(v_timeGet(), leaseDuration);
        _this->duration = leaseDuration;
        _this->lastRenewInternal = FALSE;
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

    if (lease != NULL) {
        lm = v_leaseManager(c_take(lease->observers));
        while (lm != NULL) {
            c_free(lm);
            lm = v_leaseManager(c_take(lease->observers));
        }
        c_free(lease->observers);
        lease->observers = NULL;
    }
}

/**************************************************************
 * Local functions
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
    v_lease _this,
    v_duration* leaseDuration /* may be NULL */)
{
    v_leaseRenewCommon(_this, leaseDuration, FALSE);
}

void
v_leaseRenewInternal(
    v_lease _this,
    v_duration* leaseDuration /* may be NULL */)
{
    v_leaseRenewCommon(_this, leaseDuration, TRUE);
}

static void
v_leaseRenewCommon(
    v_lease _this,
    v_duration* leaseDuration /* may be NULL */,
    c_bool internal)
{
    c_iter observers;
    v_leaseManager observer;

    if (_this != NULL) {
        assert(C_TYPECHECK(_this, v_lease));

        v_leaseLock(_this);
        /* If a duration is supplied, replace the current lease duration */
        if(leaseDuration != NULL) {
            _this->duration = *leaseDuration;
        }

        /* Update the expiryTime */
        _this->expiryTime = c_timeAdd(v_timeGet(), _this->duration);
        _this->lastRenewInternal = internal;

        /* Notify observers */
        observers = NULL;
        c_walk(_this->observers, v_leaseCollectObservers, &observers);
        v_leaseUnlock(_this);

        observer = v_leaseManager(c_iterTakeFirst(observers));
        while (observer != NULL) {
            v_leaseManagerNotify(observer, _this, V_EVENT_LEASE_RENEWED);
            c_free(observer);
            observer = v_leaseManager(c_iterTakeFirst(observers));
        }
        c_iterFree(observers);
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

v_duration
v_leaseDuration(
    v_lease _this)
{
    v_duration duration;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    v_leaseLock(_this);
    duration = v_leaseDurationNoLock(_this);
    v_leaseUnlock(_this);

    return duration;
}

v_duration
v_leaseDurationNoLock(
    v_lease _this)
{
    v_duration duration;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    duration = _this->duration;

    return duration;
}

c_time
v_leaseExpiryTime(
    v_lease _this)
{
    c_time expTime;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    v_leaseLock(_this);
    expTime = v_leaseExpiryTimeNoLock(_this);
    v_leaseUnlock(_this);

    return expTime;
}

c_time
v_leaseExpiryTimeNoLock(
    v_lease _this)
{
    c_time expTime;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    expTime = _this->expiryTime;

    return expTime;
}

c_bool
v_leaseLastRenewInternalNoLock(
    v_lease _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    return _this->lastRenewInternal;
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

    if(_this->observers) {
        found = c_setInsert(_this->observers, observer);
        if (found == observer) {
            added = TRUE;
        } else {
            added = FALSE;
        }
    } else {
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

    if(_this->observers) {
        found = c_setRemove(_this->observers, observer, NULL, NULL);
        if (found == observer) {
            removed = TRUE;
            /* delete local reference */
            c_free(observer);
        } else {
            removed = FALSE;
        }
    } else {
        removed = FALSE;
    }
    return removed;
}

/**************************************************************
 * Public functions
 **************************************************************/
