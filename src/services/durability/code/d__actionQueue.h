/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef D__ACTIONQUEUE_H
#define D__ACTIONQUEUE_H

#include "d__types.h"
#include "os_time.h"
#include "os_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_actionQueue validity.
 * Because d_actionQueue is a concrete class typechecking is required.
 */
#define             d_actionQueueIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ACTION_QUEUE)

/**
 * Macro that checks the d_action validity.
 * Because d_action is a concrete class typechecking is required.
 */
#define             d_actionIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_ACTION)

/**
 * \brief The d_actionQueue cast macro.
 *
 * This macro casts an object to a d_actionQueue object.
 */
#define d_actionQueue(_this) ((d_actionQueue)(_this))

/**
 * \brief The d_action cast macro.
 *
 * This macro casts an object to a d_action object.
 */
#define d_action(_this) ((d_action)(_this))

typedef c_bool (*d_actionFunction)(d_action action, c_bool terminate);

C_STRUCT(d_action) {
    C_EXTENDS(d_object);
    os_timeM execTime;
    os_duration sleepTime;
    d_actionFunction action;
    c_voidp args;
};

C_STRUCT(d_actionQueue) {
    C_EXTENDS(d_lock);
    os_threadId actionThread;
    os_duration sleepTime;
    c_bool terminate;
    c_iter actions;
    c_iter remove;
    c_char* name;
};

void*               d_actionQueueRun        (void* userData);

d_actionQueue       d_actionQueueNew        (const c_char* name,
                                             os_duration sleepTime,
                                             os_threadAttr attr);

void                d_actionQueueDeinit     (d_actionQueue queue);

void                d_actionQueueFree       (d_actionQueue queue);

c_bool              d_actionQueueAdd        (d_actionQueue queue,
                                             d_action action);

c_bool              d_actionQueueRemove     (d_actionQueue queue,
                                             d_action action);

d_action            d_actionNew             (os_timeM execTime,
                                             os_duration sleepTime,
                                             d_actionFunction action,
                                             c_voidp args);

void                d_actionDeinit          (d_action action);

void                d_actionFree            (d_action action);

c_voidp             d_actionGetArgs         (d_action action);

os_timeM            d_actionGetExecTime     (d_action action);

os_duration         d_actionGetSleepTime    (d_action action);

void                d_actionSetSleepTime    (d_action action,
                                             os_duration sleepTime);

#if defined (__cplusplus)
}
#endif

#endif /*D__ACTIONQUEUE_H*/
