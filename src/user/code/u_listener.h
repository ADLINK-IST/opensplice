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

#ifndef U_LISTENER_H
#define U_LISTENER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_dispatcher.h"

typedef u_dispatcherListener u_listenerCallback;

C_CLASS(u_listener);

C_STRUCT(u_listener)
{
    u_listenerCallback   listener;
    c_voidp              usrData; /* data specific for a user, just 
                                     passed as value to the function
                                     callback 
                                  */
};

#define  u_listener(o)    ((u_listener)(o))

u_listener u_listenerNew     (u_listenerCallback cb, c_voidp usrData);
void       u_listenerFree    (u_listener l);
void       u_listenerExecute (u_listener l, u_dispatcher o);

#if defined (__cplusplus)
}
#endif

#endif

