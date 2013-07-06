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

#ifndef D_LISTENER_H
#define D_LISTENER_H

#include "d__types.h"
#include "u_dispatcher.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef void (*d_listenerAction)(d_listener listener, d_message message);

#define d_listener(l) ((d_listener)(l))


d_listenerAction    d_listenerGetAction     (d_listener listener);

c_bool              d_listenerIsAttached    (d_listener listener);

d_admin             d_listenerGetAdmin      (d_listener listener);

void                d_listenerLock          (d_listener listener);

void                d_listenerUnlock        (d_listener listener);

#if defined (__cplusplus)
}
#endif

#endif /* D_LISTENER_H */
