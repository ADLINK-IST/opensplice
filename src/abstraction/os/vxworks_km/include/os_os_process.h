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
#ifndef OS_VXWORKS_PROCESS_H
#define OS_VXWORKS_PROCESS_H

#include "os_typebase.h"
#include <vxWorks.h>

#include <sys/types.h>
#include <unistd.h>
#if ! defined (OSPL_VXWORKS653)
#include "vx_unistd.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <semLib.h>
#include "os_classbase.h"
#include <os_iterator_type.h>


#if defined (__cplusplus)
extern "C" {
#endif

typedef int os_os_procId;

#define VALIDITY_RUNNING (0x70726f63) /* running */
#define VALIDITY_EXIT    (0x65786974) /* exit    */
typedef enum {
    proc_ValInvalid = 0,
    proc_ValInit    = 1,
    proc_ValInExit  = 2,
    proc_ValRunning = VALIDITY_RUNNING,
    proc_ValExit    = VALIDITY_EXIT
} proc_Validity;

#define os_procGlobalVar(o)         ((os_procGlobalVar)o)
OS_CLASS(os_procGlobalVar);
OS_STRUCT(os_procGlobalVar) {
    os_os_uint32        procVarAddr;
    os_os_uint32        procVarDataAddr;
};

#define os_procContextData(o)         ((os_procContextData)o)
OS_CLASS(os_procContextData);
OS_STRUCT(os_procContextData) {
    proc_Validity  procValidity;
    os_os_int32       procAttrPrio;
    char           *procName;
    os_os_int32       procId;
    os_os_int32       procTaskId;
    os_os_int32       procExitStatus;
    os_os_int32       procExitValue;
    os_iter        procCallbackList;
    os_iter        procThreadList;
    SEM_ID         procDataSem;
    os_iter        procGlobalVarList;
    char           *arguments;
    char           *executable;
    int            procStartWithProcCreate; /* States if created via os_procCreate or not */
};

extern os_procContextData  procContextData;

#define VXWORKS_PRIORITY_MIN (255)
#define VXWORKS_PRIORITY_MAX (0)
#define VXWORKS_PRIORITY_DEFAULT (150)
#define VXWORKS_PROC_DEFAULT_STACK (60*1024)


os_result os_procUnregisterThread(void);

os_result
os_procRemoveThreads(
    os_procContextData process_procContextData);

os_result
os_procAddTaskVar(
    os_os_int32 taskid,
    char *executable_file, 
    os_procContextData process_procContextData, 
    os_os_int32 isTask);

os_result
os_procDeleteTaskVar(
    os_os_int32 taskid, 
    char *executable_file, 
    os_procContextData process_procContextData);

os_result
os_procDeleteThreadTaskVar(
    os_os_int32 taskid, 
    char *executable_file, 
    os_procContextData process_procContextData);

void
os_procInit(
    os_procContextData process_procContextData, 
    const char *name, 
    const char *executable_file, 
    const char *arguments);

void os_procInitialize(void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_VXWORKS_PROCESS_H */
