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
/** \file os/vxworks5.5/code/os_process.c
 *  \brief vxWorks 5.5 process management
 *
 * Implements process management for vxWorks
 * by including specific vxWorks os calls
 */

/** \file os/vxworks5.5/code/os_process.c
 *  \brief Posix process management
 *
 * Implements process management for vxWorks 5.5
 */

#include "os_semaphore.h"
#include "os_process.h"
#include "os_library.h"
#include "include/os_typebase.h"
#include <vxWorks.h>
#include <version.h>
#include "os_process.h"
#include "os__process.h"
#include "os_thread.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_iterator.h"
#include "os_init.h"
#include "os_stdlib.h"

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#if ! defined (OSPL_VXWORKS653)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sched.h>
#include "os_errno.h"
#include <signal.h>
#include <stdio.h>
#if ! defined (OSPL_VXWORKS653)
#include <elf.h>
#include <loadLib.h>
#endif
#include <symLib.h>
#include <sysSymTbl.h>
#include <ioLib.h>
#include <taskLib.h>
#include "os__tlsWrapper.h"
#include <taskHookLib.h>
#include <objLib.h>
#include "include/os_dynamicLib_plugin.h"

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
#define TASK_ID int
#endif

os_procContextData  procContextData = NULL;
static os_int32     os_procApplContextInit = -1;
os_int32            os_procHookInstalled = 0;

/** \brief Register an process Call back routine
 *
 * \b os_procRegisterCallback registers an process exit
 * handler by nserting \b  function in a list
 * to be called when the process exists.
 * The standard implementation guarantees the
 * required order of execution of the exit handlers.
 */

static void
os_procRegisterCallback(
    void (*function)(void))
{
    os_iterInsert(((os_procContextData)readTLSVarSelf(procContextData))->procCallbackList, (void *)function);
    return;
}

/** \brief return the program name
 *
 * \b os_procGetProcessName returns the name
 * given by os_procCreate
 */

os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    os_int32 size =0;
    os_procContextData currentProcContext = (os_procContextData)readTLSVarSelf(procContextData);
    if (currentProcContext->procName) {
        size = snprintf(procName, procNameSize, "%s", currentProcContext->procName);
    } else {
        size = snprintf(procName, procNameSize, "");
    }
    return size;
}

/** \brief Register a thread in the list
 *
 * \b os_procRegisterThread store the threadId
 * in the proc control block
 */

os_result
os_procRegisterThread(
    os_procContextData process_procContextData)
{
    os_result rv;

    if (os_iterInsert(process_procContextData->procThreadList, (void *)os_threadIdSelf()) != NULL) {
        rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
    return (rv);
}

/** \brief compare if two thread id's are the same
 *
 * \b threadIdcompare
 *
 */

static os_equality
threadIdcompare(
    os_threadContextData threadIdL,
    os_threadContextData threadIdR)
{
    if (threadIdL == threadIdR) {
        return OS_EQ;
    } else {
        return OS_NE;
    }
}

/** \brief UnRegister a thread in the list
 *
 * \b os_procUnregisterThread remove the threadId
 * from the proc control block
 */

os_result
os_procUnregisterThread()
{
    os_result rv = os_resultSuccess;
    os_threadSetValidity(os_threadIdSelf(), thread_ValExit);
    return (rv);
}

/** \brief Remove all thread in the list
 *
 * \b os_procRemoveThreads remove all threadId's
 * from the proc control block and free all memory
 */

os_result
os_procRemoveThreads(
    os_procContextData process_procContextData)
{
    os_result rv = os_resultSuccess;
    os_threadContextData list_threadId;

    list_threadId = os_iterTakeFirst(process_procContextData->procThreadList);
    while (list_threadId!= NULL){
        os_threadDispose(list_threadId);
        list_threadId = os_iterTakeFirst(process_procContextData->procThreadList);
    }
    return (rv);
}

/** \brief set validity in the process control block
 *
 * \b os_procSetValidity set validity
 */

static void
os_procSetValidity(
    os_procContextData process_procContextData,
    proc_Validity Validity)
{
    assert(process_procContextData != NULL);
    process_procContextData->procValidity = Validity;
}

/** \brief check validity in the process control block
 *
 * \b os_procCheckValid check validity
 */

static proc_Validity
os_procCheckValid(
    os_procContextData process_procContextData)
{
    assert(process_procContextData != NULL);
    /* check if all task are not dead */
    /* if no thread running anymore return invalid) */
    return (process_procContextData->procValidity);
}

static void
os_procSetTaskId(
    os_procContextData process_procContextData,
    os_int32 taskid)
{
    process_procContextData->procTaskId = taskid;
}

static void
os_procSetExitStatus(
    os_procContextData process_procContextData,
    os_int32 exitStatus)
{
    process_procContextData->procExitStatus = exitStatus;
}

static void
os_procSetExitValue(
    os_procContextData process_procContextData,
    os_int32 exitValue)
{
    process_procContextData->procExitValue = exitValue;
}

/** \brief Register an process exit handler
 *
 * \b os_procAtExit registers an process exit
 * handler by calling \b atexit passing the \b function
 * to be called when the process exists.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */
os_result
os_procAtExit(
    void (*function)(void))
{
    assert (function != NULL);
    os_procRegisterCallback((void *)function);
    return os_resultSuccess;
}

/** \brief Returns if threads are terminated by atexit
 *
 * Return values:
 * TRUE - if threads are ungracefully terminated by atexit
 * FALSE - all other situations
 */
os_boolean
os_procAreThreadsTerminatedByAtExit(
    void)
{
    return FALSE;
}

static void
os_procAwaitThreads()
{
    os_threadContextData threadContextData;
    os_procContextData currentProcContext = (os_procContextData)readTLSVarSelf(procContextData);
    threadContextData = (void *) os_iterTakeFirst(currentProcContext->procThreadList);
    while ((threadContextData != (void *)NULL) &&
           (threadContextData->threadValidity == thread_ValRunning)) {
        os_threadAwaitTermination(threadContextData);
#ifdef DEL
        while (taskIdVerify(threadContextData->threadTaskId) == OK) {
             taskDelay (10); /* To do trigger */
        }
#endif
        threadContextData = (void *) os_iterTakeFirst (currentProcContext->procThreadList);
    }
}

static void
os_procCallbacks(
    void)
{
    void (*function)(void);
    os_procContextData currentProcContext = (os_procContextData)readTLSVarSelf(procContextData);

    function = (void *) os_iterTakeFirst(currentProcContext->procCallbackList);
    while (function != (void *)NULL) {
        function();
        function = (void *) os_iterTakeFirst(currentProcContext->procCallbackList);
    }

}

static void
os_procLocalExit (
    os_exitStatus status)
{
    os_procContextData currentProcContext;
    assert (status != OS_EXIT_SUCCESS || status != OS_EXIT_FAILURE);
    currentProcContext = (os_procContextData)readTLSVarSelf(procContextData);
    if (semTake(currentProcContext->procDataSem, WAIT_FOREVER) == ERROR) {
        OS_REPORT (OS_WARNING, "os_procExit", 1,
                     "os_procExit : semTake failed with error %d (%s)",
                      os_getErrno(), currentProcContext->procName);
    } else {
       if (currentProcContext->procValidity == proc_ValRunning) {
           os_procSetValidity(currentProcContext, proc_ValInExit); /* for os_exitStatus */
           semGive(currentProcContext->procDataSem);
           os_procCallbacks();
           os_procUnregisterThread();
           os_procAwaitThreads();
           /* os_procCallbacks(); */
           os_procRemoveThreads(currentProcContext);
           os_procSetExitStatus(currentProcContext, status);
           os_procSetValidity(currentProcContext, proc_ValExit);
       } else {
           /* an other task in os_procExit */
           os_procUnregisterThread();
           semGive(currentProcContext->procDataSem);
       }
    }
    return;
}

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
void
os_procExit(
    os_exitStatus status)
{
    os_procContextData currentProcContext = (os_procContextData)readTLSVarSelf(procContextData);
    os_procLocalExit(status);
    os_threadDeleteTaskVar(taskIdSelf(), os_threadIdSelf());
    os_procDeleteTaskVar(taskIdSelf(), "task", currentProcContext);
    exit(status);
}



static void
os_procAddGlobalVar(
    os_procGlobalVar procGlobalVar,
    os_int32 taskid)
{
        if (addTLSVar(taskid, (int *)procGlobalVar->procVarAddr) != OK) {
            OS_REPORT(OS_WARNING, "os_procAddTaskVar", 1,
                        "taskVarAdd failed with error %d (%s, %s)",
                        os_getErrno(),  ((os_procContextData)readTLSVarSelf(procContextData))->procName);
        }
        if (setTLSVar(taskid, (int *)procGlobalVar->procVarAddr,
                       (int)procGlobalVar->procVarDataAddr) != OK ) {
            OS_REPORT(OS_WARNING, "os_procAddTaskVar", 1,
                        "taskVarSet failed with error %d (%s, %s)",
                        os_getErrno(), ((os_procContextData)readTLSVarSelf(procContextData))->procName);
        }
}


os_result
os_procAddTaskVar(
    os_int32 taskid,
    char *executable_file,
    os_procContextData process_procContextData,
    os_int32 isTask)
{
    os_int32 status = os_resultSuccess;

    /* create & set context variable for the task */
    if (addTLSVar(taskid, (int *)&procContextData) != OK) {
        OS_REPORT(OS_WARNING, "os_procAddTaskVar", 1,
                    "taskVarAdd failed with error %d (%s, %s)",
                    os_getErrno(), executable_file, process_procContextData->procName);
        status = os_resultInvalid;
        os_procSetExitStatus(process_procContextData, -1);
    }
    if( setTLSVar(taskid, (int *)&procContextData, (int)process_procContextData) != OK ){
        OS_REPORT(OS_WARNING, "os_procAddTaskVar", 1,
                    "taskVarSet failed with error %d (%s, %s)",
                    os_getErrno(), executable_file, process_procContextData->procName);
        status = os_resultInvalid;
        os_procSetExitStatus(process_procContextData, -1);
    }
    if (isTask == 1) {
        os_iterWalk(process_procContextData->procGlobalVarList,os_procAddGlobalVar, taskid);
    }
    return(status);
}

os_result
os_procDeleteTaskVar(
    os_int32 taskid,
    char *executable_file,
    os_procContextData process_procContextData)
{
    os_int32 status = os_resultSuccess;
    os_procGlobalVar procGlobalVar;

    /* delete context variable for the task */
    procGlobalVar = os_iterTakeFirst(process_procContextData->procGlobalVarList);
    while (procGlobalVar!= NULL){
      if (deleteTLSVar(taskid, (int *)procGlobalVar->procVarAddr) != OK) {
            OS_REPORT(OS_WARNING, "os_procDeleteTaskVar", 1,
                        "os_procDeleteTaskVar user failed with error %d (%s, %s)",
                        os_getErrno(), executable_file, process_procContextData->procName);
            status = os_resultInvalid;
            os_procSetExitStatus(process_procContextData, -1);
        }
        os_free(procGlobalVar);
        procGlobalVar = os_iterTakeFirst(process_procContextData->procGlobalVarList);
    }
    if (deleteTLSVar(taskid, (int *)&procContextData) != OK) {
        OS_REPORT(OS_WARNING, "os_procDeleteTaskVar", 1,
                    "os_procDeleteTaskVar failed with error %d (%s, %s)",
                    os_getErrno(), executable_file, process_procContextData->procName);
        status = os_resultInvalid;
        os_procSetExitStatus(process_procContextData, -1);
    }
    return(status);
}

os_result
os_procDeleteThreadTaskVar(
    os_int32 taskid,
    char *executable_file,
    os_procContextData process_procContextData)
{
    os_int32 status = os_resultSuccess;
    os_procGlobalVar procGlobalVar;
    os_int32 index=0;

    /* delete context variable for the task */
    procGlobalVar = os_iterObject(process_procContextData->procGlobalVarList, index);
    index++;
    while (procGlobalVar!= NULL){

      if (deleteTLSVar(taskid, (int *)procGlobalVar->procVarAddr) != OK) {
            OS_REPORT(OS_WARNING, "os_procDeleteTaskVar", 1,
                        "os_procDeleteTaskVar user failed with error %d (%s, %s)",
                        os_getErrno(), executable_file, process_procContextData->procName);
            status = os_resultInvalid;
            os_procSetExitStatus(process_procContextData, -1);
        }
        procGlobalVar = os_iterObject(process_procContextData->procGlobalVarList, index);
        index++;
    }
    if (deleteTLSVar(taskid, (int *)&procContextData) != OK) {
        OS_REPORT(OS_WARNING, "os_procDeleteTaskVar", 1,
                    "os_procDeleteTaskVar failed with error %d (%s, %s)",
                    os_getErrno(), executable_file, process_procContextData->procName);
        status = os_resultInvalid;
        os_procSetExitStatus(process_procContextData, -1);
    }
    return(status);
}


static os_result
os_procWrapper(
    os_procContextData process_procContextData,
    char *executable_file,
    os_int32 *startRoutine,
    const char *arguments,
    TASK_ID parent,
    os_sem_t *blockParent)
{
    int taskid;
    os_int32 status = os_resultSuccess;
    os_int32 routine_status = -1;
    os_int32 (*this_startRoutine)(const char *);

#if ( _WRS_VXWORKS_MAJOR > 6 ) || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR > 8 )
      envPrivateCreate(taskIdSelf(), parent);
      os_sem_post(blockParent);
#endif

    taskid = taskIdSelf();
    os_procSetTaskId(process_procContextData, taskid);
    /* create & set context variable for the task */
    status = os_procAddTaskVar(taskid, executable_file, process_procContextData, 0);
    if (status == os_resultSuccess) {
        status = os_threadNew(process_procContextData->procName);
    }
    if (status == os_resultSuccess) {
        this_startRoutine = (os_int32 *)startRoutine;
        routine_status = this_startRoutine(arguments);
        os_procLocalExit(OS_EXIT_SUCCESS);
        os_threadDeleteTaskVar(taskid, os_threadIdSelf());
        os_procDeleteTaskVar(taskid, executable_file, process_procContextData);
        os_procSetExitValue(process_procContextData, routine_status);
    }

#if ( _WRS_VXWORKS_MAJOR > 6 ) || ( _WRS_VXWORKS_MAJOR > 6 && _WRS_VXWORKS_MINOR > 8 )
      envPrivateDestroy(taskIdSelf());
#endif
    return (status);
}

void
os_procInit(
    os_procContextData process_procContextData,
    const char *name,
    const char *executable_file,
    const char *arguments)
{
    bzero((char *)process_procContextData, (size_t)OS_SIZEOF(os_procContextData));
    process_procContextData->procName = (char *)os_malloc(strlen(name) + 1);
    os_strncpy(process_procContextData->procName, name, strlen(name) + 1);
    os_procSetExitStatus(process_procContextData, -1);
    process_procContextData->procId = (os_int32)process_procContextData;
    os_procSetValidity(process_procContextData, proc_ValRunning);
    process_procContextData->procCallbackList = os_iterNew(NULL);
    process_procContextData->procThreadList = os_iterNew(NULL);
    process_procContextData->procDataSem = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE);
    process_procContextData->procGlobalVarList = os_iterNew(NULL);
    process_procContextData->arguments = (char *)os_malloc(strlen(arguments) + 1);
    os_strncpy(process_procContextData->arguments, arguments, strlen(arguments) + 1);
    process_procContextData->executable = (char *)os_malloc(strlen(executable_file) + 1);
    os_strncpy(process_procContextData->executable, executable_file, strlen(executable_file) + 1);
}


/** \brief Create a process that is an instantiation of a program
 *
 * First an argument list is build from \b arguments.
 * Then \b os_procCreate creates a process by forking the current
 * process.
 *
 * The child process processes the lock policy attribute from
 * \b procAttr and sets the lock policy accordingly by calling
 * \b mlockall if required. If the process has root privileges
 * it processes the user credentials from \b procAttr and sets
 * the user credentials of the child process accordingly.
 * The child process then replaces the running program with the
 * program provided by the \b executable_file by calling \b execve.
 *
 * The parent process processes the scheduling class and
 * scheduling priority attributes from \b procAttr and
 * sets the scheduling properties of the child process
 * accordingly by calling \b sched_setscheduler.
 */

os_result
os_procCreate(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr *procAttr,
    os_procId *procId)
{
    int procTaskId;
    int sched_policy;
    os_result rv = os_resultSuccess;
    os_procContextData process_procContextData;
    os_int32 startRoutine = 0;
    os_int32 pOptions,privateSet;
    os_int32 len,i=0,n=0;
    char *converted = NULL;

    assert(executable_file != NULL);
    assert(name != NULL);
    assert(arguments != NULL);
    assert(procAttr != NULL);
    assert(procId != NULL);

    len = strlen(arguments);
    converted = (char*)os_malloc(len+1);
    for (; i < len ; i++)
    {
       if (arguments[i] != '\"')
       {
          converted[n] = arguments[i];
          n++;
       }
    }
    converted[n] = '\0';
    pOptions = 0;
    privateSet = 0;
    taskOptionsGet(taskIdSelf(),&pOptions);
    if ((pOptions & VX_PRIVATE_ENV) == 0)
    {
        envPrivateCreate(taskIdSelf(), 0);
        privateSet = 1;
    }
    putenv("SPLICE_NEW_PROCESS=no");

    if (procAttr->schedClass == OS_SCHED_REALTIME) {
        sched_policy = SCHED_FIFO;
    } else if (procAttr->schedClass == OS_SCHED_TIMESHARE) {
        return os_resultInvalid;
    } else if (procAttr->schedClass == OS_SCHED_DEFAULT) {
        sched_policy = SCHED_OTHER;
    } else {
        return os_resultInvalid;
    }

    if ((procAttr->schedPriority > VXWORKS_PRIORITY_MIN) ||
        (procAttr->schedPriority < VXWORKS_PRIORITY_MAX) ) {
        return os_resultInvalid;
    }


    {
      os_library binlib;
      os_libraryAttr attr;

      os_libraryAttrInit(&attr);
      /* Dynamic load of services is a special case, so try just static via
         os_libraryOpen */
      attr.staticLibOnly=1;
      binlib = os_libraryOpen( executable_file, &attr);
      /* FIXME existing use of os_int32 for pointer is CRAZY!! */
      if ( binlib != NULL )
      {
         startRoutine = (os_int32)os_libraryGetSymbol( binlib, executable_file );
      }

      if ( startRoutine == 0 && os_dynamicLibPlugin != NULL )
      {
         startRoutine = (os_uint32)os_dynamicLibPlugin->dlp_loadLib( executable_file );
      }

      if ( startRoutine == 0 )
      {
         OS_REPORT(OS_ERROR, "os_procCreate", 1,
                       "Unable to load %s (%s)",
                       executable_file, name);
     rv = os_resultInvalid;
      }
    }

    if (rv == os_resultSuccess)
    {
        process_procContextData = (os_procContextData )os_malloc((size_t)OS_SIZEOF(os_procContextData));
        if (process_procContextData == (os_procContextData) NULL) {
            OS_REPORT(OS_WARNING, "os_procCreate", 1,
                        "malloc failed with error %d (%s, %s)",
                        os_getErrno(), executable_file, name);
            rv = os_resultInvalid;
        }
        else
        {
#if defined ( _WRS_VXWORKS_MAJOR ) && ( _WRS_VXWORKS_MAJOR > 6 ) || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR > 8 )
        /* the blockParent semaphore is used to prevent this task exiting before the spawned task
           has completed copying its environ. */
        os_sem_t blockParent;
        os_sem_init( &blockParent, 0);
#endif
            os_procInit(process_procContextData, name, executable_file, converted);
            process_procContextData->procAttrPrio = procAttr->schedPriority;
            procTaskId = taskSpawn((char *)name, procAttr->schedPriority,
                                   VX_FP_TASK
#if ! defined ( _WRS_VXWORKS_MAJOR ) || _WRS_VXWORKS_MAJOR < 6 || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 9 )
/* VX_PRIVATE_ENV flag is no longer functional in vxworks 6.9 */
                   | VX_PRIVATE_ENV
#endif
#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
                                   | VX_STDIO | VX_DEALLOC_STACK
#endif
                                   ,
                                   VXWORKS_PROC_DEFAULT_STACK,
                                   (FUNCPTR)os_procWrapper,
                                   (int)process_procContextData, (int)process_procContextData->executable,
                                   (int)startRoutine, (int)process_procContextData->arguments, taskIdSelf(),
#if ! defined ( _WRS_VXWORKS_MAJOR ) || _WRS_VXWORKS_MAJOR < 6 || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 9 )
                                   0,
#else
                                   &blockParent,
#endif
 0, 0, 0, 0);
#if defined ( _WRS_VXWORKS_MAJOR ) && ( _WRS_VXWORKS_MAJOR > 6 || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR > 8 ) )
            os_sem_wait(&blockParent);
            os_sem_destroy(&blockParent);
#endif
            if (procTaskId == ERROR)
            {
                os_free(process_procContextData);
                rv = os_resultInvalid;
            }
            else
            {
                *procId = *(os_procId *)&process_procContextData;
                rv = os_resultSuccess;
            }
        }
    }

    putenv("SPLICE_NEW_PROCESS=empty");
    if ( privateSet == 1)
    {
        envPrivateDestroy(taskIdSelf());
    }
    os_free(converted);
    return rv;
}

/* Initialize for programs not started with os_procCreate*/
void
os_procInitialize()
{
    os_procContextData process_procContextData;
    os_int32 status = os_resultSuccess;
    os_int32 taskid;

    if ( readTLSVarSelf(procContextData) == NULL) {
        taskid = taskIdSelf();
        process_procContextData = (os_procContextData )os_malloc((size_t)OS_SIZEOF(os_procContextData));
        if (process_procContextData == (os_procContextData) NULL) {
            OS_REPORT(OS_WARNING, "os_procCreate", 1,
                        "malloc failed with error %d (%s)",
                        os_getErrno(), taskName(taskIdSelf()));
        } else {
            os_procInit(process_procContextData, taskName((int)taskid), "program" , "none");
            os_procSetTaskId(process_procContextData, (int)taskid);
            process_procContextData->procAttrPrio = VXWORKS_PRIORITY_DEFAULT;
            /* create & set context variable for the task */
            status = os_procAddTaskVar((int)taskid, "none", process_procContextData, 0);
            if (status == os_resultSuccess) {
                status = os_threadNew(process_procContextData->procName);
            } else {
                printf("os_procApplAddContext os_threadNew ERROR\n");
            }
            process_procContextData->procStartWithProcCreate = 1; /* not started with procCreate*/
       }
    }

}

void
os_procDispose(
    os_procId procId)
{
    os_procContextData process_procContextData;

    process_procContextData = (os_procContextData)procId;
    os_iterFree(process_procContextData->procCallbackList);
    os_iterFree(process_procContextData->procThreadList);
    os_iterFree(process_procContextData->procGlobalVarList);
    semDelete(process_procContextData->procDataSem);
    os_free((char *)process_procContextData->procName);
    os_free((char *)process_procContextData);
    os_free((char *)process_procContextData->arguments);
    os_free((char *)process_procContextData->executable);
}

/** \brief Check the child exit status of the identified process
 *
 * \b os_procCheckStatus calls \b waitpid with flag \b WNOHANG
 * to check the status of the child process.
 * - On return of \b waitpid with result \b procId, the process
 *   has terminated and its result status is in \b status.
 * - On return of \b waitpid with result 0, the child process is
 *   not yet terminated and \b os_resultBusy is returned.
 * - On return of \b waitpid with result -1 and \b errno is \b ECHILD
 *   the identified child process is not found and \b
 *   os_resultUnavailable is returned.
 * - On any other return from \b waitpid, \b os_resultFail is returned.
 */
os_result
os_procCheckStatus(
    os_procId procId,
    os_int32 *status)
{
    os_result rv;
    proc_Validity checkStatus;
    os_procContextData process_procContextData;

    if (OS_INVALID_PID == procId) {
        return os_resultInvalid;
    }

    process_procContextData = (os_procContextData)procId;
    if (process_procContextData == NULL)
    {
        rv = os_resultUnavailable;
    }
    else
    {
        if (semTake(process_procContextData->procDataSem, NO_WAIT) == ERROR)
        {
            if (os_getErrno() == S_objLib_OBJ_UNAVAILABLE)
            {
                rv = os_resultBusy;
            }
            else
            {
                rv = os_resultUnavailable;
            }
        }
        else
        {
            checkStatus = os_procCheckValid(process_procContextData);
            if (checkStatus == proc_ValExit)
            {
                *status = process_procContextData->procExitStatus;
                os_procSetValidity(process_procContextData, proc_ValInvalid);
                os_procDispose(process_procContextData);
                rv = os_resultSuccess;
            }
            else
            {
                if ((checkStatus == proc_ValRunning) || (checkStatus == proc_ValInExit))
                {
                    rv = os_resultBusy;
                }
                else
                {
                    rv = os_resultInvalid;
                }
                semGive(process_procContextData->procDataSem);
            }
        }
    }
    return rv;
}


/** \brief Return the process ID of the calling process
 *
 * Possible Results:
 * - returns the process ID of the calling process
 */
os_procId
os_procIdSelf(void)
{
  return (os_procId)(os_procContextData)readTLSVarSelf(procContextData);
}

/** \brief Figure out the identity of the current process
 *
 * \b os_procFigureIdentity determines the numeric identity
 * of a process. On UNIX the process name can only be determined
 * by access to argv[0]. This is not the case for this implementation.
 * If however started via \b os_procCreate the name of the process
 * can be determined via the environment variable \b SPLICE_PROCNAME
 */
os_int32
os_procFigureIdentity(
    char *procIdentity,
    os_uint procIdentitySize)
{
    os_int32 size = 0;
    char proc_name[512];
    size = os_procGetProcessName(proc_name,512);
    if (procIdentity != (char *) NULL && proc_name != (char *) NULL) {
        size = snprintf(procIdentity, procIdentitySize, "%s <%d %d>",
                        proc_name, (int) readTLSVarSelf(procContextData), (int) taskIdSelf());
    } else {
        size = snprintf(procIdentity, procIdentitySize, "<%d %d>",
                        (int)readTLSVarSelf(procContextData), (int) taskIdSelf());
    }
    return size;
}

/** \brief Send a signal to the identified process
 *
 * \b os_procDestroy sends the dignal \b signal
 * to process \b procId by calling \b kill with
 * the same parameters.
 *
 * - If \b kill returns a value != -1, \b os_resultSuccess is
 *   returned.
 * - If \b kill returns with -1 and \b errno is set to \b EINVAL,
 *   \b os_resultInvalid is returned to indicate an invalid
 *   parameter.
 * - If \b kill returns with -1 and \b errno is set to \b ESRCH,
 *   \b os_resultUnavailable is returned to indicate that
 *   the identified process is not found.
 * - On any other return from \b kill, \b os_resultFail is returned.
 */
os_result
os_procDestroy(
    os_procId procId,
    os_int32 signal)
{
    os_result rv;
    os_procContextData process_procContextData;

    if (OS_INVALID_PID == procId) {
        return os_resultInvalid;
    }

    process_procContextData = (os_procContextData)procId;
    if (kill(process_procContextData->procTaskId, signal) == -1)
    {
        if (os_getErrno() == EINVAL)
        {
            rv = os_resultInvalid;
        }
        else if (os_getErrno() == ESRCH)
        {
            rv = os_resultUnavailable;
        }
        else
        {
            rv = os_resultFail;
        }
    }
    else
    {
        rv = os_resultSuccess;
    }
    return rv;
}

/** \brief Initialize process attributes
 *
 * Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 * (take the platforms default scheduling class, Time-sharing for
 * non realtime platforms, Real-time for realtime platforms)
 * Set \b procAttr->schedPriority to \b 0
 * Set \b procAttr->lockPolicy to \b OS_LOCK_DEFAULT
 * (no locking on non realtime platforms, locking on
 * realtime platforms)
 * Set \b procAttr->userCred.uid to 0
 * (don't change the uid of the process)
 * Set \b procAttr->userCred.gid to 0
 * (don't change the gid of the process)
 */
void
os_procAttrInit(
    os_procAttr *procAttr)
{
    assert(procAttr != NULL);
    procAttr->schedClass = OS_SCHED_DEFAULT;
    procAttr->schedPriority = VXWORKS_PRIORITY_DEFAULT;
    procAttr->lockPolicy = OS_LOCK_DEFAULT;
    procAttr->userCred.uid = 0;
    procAttr->userCred.gid = 0;
    procAttr->activeRedirect = 0;
}

static void
os_procAddGlobalVarToTasks(
    os_threadContextData this_threadContextData,
    os_procGlobalVar GlobalVar)
{
    os_procAddGlobalVar(GlobalVar, this_threadContextData->threadTaskId);
}

/** \brief Make a global variable local for the program
 *
 * Global variable are meant to be global for the
 * program, in some operating system all variables
 * are global so this function will make it possible
 * to support global variables per program. the varaible
 * must be a pointer and the memory is allocated with size
 * given with the paramter size
 * Postcondition:
 * - process may only call this routine once per global var
 *
 * Possible Results:
 * - os_resultFail and user = NULL if there is no memory avail
 * - returns os_resultSuccess
 */

/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT if
 *   the class effective could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
    return OS_SCHED_REALTIME;
}

/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform and scheduling class dependent valid priority
 */
os_int32
os_procAttrGetPriority(void)
{
    return ((os_procContextData)readTLSVarSelf(procContextData))->procAttrPrio;
}


/** \brief process Hook routine
 *
 */

void
os_procDDSHook(
    WIND_TCB *pNewTcb)
{

    os_procContextData process_procContextData;
    os_int32 status = os_resultSuccess;
    os_int32 taskid;
    os_int32 init_process = 0;

    if (getenv("SPLICE_NEW_PROCESS") != NULL) {
        if (strncmp(getenv("SPLICE_NEW_PROCESS"),"no",2) == 0) {
            init_process = 1;
        }
    }

    if (init_process == 1)  {
        /* task Started with os_proCreate  */
    } else {
        if (readTLSVarSelf(procContextData) == NULL) { /* main thread */
            taskid = (int)pNewTcb;
            process_procContextData = (os_procContextData )os_malloc((size_t)OS_SIZEOF(os_procContextData));
            if (process_procContextData == (os_procContextData) NULL) {
                OS_REPORT(OS_WARNING, "os_procCreate", 1,
                            "malloc failed with error %d (%s)",
                            os_getErrno(), taskName(taskid));
            } else {
                os_procInit(process_procContextData, taskName((int)taskid), "program" , "none");
                os_procSetTaskId(process_procContextData, (int)taskid);
                process_procContextData->procAttrPrio = VXWORKS_PRIORITY_DEFAULT;
                /* create & set context variable for the task */
                status = os_procAddTaskVar((int)taskid, "none", process_procContextData, 0);
                if (status == os_resultSuccess) {
                    status = os_hookthreadNew(process_procContextData->procName, process_procContextData, taskid);
                } else {
                    printf("os_procApplAddContext os_threadNew ERROR\n");
                }
                /*
                process_procContextData->procStartWithProcCreate = 1; not started with procCreate*/
           }
        } else { /* extra threads */
            os_threadHookThreadInit(readTLSVarSelf(procContextData), pNewTcb);
        }
    }

}

/** \brief installs an process Hook routine
 *
 */

os_int32
os_procInstallHook()
{
    os_int32 result = 0;

    if (os_procHookInstalled == 0) {
        os_procHookInstalled = 1;
        result = taskCreateHookAdd(os_procDDSHook);
    }

    return (result);
}

/** \brief Register a thread in the list
 *
 * \b os_procRegisterThread store the threadId
 * in the proc control block
 */

os_result
os_procRegisterHookThread(
    os_procContextData process_procContextData,
    os_threadContextData process_threadContextData)
{
    os_result rv;

    if (os_iterInsert(process_procContextData->procThreadList, (void *)process_threadContextData) != NULL) {
        rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
    return (rv);
}

void
os_procListTask(
    os_threadContextData this_threadContextData,
    os_int32 tid)
{
    printf("  %x %s \n",this_threadContextData->threadTaskId,taskName(this_threadContextData->threadTaskId));
}

void
os_listTask(
    os_int32 tid)
{
    os_procContextData process_procContextData;
    printf("----------------------------------------\n");
    process_procContextData = (os_procContextData)readTLSVar(tid, procContextData);
    if (process_procContextData != -1) {
        os_iterWalk(process_procContextData->procThreadList,os_procListTask,tid);
    }
    printf("----------------------------------------\n");
}

os_result
os_procMLockAll(
    os_uint flags)
{
    OS_REPORT(OS_ERROR, "os_procMLockAll", 0, "Page locking not support on VxWorks 5.5");
    return os_resultFail;
}

os_result
os_procMLock(
    const void *addr,
    os_address length)
{
    OS_REPORT(OS_ERROR, "os_procMLock", 0, "Page locking not support on VxWorks 5.5");
    return os_resultFail;
}

os_result
os_procMUnlock(
    const void *addr,
    os_address length)
{
    OS_REPORT(OS_ERROR, "os_procMUnlock", 0, "Page locking not support on VxWorks 5.5");
    return os_resultFail;
}

os_result
os_procMUnlockAll(void)
{
    OS_REPORT(OS_ERROR, "os_procMUnlockAll", 0, "Page locking not support on VxWorks 5.5");
    return os_resultFail;
}

