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
/** \file os/vxworks5.5/code/os_thread.c
 *  \brief vxWorks 5.5 thread management
 *
 * Implements thread management for vxWorks
 * by including the specific vxWorks routines
 */

#include "os_thread.h"
#include "os_process.h"
#include "os__process.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_stdlib.h"

#include <version.h>
#include <sys/types.h>
#include <assert.h>
#include "os_errno.h"
#include <unistd.h>
#include <stdio.h>
#include <taskLib.h>
#include "os__tlsWrapper.h"
#include <envLib.h>
#if ! defined (OSPL_VXWORKS653)
#include <sysLib.h>
#endif
#include <pthread.h>
#include <logLib.h>

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
#define TASK_ID int
#endif

/* PR_SET_NAME doesn't seem to be available on VxWorks */
#define OS_HAS_NO_SET_NAME_PRCTL

typedef struct {
    sigset_t oldMask;
    os_uint  protectCount;
} os_threadProtectInfo;

typedef struct {
    void *pthreadMem;
    os_threadPrivMemDestructor destructor;
    void *userArgs;
} os_threadRegisteredPrivMem;

os_threadContextData threadContextData = NULL;
os_threadContextData id_none = {0, };
static os_threadHook os_threadCBs;
static sigset_t os_threadBlockAllMask;

static int
os_threadStartCallback(
    os_threadId id,
    void *arg)
{
    return 0;
}

static int
os_threadStopCallback(
    os_threadId id,
    void *arg)
{
    return 0;
}

/** \brief Return the integer representation of the given thread ID
 *
 * Possible Results:
 * - returns the integer representation of the given thread ID
 */
os_ulong_int
os_threadIdToInteger(os_threadId id)
{
   return (os_ulong_int) id;
}

static void
os_threadHookInit(void)
{
    os_threadCBs.startCb = os_threadStartCallback;
    os_threadCBs.startArg = NULL;
    os_threadCBs.stopCb = os_threadStopCallback;
    os_threadCBs.stopArg = NULL;
}

static void
os_threadHookExit(void)
{
    return;
}

/** \brief Set task Id in the Context data struct
 *
 * \b os_procSetThreadId store thread Task Id
 * handler by calling \b atexit passing the \b function
 * to be called when the process exists.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */
static void
os_procSetThreadId(
    os_threadContextData process_threadContextData,
    os_int32 threadTaskId)
{
    assert (process_threadContextData != NULL);
    process_threadContextData->threadTaskId = threadTaskId;
}


void
os_threadSetValidity (
    os_threadContextData process_threadContextData,
    thread_Validity Validity)
{
    assert (process_threadContextData != NULL);
    process_threadContextData->threadValidity = Validity;
}

static void
os_threadSetExitStatus (
    os_threadContextData process_threadContextData,
    void *exitStatus)
{
    assert (process_threadContextData != NULL);
    process_threadContextData->threadExitStatus = exitStatus;
}


os_result
os_threadNew(
    const char *name)
{
    os_threadContextData process_threadContextData = NULL;
    os_int32 status = os_resultSuccess;
    os_int32 taskid;

    taskid = taskIdSelf ();
    process_threadContextData = os_threadInit (name);   /* main is also a thread */
    os_procSetThreadId (process_threadContextData, taskid);
    os_threadAddTaskVar (taskid, process_threadContextData);
    os_threadSetValidity (process_threadContextData, thread_ValRunning);
    status = os_procRegisterThread (readTLSVarSelf(procContextData));
    return (status);
}

char *
os_threadGetName()
{
   os_threadContextData currentThreadContextData = (os_threadContextData)readTLSVarSelf(threadContextData);
   return( currentThreadContextData ? currentThreadContextData->threadName : NULL );
}

os_int32
os_threadGetThreadName (
    os_char *buffer,
    os_uint32 length)
{
    os_char *name;

    assert (buffer != NULL);

    if ((name = os_threadGetName()) == NULL) {
        name = "";
    }

    return snprintf (buffer, length, "%s", name);
}

static thread_Validity
os_threadCheckValid (
    os_threadContextData process_threadContextData)
{
    assert (process_threadContextData != NULL);
    return (process_threadContextData->threadValidity);
}

void *
os_threadCheckExitValue (
    os_threadContextData process_threadContextData)
{
    return (process_threadContextData->threadExitStatus);
}


void
os_threadDispose(
    os_threadContextData process_threadContextData)
{
    os_free (process_threadContextData->threadName);
    semDelete (process_threadContextData->threadExitTrigger);
    semDelete (process_threadContextData->threadDataSem);
    os_free (process_threadContextData);
}

os_result
os_threadAwaitTermination(
    os_threadContextData process_threadContextData)
{
    os_result rv;

    if (taskIdVerify (process_threadContextData->threadTaskId) == OK) {
        if (semTake (process_threadContextData->threadExitTrigger, WAIT_FOREVER) == OK) {
            rv = os_resultSuccess;
        } else {
            rv = os_resultUnavailable;
        }
    } else {
        rv = os_resultFail;
    }
    return (rv);
}


/** \brief Initialize the thread private memory array
 *
 * \b os_threadMemInit initializes the thread private memory array
 *    for the calling thread
 *
 * pre condition:
 *    os_threadMemKey is initialized
 *
 * post condition:
 *       pthreadMemArray is initialized
 *    or
 *       an appropiate error report is generated
 */
static void
os_threadMemInit (
    os_threadContextData process_threadContextData)
{
    process_threadContextData->threadMem = os_malloc (sizeof(os_threadRegisteredPrivMem) * OS_THREAD_MEM_ARRAY_SIZE);
    if (process_threadContextData->threadMem != NULL) {
        bzero ((void *)process_threadContextData->threadMem, sizeof(os_threadRegisteredPrivMem) * OS_THREAD_MEM_ARRAY_SIZE);
    } else {
        OS_REPORT (OS_ERROR, "os_threadMemInit", 3, "Out of heap memory");
    }
}

/** \brief Free thread private memory
 *
 * Free the memory referenced by the thread reference
 * array indexed location. If this reference is NULL,
 * no action is taken. The reference is set to NULL
 * after freeing the heap memory.
 *
 * Postcondition:
 * - os_threadMemGet (index) = NULL and allocated
 *   heap memory is freed
 */
void
os_threadMemFree (
    os_int32 index)
{
    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        os_threadContextData currentThreadContextData = (os_threadContextData)readTLSVarSelf(threadContextData);
        if (currentThreadContextData != NULL) {
           os_threadRegisteredPrivMem *threadMemInfo = (os_threadRegisteredPrivMem*)(currentThreadContextData->threadMem);
           if (threadMemInfo[index].pthreadMem != NULL) {
              if (threadMemInfo[index].destructor != NULL) {
                  threadMemInfo[index].destructor(threadMemInfo[index].pthreadMem, threadMemInfo[index].userArgs);
              }
              os_free (threadMemInfo[index].pthreadMem);
              threadMemInfo[index].pthreadMem = NULL;
           }
        }
    }
}

/** \brief Initialize the thread private memory array
 *
 * \b os_threadMemInit releases the thread private memory array
 *    for the calling thread, the allocated private memory areas
 *    referenced by the array are also freed.
 *
 * pre condition:
 *    os_threadMemKey is initialized
 *
 * post condition:
 *       pthreadMemArray is released
 *    or
 *       an appropiate error report is generated
 */
void
os_threadMemExit (
    void)
{
    os_int32 i;

    for (i = 0; i < OS_THREAD_MEM_ARRAY_SIZE; i++) {
        os_threadMemFree(i);
    }
}

/** \brief Initialize the thread module
 *
 * \b os_threadModuleInit initializes the thread module for the
 *    calling process
 */
void
os_threadModuleInit (
    void)
{
    (void)sigfillset(&os_threadBlockAllMask);
    os_threadHookInit();
}

/** \brief Deinitialize the thread module
 *
 * \b os_threadModuleExit deinitializes the thread module for the
 *    calling process
 */
void
os_threadModuleExit (
    void)
{
    os_threadHookExit();
}

os_result
os_threadModuleSetHook(
    os_threadHook *hook,
    os_threadHook *oldHook)
{
    os_result result;
    os_threadHook oh;

    result = os_resultFail;
    oh = os_threadCBs;

    if (hook) {
        if (hook->startCb) {
            os_threadCBs.startCb = hook->startCb;
            os_threadCBs.startArg = hook->startArg;
        } else {
            os_threadCBs.startCb = os_threadStartCallback;
            os_threadCBs.startArg = NULL;
        }
        if (hook->stopCb) {
            os_threadCBs.stopCb = hook->stopCb;
            os_threadCBs.stopArg = hook->stopArg;
        } else {
            os_threadCBs.stopCb = os_threadStopCallback;
            os_threadCBs.stopArg = NULL;
        }

        if (oldHook) {
            *oldHook = oh;
        }
    }

    return result;
}

/** \brief Terminate the calling thread
 *
 * \b os_threadExit terminate the calling thread by calling
 * \b pthread_exit.
 */
void
os_threadExit (
    void *thread_result)
{
    os_threadContextData currentThreadContext = (os_threadContextData)readTLSVarSelf(threadContextData);
    os_threadMemExit ();
    os_threadSetExitStatus (currentThreadContext, thread_result);
    os_threadSetValidity (currentThreadContext, thread_ValExit);
    semGive (currentThreadContext->threadExitTrigger);
    os_procUnregisterThread ();
    os_procDeleteThreadTaskVar (currentThreadContext->threadTaskId, currentThreadContext->threadName,
                readTLSVarSelf(procContextData));
    currentThreadContext->threadExitStatus = (void *)thread_result;
    exit(0);
}

os_threadContextData
os_threadInit (
    const char *name)
{
    os_threadContextData process_threadContextData = NULL;

    process_threadContextData =
                          (os_threadContextData ) os_malloc ((size_t)OS_SIZEOF(os_threadContextData));
    if (process_threadContextData != NULL) {
        bzero ((char *)process_threadContextData, (size_t)OS_SIZEOF(os_threadContextData));
        process_threadContextData->threadName = (char *) os_malloc (strlen(name) + 1);
        os_strncpy (process_threadContextData->threadName, name, strlen(name) + 1);
        os_threadSetExitStatus (process_threadContextData, NULL);
        process_threadContextData->threadTaskId = (os_int32)process_threadContextData;
        os_threadMemInit (process_threadContextData);
        process_threadContextData->procId = (os_int32)readTLSVarSelf(procContextData);
        process_threadContextData->threadExitTrigger = semBCreate (SEM_Q_PRIORITY,SEM_EMPTY);
        process_threadContextData->threadDataSem = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
        process_threadContextData->threadTaskId = -1;
        os_threadSetValidity (process_threadContextData, thread_ValInit);
    }
    return (process_threadContextData);
}


os_result
os_threadAddTaskVar(
    os_int32 taskid,
    os_threadContextData process_threadContextData)
{
    os_int32 status = os_resultSuccess;

    /* create & set context variable for the task */
    if (addTLSVar (taskid, (int *)&threadContextData) != OK) {
        OS_REPORT (OS_WARNING, "os_threadAddTaskVar", 1,
                     "taskVarAdd failed with error %d (%s)",
                     os_getErrno(), process_threadContextData->threadName);
        status = os_resultInvalid;
    }
    if( setTLSVar (taskid, (int *)&threadContextData, (int)process_threadContextData) != OK){
        OS_REPORT (OS_WARNING, "os_threadAddTaskVar", 1,
                     "taskVarSet failed with error %d (%s)",
                     os_getErrno(), process_threadContextData->threadName);
        status = os_resultInvalid;
    }
    return (status);
}

os_result
os_threadDeleteTaskVar(
    os_int32 taskid,
    os_threadContextData process_threadContextData)
{
    os_int32 status = os_resultSuccess;

    /* create & set context variable for the task */
    if (deleteTLSVar (taskid, (int *)&threadContextData) != OK) {
        OS_REPORT (OS_WARNING, "os_threadDeleteTaskVar", 1,
                     "taskVarDelete failed with error %d (%s)",
                     os_getErrno(), process_threadContextData->threadName);
        status = os_resultInvalid;
    }
    return (status);
}


/** \brief Wrap thread start routine
 *
 * \b os_startRoutineWrapper wraps a threads starting routine.
 * before calling the user routine, it sets the threads name
 * in the context of the thread. With \b pthread_getspecific,
 * the name can be retreived for different purposes.
 */

static os_result
os_threadWrapper (
    os_procContextData process_procContextData,
    os_threadContextData process_threadContextData,
    void *(* start_routine)(void *),
    void *arguments,
    TASK_ID parent)
{
    os_int32 threadTaskId;
    os_int32 rt;
    void *startroutine_status = NULL;
    os_result rv = os_resultSuccess;

#if ( _WRS_VXWORKS_MAJOR > 6 ) || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR > 8 )
    envPrivateCreate(taskIdSelf(), parent);
#else
    envPrivateDestroy(taskIdSelf());
#endif
    threadTaskId = taskIdSelf ();
    os_procSetThreadId (process_threadContextData, threadTaskId);
    os_procAddTaskVar (threadTaskId, taskName(0), process_procContextData, 1);
    os_threadAddTaskVar (threadTaskId, process_threadContextData);
    os_threadSetValidity (process_threadContextData, thread_ValRunning);
    rv = os_procRegisterThread (process_procContextData);
    if (rv == os_resultSuccess) {
        if (os_threadCBs.startCb(process_threadContextData, os_threadCBs.startArg) == 0) {
            startroutine_status = start_routine (arguments);
        }
        os_threadCBs.stopCb(process_threadContextData, os_threadCBs.stopArg);
        os_threadExit (startroutine_status);
    } else {
        rt = -1;
        os_threadExit (&rt);
        rv = os_resultFail;
    }

#if ( _WRS_VXWORKS_MAJOR > 6 ) || ( _WRS_VXWORKS_MAJOR > 6 && _WRS_VXWORKS_MINOR > 8 )
    envPrivateDestroy(taskIdSelf());
#endif
    return(rv);
}

/** \brief Create a new thread
 *
 * \b os_threadCreate creates a thread by calling \b pthread_create.
 * But first it processes all thread attributes in \b threadAttr and
 * sets the scheduling properties with \b pthread_attr_setscope
 * to create a bounded thread, \b pthread_attr_setschedpolicy to
 * set the scheduling class and \b pthread_attr_setschedparam to
 * set the scheduling priority.
 * \b pthread_attr_setdetachstate is called with parameter
 * \PTHREAD_CREATE_JOINABLE to make the thread joinable, which
 * is needed to be abale to wait for the threads termination
 * in \b os_threadWaitExit.
 */
os_result
os_threadCreate (
    os_threadId *threadId,
    const char *name,
    const os_threadAttr *threadAttr,
    void *(* start_routine)(void *),
    void *arg)
{
    os_result rv = os_resultSuccess;
    int sched_policy;
    os_threadContextData task_threadContextData = NULL;
    int threadTaskId;
    int pOptions,privateSet;
    void * currentContextData;

    assert (threadId != NULL);
    assert (name != NULL);
    assert (threadAttr != NULL);
    assert (start_routine != NULL);

    pOptions = 0;
    privateSet = 0;
    taskOptionsGet(taskIdSelf(),&pOptions);
    if ((pOptions & VX_PRIVATE_ENV) == 0) {
        envPrivateCreate(taskIdSelf(), 0);
        privateSet = 1;
    }
    putenv("SPLICE_NEW_PROCESS=no");

    os_procInitialize();
    currentContextData = readTLSVarSelf(procContextData);

    if (threadAttr->schedClass == OS_SCHED_REALTIME) {
        sched_policy = OS_SCHED_REALTIME;
    } else if (threadAttr->schedClass == OS_SCHED_TIMESHARE) {
        return os_resultInvalid;
    } else if (threadAttr->schedClass == OS_SCHED_DEFAULT) {
        sched_policy = OS_SCHED_DEFAULT;
    } else {
        return os_resultInvalid;
    }
    if ((threadAttr->schedPriority > VXWORKS_PRIORITY_MIN) ||
        (threadAttr->schedPriority < VXWORKS_PRIORITY_MAX)) {
        return os_resultInvalid;
    }
    if (currentContextData != NULL) {
        task_threadContextData = os_threadInit(name);
        if (task_threadContextData == NULL) {
            rv = os_resultFail;
        }
        if (rv == os_resultSuccess) {
            threadTaskId = taskSpawn ((char *)name, threadAttr->schedPriority,
                                VX_FP_TASK
#if ! defined ( _WRS_VXWORKS_MAJOR ) || _WRS_VXWORKS_MAJOR < 6 || ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 9 )
/* VX_PRIVATE_ENV flag is no longer functional in vxworks 6.9 */
                      |  VX_PRIVATE_ENV
#endif
#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
                                | VX_STDIO | VX_DEALLOC_STACK
#endif
                                ,
                                threadAttr->stackSize,
                                (FUNCPTR)os_threadWrapper,
                                (int)currentContextData, (int)task_threadContextData,
                                (int)start_routine,(int)arg, taskIdSelf(), 0, 0, 0, 0, 0);
           if (threadTaskId == ERROR) {
               os_threadDispose(task_threadContextData);
               rv = os_resultFail;
           } else {
               *threadId = (os_threadId *)task_threadContextData;
               rv = os_resultSuccess;
           }
        }
    } else { /* no service but apl */
       threadTaskId = taskSpawn ((char *)name, threadAttr->schedPriority,
                            VX_FP_TASK
#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )
                            | VX_STDIO | VX_DEALLOC_STACK
#endif
                            ,
                            VXWORKS_PROC_DEFAULT_STACK,
                            (FUNCPTR)start_routine,
                            (int)arg, 0, 0, 0, 0, 0, 0, 0, 0, 0);
       if (threadTaskId == ERROR) {
           rv = os_resultFail;
       }
    }
    putenv("SPLICE_NEW_PROCESS=empty");
    if (privateSet == 1) {
        envPrivateDestroy(taskIdSelf());
    }
    return (rv);
}

/** \brief Return the thread ID of the calling thread
 *
 * \b os_threadIdSelf determines the own thread ID by
 * calling \b pthread_self.
 */
os_threadId
os_threadIdSelf (
    void)
{
    os_threadContextData currentThreadContextData = (os_threadContextData)readTLSVarSelf(threadContextData);
    if (readTLSVarSelf(procContextData) == NULL) {
        os_procInitialize();
    }
    return (currentThreadContextData);
}

/** \brief Figure out the identity of the current thread
 *
 * \b os_threadFigureIdentity determines the numeric identity
 * of a thread. POSIX does not identify threads by name,
 * therefor only the numeric identification is returned,
 */
os_int32
os_threadFigureIdentity (
    char *threadIdentity,
    os_uint32 threadIdentitySize)
{
    os_int32 size;

    if (os_threadGetName() != NULL) {
        size = snprintf (threadIdentity, threadIdentitySize,
                         "%s (%d %d)", os_threadGetName(), os_threadIdSelf(), taskIdSelf());
    } else {
        size = snprintf (threadIdentity, threadIdentitySize,
                         " %d %d", os_threadIdSelf(), taskIdSelf());
    }
    return (size);
}

/** \brief Wait for the termination of the identified thread
 *
 * \b os_threadWaitExit wait for the termination of the
 * thread \b threadId by calling \b pthread_join. The return
 * value of the thread is passed via \b thread_result.
 */
os_result
os_threadWaitExit (
    os_threadId threadId,
    void **thread_result)
{
    os_result rv;
    thread_Validity thv;
    os_int32 count = 0;

    assert (threadId);

    thv = os_threadCheckValid (threadId);
    while ((thv == thread_ValInit) && (count < 5)) {
        count ++;
        taskDelay(sysClkRateGet());
        thv = os_threadCheckValid (threadId);
    }
    if ((thv == thread_ValRunning) || (thv == thread_ValExit)) {
        if (semTake (threadId->threadDataSem, 0) == ERROR) {
            rv = os_resultUnavailable;
        } else {
            os_threadAwaitTermination (threadId);
            if (thread_result != NULL) {
                *thread_result = (void *)os_threadCheckExitValue (threadId);
            }
            os_threadSetValidity (threadId, thread_ValRead);
            rv = os_resultSuccess;
        }
    } else {
         rv = os_resultFail;
    }
    return rv;
}

/** \brief Allocate thread private memory
 *
 * Allocate heap memory of the specified \b size and
 * relate it to the thread by storing the memory
 * reference in an thread specific reference array
 * indexed by \b index. If the indexed thread reference
 * array location already contains a reference, no
 * memory will be allocated and NULL is returned.
 *
 * Possible Results:
 * - returns NULL if
 *     index < 0 || index >= OS_THREAD_MEM_ARRAY_SIZE
 * - returns NULL if
 *     no sufficient memory is available on heap
 * - returns NULL if
 *     os_threadMemGet (index) returns != NULL
 * - returns reference to allocated heap memory
 *     of the requested size if
 *     memory is successfully allocated
 */
void *
os_threadMemMalloc(
    os_int32 index,
    os_size_t size,
    os_threadPrivMemDestructor destructor,
    void* userArgs)
{
    void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        os_threadRegisteredPrivMem *threadMemInfo = NULL;
        os_threadContextData currentThreadContextData = (os_threadContextData)readTLSVarSelf(threadContextData);
        if (currentThreadContextData != NULL) {
            threadMemInfo = (os_threadRegisteredPrivMem*)(currentThreadContextData->threadMem);
            if (threadMemInfo[index].pthreadMem == NULL) {
                threadMemInfo[index].pthreadMem = (void*)os_malloc (size);
                threadMemLoc = threadMemInfo[index].pthreadMem;
                threadMemInfo[index].destructor = destructor;
                threadMemInfo[index].userArgs   = userArgs;
            }
        }
    }
    return threadMemLoc;
}

/** \brief Get thread private memory
 *
 * Possible Results:
 * - returns NULL if
 *     0 < index <= OS_THREAD_MEM_ARRAY_SIZE
 * - returns NULL if
 *     No heap memory is related to the thread for
 *     the specified index
 * - returns a reference to the allocated memory
 */
void *
os_threadMemGet (
    os_int32 index)
{
    void *threadMemLoc = NULL;
    os_threadContextData currentThreadContextData = (os_threadContextData)readTLSVarSelf(threadContextData);
    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        if (currentThreadContextData != NULL) {
            os_threadRegisteredPrivMem *threadMemInfo = (os_threadRegisteredPrivMem*)(currentThreadContextData->threadMem);
            threadMemLoc = (void *)threadMemInfo[index].pthreadMem;
        }
    }
    return threadMemLoc;
}

os_result
os_threadProtect(void)
{
    os_result result;
    os_threadProtectInfo *pi;

    pi = os_threadMemGet(OS_THREAD_PROTECT);
    if (pi == NULL) {
        pi = os_threadMemMalloc(OS_THREAD_PROTECT,
                                sizeof(os_threadProtectInfo),
                                NULL, NULL);
        if (pi) {
            pi->protectCount = 1;
            result = os_resultSuccess;
        } else {
            result = os_resultFail;
        }
    } else {
        pi->protectCount++;
        result = os_resultSuccess;
    }
    if ((result == os_resultSuccess) && (pi->protectCount == 1)) {
        if (pthread_sigmask(SIG_SETMASK,
                         &os_threadBlockAllMask,
                         &pi->oldMask) != 0) {
            result = os_resultFail;
        }
    }
    return result;
}

os_result
os_threadUnprotect(void)
{
    os_result result;
    os_threadProtectInfo *pi;

    pi = os_threadMemGet(OS_THREAD_PROTECT);
    if (pi) {
        pi->protectCount--;
        if (pi->protectCount == 0) {
            if (pthread_sigmask(SIG_SETMASK,&pi->oldMask,NULL) != 0) {
                result = os_resultFail;
            } else {
                result = os_resultSuccess;
            }
        } else {
            result = os_resultSuccess;
        }
    } else {
        result = os_resultFail;
    }

    return result;
}

void
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    assert (threadAttr != NULL);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    if (readTLSVarSelf(procContextData) != NULL) {
      threadAttr->schedPriority = ((os_procContextData)readTLSVarSelf(procContextData))->procAttrPrio;
    } else {
        threadAttr->schedPriority = VXWORKS_PRIORITY_DEFAULT;
    }
    threadAttr->stackSize = VXWORKS_PROC_DEFAULT_STACK;
}

void
os_threadThreadInit(
    os_procContextData process_procContextData,
    os_int32 threadTaskId)
{
    os_threadContextData process_threadContextData = NULL;

    process_threadContextData = os_threadInit(taskName(0));
    if (process_threadContextData == NULL) {
        logMsg("Task %s could not initialize : os_threadInit os_threadInit failed\n",taskName(0), 0, 0, 0, 0, 0);
        return;
    }

    os_procSetThreadId (process_threadContextData, threadTaskId);
    os_procAddTaskVar (threadTaskId, taskName(0), process_procContextData, 1);
    os_threadAddTaskVar (threadTaskId, process_threadContextData);
    os_threadSetValidity (process_threadContextData, thread_ValRunning);
    os_procRegisterThread (process_procContextData);

}

void
os_threadHookThreadInit(
    os_procContextData process_procContextData,
    os_int32 threadTaskId)
{
    os_threadContextData process_threadContextData = NULL;

    process_threadContextData = os_threadInit(taskName(0));
    if (process_threadContextData == NULL) {
        logMsg("Task %s could not initialize : os_threadInit os_threadInit failed\n",taskName(0), 0, 0, 0, 0, 0);
        return;
    }

    os_procSetThreadId (process_threadContextData, threadTaskId);
    os_procAddTaskVar (threadTaskId, taskName(0), process_procContextData, 1);
    os_threadAddTaskVar (threadTaskId, process_threadContextData);
    os_threadSetValidity (process_threadContextData, thread_ValRunning);
    os_procRegisterHookThread (process_procContextData, process_threadContextData);

}

/** \brief install Hook for aplliction which does not use os_procCreate / os_threadCreate
 *
 */
void
os_threadInitialize(
    WIND_TCB *pNewTcb)
{
    os_procContextData process_procContextData;

    process_procContextData = (os_procContextData)os_procIdSelf();
    os_threadThreadInit(readTLSVarSelf(procContextData), (os_int32)pNewTcb);

}

os_result
os_hookthreadNew(
    const char *name,
    os_procContextData process_procContextData,
    os_int32 taskid)
{
    os_threadContextData process_threadContextData = NULL;
    os_int32 status = os_resultSuccess;

    process_threadContextData = os_threadInit (name);   /* main is also a thread */
    os_procSetThreadId (process_threadContextData, taskid);
    os_threadAddTaskVar (taskid, process_threadContextData);
    os_threadSetValidity (process_threadContextData, thread_ValRunning);
    status = os_procRegisterHookThread (process_procContextData, process_threadContextData);
    return (status);
}


