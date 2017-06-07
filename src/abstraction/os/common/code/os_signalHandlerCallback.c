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

#include "../common/include/os_signalHandlerCallback.h"
#include "os_heap.h"
#include "os_report.h"

os_signalHandlerExitRequestHandle
os_signalHandlerRegisterExitRequestCallback(
    os_signalHandlerExitRequestCallback callback,
    void * arg)
{
    os_signalHandlerExitRequestHandle ret;
    os_signalHandlerExitRequestCallbackInfo *cb;
    os_signalHandlerCallbackInfo *_this = os__signalHandlerGetCallbackInfo();

    ret.handle = cb = os_malloc(sizeof *cb);

    cb->callback = callback;
    cb->arg = arg;

    os_mutexLock(&_this->exitRequestMtx);
    cb->next = _this->exitRequestCallbackInfo;
    _this->exitRequestCallbackInfo = cb;
    os_mutexUnlock(&_this->exitRequestMtx);

    return ret;
}

void
os_signalHandlerUnregisterExitRequestCallback(
        os_signalHandlerExitRequestHandle erh)
{
    os_signalHandlerExitRequestCallbackInfo *cb, **cbPrev;
    os_signalHandlerCallbackInfo *_this;

    if(!erh.handle){
        return;
    }

    _this = os__signalHandlerGetCallbackInfo();
    os_mutexLock(&_this->exitRequestMtx);
    cbPrev = &_this->exitRequestCallbackInfo;
    cb = _this->exitRequestCallbackInfo;
    while(cb != erh.handle){
        cbPrev = &cb->next;
        cb = cb->next;
    }
    if(cb == erh.handle){
        *cbPrev = cb->next;
        os_free(cb);
    }
    os_mutexUnlock(&_this->exitRequestMtx);
}

os_signalHandlerExceptionHandle
os_signalHandlerRegisterExceptionCallback(
    os_signalHandlerExceptionCallback callback,
    void * arg)
{
    os_signalHandlerExceptionHandle ret;
    os_signalHandlerExceptionCallbackInfo *cb;
    os_signalHandlerCallbackInfo *_this = os__signalHandlerGetCallbackInfo();

    ret.handle = cb = os_malloc(sizeof *cb);

    cb->callback = callback;
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
        os_free(cb);
    }
    os_mutexUnlock(&_this->exceptionMtx);
}

static unsigned int
os__signalHandlerExitRequestCallbackInvoke(
        os_signalHandlerCallbackInfo *_this,
        os_callbackArg arg)
{
    os_result osr;
    os_signalHandlerExitRequestCallbackInfo *cbExit;
    unsigned int nrofCallbacks = 0;

    assert(_this);
    os_mutexLock(&_this->exitRequestMtx);
    cbExit = _this->exitRequestCallbackInfo;
    while(cbExit){
        if (cbExit->callback){
            nrofCallbacks++;
            osr = cbExit->callback(arg, cbExit->arg);
            if(osr != os_resultSuccess) {
                OS_REPORT(OS_ERROR, "os_signalHandlerThread", 0,
                        "Exit request-callback returned: %s",
                        os_resultImage(osr));
            }
        }
        cbExit = cbExit->next;
    }
    os_mutexUnlock(&_this->exitRequestMtx);

    return nrofCallbacks;
}

static void
os__signalHandlerExceptionCallbackInvoke(
        os_signalHandlerCallbackInfo *_this,
        os_callbackArg arg)
{
    os_result osr;
    os_signalHandlerExceptionCallbackInfo *cbException;

    assert(_this);

    os_mutexLock(&_this->exceptionMtx);
    cbException = _this->exceptionCallbackInfo;
    while(cbException){
        if (cbException->callback){
            osr = cbException->callback(arg, cbException->arg);
            if(osr != os_resultSuccess) {
                OS_REPORT(OS_ERROR, "os_signalHandlerThread", 0,
                        "Exception-callback returned: %s",
                        os_resultImage(osr));
            }
        }
        cbException = cbException->next;
    }
    os_mutexUnlock(&_this->exceptionMtx);
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
        os_free(ecb);
    }
    os_mutexUnlock(&_this->exceptionMtx);
    os_mutexDestroy(&_this->exceptionMtx);

    os_mutexLock(&_this->exitRequestMtx);
    while((ercb = _this->exitRequestCallbackInfo) != NULL){
        _this->exitRequestCallbackInfo = _this->exitRequestCallbackInfo->next;
        os_free(ercb);
    }
    os_mutexUnlock(&_this->exitRequestMtx);
    os_mutexDestroy(&_this->exitRequestMtx);
}
