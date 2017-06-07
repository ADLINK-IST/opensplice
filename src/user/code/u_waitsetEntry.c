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
#include "u__user.h"
#include "u__types.h"
#include "u_waitsetEntry.h"
#include "u_waitset.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u_participant.h"

#include "v_waitset.h"
#include "v_observable.h"
#include "os_heap.h"
#include "os_report.h"

#define TIME_CONVERTION_REQUIRED

static os_boolean
check_trigger(
    c_voidp e,
    c_voidp arg)
{
    os_boolean proceed;

    OS_UNUSED_ARG(arg);

    if (e == NULL) {
        proceed = OS_TRUE;
    } else {
        proceed = OS_FALSE;
    }

    return proceed;
}

static void *
u_waitsetEntryRun(
    u_waitsetEntry _this)
{
    os_duration delay;
    u_result result = U_RESULT_OK;
    v_waitset kw;
    v_result kr;

    assert(_this);

    delay = OS_DURATION_INIT(0, 10000); /* 10 usec */

    os_mutexLock(&_this->mutex);
    _this->waitCount++;
    while (_this->waitCount > 1) {
        /* do nothing,
         * just wait until all application waits has finished before starting.
         */
        os_mutexUnlock(&_this->mutex);
        os_sleep(delay);
        os_mutexLock(&_this->mutex);
    }
    while (os_threadIdToInteger(_this->thread) != os_threadIdToInteger(OS_THREAD_ID_NONE) &&
           (result == U_RESULT_OK) && _this->alive)
    {
        u_domain domain = u_observableDomain(u_observable(_this));
        result = u_domainProtect(domain);
        if (result == U_RESULT_OK) {
            result = u_handleClaim(_this->handle, &kw);
            if (result == U_RESULT_OK) {
                os_mutexUnlock(&_this->mutex);
                kr = v_waitsetWait2(kw,check_trigger,NULL,OS_DURATION_INFINITE);
                result = u_resultFromKernel(kr);
                os_mutexLock(&_this->mutex);
                (void)u_handleRelease(_this->handle);
                (void)u_waitsetTrigger(_this->waitset);
            }
            u_domainUnprotect();
        }
    }
    _this->waitCount--;
    if (result != U_RESULT_OK) {
        _this->thread = OS_THREAD_ID_NONE;
        OS_REPORT(OS_ERROR,
                  "user::u_waitsetEntryRun", U_RESULT_INTERNAL_ERROR,
                  "Domain thread stopped by unexpected error");
    }
    os_mutexUnlock(&_this->mutex);
    return NULL;
}

static u_result
u__waitsetEntryDeinitW (
    void *_vthis)
{
    u_waitsetEntry _this = _vthis;
    u_result result;
    os_threadId thread;
    os_mutexLock(&_this->mutex);
    _this->alive = FALSE;
    thread = _this->thread;
    _this->thread = OS_THREAD_ID_NONE;
    result = u_waitsetEntryTrigger(_this,NULL);
    while (_this->waitCount != 0) {
        const os_duration delay = OS_DURATION_INIT(0, 1 * 1000 * 1000);
        os_mutexUnlock(&_this->mutex);
        os_sleep(delay);
        os_mutexLock(&_this->mutex);
    }
    os_mutexUnlock(&_this->mutex);
    if (os_threadIdToInteger(thread) != os_threadIdToInteger(OS_THREAD_ID_NONE)) {
        /* If the local thread was running then wait until its gone. */
        (void)os_threadWaitExit(thread, NULL);
    }
    u__observableDeinitW(_this);
    return result;
}

static void
u__waitsetEntryFreeW (
    void *_this)
{
    os_mutexDestroy(&(u_waitsetEntry(_this)->mutex));
    u__observableFreeW(_this);
}

u_waitsetEntry
u_waitsetEntryNew(
    const u_waitset waitset,
    const u_domain domain,
    const c_ulong eventMask)
{
    u_waitsetEntry _this = NULL;
    v_waitset kw;

    assert(waitset != NULL);
    assert(domain != NULL);

    /* Waitset conditions are associated with the built-in participant
     * because the waitset is domain agnostic and the 'creating' participant
     * may be removed before the waitset.
     */
    kw = v_waitsetNew(domain->kernel->builtin->participant);
    if (kw != NULL) {
        _this = u_objectAlloc(sizeof(*_this), U_WAITSETENTRY,u__waitsetEntryDeinitW,u__waitsetEntryFreeW);
        if (_this != NULL) {
            u_result result = u_observableInit(u_observable(_this), v_public(kw), domain);
            if (result == U_RESULT_OK) {
                 os_mutexInit(&_this->mutex, NULL);

                _this->handle = u_handleNew(v_public(kw));
                _this->thread = OS_THREAD_ID_NONE;
                _this->waitset = waitset;
                _this->waitCount = 0;
                _this->alive = TRUE;
                if (!waitset->eventsEnabled) {
                    v_waitsetLogEvents(kw, OS_FALSE);
                }
            } else {
                u_objectFree(_this);
                _this = NULL;
            }
        }
        v_observerSetEventMask(v_observer(kw), eventMask);
        c_free(kw);
    } else {
        OS_REPORT(OS_ERROR, "u_waitsetEntryNew", U_RESULT_OUT_OF_MEMORY,
                  "Operation failed to create a kernel waitset");
    }
    return _this;
}

u_result
u_waitsetEntrySetMode(
    u_waitsetEntry _this,
    u_bool multimode)
{
    u_result result = U_RESULT_OK;
    v_waitset kw;
    os_result osResult;
    os_threadAttr osThreadAttr;
    os_threadId thread;

    assert(_this != NULL);

    if ((multimode == TRUE) &&
        (os_threadIdToInteger(_this->thread) == os_threadIdToInteger(OS_THREAD_ID_NONE)))
    {
        /* Switch to multi domain mode and start waitset entry thread.
         */
        os_threadAttrInit(&osThreadAttr);
        osResult = os_threadCreate(
                        &_this->thread,
                        "WaitSetEntryThread",
                        &osThreadAttr,
                        (void *(*)(void *))u_waitsetEntryRun,
                        (void *)_this);
        if (osResult != os_resultSuccess) {
            result = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR,
                      "u_waitsetEntrySetMode", result,
                      "Operation failed to start waitset entry thread");
        }
    }
    if ((multimode == FALSE) &&
        (os_threadIdToInteger(_this->thread) != os_threadIdToInteger(OS_THREAD_ID_NONE)))
    {
        /* Switch to single domain mode and stop waitset entry thread.
         */
        u_domain domain = u_observableDomain(u_observable(_this));
        result = u_domainProtect(domain);
        if (result == U_RESULT_OK) {
            result = u_handleClaim(_this->handle, &kw);
            if (result == U_RESULT_OK) {
                thread = _this->thread;
                _this->thread = OS_THREAD_ID_NONE;
                v_waitsetTrigger(kw, NULL);
                osResult = os_threadWaitExit(thread, NULL);
                (void)u_handleRelease(_this->handle);
                if (osResult != os_resultSuccess) {
                    result = U_RESULT_INTERNAL_ERROR;
                    OS_REPORT(OS_ERROR,
                              "u_waitsetEntrySetMode", result,
                              "Operation failed to stop waitset entry thread");
                }
            }
            u_domainUnprotect();
        }
    }
    return result;
}

u_result
u_waitsetEntryAttach (
    const u_waitsetEntry _this,
    const u_observable observable,
    c_voidp context)
{
    u_result result;
    u_domain domain;
    v_waitset kWaitset;
    v_public kObject;

    assert(_this != NULL);
    assert(observable != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        result = u_handleClaim(_this->handle, &kWaitset);
        if (result == U_RESULT_OK) {
            result = u_observableReadClaim(observable, &kObject, C_MM_RESERVATION_ZERO);
            if (result == U_RESULT_OK) {
                v_waitsetAttach(kWaitset,v_observable(kObject),context);
                u_observableRelease(observable, C_MM_RESERVATION_ZERO);
            } else {
                OS_REPORT(OS_ERROR, "u_waitsetEntryAttach", result,
                            "Could not claim supplied observable (0x%"PA_PRIxADDR").",
                            (os_address)observable);
            }
            (void)u_handleRelease(_this->handle);
        } else {
            OS_REPORT(OS_ERROR, "u_waitsetEntryAttach", result,
                        "Could not claim kernel waitset (%"PA_PRIxADDR":%x:%x).",
                        _this->handle.server, _this->handle.index, _this->handle.serial);
        }
        u_domainUnprotect();
    }
    return result;
}

u_result
u_waitsetEntryDetach (
    const u_waitsetEntry _this,
    const u_observable observable)
{
    u_result result;
    u_domain domain;
    v_waitset kWaitset;
    v_public kObject;
    c_long count;

    assert(_this != NULL);
    assert(observable != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        result = u_handleClaim(_this->handle, &kWaitset);
        if (result == U_RESULT_OK) {
            result = u_observableReadClaim(observable, &kObject, C_MM_RESERVATION_NO_CHECK);
            if (result == U_RESULT_OK) {
                count = v_waitsetDetach(kWaitset,v_observable(kObject));
                if (count == 0) {
                    /* This is used to signal the caller that there is nothing to wait for anymore. */
                    result = U_RESULT_UNSUPPORTED;
                }
                u_observableRelease(observable, C_MM_RESERVATION_NO_CHECK);
            } else {
                OS_REPORT(OS_ERROR, "u_waitsetEntryDetach", result,
                            "Could not claim supplied entity (0x%"PA_PRIxADDR").",
                            (os_address)observable);
            }
            (void)u_handleRelease(_this->handle);
        } else {
            OS_REPORT(OS_ERROR, "u_waitsetEntryDetach", result,
                        "Could not claim kernel waitset (%"PA_PRIxADDR":%x:%x).",
                        _this->handle.server, _this->handle.index, _this->handle.serial);
        }
        u_domainUnprotect();
    }
    return result;
}

u_result
u_waitsetEntryWait(
    const u_waitsetEntry _this,
    u_waitsetAction action,
    void *arg,
    const os_duration timeout)
{
    u_result result;
    u_domain domain;
    v_result kr;
    v_waitset kWaitset;

    assert(_this != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        os_mutexLock(&_this->mutex);
        if (_this->alive) {
           result = u_handleClaim(_this->handle, &kWaitset);
        } else {
           result = U_RESULT_ALREADY_DELETED;
        }
        if (result == U_RESULT_OK) {
            _this->waitCount++;
            os_mutexUnlock(&_this->mutex);
            kr = v_waitsetWait(kWaitset, action, arg, timeout);
            result = u_resultFromKernel(kr);
            os_mutexLock(&_this->mutex);
            _this->waitCount--;
            u_handleRelease(_this->handle);
            os_mutexUnlock(&_this->mutex);
        } else {
            os_mutexUnlock(&_this->mutex);
        }
        u_domainUnprotect();
    }
    return result;
}

u_result
u_waitsetEntryWait2(
    const u_waitsetEntry _this,
    u_waitsetAction2 action,
    void *arg,
    const os_duration timeout)
{
    u_result result;
    u_domain domain;
    v_result kr;
    v_waitset kWaitset;

    assert(_this != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        os_mutexLock(&_this->mutex);
        if (_this->alive) {
           result = u_handleClaim(_this->handle, &kWaitset);
        } else {
           result = U_RESULT_ALREADY_DELETED;
        }
        if (result == U_RESULT_OK) {
            _this->waitCount++;
            os_mutexUnlock(&_this->mutex);
            kr = v_waitsetWait2(kWaitset, action, arg, timeout);
            result = u_resultFromKernel(kr);
            os_mutexLock(&_this->mutex);
            _this->waitCount--;
            (void)u_handleRelease(_this->handle);
            os_mutexUnlock(&_this->mutex);
        } else {
            os_mutexUnlock(&_this->mutex);
        }
        u_domainUnprotect();
    }
    return result;
}

u_result
u_waitsetEntryTrigger(
    const u_waitsetEntry _this,
    c_voidp eventArg)
{
    u_result result;
    u_domain domain;
    v_waitset kWaitset;

    assert(_this != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        result = u_handleClaim(_this->handle, &kWaitset);
        if (result == U_RESULT_OK) {
            v_waitsetTrigger(kWaitset, eventArg);
            u_handleRelease(_this->handle);
        }
        u_domainUnprotect();
    }
    return result;
}

u_result
u_waitsetEntryGetEventMask(
    const u_waitsetEntry _this,
    c_ulong *eventMask)
{
    v_waitset kWaitset;
    u_domain domain;
    u_result result;

    assert(_this != NULL);
    assert(eventMask != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        result = u_handleClaim(_this->handle, &kWaitset);
        if (result == U_RESULT_OK) {
            assert(kWaitset);
            if (C_TYPECHECK(kWaitset,v_waitset)) {
                *eventMask = v_observerGetEventMask(v_observer(kWaitset));
            } else {
                result = U_RESULT_CLASS_MISMATCH;
                OS_REPORT(OS_ERROR,
                          "u_waitsetEntryGetEventMask", result,
                          "Class mismatch.");
            }
            (void)u_handleRelease(_this->handle);
        } else {
            OS_REPORT(OS_ERROR,
                      "u_waitsetEntryGetEventMask", result,
                      "Could not claim kernel waitset.");
        }
        u_domainUnprotect();
    }
    return result;
}

u_result
u_waitsetEntrySetEventMask(
    const u_waitsetEntry _this,
    c_ulong eventMask)
{
    v_waitset kWaitset;
    u_result result;
    u_domain domain;

    assert(_this != NULL);

    domain = u_observableDomain(u_observable(_this));
    result = u_domainProtect(domain);
    if (result == U_RESULT_OK) {
        result = u_handleClaim(_this->handle, &kWaitset);
        if (result == U_RESULT_OK) {
            assert(kWaitset);
            if (C_TYPECHECK(kWaitset,v_waitset)) {
                v_observerSetEventMask(v_observer(kWaitset), eventMask);
            } else {
                result = U_RESULT_CLASS_MISMATCH;
                OS_REPORT(OS_ERROR, "u_waitsetEntrySetEventMask", result,
                          "Class mismatch.");
            }
            (void)u_handleRelease(_this->handle);
        } else {
            OS_REPORT(OS_ERROR, "u_waitsetEntrySetEventMask", result,
                      "Could not claim waitset.");
        }
        u_domainUnprotect();
    }
    return result;
}

