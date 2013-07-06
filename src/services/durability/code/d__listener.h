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

#ifndef D__LISTENER_H
#define D__LISTENER_H

#include "d_subscriber.h"
#include "d__types.h"
#include "d_lock.h"
#include "d_listener.h"
#include "u_dispatcher.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define USE_WAITSET 1

typedef enum d_listenerKind {
    D_STATUS_LISTENER, D_GROUP_REMOTE_LISTENER, D_GROUP_LOCAL_LISTENER, 
    D_GROUPS_REQ_LISTENER, D_SAMPLE_REQ_LISTENER, D_STATUS_REQ_LISTENER,
    D_SAMPLE_CHAIN_LISTENER, D_NAMESPACES_REQ_LISTENER, D_NAMESPACES_LISTENER,
    D_PERSISTENT_DATA_LISTENER, D_DELETE_DATA_LISTENER
} d_listenerKind;

C_STRUCT(d_listener){
    C_EXTENDS(d_lock);
    d_listenerKind      kind;
    d_admin             admin;
    d_listenerAction    action;
    c_bool              attached;
};

#define d_listener(l) ((d_listener)(l))

void        d_listenerInit          (d_listener listener,
                                     d_subscriber subscriber,
                                     d_listenerAction action,
                                     d_objectDeinitFunc deinit);

void        d_listenerFree          (d_listener listener);

c_bool      d_listenerIsValid       (d_listener listener,
                                     d_listenerKind kind);

#if defined (__cplusplus)
}
#endif

#endif /* D__LISTENER_H */
