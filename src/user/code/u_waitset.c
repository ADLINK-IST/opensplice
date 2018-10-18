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
#include "u_waitset.h"
#include "u_waitsetEntry.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__domain.h"
#include "v_waitset.h"
#include "v_observable.h"
#include "v_observer.h"
#include "v_entity.h"
#include "v_event.h"
#include "os_report.h"

#include "u__user.h"
#include "u__waitset.h"
#include "os_atomics.h"


static u_result
waitset_notify (
    const u_waitset _this,
    void *eventArg)
{
    u_result result = U_RESULT_OK;
    c_ulong length;

    assert(_this != NULL);

    length = c_iterLength(_this->entries);
    if (length == 1) {
        /* Single Domain Mode. */
        result = u_waitsetEntryTrigger(c_iterObject(_this->entries,0), eventArg);
    } else {
        /* Multi Domain Mode (or no Domain). */
        os_condSignal(&_this->cv);
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_waitsetTrigger(
    const u_waitset _this)
{
    assert(_this != NULL);
    os_condSignal(&_this->cv);
    return U_RESULT_OK;
}

static u_result
u__waitsetDeinitW(
    void *_vthis)
{
    u_waitset _this;
    u_waitsetEntry entry;
    u_result result = U_RESULT_OK;

    _this = u_waitset(_vthis);
    os_mutexLock(&_this->mutex);

    _this->alive = FALSE;
    while (_this->waitBusy) {
        waitset_notify(_this, NULL);
        os_condWait(&_this->waitCv, &_this->mutex);
    }
    entry = c_iterTakeFirst(_this->entries);
    while (entry != NULL) {
        u_domain domain = u_observableDomain(u_observable(entry));
        result = u_domainRemoveWaitset(domain, _this);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR,
                      "u__waitsetDeinitW", result,
                      "Operation u_domainRemoveWaitset failed: "
                      "Waitset = 0x%"PA_PRIxADDR", result = %s",
                      (os_address)_this, u_resultImage(result));
            assert(FALSE);
        }
        result = u_objectFree_s(entry);
        if (result == U_RESULT_ALREADY_DELETED) {
            result = U_RESULT_OK;
        } else if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR,
                      "u__waitsetDeinitW", result,
                      "Operation u_waitsetEntryFree failed: "
                      "Waitset = 0x%"PA_PRIxADDR", result = %s",
                      (os_address)_this, u_resultImage(result));
            result = U_RESULT_OK;
            (void)result;
            assert(FALSE);
        }
        entry = c_iterTakeFirst(_this->entries);
    }
    c_iterFree(_this->entries);
    _this->entries = NULL;

    os_mutexUnlock(&_this->mutex);
    u__objectDeinitW(_this);
    return result;
}

static void
u__waitsetFreeW(
    void *_this)
{
    u_waitset w;
    w = u_waitset(_this);

    while (pa_ld32(&w->useCount) > 0) {
        os_duration t = OS_DURATION_INIT(0, 100000000);
        ospl_os_sleep(t);
    }

    (void) os_condDestroy(&w->waitCv);
    (void) os_condDestroy(&w->cv);
    (void) os_mutexDestroy(&w->mutex);
    u__objectFreeW(_this);
}

u_waitset
u_waitsetNew2(void)
{
    u_waitset _this = u_waitsetNew();
    _this->eventsEnabled = OS_FALSE;
    return _this;
}

u_waitset
u_waitsetNew(void)
{
    u_waitset _this = NULL;
    u_result result;

    result = u_userInitialise();
    if (result == U_RESULT_OK) {
        _this = u_objectAlloc(sizeof(*_this), U_WAITSET, u__waitsetDeinitW, u__waitsetFreeW);

        _this->entries = NULL;
        _this->eventMask = V_EVENTMASK_ALL;
        _this->alive = TRUE;
        _this->waitBusy = FALSE;
        _this->detachCnt = 0;
        _this->multi_mode = OS_TRUE;
        _this->eventsEnabled = OS_TRUE;
        _this->notifyDetached = OS_FALSE;
        pa_st32(&_this->useCount, 0);

        os_mutexInit(&_this->mutex, NULL);
        os_condInit(&_this->cv, &_this->mutex, NULL);
        os_condInit(&_this->waitCv, &_this->mutex, NULL);
    } else {
        OS_REPORT(OS_ERROR, "u_waitsetNew", result,
                  "Initialization failed. ");
    }

    return _this;
}

void
u_waitsetIncUseCount(
    _Inout_ u_waitset _this)
{
    pa_inc32(&_this->useCount);
}

void
u_waitsetDecUseCount(
    _Inout_ u_waitset _this)
{
    pa_dec32(&_this->useCount);
}

u_result
u_waitsetNotify(
    const u_waitset _this,
    void *eventArg)
{
    u_result result = U_RESULT_OK;
    os_result osr;

    assert(_this != NULL);

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        result = waitset_notify(_this, eventArg);
        os_mutexUnlock(&_this->mutex);
    } else {
        result = U_RESULT_INTERNAL_ERROR;
    }
    return result;
}

struct checkArg {
    os_boolean (*action)(c_voidp, c_voidp);
    void *arg;
    os_int32 count;
};

void
u_waitsetAnnounceDestruction(
    const u_waitset _this)
{
    os_result osr;

    assert(_this != NULL);
    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        _this->alive = FALSE;
        os_mutexUnlock(&_this->mutex);
    }
}

static os_boolean
wait_action(
    void *userData,
    void *arg)
{
    struct checkArg *a = (struct checkArg *)arg;
    if (userData) {
        if (!a->action(userData,a->arg)) a->count++;
    }
    return OS_TRUE;
}

static c_bool
check_entry_conditions(
    void *entry,
    c_iterResolveCompareArg arg)
{
    (void)u_waitsetEntryWait2(entry, wait_action, arg, OS_DURATION_ZERO);
    return TRUE;
}

u_result
u_waitsetWaitAction2 (
    const u_waitset _this,
    u_waitsetAction2 action,
    void *arg,
    const os_duration timeout)
{
    u_result result = U_RESULT_OK;
    os_result osr;
    c_ulong length;
    struct checkArg a;
    a.action = action;
    a.arg = arg;
    a.count = 0;

    assert(_this != NULL);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        if (!_this->alive) {
            result = U_RESULT_ALREADY_DELETED;
            OS_REPORT(OS_ERROR, "u_waitsetWaitAction2", result,
                      "Precondition not met: Waitset is already deleted");
        }
        if (_this->waitBusy) {
            result = U_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR, "u_waitsetWaitAction2", result,
                      "Precondition not met: A Wait call is already active on this Waitset");
        }
        if (result == U_RESULT_OK) {
            /* Wait for possible detach to complete.
             * If you don't do that, it's possible that this wait call sets
             * the waitBusy flag before the detach can wake up of its waitBusy
             * loop, meaning that the detach will block at least until the
             * waitset is triggered again.
             */
            while (_this->detachCnt > 0) {
                os_condWait(&_this->waitCv, &_this->mutex);
            }

            length = c_iterLength(_this->entries);
            if (length == 1) {
                /* Single Domain Mode. */
                u_waitsetEntry entry = c_iterObject(_this->entries,0);
                _this->waitBusy = TRUE;
                os_mutexUnlock(&_this->mutex);

                result = u_waitsetEntryWait2(entry, action, arg, timeout);

                os_mutexLock(&_this->mutex);
                _this->waitBusy = FALSE;

                if (_this->notifyDetached) {
                    result = U_RESULT_DETACHING;
                    _this->notifyDetached = OS_FALSE;
                }

                os_condBroadcast(&_this->waitCv);
                os_mutexUnlock(&_this->mutex);

                if ((result == U_RESULT_OK) && (_this->alive == FALSE)) {
                    result = U_RESULT_ALREADY_DELETED;
                    OS_REPORT(OS_ERROR, "u_waitsetWaitAction2", result,
                              "Precondition not met: Waitset is already deleted");
                }
            } else {
                /* Multi Domain Mode (or no Domain). */
                a.count = 0;
                /* For each Domain test Conditions. */
                (void)c_iterWalkUntil(_this->entries, check_entry_conditions, &a);
                /* Test Guard Conditions */
                if ((a.count == 0) && (!action(NULL,arg))) {
                    a.count++;
                }
                /* If No Conditions are true then wait. */
                if (a.count == 0) {
                    _this->waitBusy = TRUE;
                    if (OS_DURATION_ISINFINITE(timeout)) {
                        os_condWait(&_this->cv, &_this->mutex);
                        osr = os_resultSuccess;
                    } else {
                        osr = os_condTimedWait(&_this->cv, &_this->mutex, timeout);
                    }
                    _this->waitBusy = FALSE;
                    os_condBroadcast(&_this->waitCv);
                    switch (osr) {
                    case os_resultSuccess:
                        if (_this->alive == TRUE) {
                            if (_this->notifyDetached) {
                                result = U_RESULT_DETACHING;
                                _this->notifyDetached = OS_FALSE;
                            } else {
                                result = U_RESULT_OK;
                            }
                        } else {
                            result = U_RESULT_ALREADY_DELETED;
                            OS_REPORT(OS_ERROR, "u_waitsetWaitAction2", result,
                                      "Precondition not met: Waitset is already deleted");
                        }
                    break;
                    case os_resultTimeout:
                        result = U_RESULT_TIMEOUT;
                    break;
                    default:
                        result = U_RESULT_INTERNAL_ERROR;
                        OS_REPORT(OS_ERROR, "u_waitsetWaitAction2", result,
                                    "os_condWait failed for waitset 0x" PA_ADDRFMT,
                                    (PA_ADDRCAST)_this);
                    break;
                    }
                }
                os_mutexUnlock(&_this->mutex);
            }
        } else {
            os_mutexUnlock(&_this->mutex);
        }
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR, "u_waitsetWaitAction2", result,
                    "os_mutexLock failed for waitset 0x" PA_ADDRFMT,
                    (PA_ADDRCAST)_this);
    }
    return result;
}

u_result
u_waitsetWaitAction (
    const u_waitset _this,
    u_waitsetAction action,
    void *arg,
    const os_duration timeout)
{
    u_result result = U_RESULT_OK;
    os_result osr;
    c_ulong length;

    assert(_this != NULL);
    assert(action != NULL);
    assert(OS_DURATION_ISPOSITIVE(timeout));

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        if (!_this->alive) {
            result = U_RESULT_ALREADY_DELETED;
        }
        if (result == U_RESULT_OK) {
            if (!_this->waitBusy) {
                /* Wait for possible detach to complete.
                 * If you don't do that, it's possible that this wait call sets
                 * the waitBusy flag before the detach can wake up of its waitBusy
                 * loop, meaning that the detach will block at least until the
                 * waitset is triggered again.
                 */
                while (_this->detachCnt > 0) {
                    os_condWait(&_this->waitCv, &_this->mutex);
                }

                _this->waitBusy = TRUE;
                length = c_iterLength(_this->entries);
                if (length == 1) {
                    /* Single Domain Mode. */
                    u_waitsetEntry entry = c_iterObject(_this->entries,0);
                    os_mutexUnlock(&_this->mutex);
                    result = u_waitsetEntryWait(entry, action, arg, timeout);

                    os_mutexLock(&_this->mutex);
                    _this->waitBusy = FALSE;
                    os_condBroadcast(&_this->waitCv);
                    os_mutexUnlock(&_this->mutex);

                    if ((result == U_RESULT_OK) && (_this->alive == FALSE)) {
                        result = U_RESULT_ALREADY_DELETED;
                    }
                } else {
                    /* Multi Domain Mode (or no Domain). */
                    if (OS_DURATION_ISINFINITE(timeout)) {
                        os_condWait(&_this->cv, &_this->mutex);
                        osr = os_resultSuccess;
                    } else {
                        osr = os_condTimedWait(&_this->cv, &_this->mutex, timeout);
                    }
                    _this->waitBusy = FALSE;
                    os_condBroadcast(&_this->waitCv);
                    switch (osr) {
                    case os_resultSuccess:
                        if (_this->alive == TRUE) {
                            result = U_RESULT_OK;
                        } else {
                            result = U_RESULT_ALREADY_DELETED;
                        }
                    break;
                    case os_resultTimeout:
                        result = U_RESULT_TIMEOUT;
                    break;
                    default:
                        result = U_RESULT_INTERNAL_ERROR;
                        OS_REPORT(OS_ERROR, "u_waitsetWaitAction", result,
                                    "os_condWait failed for waitset 0x" PA_ADDRFMT,
                                    (PA_ADDRCAST)_this);
                    break;
                    }
                    os_mutexUnlock(&_this->mutex);
                }
            } else {
                os_mutexUnlock(&_this->mutex);
                result = U_RESULT_PRECONDITION_NOT_MET;
            }
        } else {
            os_mutexUnlock(&_this->mutex);
        }
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR, "u_waitsetWaitAction", result,
                    "os_mutexLock failed for waitset 0x" PA_ADDRFMT,
                    (PA_ADDRCAST)_this);
    }
    return result;
}

static c_equality
compare_domain(
    void *o,
    c_iterResolveCompareArg arg)
{
    c_equality result;
    u_waitsetEntry entry;
    u_domain domain;

    entry = u_waitsetEntry(o);
    domain = u_domain(arg);

    if ((entry != NULL) &&
        (domain != NULL) &&
        (domain == u_observableDomain(u_observable(entry))))
    {
        result = C_EQ;
    } else {
        result = C_NE;
    }
    return result;
}

static void
set_multi_mode (
    c_object o,
    c_voidp arg)
{
    u_waitsetEntry entry = (u_waitsetEntry)o;

    u_waitsetEntrySetMode(entry, *(os_boolean *)arg);
}

u_result
u_waitsetAttach(
    const u_waitset _this,
    const u_observable observable,
    void *context)
{
    u_waitsetEntry entry;
    u_domain domain;
    u_result result;
    c_ulong length;
    u_bool changed = FALSE;
    os_result osr;

    assert(_this != NULL);
    assert(observable != NULL);

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        length = c_iterLength(_this->entries);
        domain = u_observableDomain(observable);
        if (domain != NULL) {
            entry = c_iterResolve(_this->entries, compare_domain, domain);
        } else {
            entry = NULL;
        }
        if ((entry == NULL)&&(domain != NULL)) {
            result = u_domainAddWaitset(domain, _this);
            if (result == U_RESULT_OK) {
                entry = u_waitsetEntryNew(_this, domain, _this->eventMask);
                if (entry != NULL) {
                    _this->entries = c_iterInsert(_this->entries, entry);
                    changed = TRUE;
                }
            } else {
                result = U_RESULT_INTERNAL_ERROR;
                OS_REPORT(OS_ERROR, "u_waitSetAttach", result, "Failed to add waitset to domain.");
            }
        }
        if (entry != NULL) {
            result = u_waitsetEntryAttach(entry, observable, context);
        } else {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR, "u_waitSetAttach", result, "Failed to connect to domain.");
        }
        if (changed == TRUE) {
            if (length == 0) {
                /* Wakeup waitset because its no longer in zero domain mode */
                _this->multi_mode = OS_FALSE;
                os_condSignal(&_this->cv);
                result = U_RESULT_OK;
            } else if (length == 1) {
                _this->multi_mode = OS_TRUE;
                c_iterWalk(_this->entries, set_multi_mode, (c_voidp)&_this->multi_mode);
            }
        }
        os_mutexUnlock(&_this->mutex);
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR, "u_waitSetAttach", result, "Could not lock the waitset.");
    }
    return result;
}

u_result
u_waitsetDetach_s(
    const u_waitset _this,
    const u_observable observable)
{
    u_waitsetEntry entry;
    u_domain domain;
    u_result result;
    os_result osr;

    assert(_this != NULL);
    assert(observable != NULL);

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        domain = u_observableDomain(observable);
        if (domain != NULL) {
            entry = c_iterResolve(_this->entries, compare_domain, domain);
            if (entry != NULL) {
                /* The following detach will wakeup any blocking wait call on this entry. */
                result = u_waitsetEntryDetach(entry, observable);
                if (result == U_RESULT_UNSUPPORTED) {
                    _this->detachCnt++;
                    /* removed last observable for this entry so can detach from domain. */
                    entry = c_iterTake(_this->entries, entry); /* use overwrite entry */
                    result = u_domainRemoveWaitset(domain, _this);
                    if (c_iterLength(_this->entries) == 1) {
                        _this->multi_mode = OS_FALSE;
                    } else {
                        _this->multi_mode = OS_TRUE;
                    }
                    while (_this->waitBusy) {
                        os_condWait(&_this->waitCv, &_this->mutex);
                    }

                    _this->detachCnt--;
                    /* Broadcast the detachCnt update. */
                    os_condBroadcast(&_this->waitCv);

                    os_mutexUnlock(&_this->mutex);
                    u_objectFree(entry);
                } else {
                    os_mutexUnlock(&_this->mutex);
                }
            } else {
                /* Check if the condition is already deleted */
                v_public ko;
                result = u_observableReadClaim(observable, &ko, C_MM_RESERVATION_NO_CHECK);
                if (result == U_RESULT_OK) {
                    u_observableRelease(observable, C_MM_RESERVATION_NO_CHECK);
                    result = U_RESULT_PRECONDITION_NOT_MET;
                    OS_REPORT(OS_ERROR, "u_waitSetDetach_s", result, "Condition is not attached to Waitset");
                }
                os_mutexUnlock(&_this->mutex);
            }
        } else {
            os_mutexUnlock(&_this->mutex);
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR, "u_waitsetDetach_s", result,
                      "Failed to connect to domain.");
        }
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR, "u_waitSetDetach_s", result, "Could not lock the waitset.");
    }
    return result;
}

void
u_waitsetDetach(
    const u_waitset _this,
    const u_observable observable)
{
    /* u_waitsetDetach_s already reports errors */
    (void) u_waitsetDetach_s (_this, observable);
}

u_result
u_waitsetGetEventMask(
    const u_waitset _this,
    u_eventMask *eventMask)
{
    u_result result;
    os_result osr;

    assert(_this != NULL);
    assert(eventMask != NULL);

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        result = U_RESULT_OK;
        *eventMask = _this->eventMask;
        os_mutexUnlock(&_this->mutex);
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_WARNING, "u_waitGetEventMask", result,
                  "Could not claim waitset.");
    }
    return result;
}

static void
set_event_mask(
    void *o,
    c_iterResolveCompareArg arg)
{
    u_waitsetEntry entry;
    u_eventMask eventMask;
    u_result result;

    entry = u_waitsetEntry(o);
    eventMask = *(c_ulong *)arg;

    if (entry != NULL)
    {
        result = u_waitsetEntrySetEventMask(entry, eventMask);
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    assert(result == U_RESULT_OK);
    OS_UNUSED_ARG(result);
}

u_result
u_waitsetSetEventMask(
    const u_waitset _this,
    u_eventMask eventMask)
{
    u_result result;
    os_result osr;

    assert(_this != NULL);

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        _this->eventMask = eventMask;
        c_iterWalk(_this->entries, set_event_mask, &eventMask);
        os_mutexUnlock(&_this->mutex);
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_WARNING, "u_waitSetEventMask", result,
                  "Could not claim waitset.");
    }
    return result;
}

u_result
u_waitsetDetachFromDomain(
    _Inout_ u_waitset _this,
    _Inout_ u_domain domain)
{
    u_result result;
    os_result osr;
    u_waitsetEntry entry;

    assert(_this != NULL);
    assert(domain != NULL);

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        entry = c_iterResolve(_this->entries, compare_domain, domain);
        if (entry != NULL) {
            _this->notifyDetached = OS_TRUE;
            result = u_objectClose(entry);
            if (result == U_RESULT_ALREADY_DELETED) {
                result = U_RESULT_OK;
            }
            if (result == U_RESULT_OK) {
                /* The entry is already freed but the address value can still
                 * be used to update the administration because it only removes
                 * the address value from the list.
                 */
                c_iterTake(_this->entries, entry);
            } else {
                result = U_RESULT_INTERNAL_ERROR;
                OS_REPORT(OS_ERROR,
                            "u_waitsetDetachFromDomain", result,
                            "Operation u_waitsetEntryFree failed: "
                            "Waitset = 0x%"PA_PRIxADDR", result = %s",
                             (os_address)_this, u_resultImage(result));
                assert(FALSE);
            }
        } else {
            result = U_RESULT_PRECONDITION_NOT_MET;
        }
        (void)u_domainRemoveWaitset(domain, _this);
        os_mutexUnlock(&_this->mutex);
    } else {
        result = U_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_WARNING, "u_waitsetDetachFromDomain", result,
                  "Could not claim waitset.");
    }

    return result;
}

u_domainId_t
u_waitsetGetDomainId(
    u_waitset _this)
{
    os_result osr;
    u_domainId_t domainId = -1;
    u_waitsetEntry entry;

    osr = os_mutexLock_s(&_this->mutex);
    if (osr == os_resultSuccess) {
        if (c_iterLength(_this->entries) == 1) {
            entry = c_iterObject(_this->entries, 0);
            domainId = u_observableGetDomainId(u_observable(entry));
        }
        os_mutexUnlock(&_this->mutex);
    }

    return domainId;
}
