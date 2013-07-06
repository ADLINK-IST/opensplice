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

#ifndef D__ACTIONQUEUE_H
#define D__ACTIONQUEUE_H

#include "d__types.h"
#include "d_lock.h"
#include "d_actionQueue.h"
#include "os_thread.h"
#include "os_time.h"
#include "os_stdlib.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_STRUCT(d_action) {
    C_EXTENDS(d_object);
    os_time execTime;
    os_time sleepTime;
    d_actionFunction action;
    c_voidp args;
};

C_STRUCT(d_actionQueue) {
    C_EXTENDS(d_lock);
    os_threadId actionThread;
    os_time sleepTime;
    c_bool terminate;
    c_iter actions;
    c_iter remove;
    c_char* name;
};

void*   d_actionQueueRun    (void* userData);

void    d_actionQueueDeinit (d_object object);

void    d_actionDeinit      (d_object object);

#if defined (__cplusplus)
}
#endif

#endif /*D__ACTIONQUEUE_H*/
