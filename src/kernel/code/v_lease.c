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

#include "v__lease.h"
#include "v__leaseManager.h"

#include "v_event.h"

#include "vortex_os.h"
#include "os_report.h"

/* For design information, see v_leaseManager.c */

/**************************************************************
 * Private function/struct declarations
 **************************************************************/

static void
v_leaseInit(
    v_lease _this,
    v_kernel k,
    v_leaseKind kind,
    os_duration leaseDuration);

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
    v_leaseKind kind,
    os_duration leaseDuration)
{
    v_lease _this;

    _this = v_lease(v_objectNew(k, K_LEASE));
    if(_this) {
        v_leaseInit(_this, k, kind, leaseDuration);
    }
    return _this;
}

v_lease
v_leaseMonotonicNew(
    v_kernel k,
    os_duration leaseDuration)
{
    return v_leaseNew(k, V_LEASE_KIND_MONOTONIC, leaseDuration);
}

v_lease
v_leaseElapsedNew(
    v_kernel k,
    os_duration leaseDuration)
{
    return v_leaseNew(k, V_LEASE_KIND_ELAPSED, leaseDuration);
}

void
v_leaseInit(
    v_lease _this,
    v_kernel k,
    v_leaseKind kind,
    os_duration leaseDuration)
{
    if (_this != NULL) {
        assert(C_TYPECHECK(_this, v_lease));

        c_mutexInit(c_getBase(_this), &_this->mutex);
        v_leaseTimeInit(&_this->expiryTime, kind, leaseDuration);
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
    os_duration leaseDuration)
{
    c_iter observers;
    v_leaseManager observer;

    if (_this != NULL) {
        assert(C_TYPECHECK(_this, v_lease));

        v_leaseLock(_this);
        /* If a duration is supplied, replace the current lease duration */
        if(!OS_DURATION_ISINVALID(leaseDuration)) {
            _this->duration = leaseDuration;
        }

        /* Update the expiryTime */
        v_leaseTimeUpdate(&_this->expiryTime, _this->duration);
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

os_duration
v_leaseDuration(
    v_lease _this)
{
    os_duration duration;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    v_leaseLock(_this);
    duration = v_leaseDurationNoLock(_this);
    v_leaseUnlock(_this);

    return duration;
}

os_duration
v_leaseDurationNoLock(
    v_lease _this)
{
    os_duration duration;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    duration = _this->duration;

    return duration;
}

v_leaseTime
v_leaseExpiryTime(
    v_lease _this)
{
    v_leaseTime expTime;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    v_leaseLock(_this);
    expTime = v_leaseExpiryTimeNoLock(_this);
    v_leaseUnlock(_this);

    return expTime;
}

v_leaseTime
v_leaseExpiryTimeNoLock(
    v_lease _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_lease));

    return _this->expiryTime;
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
