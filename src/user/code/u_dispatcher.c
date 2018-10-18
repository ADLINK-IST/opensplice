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

#include "u_user.h"
#include "u__dispatcher.h"
#include "u__observable.h"
#include "u__types.h"
#include "u__entity.h"

#include "v_entity.h"
#include "v_observer.h"
#include "v_event.h"

#include "os_report.h"

C_CLASS(u_callback);
C_STRUCT(u_callback)
{
    u_observableListener listener;
    u_eventMask eventMask;
    void *usrData; /* data specific for a user, just passed as value to the function listener */
};

struct callbackExecArg {
    c_ulong mask;
    u_observable o;
};

static void
callbackExecute (
    u_callback callback,
    void *arg)
{
    struct callbackExecArg *a = (struct callbackExecArg *)arg;

    a->mask |= callback->listener(a->o,a->o->dispatcher->event,callback->usrData);
}

static void *
dispatch(
    void *o)
{
    u_dispatcher _this;
    v_observer claim;
    struct callbackExecArg arg;
    u_result result;

    _this = u_dispatcher(o);
    if (_this != NULL) {
        v_kernelThreadFlags(V_KERNEL_THREAD_FLAG_SERVICETHREAD, V_KERNEL_THREAD_FLAG_SERVICETHREAD);
        os_mutexLock(&_this->mutex);
        result = u_observableReadClaim(_this->observable, (v_public *)(&claim), C_MM_RESERVATION_ZERO);
        if(result == U_RESULT_OK) {
            assert(claim);
            while ((!(_this->event & V_EVENT_OBJECT_DESTROYED)) &&
                   (_this->callbacks != NULL) &&
                   (c_iterLength(_this->callbacks) > 0)) {

                os_mutexUnlock(&_this->mutex);
                _this->event = v_observerWait(claim);
                os_mutexLock(&_this->mutex);
                if (!(_this->event & V_EVENT_OBJECT_DESTROYED)) {
                    /* do not call listener when  observable is destroyed! */
                    arg.mask = 0;
                    arg.o = _this->observable;
                    c_iterWalk(_this->callbacks,
                               (c_iterWalkAction)callbackExecute,
                               &arg);
                }
            }
            _this->threadId = OS_THREAD_ID_NONE;
            u_observableRelease(_this->observable, C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_WARNING, "u_dispatcher::dispatch", result,
                      "Failed to claim Dispatcher.");
        }
        os_mutexUnlock(&_this->mutex);
    } else {
        OS_REPORT(OS_ERROR, "u_dispatcher::dispatch", U_RESULT_PRECONDITION_NOT_MET,
                  "No dispatcher specified.");
    }
    return NULL;
}

u_dispatcher
u_dispatcherNew(
    const u_observable observable)
{
    u_dispatcher _this;
    os_result osr;

    assert(observable != NULL);

    _this = os_malloc(sizeof *_this);
    memset(_this, 0, sizeof *_this);
    osr = os_mutexInit(&_this->mutex, NULL);
    if (osr == os_resultSuccess) {
        _this->callbacks = NULL;
        _this->threadId = OS_THREAD_ID_NONE;
        _this->event = 0;
        _this->observable = observable;
    } else {
        os_free(_this);
        _this = NULL;
    }
    return _this;
}

u_result
u_dispatcherFree(
    u_dispatcher _this)
{
    v_observer ko;
    u_callback callback;
    os_threadId tid;
    u_result result = U_RESULT_OK;

    assert(_this != NULL);

    os_mutexLock(&_this->mutex);
    callback = (u_callback)c_iterTakeFirst(_this->callbacks);
    while (callback != NULL) {
        os_free(callback);
        callback = (u_callback)c_iterTakeFirst(_this->callbacks);
    }
    c_iterFree(_this->callbacks);
    _this->callbacks = NULL; /* Flags the dispatch thread to stop */
    if (os_threadIdToInteger(_this->threadId) != 0U) {
        tid = _this->threadId;
        result = u_observableReadClaim(_this->observable, (v_public *)(&ko), C_MM_RESERVATION_NO_CHECK);
        if(result == U_RESULT_OK) {
            assert(ko);
            _this->threadId = OS_THREAD_ID_NONE;
            /* Wakeup the dispatch thread */
            v_observerLock(ko);
            v_observerNotify(ko, NULL, NULL);
            v_observerUnlock(ko);
            os_mutexUnlock(&_this->mutex);
            (void)os_threadWaitExit(tid, NULL);
            os_mutexDestroy(&_this->mutex);
            u_observableRelease(_this->observable, C_MM_RESERVATION_NO_CHECK);
        } else {
            os_mutexUnlock(&_this->mutex);
            OS_REPORT(OS_WARNING, "u_dispatcherFree", result,
                      "Failed to claim Dispatcher.");
        }
    } else {
        os_mutexUnlock(&_this->mutex);
        os_mutexDestroy(&_this->mutex);
    }
    if (result == U_RESULT_OK) {
        os_free(_this);
    }
    return result;
}

u_result
u_dispatcherInsertListener(
    const u_dispatcher _this,
    const u_eventMask eventMask,
    const u_observableListener listener,
    void *userData)
{
    u_callback callback;
    os_threadAttr attr;
    v_observer ke;
    u_result result = U_RESULT_OK;
    c_char *name;

    assert(_this != NULL);
    assert(listener != NULL);

    os_mutexLock(&_this->mutex);
    callback = os_malloc(C_SIZEOF(u_callback));
    callback->listener = listener;
    callback->usrData = userData;
    callback->eventMask = eventMask;
    _this->callbacks = c_iterInsert(_this->callbacks,callback);

    result = u_observableReadClaim(_this->observable, (v_public *)(&ke), C_MM_RESERVATION_ZERO);
    if(result == U_RESULT_OK) {
        assert(ke);
        if (os_threadIdToInteger(_this->threadId) == 0U) {
            name = v_entityName(ke);
            if (name == NULL) {
                name = "NoName";
            }
            os_threadAttrInit(&attr);
            os_threadCreate(&_this->threadId, name, &attr,dispatch, (void *)_this);
        }
        v_observerSetEvent(ke,eventMask);
        u_observableRelease(_this->observable, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_dispatcherInsertListener", result,
                  "Failed to claim Dispatcher.");
    }
    os_mutexUnlock(&_this->mutex);

    return result;
}

static c_equality
compare(
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    u_callback callback = (u_callback) o;
    u_callback template = (u_callback)arg;

    if (callback->listener == template->listener) {
        return C_EQ;
    } else {
        return C_NE;
    }
}

u_result
u_dispatcherRemoveListener(
    const u_dispatcher _this,
    const u_observableListener listener)
{
    u_callback callback;
    C_STRUCT(u_callback) template;
    v_observer ko;
    os_threadId tid;
    u_result result = U_RESULT_OK;

    assert(_this != NULL);
    assert(listener != NULL);

    os_mutexLock(&_this->mutex);
    template.listener = listener;
    callback = (u_callback) c_iterResolve(_this->callbacks, compare, &template);
    tid = _this->threadId;
    if (callback != NULL) {
        c_iterTake(_this->callbacks, callback);
        if (c_iterLength(_this->callbacks) == 0) {
            result = u_observableReadClaim(_this->observable, (v_public *)(&ko), C_MM_RESERVATION_ZERO);
            if(result == U_RESULT_OK) {
                assert(ko);
                v_observerClearEvent(ko,callback->eventMask);
                /* Wakeup the dispatch thread */
                v_observerLock(ko);
                v_observerNotify(ko, NULL, NULL);
                v_observerUnlock(ko);
                u_observableRelease(_this->observable, C_MM_RESERVATION_ZERO);
            } else {
                OS_REPORT(OS_WARNING, "u_dispatcherRemoveListener", result,
                          "Failed to claim Dispatcher.");
            }
        }
        os_free(callback);
    }
    os_mutexUnlock(&_this->mutex);
    if ((c_iterLength(_this->callbacks) == 0)
        && (os_threadIdToInteger(tid) != 0U)) {
        os_threadWaitExit(tid, NULL);
        _this->threadId = OS_THREAD_ID_NONE;
    }
    return result;
}
