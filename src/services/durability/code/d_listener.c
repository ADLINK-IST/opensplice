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

#include "d__listener.h"
#include "d_listener.h"

void
d_listenerInit(
    d_listener listener,
    d_subscriber subscriber,
    d_listenerAction action,
    d_objectDeinitFunc deinit)
{
    
    d_lockInit(d_lock(listener), D_LISTENER, deinit);
    
    if(listener){
        listener->admin     = d_subscriberGetAdmin(subscriber);
        listener->action    = action;
        listener->attached  = FALSE;
    }
}

void
d_listenerFree(
    d_listener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    
    if(listener){
        d_lockFree(d_lock(listener), D_LISTENER);
    }
}

c_bool
d_listenerIsValid(
    d_listener listener,
    d_listenerKind kind)
{
    c_bool result = FALSE;
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    
    if(listener->kind == kind){
        result = TRUE;
    }
    return result;
    
    
}

void
d_listenerLock(
    d_listener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    d_lockLock(d_lock(listener));
}

void
d_listenerUnlock(
    d_listener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    d_lockUnlock(d_lock(listener));
}

d_listenerAction
d_listenerGetAction(
    d_listener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    return listener->action;
}

c_bool
d_listenerIsAttached(
    d_listener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    return listener->attached;
}

d_admin
d_listenerGetAdmin(
    d_listener listener)
{
    assert(d_objectIsValid(d_object(listener), D_LISTENER) == TRUE);
    return listener->admin;
}
