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

#ifndef D_ACTIONQUEUE_H
#define D_ACTIONQUEUE_H

#include "d__types.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef c_bool (*d_actionFunction)(d_action action, c_bool terminate);

#define d_actionQueue(q)    ((d_actionQueue)(q))
#define d_action(a)         ((d_action)(a))

d_actionQueue       d_actionQueueNew        (const c_char* name,
											 os_time sleepTime,
											 os_threadAttr attr);

void                d_actionQueueFree       (d_actionQueue queue);

c_bool              d_actionQueueAdd        (d_actionQueue queue,
                                             d_action action);
                                             
c_bool              d_actionQueueRemove     (d_actionQueue queue,
                                             d_action action);

d_action            d_actionNew             (os_time execTime,
                                             os_time sleepTime,
                                             d_actionFunction action,
                                             c_voidp args);
                                             
void                d_actionFree            (d_action action);

c_voidp             d_actionGetArgs         (d_action action);

os_time             d_actionGetExecTime     (d_action action);

os_time             d_actionGetSleepTime    (d_action action);

void                d_actionSetSleepTime    (d_action action,
                                             os_time sleepTime);

#if defined (__cplusplus)
}
#endif

#endif /*D_ACTIONQUEUE_H*/
