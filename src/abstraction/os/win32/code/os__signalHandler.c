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
/** \file os/win32/code/os_signalHandler.c
 *  \brief WIN32 signal handler management
 *
 * Implements signal handler management for WIN32
 */
#include "vortex_os.h"
#include "os__process.h"
#include "os__thread.h"
#include "os__debug.h"
#include "../common/include/os_signalHandlerCallback.h"
#include "os_win32incs.h"
#include <process.h>

static os_signalHandler signalHandlerObj;


struct os_signalHandler_s {
    HANDLE signalPipe; /* pipe to communicate with the signal handler thread. */
    os_threadId threadId;   /* The thread id of the signalHandlerThread. */
    os_signalHandlerCallbackInfo callbackInfo;
};

/* Include the common implementation for callback handling */
#include "../common/code/os_signalHandlerCallback.c"

static os_result os_signalHandlerInit(os_signalHandler _this);
static void * signalEmulatorThread(void *arg);
static BOOL CtrlHandler(DWORD fdwCtrlType);
static void os__signalHandlerThreadStop(os_signalHandler _this);
static os_boolean ignoreJavaSignals = OS_FALSE;

void
os_signalHandlerIgnoreJavaSignals (void)
{
    ignoreJavaSignals = OS_TRUE;
}

os_result
os_signalHandlerNew()
{
    /* The signalHandler is always started in the os_processModuleInit() on Win32.
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
    signalHandlerObj = _this;
    if((result = os_signalHandlerInit(_this)) != os_resultSuccess){
        goto err_init;
    }
    return result;

err_init:
    signalHandlerObj = NULL;
    os_free(_this);
    return os_resultFail;
}

static os_result
os_signalHandlerInit(
    os_signalHandler _this)
{
    os_result result = os_resultSuccess;
    os_threadAttr thrAttr;
    char pname[32];
    int sz;

    assert(_this);

    if(os__signalHandlerCallbackInit(&_this->callbackInfo) != os_resultSuccess) {
        goto err_callbackInfoInit;
    }

    /* Create signal handling pipes */
    sz = snprintf(pname, sizeof pname, "\\\\.\\pipe\\osplpipe_%d", _getpid());
    assert(sz < sizeof pname);

    _this->signalPipe = CreateNamedPipe(
                        pname,
                        PIPE_ACCESS_INBOUND,
                        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                        1 /* max instances */,
                        sizeof(os_int32),
                        sizeof(os_int32),
                        200 /* timeout in millisec */,
                        NULL);
    if(_this->signalPipe == INVALID_HANDLE_VALUE){
        OS_DEBUG_2("os_signalHandlerInit", "failure on creating named pipe '%s': %d",
                pname, os_getErrno());
        goto err_signalEmulatorPipe;
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

    /* first setup termination handling */
    if(!ignoreJavaSignals && SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE) == 0){
        OS_REPORT(OS_ERROR, "os_signalHandlerInit", 0,
                       "Failed to set CtrlHandler: %d",
                       os_getErrno());
        goto err_setCtrlHandler;
    }

    return os_resultSuccess;


/* Error handling */
err_setCtrlHandler:
    os__signalHandlerThreadStop(_this);
err_threadCreate:
    (void) CloseHandle(_this->signalPipe);
err_signalEmulatorPipe:
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
        (void) SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, FALSE);
        os__signalHandlerThreadStop(_this);
        (void) CloseHandle(_this->signalPipe);
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
    HANDLE signalPipe;
    char pname[32];
    int sz;

    assert(_this);

    sz = snprintf(pname, sizeof pname, "\\\\.\\pipe\\osplpipe_%d", _getpid());
    assert(sz < sizeof pname);

    do {
        /* Wait for an instance to become available */
        WaitNamedPipe(pname, NMPWAIT_WAIT_FOREVER);

        signalPipe = CreateFile(
                pname,
                GENERIC_WRITE,      // Write-access only
                0,                  // No sharing
                NULL,               // Default security attributes
                OPEN_EXISTING,      // Open existing pipe
                0,                  // Default attributes
                NULL);              // No template
    } while (signalPipe == INVALID_HANDLE_VALUE && os_getErrno() == ERROR_PIPE_BUSY);

    if(signalPipe != INVALID_HANDLE_VALUE){
        if(WriteFile(signalPipe, &signal, sizeof signal, &written, NULL)) {
            assert(written == sizeof signal);

            /* When the signalHandlerThread itself is the exiting thread (this can happen
             * when an exit call is done in a signalHandler installed by a customer for
             * example), we should not invoke os_threadWaitExit but just let this call
             * return immediately. */
            if (os_threadIdToInteger(os_threadIdSelf()) != os_threadIdToInteger(_this->threadId)) {
                (void) os_threadWaitExit(_this->threadId, NULL);
            }
        }

        CloseHandle(signalPipe);
    }
}

static BOOL
CtrlHandler(
    DWORD fdwCtrlType)
{
    os_signalHandler _this = signalHandlerObj;
    os_callbackArg arg;
    os_uint32 index;

    assert(_this);

    switch (fdwCtrlType) {
    case CTRL_LOGOFF_EVENT:
        /* It is not correct to ever exit when this event is received. see dds#2732
        From MSDN re CTRL_LOGOFF_EVENT:
        "A signal that the system sends to all console processes when a user is
        logging off. This signal does not indicate which user is logging off, so
        no assumptions can be made. Note that this signal is received only by
        services. Interactive applications are terminated at logoff, so they are
        not present when the system sends this signal." */
        return TRUE;
    }

    arg.sig = (void*)(os_address)0;
    index = pa_inc32_nv(&_this->callbackInfo.exitRequestInsertionIndex) % EXIT_REQUEST_BUFFER_SIZE;
    os__signalHandlerExitRequestGetThreadContextCallbackInvoke(&_this->callbackInfo, index);
    os_signalHandlerDeleteDeregisteredExitRequestCallbacks(&_this->callbackInfo);
    return TRUE;
}

static void *
signalEmulatorThread(
    void *arg)
{
    os_int32 sig;
    DWORD nread;
    BOOL result;
    const os_duration delay = 100*OS_DURATION_MILLISECOND;
    os_signalHandler _this = signalHandlerObj;

    assert(_this);

    do {
        sig = 0;
        result = (ConnectNamedPipe(_this->signalPipe, NULL) ? TRUE : (os_getErrno() == ERROR_PIPE_CONNECTED));
        if (result) {
            /* read signal */

            result = ReadFile(_this->signalPipe, &sig, sizeof(sig), &nread, NULL);
            if (result && (nread == sizeof(sig)) && sig > 0) {
                os_callbackArg arg;
                os_uint32 index;

                OS_DEBUG_1("signalEmulatorThread", "Received signal %d", sig);

                if (sig == OS_SIGKILL) {
                   _exit(0);
                }
                arg.sig = (void*)(os_address)sig;
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
            /* Poll for availability of pipe. Would be odd, since it is set to
             * block until connected, but doesn't really hurt. */
            ospl_os_sleep(delay);
        }
    } while (sig >= 0);

    return NULL;
}


os_result
os_signalHandlerFinishExitRequest(
    os_callbackArg arg)
{
    int sig = (int)(os_address)arg.sig;

    /* If sig == 0, the exit-request was from a CtrlHandler and we may need to
     * do chaining. Otherwise it originates from the signalEmulator and no
     * chaining is needed. */
    if(sig == 0){
        /* Remove our handler and trigger CtrlHandler again. */
        (void) SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, FALSE);
    }

    os_procCallExitHandlers();

    if(sig == 0){
        /* Generate ConsolCtrlEvent to ensure other handlers are still invoked. */
        if(GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0) == 0){
            ExitProcess(0); /* Will not return */
        }
    } else {
        ExitProcess(0); /* Will not return */
    }

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
