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

#ifndef OS_SIGNALHANDLER_H
#define OS_SIGNALHANDLER_H

#include "os_defs.h"
#include "os_signal.h"
#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef struct os_signalHandler_s* os_signalHandler;

typedef struct { void * sig; os_ulong_int ThreadId;} os_callbackArg;

typedef os_result (*os_signalHandlerExitRequestCallback)(os_callbackArg, void *);
typedef struct { void *handle; } os_signalHandlerExitRequestHandle;

/**
 * NULL-handle that can be used to initialize an os_signalHandlerExitRequestHandle.
 * This handle can be safely passed to os_signalHandlerUnregisterExitRequestCallback
 * without causing a side-effect. */
static const os_signalHandlerExitRequestHandle os_signalHandlerExitRequestHandleNil = { NULL };

typedef os_result (*os_signalHandlerExceptionCallback)(os_callbackArg, void *);
typedef struct { void *handle; } os_signalHandlerExceptionHandle;

/**
 * NULL-handle that can be used to initialize an os_signalHandlerExceptionHandle.
 * This handle can be safely passed to os_signalHandlerUnregisterExceptionCallback
 * without causing a side-effect. */
static const os_signalHandlerExceptionHandle os_signalHandlerExceptionHandleNil = { NULL };

OS_API os_result
os_signalHandlerNew(
    void);

OS_API void
os_signalHandlerFree(
    void);

OS_API os_result
os_signalHandlerEnableExceptionSignals (
    void);

/**
 * Registers the supplied callback for invocation on receiving an exit-request.
 *
 * The callback should not call os_signalHandlerUnregisterExitRequestCallback or
 * os_signalHandlerRegisterExitRequestCallback itself.
 *
 * The memory allocation done by this call is assumed to always succeed. In case
 * it would fail, it cannot be distinguished from a platform that doesn't support
 * these callbacks, since in both cases os_signalHandlerExitRequestHandleNil
 * will be returned.
 *
 * @param  cb  the callback to be registered
 * @param  arg the parameter to be passed to callback cb when invoked
 * @return     an opaque handle to the newly registered exit-request callback
 */
OS_API os_signalHandlerExitRequestHandle
os_signalHandlerRegisterExitRequestCallback(
    os_signalHandlerExitRequestCallback cb,
    void * arg);

/**
 * Unregisters the supplied callback.
 *
 * The supplied handle must be valid (i.e., obtained from an invocation of
 * os_signalHandlerRegisterExitRequestCallback or the special
 * os_signalHandlerExitRequestHandleNil). Supplying an invalid handle may
 * result in undefined behaviour.
 *
 * @pre        the supplied handle must either be obtained from an invocation
 *             of os_signalHandlerRegisterExitRequestCallback or be the special
 *             os_signalHandlerExitRequestHandleNil
 * @post       the callback will not be invoked after this call returns
 * @param  erh the handle for the callback to be unregistered
 */
OS_API void
os_signalHandlerUnregisterExitRequestCallback(
    os_signalHandlerExitRequestHandle erh);

/**
 * Registers the supplied callback for invocation on receiving an exception.
 *
 * The callback should not call os_signalHandlerRegisterExceptionCallback or
 * os_signalHandlerUnregisterExceptionCallback itself.
 *
 * The memory allocation done by this call is assumed to always succeed. In case
 * it would fail, it cannot be distinguished from a platform that doesn't support
 * these callbacks, since in both cases os_signalHandlerExceptionHandleNil
 * will be returned.
 *
 * @param  cb  the callback to be registered
 * @param  arg the parameter to be passed to callback cb when invoked
 * @return     an opaque handle to the newly registered exception callback
 */
OS_API os_signalHandlerExceptionHandle
os_signalHandlerRegisterExceptionCallback(
    os_signalHandlerExceptionCallback cb,
    void * arg);

/**
 * Unregisters the supplied callback.
 *
 * The supplied handle must be valid (i.e., obtained from an invocation of
 * os_signalHandlerRegisterExceptionCallback or the special
 * os_signalHandlerExceptionHandleNil). Supplying an invalid handle may
 * result in undefined behaviour.
 *
 * @pre        the supplied handle must either be obtained from an invocation
 *             of os_signalHandlerRegisterExceptionCallback or be the special
 *             os_signalHandlerExceptionHandleNil
 * @post       the callback will not be invoked after this call returns
 * @param  eh  the handle for the callback to be unregistered
 */
OS_API void
os_signalHandlerUnregisterExceptionCallback(
    os_signalHandlerExceptionHandle eh);

OS_API os_result
os_signalHandlerFinishExitRequest(
    os_callbackArg arg);

OS_API os_result
os_signalHandlerSetHandler(
    os_signal signal,
    os_actionHandler handler);

OS_API os_result
os_signalHandlerSetEnabled(
    os_uint enabled);

OS_API void
os_signalHandlerIgnoreJavaSignals (void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

