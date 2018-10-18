/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file os/wince/code/os__signalHandler.c
 *  \brief WINCE signal handler management
 *
 * Implements signal handler management for WINCE
 */
#include "vortex_os.h"
#include "os_thread.h"
#include "code/os__process.h"
#include "os__debug.h"
#include "../common/include/os_signalHandlerCallback.h"

#include <cassert>

static os_signalHandler signalHandlerObj;

struct os_signalHandler_s {
    HANDLE msgQueue; /* Queue to communicate with the signal handler thread. */
    os_threadId threadId;   /* The thread id of the signalHandlerThread. */
    os_signalHandlerCallbackInfo callbackInfo;
};

/* Include the common implementation for callback handling */
#include "../common/code/os_signalHandlerCallback.c"

static os_result os_signalHandlerInit(os_signalHandler _this);
static void * signalEmulatorThread(void *arg);
static BOOL CtrlHandler(DWORD fdwCtrlType);
static void os__signalHandlerThreadStop(os_signalHandler _this);

void
os_signalHandlerIgnoreJavaSignals (void)
{

}

os_result
os_signalHandlerNew(
    void)
{
    /* The signalHandler is always started in the os_processModuleInit() on WinCE.
     * Therefore later invocations to os_signalHandlerNew() have no effect on
     * this platform. */
    assert(signalHandlerObj);

    return os_resultSuccess;
}

os_result
os_signalHandlerEnableExceptionSignals (
    void)
{
    return os_resultSuccess;
}

os_result
os__signalHandlerNew(
    void)
{
    os_signalHandler _this;
    os_result result = os_resultFail;

    _this = os_malloc(sizeof(*_this));
    if (_this == NULL) {
        OS_REPORT(OS_ERROR, "os_signalHandlerNew", 0, "os_malloc(%d) failed.", sizeof(*_this));
        goto err_malloc;
    }
    signalHandlerObj = _this;
    if((result = os_signalHandlerInit(_this)) != os_resultSuccess){
        goto err_init;
    }
    return result;

err_init:
    signalHandlerObj = NULL;
    os_free(_this);
err_malloc:
    return os_resultFail;
}

static os_result
os_signalHandlerInit(
    os_signalHandler _this)
{
    os_result result = os_resultSuccess;
    os_threadAttr thrAttr;
    MSGQUEUEOPTIONS msgqOptionsRead;
    size_t size;
    wchar_t wname[32]; /* Should hold at least the wide string "osplpipe_" + max DWORD decimally */

    assert(_this);

    if(os__signalHandlerCallbackInit(&_this->callbackInfo) != os_resultSuccess) {
        goto err_callbackInfoInit;
    }

    // Read message queue options
    msgqOptionsRead.dwFlags = MSGQUEUE_NOPRECOMMIT;
    msgqOptionsRead.dwMaxMessages = 0; // unlimited
    msgqOptionsRead.cbMaxMessage = 4096;
    msgqOptionsRead.bReadAccess = TRUE;
    msgqOptionsRead.dwSize = sizeof(msgqOptionsRead);

    /* Create signal handling pipes */
    size = _snwprintf(wname, sizeof wname, L"osplpipe_%d", GetCurrentProcessId());
    assert(size < sizeof wname);

    _this->msgQueue = CreateMsgQueue(wname, &msgqOptionsRead);
    if(_this->msgQueue == NULL){
        OS_DEBUG_2("os_signalHandlerInit", "failure on creating named message queue '%s': %d",
                name, os_getErrno());
        goto err_signalEmulatorQueue;
    }

    /* Setup signal handler thread. */
    os_threadAttrInit(&thrAttr);
    thrAttr.stackSize = 1024*1024; /* 1 MB */
    result = os_threadCreate(&_this->threadId,
                            "signalHandler",
                            &thrAttr,
                            signalEmulatorThread,
                            NULL);

    if (result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                   "os_threadCreate(0x%x, 0x%x,0x%x,0) failed: %s",
                   &_this->threadId,
                   &thrAttr,
                   signalEmulatorThread,
                   os_resultImage(result));
        goto err_threadCreate;
    }

    /* The SetConsolseCtrlHandler used on Win32 platforms isn't available on
     * WinCE. As an alternative an event can be registered for stdout device for
     * the IOCTL_CONSOLE_SETCONTROLCEVENT (apparently; not actually tested).
     * Since we so far haven't supported it for WinCE, it is just not supported. */

    return os_resultSuccess;


/* Error handling */
err_threadCreate:
    (void) CloseHandle(_this->msgQueue);
err_signalEmulatorQueue:
    os__signalHandlerCallbackDeinit(&_this->callbackInfo);
err_callbackInfoInit:
    return os_resultFail;
}

void
os_signalHandlerFree(
    void)
{
    /* The signalHandler is always stopped in the os_processModuleExit() on WinCE.
     * Therefore invocations to os_signalHandlerFree() have no effect on this
     * platform. */
    assert(signalHandlerObj);

    return;
}

void
os__signalHandlerFree(
    void)
{
    os_signalHandler _this = signalHandlerObj;

    if (_this) {
        os__signalHandlerThreadStop(_this);
        (void) CloseHandle(_this->msgQueue);
        os__signalHandlerCallbackDeinit(&_this->callbackInfo);
        signalHandlerObj = NULL;
        os_free(_this);
    }
}

static void
os__signalHandlerThreadStop(
        os_signalHandler _this)
{
    os_int32 signal = -1; /* use negative number to stop the thread */
    DWORD written;

    assert(_this);

    if(WriteFile(_this->msgQueue, &signal, sizeof signal, &written, NULL)) {
        assert(written == sizeof signal);

        /* When the signalHandlerThread itself is the exiting thread (this can happen
         * when an exit call is done in a signalHandler installed by a customer for
         * example), we should not invoke os_threadWaitExit but just let this call
         * return immediately. */
        if (os_threadIdToInteger(os_threadIdSelf()) != os_threadIdToInteger(_this->threadId)) {
            (void) os_threadWaitExit(_this->threadId, NULL);
        }
    }
}

static void *
signalEmulatorThread(
    void *arg)
{
    os_int32 sig;
    DWORD nread, cbRet;
    BOOL result;
    const os_duration delay = 100*OS_DURATION_MILLISECOND; /* 100ms */
    os_signalHandler _this = signalHandlerObj;

    assert(_this);

    do {
        sig = 0;
        result = WaitForSingleObject(_this->msgQueue, INFINITE);
        if (result == WAIT_OBJECT_0) {
            /* read signal */

            result = ReadMsgQueue(_this->msgQueue, &sig, sizeof(sig), &nread, INFINITE, &cbRet);
            if (result && (nread == sizeof(sig)) && sig > 0) {
                os_callbackArg arg;
                os_uint32 index;
                OS_DEBUG_1("signalEmulatorThread", "Received signal %d", sig);

                if (sig == OS_SIGKILL) {
                   _exit(0);
                }
                index = pa_inc32_nv(&_this->callbackInfo.exitRequestInsertionIndex) % EXIT_REQUEST_BUFFER_SIZE;
                os__signalHandlerExitRequestGetThreadContextCallbackInvoke(&_this->callbackInfo, index);
                if(os__signalHandlerExitRequestCallbackInvoke(&_this->callbackInfo, arg) == 0){
                    /* Implement the default-behaviour in case no exit-request
                     * handlers are installed. */
                    os_signalHandlerFinishExitRequest(arg);
                }
                os_signalHandlerDeleteDeregisteredExitRequestCallbacks(&_this->callbackInfo);
            }
        } else {
            /* Re-enter wait after a delay. */
            ospl_os_sleep(delay);
        }
    } while (sig >= 0);

    return NULL;
}


os_result
os_signalHandlerFinishExitRequest(
    os_callbackArg arg)
{
    OS_UNUSED_ARG(arg);

    os_procCallExitHandlers();

    ExitProcess(0); /* Will not return */

    return os_resultSuccess;
}

os_result
os_signalHandlerSetEnabled(
    os_uint enabled)
{
    OS_UNUSED_ARG(enabled);

    return os_resultSuccess;
}

static os_signalHandlerCallbackInfo*
os__signalHandlerGetCallbackInfo(void)
{
    os_signalHandler _this = signalHandlerObj;

    assert(_this);

    return &_this->callbackInfo;
}
