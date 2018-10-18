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

#include "../common/include/os_signalHandlerCallback.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_atomics.h"

os_signalHandlerExitRequestHandle
os_signalHandlerRegisterExitRequestCallback(
    os_signalHandlerExitRequestCallback callback,
    os_signalHandlerAllocThreadContextCallback cbAllocThreadContext,
    os_signalHandlerGetThreadContextCallback cbGetThreadContext,
    os_signalHandlerFreeThreadContextCallback cbFreeThreadContext,
    void * arg)
{
    os_signalHandlerExitRequestHandle ret;
    os_signalHandlerExitRequestCallbackInfo *cb;
    os_signalHandlerCallbackInfo *_this = os__signalHandlerGetCallbackInfo();
    int i;

    ret.handle = cb = os_malloc(sizeof *cb);

    cb->callbackExitRequest = callback;
    cb->callbackGetThreadContext = cbGetThreadContext;
    cb->callbackFreeThreadContext = cbFreeThreadContext;
    for (i = 0; i < EXIT_REQUEST_BUFFER_SIZE; i++) {
        cb->contextBuffer[i].contextAssigned = FALSE;
        cb->contextBuffer[i].threadContext = cbAllocThreadContext ? cbAllocThreadContext() : NULL;
    }
    cb->arg = arg;
    cb->deregistered = OS_FALSE;

    os_mutexLock(&_this->exitRequestMtx);
    cb->next = _this->exitRequestCallbackInfo;
    _this->exitRequestCallbackInfo = cb;
    _this->nrExitRequestHandlers++;
    os_mutexUnlock(&_this->exitRequestMtx);

    return ret;
}

void
os_signalHandlerUnregisterExitRequestCallback(
        os_signalHandlerExitRequestHandle erh)
{
    os_signalHandlerExitRequestCallbackInfo *cb;
    os_signalHandlerCallbackInfo *_this;

    if(!erh.handle){
        return;
    }

    _this = os__signalHandlerGetCallbackInfo();
    os_mutexLock(&_this->exitRequestMtx);
    cb = _this->exitRequestCallbackInfo;
    while(cb != erh.handle){
        cb = cb->next;
    }
    if(cb == erh.handle){
        cb->deregistered = OS_TRUE;
    }
    os_mutexUnlock(&_this->exitRequestMtx);
}

static void
os_signalHandlerExitRequestCallbackInfoDeinit(
     os_signalHandlerExitRequestCallbackInfo *_this)
{
    int i;

    if (_this) {
        if (_this->callbackFreeThreadContext) {
            for (i = 0; i < EXIT_REQUEST_BUFFER_SIZE; i++) {
                if (_this->contextBuffer[i].threadContext) {
                    _this->callbackFreeThreadContext(_this->contextBuffer[i].threadContext);
                    _this->contextBuffer[i].threadContext = NULL;
                }
            }
        }
    }
}

static void
os_signalHandlerExceptionRequestCallbackInfoDeinit(
     os_signalHandlerExceptionCallbackInfo *_this)
{
    if (_this) {
        if (_this->callbackFreeThreadContext) {
            if (_this->threadContext) {
                _this->callbackFreeThreadContext(_this->threadContext);
                _this->threadContext = NULL;
            }
        }
    }
}


void
os_signalHandlerDeleteDeregisteredExitRequestCallbacks(
    os_signalHandlerCallbackInfo *_this)
{
    os_signalHandlerExitRequestCallbackInfo **cbPrev;

    os_mutexLock(&_this->exitRequestMtx);
    cbPrev = &_this->exitRequestCallbackInfo;
    while(*cbPrev != NULL){
        if ((*cbPrev)->deregistered) {
            os_signalHandlerExitRequestCallbackInfo *cb;

            cb = *cbPrev;
            *cbPrev = cb->next;
            os_signalHandlerExitRequestCallbackInfoDeinit(cb);
            os_free(cb);
            _this->nrExitRequestHandlers--;
        } else {
            cbPrev = &(*cbPrev)->next;
        }
    }
    os_mutexUnlock(&_this->exitRequestMtx);
}

os_signalHandlerExceptionHandle
os_signalHandlerRegisterExceptionCallback(
    os_signalHandlerExceptionCallback cbException,
    os_signalHandlerAllocThreadContextCallback cbAllocThreadContext,
    os_signalHandlerGetThreadContextCallback cbGetThreadContext,
    os_signalHandlerFreeThreadContextCallback cbFreeThreadContext,
    void * arg)
{
    os_signalHandlerExceptionHandle ret;
    os_signalHandlerExceptionCallbackInfo *cb;
    os_signalHandlerCallbackInfo *_this = os__signalHandlerGetCallbackInfo();

    ret.handle = cb = os_malloc(sizeof *cb);

    cb->callbackException = cbException;
    cb->callbackGetThreadContext = cbGetThreadContext;
    cb->callbackFreeThreadContext = cbFreeThreadContext;
    cb->threadContext = cbAllocThreadContext ? cbAllocThreadContext() : NULL;
    cb->arg = arg;

    os_mutexLock(&_this->exceptionMtx);
    cb->next = _this->exceptionCallbackInfo;
    _this->exceptionCallbackInfo = cb;
    os_mutexUnlock(&_this->exceptionMtx);

    return ret;
}

void
os_signalHandlerUnregisterExceptionCallback(
    os_signalHandlerExceptionHandle eh)
{
    os_signalHandlerExceptionCallbackInfo *cb, **cbPrev;
    os_signalHandlerCallbackInfo *_this;

    if(!eh.handle){
        return;
    }
    _this = os__signalHandlerGetCallbackInfo();
    os_mutexLock(&_this->exceptionMtx);
    cbPrev = &_this->exceptionCallbackInfo;
    cb = _this->exceptionCallbackInfo;
    while(cb != eh.handle){
        cbPrev = &cb->next;
        cb = cb->next;
    }
    if(cb == eh.handle){
        *cbPrev = cb->next;
        os_signalHandlerExceptionRequestCallbackInfoDeinit(cb);
        os_free(cb);
    }
    os_mutexUnlock(&_this->exceptionMtx);
}

static void
os__signalHandlerExitRequestGetThreadContextCallbackInvoke(
        os_signalHandlerCallbackInfo *_this,
        os_uint32 exitRequestInsertionIndex)
{
    os_signalHandlerExitRequestCallbackInfo *cbExitRequest;

    assert(_this);

    os_mutexLock(&_this->exitRequestMtx);
    cbExitRequest = _this->exitRequestCallbackInfo;
    while (cbExitRequest){
        if (!cbExitRequest->deregistered && cbExitRequest->callbackGetThreadContext) {
            cbExitRequest->callbackGetThreadContext(cbExitRequest->contextBuffer[exitRequestInsertionIndex].threadContext);
            cbExitRequest->contextBuffer[exitRequestInsertionIndex].contextAssigned = TRUE;
        }
        cbExitRequest = cbExitRequest->next;
    }
    os_mutexUnlock(&_this->exitRequestMtx);
}

static os_uint32
os__signalHandlerExitRequestCallbackInvoke(
        os_signalHandlerCallbackInfo *_this,
        os_callbackArg arg)
{
    os_result osr;
    os_signalHandlerExitRequestCallbackInfo *cbExit;
    os_uint32 nrCallbacks = 0;

    assert(_this);
    os_mutexLock(&_this->exitRequestMtx);
    cbExit = _this->exitRequestCallbackInfo;
    /* Don't process handlers that have just been added, since we did not obtain
     * the proper context. */
    while(cbExit){
        os_signalHandlerThreadContextBuffer *contextBuffer = &(cbExit->contextBuffer[_this->exitRequestConsumptionIndex]);
        if (!cbExit->deregistered && cbExit->callbackExitRequest &&
                (contextBuffer->contextAssigned || !cbExit->callbackGetThreadContext)) {
            osr = cbExit->callbackExitRequest(arg, contextBuffer->threadContext, cbExit->arg);
            nrCallbacks++;
            if(osr != os_resultSuccess) {
                OS_REPORT(OS_ERROR, "os_signalHandlerThread", 0,
                          "Exit request-callback returned: %s",
                          os_resultImage(osr));
            }
        }
        contextBuffer->contextAssigned = FALSE;
        cbExit = cbExit->next;
    }
    _this->exitRequestConsumptionIndex++;
    os_mutexUnlock(&_this->exitRequestMtx);

    return nrCallbacks;
}

static void
os__signalHandlerExceptionGetThreadContextCallbackInvoke(
        os_signalHandlerCallbackInfo *_this)
{
    os_signalHandlerExceptionCallbackInfo *cbException;

    assert(_this);

    /* Do not obtain _this->exceptionMtx here: the signalHandler
     * already did this and holds on to it until the exception is
     * fully processed, so that nobody else can fiddle with the callback
     * stack and  thread contexts.
     */
    cbException = _this->exceptionCallbackInfo;
    while (cbException){
        if (cbException->callbackGetThreadContext) {
            cbException->callbackGetThreadContext(cbException->threadContext);
        }
        cbException = cbException->next;
    }
}

static void
os__signalHandlerExceptionCallbackInvoke(
        os_signalHandlerCallbackInfo *_this)
{
    os_result osr;
    os_signalHandlerExceptionCallbackInfo *cbException;

    assert(_this);

    /* Do not obtain _this->exceptionMtx here: the raising thread
     * blocks until we are done, and makes sure the Mutex is still
     * occupied so that nobody else can fiddle with the callback stack.
     */
    cbException = _this->exceptionCallbackInfo;
    while(cbException){
        if (cbException->callbackException){
            osr = cbException->callbackException(cbException->threadContext, cbException->arg);
            if(osr != os_resultSuccess) {
                OS_REPORT(OS_ERROR, "os_signalHandlerThread", 0,
                        "Exception-callback returned: %s",
                        os_resultImage(osr));
            }
        }
        cbException = cbException->next;
    }
}

static os_result
os__signalHandlerCallbackInit(
        os_signalHandlerCallbackInfo *_this)
{
    os_result osr;

    assert(_this);

    osr = os_mutexInit(&_this->exitRequestMtx, NULL);
    if(osr != os_resultSuccess){
        goto err_exitRequestMtxInit;
    }
    _this->exitRequestCallbackInfo = NULL;
    _this->nrExitRequestHandlers = 0;
    pa_st32(&_this->exitRequestInsertionIndex, 0xffffffff);
    _this->exitRequestConsumptionIndex = 0;

    osr = os_mutexInit(&_this->exceptionMtx, NULL);
    if(osr != os_resultSuccess){
        goto err_exceptionMtxInit;
    }
    _this->exceptionCallbackInfo = NULL;

    return os_resultSuccess;

/* Error handling */
err_exceptionMtxInit:
    os_mutexDestroy(&_this->exitRequestMtx);
err_exitRequestMtxInit:
    return os_resultFail;
}

static void
os__signalHandlerCallbackDeinit(
        os_signalHandlerCallbackInfo *_this)
{
    os_signalHandlerExceptionCallbackInfo *ecb;
    os_signalHandlerExitRequestCallbackInfo *ercb;

    assert(_this);

    os_mutexLock(&_this->exceptionMtx);
    while((ecb = _this->exceptionCallbackInfo) != NULL){
        _this->exceptionCallbackInfo = _this->exceptionCallbackInfo->next;
        os_signalHandlerExceptionRequestCallbackInfoDeinit(ecb);
        os_free(ecb);
    }
    os_mutexUnlock(&_this->exceptionMtx);
    os_mutexDestroy(&_this->exceptionMtx);

    os_mutexLock(&_this->exitRequestMtx);
    while((ercb = _this->exitRequestCallbackInfo) != NULL){
        _this->exitRequestCallbackInfo = _this->exitRequestCallbackInfo->next;
        os_signalHandlerExitRequestCallbackInfoDeinit(ercb);
        os_free(ercb);
    }
    os_mutexUnlock(&_this->exitRequestMtx);
    os_mutexDestroy(&_this->exitRequestMtx);
}
