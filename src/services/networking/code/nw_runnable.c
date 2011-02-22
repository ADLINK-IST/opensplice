/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/* Interface */
#include "nw__runnable.h"
#include "nw_runnable.h"

/* Implementation */
#include "nw__confidence.h"
#include "os_heap.h"
#include "u_user.h"
#include "nw_report.h"
#include "nw_misc.h"
#include "nw_configuration.h"

/* Private functions */

static void *
nw_runnableDispatcher(
    void *userData)
{
    nw_runnable runnable = (nw_runnable)userData;
    
    /* All threads are protected for this daemon */
    if (runnable && runnable->mainFunc) {
        NW_TRACE_1(Mainloop, 2, "Entering thread %s",
                   nw_runnableGetName(runnable));
        os_threadProtect();
        runnable->mainFunc(runnable,runnable->mainFuncArg);
        os_threadUnprotect();
        NW_TRACE_1(Mainloop, 2, "Leaving thread %s",
                   nw_runnableGetName(runnable));
    }
    
    return NULL;
}



/* Protected functions */

#define NW_SCHEDCLASS_REALTIME  "Realtime"
#define NW_SCHEDCLASS_TIMESHARE "Timeshare"

void
nw_runnableInitialize(
    nw_runnable runnable,
    const char *name,
    const char *pathName,
    const nw_runnableMainFunc runnableMainFunc,
    const c_voidp mainFuncArg,
    const nw_runnableTriggerFunc triggerFunc,
    const nw_runnableFinalizeFunc finalizeFunc)
{
    os_mutexAttr mutexAttr;
    struct nw_schedulingAttr_s schedulingAttr = {FALSE,0,OS_SCHED_DEFAULT};
    os_int32 schedPrio;
    char *schedClassString;
 
    if (runnable) {
        runnable->name = nw_stringDup(name);
        runnable->mainFunc = runnableMainFunc;
        runnable->mainFuncArg = mainFuncArg;
        runnable->triggerFunc = triggerFunc;
        runnable->finalizeFunc = finalizeFunc;
        
        runnable->runState = rsNone;
        runnable->terminate = FALSE;

        os_mutexAttrInit(&mutexAttr);
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&runnable->mutex, &mutexAttr);
        
        /* Get scheduling attributes */
        schedPrio = NWCF_SIMPLE_PARAM(Long, pathName, Priority);
        schedulingAttr.schedulingPrioUseDefault =
            (schedPrio == NWCF_DEF(Priority));
        if (!schedulingAttr.schedulingPrioUseDefault) {
            schedulingAttr.schedulingPrioValue = schedPrio;
        }

        schedulingAttr.schedulingClass = OS_SCHED_DEFAULT;
        schedClassString = NWCF_SIMPLE_PARAM(String, pathName, Class);
        if (strcmp(schedClassString, NW_SCHEDCLASS_REALTIME) == 0) {
            schedulingAttr.schedulingClass = OS_SCHED_REALTIME;
        } else if (strcmp(schedClassString, NW_SCHEDCLASS_TIMESHARE) == 0) {
            schedulingAttr.schedulingClass = OS_SCHED_TIMESHARE;
        }
        os_free(schedClassString);
        runnable->schedulingAttr = schedulingAttr;
    }
}
    
                
void
nw_runnableFinalize(
    nw_runnable runnable)
{
    if (runnable) {
        NW_TRACE_1(Destruction, 4, "Finalizing runnable %s", nw_runnableGetName(runnable));
        os_mutexDestroy(&runnable->mutex);

        /* Free allocated resources */
        os_free(runnable->name);
    }
}


const char *
nw_runnableGetName(
    nw_runnable runnable)
{
    const char *result = NULL;
    
    if (runnable) {
       os_mutexLock(&runnable->mutex);
       result = runnable->name;
       os_mutexUnlock(&runnable->mutex);
    }
    
    return result;
}

                
void
nw_runnableSetRunState(
    nw_runnable runnable,
    nw_runState runState)
{
    if (runnable) {
        os_mutexLock(&runnable->mutex);
        runnable->runState = runState;
        os_mutexUnlock(&runnable->mutex);
    }
}

                
c_bool
nw_runnableTerminationRequested(
    nw_runnable runnable)
{
    c_bool result = TRUE;
    
    if (runnable) {
        os_mutexLock(&runnable->mutex);
        result = runnable->terminate;
        os_mutexUnlock(&runnable->mutex);
    }
    
    return result;
}


void
nw_runnableTrigger(
    nw_runnable runnable)
{
    if (runnable && runnable->triggerFunc) {
        runnable->triggerFunc(runnable);
    }
}    


/* Public functions */
void
nw_runnableStart(
    nw_runnable runnable)
{
    os_threadAttr threadAttr;
    os_result result;
    
    if (runnable) {
        NW_TRACE_1(Mainloop, 1, "Starting thread %s", nw_runnableGetName(runnable));
        os_threadAttrInit(&threadAttr);
        if (!runnable->schedulingAttr.schedulingPrioUseDefault) {
            threadAttr.schedPriority = runnable->schedulingAttr.schedulingPrioValue;
        }
        threadAttr.schedClass = runnable->schedulingAttr.schedulingClass;
        
        result = os_threadCreate(&runnable->threadId,
                                 runnable->name,
                                 &threadAttr,
                                 nw_runnableDispatcher,
                                 runnable);
        if (result == os_resultSuccess) {
            NW_TRACE_1(Mainloop, 1, "Thread %s started", nw_runnableGetName(runnable));
        } else {
            NW_REPORT_ERROR_1("thread creation",
                "Creation of thread \"%s\" failed",
                nw_runnableGetName(runnable));
        }
    }
}
 
                
void
nw_runnableStop(
    nw_runnable runnable)
{
    if (runnable && (runnable->runState == rsRunning)) {
        NW_TRACE_1(Mainloop, 1, "Stopping thread %s", nw_runnableGetName(runnable));
        /* First stop the running thread */
        os_mutexLock(&runnable->mutex);
        runnable->terminate = TRUE;
        nw_runnableTrigger(runnable);        
        os_mutexUnlock(&runnable->mutex);

        /* Wait for termination of the thread */
        /* The thread mainfunc itself has to react on the
         * change of the status of runnable->terminate
         * by setting the runState to rs_Terminated. */
        os_threadWaitExit(runnable->threadId, NULL);
        NW_CONFIDENCE(runnable->runState != rsRunning);
        NW_TRACE_1(Mainloop, 1, "Thread %s stopped", nw_runnableGetName(runnable));
    }
}

void
nw_runnableFree(
    nw_runnable runnable)
{
    if (runnable) {
        NW_TRACE_1(Destruction, 3, "Freeing runnable %s", nw_runnableGetName(runnable));
        if (runnable->finalizeFunc) {
            runnable->finalizeFunc(runnable);
        }
        os_free(runnable);
    }
}    

