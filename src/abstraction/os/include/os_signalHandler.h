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

typedef struct { void * sig;} os_callbackArg;

typedef os_result (*os_signalHandlerExitRequestCallback)(os_callbackArg);

typedef os_result (*os_signalHandlerExceptionCallback)();

OS_API os_result
os_signalHandlerNew(
    void);

OS_API void
os_signalHandlerFree(
    void);

OS_API os_signalHandlerExitRequestCallback
os_signalHandlerSetExitRequestCallback(
    os_signalHandlerExitRequestCallback cb);

OS_API os_signalHandlerExceptionCallback
os_signalHandlerSetExceptionCallback(
    os_signalHandlerExceptionCallback cb);

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

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

