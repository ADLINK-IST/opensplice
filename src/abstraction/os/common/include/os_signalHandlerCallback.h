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

#ifndef OS_COMMON_SIGNALHANDLERCALLBACK_H
#define OS_COMMON_SIGNALHANDLERCALLBACK_H

#include "os_mutex.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct os_signalHandlerExitRequestCallbackInfo_s os_signalHandlerExitRequestCallbackInfo;
struct os_signalHandlerExitRequestCallbackInfo_s {
    os_signalHandlerExitRequestCallbackInfo *next;
    os_signalHandlerExitRequestCallback callback;
    void * arg;
};

typedef struct os_signalHandlerExceptionCallbackInfo_s os_signalHandlerExceptionCallbackInfo;
struct os_signalHandlerExceptionCallbackInfo_s {
    os_signalHandlerExceptionCallbackInfo *next;
    os_signalHandlerExceptionCallback callback;
    void * arg;
};

typedef struct os_signalHandlerCallbackInfo_s os_signalHandlerCallbackInfo;
struct os_signalHandlerCallbackInfo_s {
    os_mutex exitRequestMtx;
    os_signalHandlerExitRequestCallbackInfo *exitRequestCallbackInfo;
    os_mutex exceptionMtx;
    os_signalHandlerExceptionCallbackInfo *exceptionCallbackInfo;
};

static os_result    os__signalHandlerCallbackInit(os_signalHandlerCallbackInfo *_this);
static void         os__signalHandlerCallbackDeinit(os_signalHandlerCallbackInfo *_this);

static unsigned int os__signalHandlerExitRequestCallbackInvoke(os_signalHandlerCallbackInfo *_this, os_callbackArg arg);
static void         os__signalHandlerExceptionCallbackInvoke(os_signalHandlerCallbackInfo *_this, os_callbackArg arg);

/* The implementation of the below static call must be provided by the platform-
 * specific implementation of the signalhandler. */
static os_signalHandlerCallbackInfo* os__signalHandlerGetCallbackInfo(void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_COMMON_SIGNALHANDLERCALLBACK_H */
