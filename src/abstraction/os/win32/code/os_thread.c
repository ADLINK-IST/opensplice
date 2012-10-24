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
/** \file os/win32/code/os_thread.c
 *  \brief WIN32 thread management
 *
 * Implements thread management for WIN32
 */

#include "os_thread.h"
#include "os_stdlib.h"
#include "os_time.h"
#include "os_heap.h"
#include "os_abstract.h"
#include "code/os__debug.h"

#include <stdio.h>
#include <assert.h>

os_threadInfo id_none = {0, };

typedef struct {
    char *threadName;
    void *arguments;
    void *(*startRoutine)(void *);
} os_threadContext;

typedef struct {
    os_uint  protectCount;
} os_threadProtectInfo;

static DWORD tlsIndex;
static os_threadHook os_threadCBs;

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

static void
os_threadMemInit(void)
{
    void **tlsMemArray;
    BOOL result;

    tlsMemArray = malloc (sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
    if (tlsMemArray) {
        memset(tlsMemArray, 0, sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
        result = TlsSetValue(tlsIndex, tlsMemArray);
        if (!result) {
            OS_DEBUG("os_threadMemInit", "Failed to set TLS");
        }
    }
}

static void
os_threadMemExit(void)
{
    void **tlsMemArray;
    int i;

    tlsMemArray = (void **)TlsGetValue(tlsIndex);
    if (tlsMemArray != NULL) {
        for (i = 0; i < OS_THREAD_MEM_ARRAY_SIZE; i++) {
            if (tlsMemArray[i] != NULL) {
                free(tlsMemArray[i]);
            }
        }
        free(tlsMemArray);
        TlsSetValue(tlsIndex, NULL);
    }
}

/** \brief Initialize thread attributes
 *
 * - Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 *   (take the platforms default scheduling class, Time-sharing for
 *   non realtime platforms, Real-time for realtime platforms)
 * - Set \b procAttr->schedPriority to \b 0
 */
os_result
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    assert (threadAttr != NULL);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = 0;
    threadAttr->stackSize = 1024*1024; /* 1MB */
    return os_resultSuccess;
}

/** \brief Initialize the thread module
 *
 * \b os_threadModuleInit initializes the thread module for the
 *    calling process
 */
void
os_threadModuleInit(void)
{
   tlsIndex = TlsAlloc();
   if (tlsIndex == 0xFFFFFFFF) {
      OS_DEBUG_1("os_threadModuleInit", "Warning: could not allocate thread-local memory (System Error Code: %i)", GetLastError ());
   }
   os_threadHookInit();
}

/** \brief Deinitialize the thread module
 *
 * \b os_threadModuleExit deinitializes the thread module for the
 *    calling process
 */
void
os_threadModuleExit(void)
{
   LPVOID data = TlsGetValue(tlsIndex);

   if (data != NULL) {
      LocalFree((HLOCAL) data);
   }

   TlsFree(tlsIndex);
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
 * \b os_threadExit terminate the calling thread  by calling
 * \b Exit
 */
void
os_threadExit(
    void *thread_result)
{
    ExitThread((DWORD)thread_result);
}

/** \brief Wrap thread start routine
 *
 * \b os_startRoutineWrapper wraps a threads starting routine.
 * before calling the user routine, it sets the threads name
 * in the context of the thread. With \b pthread_getspecific,
 * the name can be retreived for different purposes.
 */
static void *
os_startRoutineWrapper(
    void *threadContext)
{
    os_threadContext *context = threadContext;
    void *resultValue;
    os_threadId id;

    /* allocate an array to store thread private memory references */
    os_threadMemInit();

    id.threadId = GetCurrentThreadId();
    id.handle = GetCurrentThread();
    /* Call the start callback */
    if (os_threadCBs.startCb(id, os_threadCBs.startArg) == 0) {
        /* Call the user routine */
        resultValue = context->startRoutine(context->arguments);
    }

    os_threadCBs.stopCb(id, os_threadCBs.stopArg);

    /* Free the thread context resources, arguments is responsibility */
    /* for the caller of os_procCreate                                */
    os_free (context->threadName);
    os_free (context);

    /* deallocate the array to store thread private memory references */
    os_threadMemExit ();

    /* return the result of the user routine */
    return resultValue;
}

/** \brief Create a new thread
 *
 * \b os_threadCreate creates a thread by calling \b CreateThread.
 */
os_result
os_threadCreate(
    os_threadId *threadId,
    const char *name,
    const os_threadAttr *threadAttr,
    void *(* start_routine)(void *),
    void *arg)
{
    HANDLE threadHandle;
    DWORD threadIdent;
    os_threadContext *threadContext;

    os_int32 effective_priority;

    assert(threadId != NULL);
    assert(name != NULL);
    assert(threadAttr != NULL);
    assert(start_routine != NULL);

    /* Take over the thread context: name, start routine and argument */
    threadContext = os_malloc(sizeof (os_threadContext));
    threadContext->threadName = os_malloc(strlen (name)+1);
    os_strncpy(threadContext->threadName, name, strlen (name)+1);
    threadContext->startRoutine = start_routine;
    threadContext->arguments = arg;
    threadHandle = CreateThread(NULL,
        (SIZE_T)threadAttr->stackSize,
        (LPTHREAD_START_ROUTINE)os_startRoutineWrapper,
        (LPVOID)threadContext,
        (DWORD)0, &threadIdent);
    if (threadHandle == 0) {
        OS_DEBUG_1("os_threadCreate", "Failed with System Error Code: %i\n", GetLastError ());
        return os_resultFail;
    }

    fflush(stdout);

    threadId->handle   = threadHandle;
    threadId->threadId = threadIdent;

    /*  #642 fix (JCM)
     *  Windows thread priorities are in the range below :
    -15 : THREAD_PRIORITY_IDLE
    -2  : THREAD_PRIORITY_LOWEST
    -1  : THREAD_PRIORITY_BELOW_NORMAL
     0  : THREAD_PRIORITY_NORMAL
     1  : THREAD_PRIORITY_ABOVE_NORMAL
     2  : THREAD_PRIORITY_HIGHEST
    15  : THREAD_PRIORITY_TIME_CRITICAL
    For realtime threads additional values are allowed : */

    /* PROCESS_QUERY_INFORMATION rights required
     * to call GetPriorityClass
     * Ensure that priorities are efectively in the allowed range depending
     * on GetPriorityClass result */
	effective_priority = threadAttr->schedPriority;
    if (GetPriorityClass(GetCurrentProcess()) == REALTIME_PRIORITY_CLASS) {
        if (threadAttr->schedPriority < -7) {
        	effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (threadAttr->schedPriority > 6) {
        	effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    } else {
        if (threadAttr->schedPriority < THREAD_PRIORITY_LOWEST) {
        	effective_priority = THREAD_PRIORITY_IDLE;
        }
        if (threadAttr->schedPriority > THREAD_PRIORITY_HIGHEST) {
        	effective_priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
    }
	if (SetThreadPriority (threadHandle, effective_priority) == 0) {
		OS_DEBUG_1("os_threadCreate", "SetThreadPriority failed with %d", (int)GetLastError());
	}

   /* ES: dds2086: Close handle should not be performed here. Instead the handle
    * should not be closed until the os_threadWaitExit(...) call is called.
    * CloseHandle (threadHandle);
    */
   return os_resultSuccess;
}

/** \brief Return the integer representation of the given thread ID
 *
 * Possible Results:
 * - returns the integer representation of the given thread ID
 */
os_ulong_int
os_threadIdToInteger(os_threadId id)
{
   return id.threadId;
}

/** \brief Return the thread ID of the calling thread
 *
 * \b os_threadIdSelf determines the own thread ID by
 * calling \b GetCurrentThreadId ().
 */
os_threadId
os_threadIdSelf(
    void)
{
   os_threadId id;
   id.threadId = GetCurrentThreadId();
   id.handle = GetCurrentThread();   /* pseudo HANDLE, no need to close it */

   return id;
}

/** \brief Wait for the termination of the identified thread
 *
 * \b os_threadWaitExit wait for the termination of the
 * thread \b threadId by calling \b pthread_join. The return
 * value of the thread is passed via \b thread_result.
 */
os_result
os_threadWaitExit(
    os_threadId threadId,
    void **thread_result)
{
/*     HANDLE threadHandle; */
    DWORD tr;
    DWORD err;
    register int callstatus;

/*     threadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, (DWORD)threadId); */

    if(threadId.handle == NULL){
        OS_DEBUG("os_threadWaitExit", "Parameter threadId is null");
        return os_resultFail;
    }

    callstatus = GetExitCodeThread(threadId.handle, &tr);
    
    if (callstatus == 0) {
       err = GetLastError();
       OS_DEBUG_1("os_threadWaitExit", "GetExitCodeThread Failed %d", err);
        return os_resultFail;
    } else {
        while (tr == STILL_ACTIVE) {
           Sleep(100);
           if (GetExitCodeThread(threadId.handle, &tr) == 0) {
              err = GetLastError();
              OS_DEBUG_1("os_threadWaitExit",  "GetExitCodeThread Failed %d", err);
              return os_resultFail;
            }
        }
    }
    if (thread_result != NULL) {
        *thread_result = (VOID *)tr;
    }
    CloseHandle(threadId.handle);
    /* ES: dds2086: Perform a second close of the handle, this in effect closes
     * the handle opened by the thread creation in the os_threadCreate(...) call.
     */
/*     CloseHandle(threadHandle); */

    return os_resultSuccess;
}

/** \brief Figure out the identity of the current thread
 *
 * Possible Results:
 * - returns the actual length of threadIdentity
 */
os_int32
os_threadFigureIdentity(
    char *threadIdentity,
    os_uint threadIdentitySize)
{
   os_int32 size;

   size = snprintf(threadIdentity, threadIdentitySize, "%u", GetCurrentThreadId());

   return size;
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
    os_int32 size)
{
   void **tlsMemArray;
   void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        tlsMemArray = (void **)TlsGetValue(tlsIndex);
        if (tlsMemArray == NULL) {
            os_threadMemInit ();
            tlsMemArray = (void **)TlsGetValue(tlsIndex);
        }
        if (tlsMemArray != NULL) {
            if (tlsMemArray[index] == NULL) {
                threadMemLoc = malloc(size);
                if (threadMemLoc != NULL) {
                    tlsMemArray[index] = threadMemLoc;
                }
            }
        }
    }
    return threadMemLoc;
}

/** \brief Free thread private memory
 *
 * Free the memory referenced by the thread reference
 * array indexed location. If this reference is NULL,
 * or index is invalid, no action is taken.
 * The reference is set to NULL after freeing the
 * heap memory.
 *
 * Postcondition:
 * - os_threadMemGet (index) = NULL and allocated
 *   heap memory is freed
 */
void
os_threadMemFree(
    os_int32 index)
{
    void **tlsMemArray;
    void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        tlsMemArray = (void **)TlsGetValue(tlsIndex);
        if (tlsMemArray != NULL) {
            threadMemLoc = tlsMemArray[index];
            if (threadMemLoc != NULL) {
                tlsMemArray[index] = NULL;
                free(threadMemLoc);
            }
        }
    }
}

/** \brief Get thread private memory
 *
 * Possible Results:
 * - returns NULL if
 *     No heap memory is related to the thread for
 *     the specified index
 * - returns a reference to the allocated memory
 */
void *
os_threadMemGet(
    os_int32 index)
{
    void **tlsMemArray;
    void *data;

    data = NULL;
    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        tlsMemArray = TlsGetValue(tlsIndex);
        if (tlsMemArray != NULL) {
            data = tlsMemArray[index];
        }
    }

    return data;
}

os_result
os_threadProtect(void)
{
    os_result result;
    os_threadProtectInfo *pi;

    pi = os_threadMemGet(OS_THREAD_PROTECT);
    if (pi == NULL) {
        pi = os_threadMemMalloc(OS_THREAD_PROTECT,
                                sizeof(os_threadProtectInfo));
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
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }

    return result;
}
