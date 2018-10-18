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
/** \file os/vxwork5.5/code/os_signalHandler.c
 *  \brief vxwork5.5 signal handler management
 *
 * Implements signal handler management for vxwork5.5
 */
#include "os_signalHandler.h"

void
os_signalHandlerIgnoreJavaSignals (void)
{

}

os_result
os_signalHandlerNew(
    void)
{
    return os_resultSuccess;
}

void
os_signalHandlerFree(
    void)
{

}

os_result
os_signalHandlerEnableExceptionSignals (
    void)
{
    return os_resultSuccess;
}

os_signalHandlerExitRequestHandle
os_signalHandlerRegisterExitRequestCallback(
    os_signalHandlerExitRequestCallback callback,
    os_signalHandlerAllocThreadContextCallback cbAllocThreadContext,
    os_signalHandlerGetThreadContextCallback cbGetThreadContext,
    os_signalHandlerFreeThreadContextCallback cbFreeThreadContext,
    void * arg)
{
    OS_UNUSED_ARG(callback);
    OS_UNUSED_ARG(cbAllocThreadContext);
    OS_UNUSED_ARG(cbGetThreadContext);
    OS_UNUSED_ARG(cbFreeThreadContext);
    OS_UNUSED_ARG(arg);

    return os_signalHandlerExitRequestHandleNil;
}


os_signalHandlerExceptionHandle
os_signalHandlerRegisterExceptionCallback(
    os_signalHandlerExceptionCallback callback,
    os_signalHandlerAllocThreadContextCallback cbAllocThreadContext,
    os_signalHandlerGetThreadContextCallback cbGetThreadContext,
    os_signalHandlerFreeThreadContextCallback cbFreeThreadContext,
    void * arg)
{
    OS_UNUSED_ARG(callback);
    OS_UNUSED_ARG(cbAllocThreadContext);
    OS_UNUSED_ARG(cbGetThreadContext);
    OS_UNUSED_ARG(cbFreeThreadContext);
    OS_UNUSED_ARG(arg);

    return os_signalHandlerExceptionHandleNil;
}

os_signalHandlerExceptionHandle
os_signalHandlerRegisterCheckExceptionCallback(
    os_signalHandlerExceptionCallback callback,
    void * arg)
{
    OS_UNUSED_ARG(callback);
    OS_UNUSED_ARG(arg);

    return os_signalHandlerExceptionHandleNil;
}

void
os_signalHandlerUnregisterExitRequestCallback(
    os_signalHandlerExitRequestHandle erh)
{
    OS_UNUSED_ARG(erh);
}

void
os_signalHandlerUnregisterExceptionCallback(
    os_signalHandlerExceptionHandle eh)
{
    OS_UNUSED_ARG(eh);
}

os_result
os_signalHandlerFinishExitRequest(
    os_callbackArg arg)
{
    OS_UNUSED_ARG(arg);

    return os_resultSuccess;
}

os_result
os_signalHandlerSetEnabled(
    os_uint enabled)
{
    OS_UNUSED_ARG(enabled);

    return os_resultSuccess;
}
