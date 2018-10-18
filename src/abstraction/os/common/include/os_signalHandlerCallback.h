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

#ifndef OS_COMMON_SIGNALHANDLERCALLBACK_H
#define OS_COMMON_SIGNALHANDLERCALLBACK_H

#include "os_mutex.h"
#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Exit Signal Buffer Size determines how many ExitSignals can be processed simultaneously. */
#define EXIT_REQUEST_BUFFER_SIZE 16

typedef struct os_signalHandlerThreadContextBuffer_s os_signalHandlerThreadContextBuffer;
struct os_signalHandlerThreadContextBuffer_s {
    os_boolean contextAssigned;
    void *threadContext;
};

typedef struct os_signalHandlerExitRequestCallbackInfo_s os_signalHandlerExitRequestCallbackInfo;
struct os_signalHandlerExitRequestCallbackInfo_s {
    os_signalHandlerExitRequestCallbackInfo *next;
    os_signalHandlerExitRequestCallback callbackExitRequest;
    os_signalHandlerGetThreadContextCallback callbackGetThreadContext;
    os_signalHandlerGetThreadContextCallback callbackFreeThreadContext;
    os_signalHandlerThreadContextBuffer contextBuffer[EXIT_REQUEST_BUFFER_SIZE];

    void * arg;
    os_boolean deregistered;
};

typedef struct os_signalHandlerExceptionCallbackInfo_s os_signalHandlerExceptionCallbackInfo;
struct os_signalHandlerExceptionCallbackInfo_s {
    os_signalHandlerExceptionCallbackInfo *next;
    os_signalHandlerExceptionCallback callbackException;
    os_signalHandlerGetThreadContextCallback callbackGetThreadContext;
    os_signalHandlerGetThreadContextCallback callbackFreeThreadContext;
    void *threadContext;

    void * arg;
};

typedef struct os_signalHandlerCallbackInfo_s os_signalHandlerCallbackInfo;
struct os_signalHandlerCallbackInfo_s {
    os_mutex exitRequestMtx;
    os_signalHandlerExitRequestCallbackInfo *exitRequestCallbackInfo;
    os_uint32 nrExitRequestHandlers;
    pa_uint32_t exitRequestInsertionIndex;
    os_uint32 exitRequestConsumptionIndex;
    os_mutex exceptionMtx;
    os_signalHandlerExceptionCallbackInfo *exceptionCallbackInfo;
};

static os_result    os__signalHandlerCallbackInit(os_signalHandlerCallbackInfo *_this);
static void         os__signalHandlerCallbackDeinit(os_signalHandlerCallbackInfo *_this);

static os_uint32 os__signalHandlerExitRequestCallbackInvoke(
    os_signalHandlerCallbackInfo *_this,
    os_callbackArg arg);

static void         os__signalHandlerExceptionCallbackInvoke(
    os_signalHandlerCallbackInfo *_this);

static void         os__signalHandlerExceptionGetThreadContextCallbackInvoke(
    os_signalHandlerCallbackInfo *_this);

static void         os__signalHandlerExitRequestGetThreadContextCallbackInvoke(
    os_signalHandlerCallbackInfo *_this,
    os_uint32 exitRequestInsertionIndex);
/* The implementation of the below static call must be provided by the platform-
 * specific implementation of the signalhandler. */
static os_signalHandlerCallbackInfo* os__signalHandlerGetCallbackInfo(void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_COMMON_SIGNALHANDLERCALLBACK_H */
