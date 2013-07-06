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


#include "d_lock.h"
#include "d_actionQueue.h"
#include "d__actionQueue.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_thread.h"

void*
d_actionQueueRun(
    void* userData)
{
    d_actionQueue queue;
    d_action action;
    c_bool result;
    os_time curTime;
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
            curTime = os_timeGet();
            eq = os_timeCompare(curTime, action->execTime);
            
            if(eq != OS_LESS) {
                result = action->action(action, FALSE);
                
                if(result == FALSE) {
                    d_actionFree(action);
                } else {
                    curTime = os_timeGet();
                    curTime = os_timeAdd(curTime, action->sleepTime);
                    action->execTime.tv_sec = curTime.tv_sec;
                    action->execTime.tv_nsec = curTime.tv_nsec;
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
            os_nanoSleep(queue->sleepTime);
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
    os_time sleepTime,
    os_threadAttr attr)
{
    d_actionQueue queue;
    os_result osr;
    const c_char* actualName;
    
    queue = d_actionQueue(os_malloc(C_SIZEOF(d_actionQueue)));
    
    if(queue) {
        d_lockInit(d_lock(queue), D_ACTION_QUEUE, d_actionQueueDeinit);
        
        if(queue) {
            queue->actions   = c_iterNew(NULL);
            queue->remove    = c_iterNew(NULL);
            queue->sleepTime.tv_sec = sleepTime.tv_sec;
            queue->sleepTime.tv_nsec = sleepTime.tv_nsec;
            queue->terminate = FALSE;
            queue->name      = NULL;
                    
        	if(name == NULL){
        		actualName = "actionQueueThread";
        	} else {
        		actualName = name;
        	}
        	
            queue->name = os_strdup(actualName);
            osr = os_threadCreate(&queue->actionThread, actualName, 
                                  &attr, (void*(*)(void*))d_actionQueueRun, 
                                  (void*)queue);
            
            if(osr != os_resultSuccess) {
                d_actionQueueFree(queue);
                queue = NULL;
            }
            
        }
    }
    return queue;
}

void
d_actionQueueDeinit(
    d_object object)
{
    d_actionQueue queue;
    
    assert(d_objectIsValid(object, D_ACTION_QUEUE) == TRUE);
    
    if(object){
        queue = d_actionQueue(object);
        
        if(os_threadIdToInteger(queue->actionThread)) {
            queue->terminate = TRUE;
            os_threadWaitExit(queue->actionThread, NULL);
        }
        if(queue->actions){
            c_iterFree(queue->actions);
        }
        if(queue->remove) {
            c_iterFree(queue->remove);
        }
        if(queue->name){
            os_free(queue->name);
        }
    }
}

void
d_actionQueueFree(
    d_actionQueue queue)
{
    assert(d_objectIsValid(d_object(queue), D_ACTION_QUEUE) == TRUE);
    
    if(queue) {
        d_lockFree(d_lock(queue), D_ACTION_QUEUE);
    }
    return;
}

c_bool
d_actionQueueAdd(
    d_actionQueue queue,
    d_action action)
{
    c_bool result;
    
    assert(d_objectIsValid(d_object(queue), D_ACTION_QUEUE) == TRUE);
    
    result = FALSE;
    
    if(queue) {
        d_lockLock(d_lock(queue));
        
        if(c_iterContains(queue->actions, action) == FALSE){
            queue->actions = c_iterInsert(queue->actions, action);
            result = TRUE;
        }
        d_lockUnlock(d_lock(queue));
        
    }
    return result;
}
                                             
c_bool
d_actionQueueRemove(
    d_actionQueue queue,
    d_action action)
{
    c_bool result;
    
    assert(d_objectIsValid(d_object(queue), D_ACTION_QUEUE) == TRUE);
    
    result = FALSE;
    
    if(queue) {
        d_lockLock(d_lock(queue));
        
        if(c_iterContains(queue->actions, action) == TRUE){
            c_iterTake(queue->actions, action);
            result = TRUE;
        }
        d_lockUnlock(d_lock(queue));
    }
    return result;
}

d_action
d_actionNew(
    os_time execTime,
    os_time sleepTime,
    d_actionFunction actionFunc,
    c_voidp args)
{
    d_action action;
    
    action = d_action(os_malloc(C_SIZEOF(d_action)));
    
    if(action) {
        d_objectInit(d_object(action), D_ACTION, d_actionDeinit);
        
        if(action) {
            action->execTime.tv_sec     = execTime.tv_sec;
            action->execTime.tv_nsec    = execTime.tv_nsec;
            action->sleepTime.tv_sec    = sleepTime.tv_sec;
            action->sleepTime.tv_nsec   = sleepTime.tv_nsec;
            action->action              = actionFunc;
            action->args                = args;
        }
    }
    return action;
}
                                             
void
d_actionDeinit(
    d_object object)
{
    OS_UNUSED_ARG(object);
    assert(d_objectIsValid(object, D_ACTION) == TRUE);
    
    return;
}

void
d_actionFree(
    d_action action)
{
    assert(d_objectIsValid(d_object(action), D_ACTION) == TRUE);
    
    if(action) {
        d_objectFree(d_object(action), D_ACTION);
    }
    return;
}

c_voidp
d_actionGetArgs(
    d_action action)
{
    c_voidp args = NULL;
    
    assert(d_objectIsValid(d_object(action), D_ACTION) == TRUE);
    
    if(action) {
        args = action->args;
    }
    return args;
}

os_time
d_actionGetExecTime(
    d_action action)
{
    os_time execTime;
    
    assert(d_objectIsValid(d_object(action), D_ACTION) == TRUE);
    
    if(action){
        execTime.tv_sec  = action->execTime.tv_sec;
        execTime.tv_nsec = action->execTime.tv_nsec;
    }else {
        execTime.tv_sec = 0;
        execTime.tv_nsec = 0;
    }
    return execTime;
}

os_time
d_actionGetSleepTime(
    d_action action)
{
    os_time sleepTime;
    
    assert(d_objectIsValid(d_object(action), D_ACTION) == TRUE);
    
    if(action){
        sleepTime.tv_sec  = action->sleepTime.tv_sec;
        sleepTime.tv_nsec = action->sleepTime.tv_nsec;
    }else {
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 0;
    }

    return sleepTime;
}

void
d_actionSetSleepTime(
    d_action action,
    os_time sleepTime)
{
    assert(d_objectIsValid(d_object(action), D_ACTION) == TRUE);
    
    if(action){
        action->sleepTime.tv_sec     = sleepTime.tv_sec;
        action->sleepTime.tv_nsec    = sleepTime.tv_nsec;
    }
}

