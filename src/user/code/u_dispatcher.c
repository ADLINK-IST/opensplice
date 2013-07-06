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

#include "u_user.h"
#include "u__dispatcher.h"
#include "u__types.h"
#include "u__entity.h"
#include "u_listener.h"

#include "v_entity.h"
#include "v_observable.h"
#include "v_observer.h"
#include "v_event.h"

#include "os_report.h"

struct listenerExecArg {
    c_ulong mask;
    u_dispatcher o;
};

static void
listenerExecute (
    u_listener listener,
    c_voidp arg)
{
    struct listenerExecArg *a = (struct listenerExecArg *)arg;

    a->mask |= listener->listener(a->o,a->o->event,listener->usrData);
}

static void *
dispatch(
    void *o)
{
    u_dispatcher _this;
    v_observer claim;
    struct listenerExecArg arg;
    u_result result;

    _this = u_dispatcher(o);
    if (_this != NULL) {
        if (_this->startAction) {
            _this->startAction(_this, _this->actionData);
        }
        os_mutexLock(&_this->mutex);
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&claim));
        if(result == U_RESULT_OK) {
            assert(claim);
            while ((!(_this->event & V_EVENT_OBJECT_DESTROYED)) &&
                   (_this->listeners != NULL) &&
                   (c_iterLength(_this->listeners) > 0)) {

                os_mutexUnlock(&_this->mutex);
                _this->event = v_observerWait(claim);
                os_mutexLock(&_this->mutex);
                if (!(_this->event & V_EVENT_OBJECT_DESTROYED)) {
                    /* do not call listeners when  object is destroyed! */
                    arg.mask = 0;
                    arg.o = _this;
                    c_iterWalk(_this->listeners,
                               (c_iterWalkAction)listenerExecute,
                               &arg);
                }
            }
            _this->threadId = OS_THREAD_ID_NONE;
            result = u_entityRelease(u_entity(_this));
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, "u_dispatcher::dispatch", 0,
                          "Release observer failed.");
            }
        } else {
            OS_REPORT(OS_WARNING, "u_dispatcher::dispatch", 0,
                      "Failed to claim Dispatcher.");
        }
        os_mutexUnlock(&_this->mutex);
        if (_this->stopAction) {
            _this->stopAction(_this, _this->actionData);
        }
    } else {
        OS_REPORT(OS_ERROR, "u_dispatcher::dispatch", 0,
                  "No dispatcher specified.");
    }
    return NULL;
}

u_result
u_dispatcherInit(
    u_dispatcher _this)
{
    v_observer ko;
    os_mutexAttr mutexAttr;
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
        if(result == U_RESULT_OK) {
            assert(ko);
            os_mutexAttrInit(&mutexAttr);
            mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
            os_mutexInit(&_this->mutex,&mutexAttr);
            _this->listeners = NULL;
            _this->threadId = OS_THREAD_ID_NONE;
            _this->startAction = NULL;
            _this->stopAction = NULL;
            _this->actionData = NULL;
            _this->event = 0;
            u_entity(_this)->flags |= U_ECREATE_INITIALISED;
            result = u_entityRelease(u_entity(_this));
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, "u_dispatcherInit", 0,
                          "Release observer failed.");
            }
        } else {
            OS_REPORT(OS_WARNING,"u_dispatcherInit",0,
                      "Failed to claim kernel object");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherInit",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dispatcherDeinit(
    u_dispatcher _this)
{
    v_observer ko;
    u_listener listener;
    os_threadId tid;
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        os_mutexLock(&_this->mutex);
        listener = u_listener(c_iterTakeFirst(_this->listeners));
        while (listener != NULL) {
            u_listenerFree(listener);
            listener = u_listener(c_iterTakeFirst(_this->listeners));
        }
        c_iterFree(_this->listeners);
        _this->listeners = NULL; /* Flags the dispatch thread to stop */
        if (os_threadIdToInteger(_this->threadId) != 0U) {
            tid = _this->threadId;
            result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
            if(result != U_RESULT_OK) {
                /* This is a valid situation when a participant has been
                 * freed prior to the freeing of a dispatcher within the
                 * participant.
                 */
                os_mutexUnlock(&_this->mutex);
                os_threadWaitExit(tid, NULL);
                os_mutexDestroy(&_this->mutex);
                /*return U_RESULT_INTERNAL_ERROR;*/
            } else {
                /* Wakeup the dispatch thread */
                v_observerLock(ko);
                v_observerNotify(ko, NULL, NULL);
                v_observerUnlock(ko);
                u_entityRelease(u_entity(_this));
                os_mutexUnlock(&_this->mutex);
                os_threadWaitExit(tid, NULL);
                os_mutexDestroy(&_this->mutex);
            }
        } else {
            os_mutexUnlock(&_this->mutex);
            os_mutexDestroy(&_this->mutex);
        }
        result = u_entityDeinit(u_entity(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherDeinit",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dispatcherInsertListener(
    u_dispatcher _this,
    u_dispatcherListener listener,
    c_voidp userData)
{
    u_listener ul;
    os_threadAttr attr;
    v_observer ke;
    u_result result = U_RESULT_OK;
    c_char *name;

    if ((_this != NULL) && (listener != NULL)) {
        os_mutexLock(&_this->mutex);
        ul = u_listenerNew(listener,userData);
        _this->listeners = c_iterInsert(_this->listeners,ul);

        if (os_threadIdToInteger(_this->threadId) == 0U) {
            result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ke));
            if(result == U_RESULT_OK) {
                assert(ke);
                name = v_entityName(ke);
                if (name == NULL) {
                    name = "NoName";
                }
                os_threadAttrInit(&attr);
                os_threadCreate(&_this->threadId,
                                name,
                                &attr,dispatch,
                                (void *)_this);
                result = u_entityRelease(u_entity(_this));
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_dispatcherInsertListener", 0,
                              "Release observer failed.");
                }
            } else {
                OS_REPORT(OS_WARNING, "u_dispatcherInsertListener", 0,
                          "Failed to claim Dispatcher.");
            }
        }
        u_entityEnable(u_entity(_this));
        os_mutexUnlock(&_this->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherInsertListener",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

u_result
u_dispatcherAppendListener(
    u_dispatcher _this,
    u_dispatcherListener listener,
    c_voidp userData)
{
    u_listener ul;
    os_threadAttr attr;
    v_observer ko;
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (listener != NULL)) {
        os_mutexLock(&_this->mutex);
        ul = u_listenerNew(listener,userData);
        _this->listeners = c_iterAppend(_this->listeners,ul);
        if (os_threadIdToInteger(_this->threadId) == 0U) {
            result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
            if(result == U_RESULT_OK) {
                assert(ko);
                os_threadAttrInit(&attr);
                os_threadCreate(&_this->threadId,
                                v_entityName(ko),
                                &attr,
                                dispatch,
                                (void *)_this);
                result = u_entityRelease(u_entity(_this));
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_dispatcherAppendListener", 0,
                              "Release observer failed.");
                }
            } else {
                OS_REPORT(OS_WARNING, "u_dispatcherAppendListener", 0,
                          "Failed to claim Dispatcher.");
            }
        }
        u_entityEnable(u_entity(_this));
        os_mutexUnlock(&_this->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherInsertListener",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

struct compareArg {
    u_listenerCallback listener;
};

static c_equality
compare(
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    u_listener listenerA = (u_listener) o;
    struct compareArg *cArg = (struct compareArg*) arg;

    if (listenerA->listener == cArg->listener) {
        return C_EQ;
    } else {
        return C_NE;
    }
}

u_result
u_dispatcherRemoveListener(
    u_dispatcher _this,
    u_dispatcherListener listener)
{
    u_listener ul;
    v_observer ko;
    os_threadId tid;
    u_result result = U_RESULT_OK;
    struct compareArg arg;

    if ((_this != NULL) && (listener != NULL)) {
        os_mutexLock(&_this->mutex);
        arg.listener = listener;
        ul = (u_listener) c_iterResolve(_this->listeners, compare, &arg);
        tid = _this->threadId;
        if (ul != NULL) {
            c_iterTake(_this->listeners, ul);
            if (c_iterLength(_this->listeners) == 0) {
                result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
                if(result == U_RESULT_OK) {
                    assert(ko);
                    /* Wakeup the dispatch thread */
                    v_observerLock(ko);
                    v_observerNotify(ko, NULL, NULL);
                    v_observerUnlock(ko);
                    result = u_entityRelease(u_entity(_this));
                    if (result != U_RESULT_OK) {
                        OS_REPORT(OS_ERROR, "u_dispatcherRemoveListener", 0,
                                  "Release observer failed.");
                    }
                } else {
                    OS_REPORT(OS_WARNING, "u_dispatcherRemoveListener", 0,
                              "Failed to claim Dispatcher.");
                }
            }
            u_listenerFree(ul);
        }
        os_mutexUnlock(&_this->mutex);
        if ((c_iterLength(_this->listeners) == 0)
            && (os_threadIdToInteger(tid) != 0U)) {
            os_threadWaitExit(tid, NULL);
        }
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherInsertListener",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dispatcherNotify(
    u_dispatcher _this)
{
    v_observer ko;
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
         result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
        if (result == U_RESULT_OK) {
            assert(ko);
            /* Wakeup the dispatch thread */
            v_observerLock(ko);
            v_observerNotify(ko, NULL, NULL);
            v_observerUnlock(ko);
            result = u_entityRelease(u_entity(_this));
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, "u_dispatcherNotify", 0,
                          "Release observer failed.");
            }
        } else {
            OS_REPORT(OS_WARNING, "u_dispatcherNotify", 0,
                      "Failed to claim Dispatcher.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherNotify",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dispatcherSetEventMask(
    u_dispatcher _this,
    c_ulong eventMask)
{
    v_observer ko;
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
         result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
        if (result == U_RESULT_OK) {
            assert(ko);
            v_observerSetEventMask(ko,eventMask);
            result = u_entityRelease(u_entity(_this));
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, "u_dispatcherSetEventMask", 0,
                          "Release observer failed.");
            }
        } else {
            OS_REPORT(OS_WARNING, "u_dispatcherSetEventMask", 0,
                      "Failed to claim Dispatcher.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherSetEventMask",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dispatcherGetEventMask(
    u_dispatcher _this,
    c_ulong *eventMask)
{
    v_observer ko;
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (eventMask != NULL)) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&ko));
        if (result == U_RESULT_OK) {
            assert(ko);
            *eventMask = v_observerGetEventMask(ko);
            result = u_entityRelease(u_entity(_this));
            if (result != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, "u_dispatcherGetEventMask", 0,
                          "Release observer failed.");
            }
        } else {
            OS_REPORT(OS_WARNING, "u_dispatcherGetEventMask", 0,
                      "Failed to claim Dispatcher.");
        }
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherGetEventMask",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_dispatcherSetThreadAction(
    u_dispatcher o,
    u_dispatcherThreadAction startAction,
    u_dispatcherThreadAction stopAction,
    c_voidp actionData)
{
    u_result result = U_RESULT_OK;

    if (o != NULL) {
        os_mutexLock(&o->mutex);
        o->startAction = startAction;
        o->stopAction = stopAction;
        o->actionData = actionData;
        os_mutexUnlock(&o->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_dispatcherSetThreadAction",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}
