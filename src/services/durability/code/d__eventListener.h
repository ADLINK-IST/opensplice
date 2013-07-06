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

#ifndef D__EVENTLISTENER_H
#define D__EVENTLISTENER_H

#include "d__types.h"
#include "d_eventListener.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_eventListener){
    C_EXTENDS(d_object);
    c_ulong interest;
    d_eventListenerFunc func;
    c_voidp args;
};

void
d_eventListenerDeinit(
    d_object object);

#if defined (__cplusplus)
}
#endif

#endif /*D__EVENTLISTENER_H*/
