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
/** \file os/win32/code/os_signalHandler.c
 *  \brief WIN32 signal handler management
 *
 * Implements signal handler management for WIN32
 */
#include "os_signalHandler.h"

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

os_signalHandlerExitRequestCallback
os_signalHandlerSetExitRequestCallback(
    os_signalHandlerExitRequestCallback cb)
{
    return (os_signalHandlerExitRequestCallback)0;
}


os_signalHandlerExceptionCallback
os_signalHandlerSetExceptionCallback(
    os_signalHandlerExceptionCallback cb)
{
    return (os_signalHandlerExceptionCallback)0;
}

os_result
os_signalHandlerFinishExitRequest(
    os_callbackArg arg)
{
    return os_resultSuccess;
}

os_result
os_signalHandlerSetHandler(
    os_signal signal,
    os_actionHandler handler)
{
    return os_resultSuccess;
}

os_result
os_signalHandlerSetEnabled(
    os_uint enabled)
{
    return os_resultSuccess;
}
