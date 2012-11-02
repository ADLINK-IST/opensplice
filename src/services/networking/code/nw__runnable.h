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

#ifndef NW__RUNNABLE_H
#define NW__RUNNABLE_H

#include "nw_runnable.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_thread.h"
#include "os_time.h"
#include "c_typebase.h"

typedef enum nw_runState_e {
    rsNone,
    rsRunning,
    rsTerminated
} nw_runState;

typedef void * (*nw_runnableMainFunc)(nw_runnable runnable, c_voidp arg);
typedef void   (*nw_runnableTriggerFunc)(nw_runnable runnable);
typedef void   (*nw_runnableFinalizeFunc)(nw_runnable runnable);


typedef struct nw_schedulingAttr_s {
    nw_bool       schedulingPrioUseDefault;
    os_int32      schedulingPrioValue; /* Only valid if UseDefault = FALSE */
    os_schedClass schedulingClass;
} *nw_schedulingAttr;

NW_STRUCT(nw_runnable) {
    char *name;
    os_threadId threadId;
    c_bool terminate;
    os_mutex mutex;
    nw_runState runState;
    /* VMT */
    nw_runnableMainFunc mainFunc;
    c_voidp mainFuncArg;
    nw_runnableTriggerFunc triggerFunc;
    nw_runnableFinalizeFunc finalizeFunc;
    /* Scheduling attributes */
    struct nw_schedulingAttr_s schedulingAttr;
};

void        nw_runnableInitialize(
                nw_runnable runnable,
                const char *name,
                const char *pathName,
                const nw_runnableMainFunc runnableMainFunc,
                const c_voidp mainFuncArg,
                const nw_runnableTriggerFunc triggerFunc,
                const nw_runnableFinalizeFunc finalizeFunc);
                
void        nw_runnableFinalize(
                nw_runnable runnable);

const char *nw_runnableGetName(
                nw_runnable runnable);
                
void        nw_runnableSetRunState(
                nw_runnable runnable,
                nw_runState runState);
                
c_bool      nw_runnableTerminationRequested(
                nw_runnable runnable);                

void        nw_runnableTrigger(
                nw_runnable runnable);
                
#endif /* NW__RUNNABLE_H */

