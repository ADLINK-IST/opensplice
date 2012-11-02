/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef U_DISPATCHER_H
#define U_DISPATCHER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* Returns mask of events that have been handled by the listener. */
typedef c_ulong (*u_dispatcherListener)(u_dispatcher o, c_ulong event, c_voidp usrData);
typedef void (*u_dispatcherThreadAction)(u_dispatcher o, c_voidp usrData);

#define  u_dispatcher(o)            ((u_dispatcher)(o))

OS_API u_result 
u_dispatcherInsertListener(
    u_dispatcher o,
    u_dispatcherListener l,
    c_voidp userData);
    
OS_API u_result 
u_dispatcherAppendListener(
    u_dispatcher o,
    u_dispatcherListener l, 
    c_voidp userData);
    
OS_API u_result 
u_dispatcherRemoveListener(
     u_dispatcher o,
     u_dispatcherListener l);

OS_API u_result 
u_dispatcherNotify(
    u_dispatcher o);
    
OS_API u_result 
u_dispatcherSetEventMask(
    u_dispatcher o, 
    c_ulong eventMask);
    
OS_API u_result 
u_dispatcherGetEventMask(
    u_dispatcher o, 
    c_ulong *eventMask);

/* Sets two callback functions, which are called on start and stop of the dispatcher thread */
OS_API u_result 
u_dispatcherSetThreadAction(
    u_dispatcher o, 
    u_dispatcherThreadAction startAction,
    u_dispatcherThreadAction stopAction, 
    c_voidp actionData);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

