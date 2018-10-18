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


#include "d__lock.h"
#include "d__actionQueue.h"
#include "d__thread.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_stdlib.h"

void*
d_actionQueueRun(
    void* userData)
{
    d_thread self = d_threadLookupSelf ();
    d_actionQueue queue;
    d_action action;
    c_bool result;
    os_timeM curTime;
    os_compare eq;
    c_iter actions, redo;

    redo    = c_iterNew(NULL);
    actions = c_iterNew(NULL);
    queue   = d_actionQueue(userData);

    while(queue->terminate == FALSE) {
        /**
         * Copy the actions here to allow other threads to add actions during the
         * execution of the actions.
         */
        d_lockLock(d_lock(queue));
        action = d_action(c_iterTakeFirst(queue->actions));

        while(action){
            actions = c_iterInsert(actions, action);
            action = d_action(c_iterTakeFirst(queue->actions));
        }
        d_lockUnlock(d_lock(queue));

        action = d_action(c_iterTakeFirst(actions));

        while(action && (queue->terminate == FALSE)){
            curTime = os_timeMGet();
            eq = os_timeMCompare(curTime, action->execTime);

            if(eq != OS_LESS) {
                result = action->action(action, FALSE);

                if(result == FALSE) {
                    d_actionFree(action);
                } else {
                    curTime = os_timeMGet();
                    curTime = os_timeMAdd(curTime, action->sleepTime);
                    action->execTime = curTime;
                    redo = c_iterInsert(redo, action);
                }
            } else {
                redo = c_iterInsert(redo, action);
            }
            action = d_action(c_iterTakeFirst(actions));
        }

        while(action){ /*in case of terminate*/
            redo = c_iterInsert(redo, action);
            action = d_action(c_iterTakeFirst(actions));
        }

        c_iterFree(actions);
        actions = redo;
        redo = c_iterNew(NULL);

        if(queue->terminate == FALSE) {
            d_sleep(self, queue->sleepTime);
        }
    }
    action = d_action(c_iterTakeFirst(actions));

    while(action) {
        action->action(action, TRUE);
        d_actionFree(action);
        action = d_action(c_iterTakeFirst(actions));
    }
    d_lockLock(d_lock(queue));
    action = d_action(c_iterTakeFirst(queue->actions));

    while(action) {
        action->action(action, TRUE);
        d_actionFree(action);
        action = d_action(c_iterTakeFirst(queue->actions));
    }
    d_lockUnlock(d_lock(queue));
    c_iterFree(actions);
    c_iterFree(redo);

    return NULL;
}


d_actionQueue
d_actionQueueNew(
    const c_char* name,
    os_duration sleepTime,
    os_threadAttr attr)
{
    d_actionQueue queue;
    os_result osr;
    const c_char* actualName;

    /* Allocate actionQueue object */
    queue = d_actionQueue(os_malloc(C_SIZEOF(d_actionQueue)));
    if (queue) {
        /* Call super-init */
        d_lockInit(d_lock(queue), D_ACTION_QUEUE,
                   (d_objectDeinitFunc)d_actionQueueDeinit);
        /* Initialize the actionQueue */
        if (queue) {
            queue->actions   = c_iterNew(NULL);
            queue->remove    = c_iterNew(NULL);
            queue->sleepTime = sleepTime;
            queue->terminate = FALSE;
            queue->name      = NULL;
            if (name == NULL) {
                actualName = "actionQueueThread";
            } else {
                actualName = name;
            }
            queue->name = os_strdup(actualName);
            osr = d_threadCreate(&queue->actionThread, actualName,
                                 &attr, (void*(*)(void*))d_actionQueueRun,
                                 (void*)queue);
            if (osr != os_resultSuccess) {
                d_actionQueueFree(queue);
                queue = NULL;
            }
        }
    }
    return queue;
}


void
d_actionQueueDeinit(
    d_actionQueue queue)
{
    assert(d_actionQueueIsValid(queue));

    if (os_threadIdToInteger(queue->actionThread)) {
        queue->terminate = TRUE;
        d_threadWaitExit(queue->actionThread, NULL);
    }
    if (queue->actions) {
        c_iterFree(queue->actions);
    }
    if (queue->remove) {
        c_iterFree(queue->remove);
    }
    if (queue->name) {
        os_free(queue->name);
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(queue));
}


void
d_actionQueueFree(
    d_actionQueue queue)
{
    assert(d_actionQueueIsValid(queue));

    d_objectFree(d_object(queue));
}


c_bool
d_actionQueueAdd(
    d_actionQueue queue,
    d_action action)
{
    c_bool result = FALSE;

    assert(d_actionQueueIsValid(queue));
    assert(d_actionIsValid(action));

    d_lockLock(d_lock(queue));
    if (c_iterContains(queue->actions, action) == FALSE) {
        queue->actions = c_iterInsert(queue->actions, action);
        result = TRUE;
    }
    d_lockUnlock(d_lock(queue));

    return result;
}


c_bool
d_actionQueueRemove(
    d_actionQueue queue,
    d_action action)
{
    c_bool result = FALSE;

    assert(d_actionQueueIsValid(queue));
    assert(d_actionIsValid(action));

    d_lockLock(d_lock(queue));
    if (c_iterContains(queue->actions, action) == TRUE) {
        c_iterTake(queue->actions, action);
        result = TRUE;
    }
    d_lockUnlock(d_lock(queue));

    return result;
}


d_action
d_actionNew(
    os_timeM execTime,
    os_duration sleepTime,
    d_actionFunction actionFunc,
    c_voidp args)
{
    d_action action;

    /* Allocate action object */
    action = d_action(os_malloc(C_SIZEOF(d_action)));
    if (action) {
        /* Call super-init */
        d_objectInit(d_object(action), D_ACTION,
                     (d_objectDeinitFunc)d_actionDeinit);
        /* Initialize the action */
        if (action) {
            action->execTime     = execTime;
            action->sleepTime    = sleepTime;
            action->action       = actionFunc;
            action->args         = args;
        }
    }
    return action;
}


void
d_actionDeinit(
    d_action action)
{
    assert(d_actionIsValid(action));

    /* Nothing to deallocate */
    /* Call super-deinit */
    d_objectDeinit(d_object(action));
}


void
d_actionFree(
    d_action action)
{
    assert(d_actionIsValid(action));

    d_objectFree(d_object(action));
}


c_voidp
d_actionGetArgs(
    d_action action)
{
    assert(d_actionIsValid(action));

    return action->args;
}


os_timeM
d_actionGetExecTime(
    d_action action)
{
    assert(d_actionIsValid(action));

    return action->execTime;
}


os_duration
d_actionGetSleepTime(
    d_action action)
{
    assert(d_actionIsValid(action));

    return action->sleepTime;
}


void
d_actionSetSleepTime(
    d_action action,
    os_duration sleepTime)
{
    assert(d_actionIsValid(action));

    action->sleepTime = sleepTime;
}
