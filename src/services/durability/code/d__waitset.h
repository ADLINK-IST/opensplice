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

#ifndef D__WAITSET_H
#define D__WAITSET_H

#include "d__types.h"
#include "d_lock.h"
#include "d_waitset.h"
#include "u_dispatcher.h"
#include "os_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_waitsetEntity) {
    C_EXTENDS(d_object);
    c_char* name;
    d_waitset waitset;
    u_dispatcher dispatcher;
    c_ulong mask;
    d_waitsetAction action;
    os_threadAttr attr;
    c_voidp usrData;
};

C_STRUCT(d_waitset) {
    C_EXTENDS(d_lock);
    c_bool terminate;
    c_bool runToCompletion;
    c_bool timedWait;
    d_subscriber subscriber;
    u_waitset uwaitset;
    os_threadId thread;
    c_iter entities;
    c_iter threads;
};

c_equality          d_waitsetEntityFind                 (d_waitsetEntity we,
                                                         u_dispatcher entity);

void                d_waitsetEntityDeinit               (d_object object);

void                d_waitsetDeinit                     (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /*D__WAITSET_H*/
