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

#include "u_listener.h"
#include "os.h"
#include "u__types.h"

u_listener
u_listenerNew(
    u_listenerCallback cb,
    c_voidp usrData)
{
    u_listener l;

    l = os_malloc(C_SIZEOF(u_listener));
    l->listener = cb;
    l->usrData = usrData;

    return l;
}

void
u_listenerFree(
    u_listener l)
{
    l->listener = NULL;
    l->usrData = NULL;
    os_free(l);
}

void
u_listenerExecute (
    u_listener l,
    u_dispatcher o)
{
    l->listener(o,o->event,l->usrData);
}

