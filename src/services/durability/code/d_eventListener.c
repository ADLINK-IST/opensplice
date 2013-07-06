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

#include "d__eventListener.h" 
#include "d_eventListener.h"
#include "os_heap.h"

d_eventListener
d_eventListenerNew(
    c_ulong interest,
    d_eventListenerFunc func,
    c_voidp args)
{
    d_eventListener listener;
    
    listener = NULL;
    
    if(func){
        listener = d_eventListener(os_malloc(C_SIZEOF(d_eventListener)));
        d_objectInit(d_object(listener), D_EVENT_LISTENER, d_eventListenerDeinit);
        listener->interest  = interest;
        listener->func      = func;
        listener->args      = args;
    }
    return listener;
}

c_voidp
d_eventListenerGetUserData(
    d_eventListener listener)
{
    c_voidp userData = NULL;
    
    assert(d_objectIsValid(d_object(listener), D_EVENT_LISTENER) == TRUE);
    
    if(listener){
        userData = listener->args;
    }
    return userData;
}

void
d_eventListenerDeinit(
    d_object object)
{
    OS_UNUSED_ARG(object);
    assert(d_objectIsValid(object, D_EVENT_LISTENER) == TRUE);
    
    return;
}

void
d_eventListenerFree(
    d_eventListener listener)
{
    assert(d_objectIsValid(d_object(listener), D_EVENT_LISTENER) == TRUE);
    
    if(listener){
        d_objectFree(d_object(listener), D_EVENT_LISTENER);
    }
}
