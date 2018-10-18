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
/** \file os/win32/code/os_thread.c
 *  \brief WIN32 thread management
 *
 * Implements thread management for WIN32
 */

#include "os_thread.h"
#include "os__thread.h"
#include "os_stdlib.h"
#include "os_time.h"
#include "os_heap.h"
#include "os_abstract.h"
#include "code/os__debug.h"
#include "os_errno.h"

#include <stdio.h>
#include <assert.h>

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

    tlsMemArray = (void**)malloc (sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
    if (tlsMemArray) {
        memset(tlsMemArray, 0, sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
        result = TlsSetValue(tlsIndex, tlsMemArray);
        if (!result) {
            OS_DEBUG("os_threadMemInit", "Failed to set TLS");
        }
    }
}

void
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
 * - Set \b procAttr->schedPriority to \b CE_THREAD_PRIO_256_NORMAL
 */
void
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    assert (threadAttr != NULL);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = CE_THREAD_PRIO_256_NORMAL;
    threadAttr->stackSize = 1024*1024; /* 1MB */
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
      OS_DEBUG_1("os_threadModuleInit", "Warning: could not allocate thread-local memory (System Error Code: %i)", os_getErrno ());
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
    os_threadContext *context = (os_threadContext*)threadContext;
    void *resultValue;
    os_threadId id;

    /* allocate an array to store thread private memory references */
    os_threadMemInit();

    id = (os_threadId)GetCurrentThreadId();
    /* Call the start callback */
    if (os_threadCBs.startCb(id, os_threadCBs.startArg) == 0) {
        /* Call the user routine */
        resultValue = context->startRoutine(context->arguments);
    }

    os_threadCBs.stopCb(id, os_threadCBs.stopArg);

    os_report_stack_free();
    os_reportClearApiInfo();

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

    assert(threadId != NULL);
    assert(name != NULL);
    assert(threadAttr != NULL);
    assert(start_routine != NULL);

    /* Take over the thread context: name, start routine and argument */
    threadContext = (os_threadContext*)os_malloc(sizeof (os_threadContext));
    threadContext->threadName = (char*)os_malloc(strlen (name)+1);
    strncpy(threadContext->threadName, name, strlen (name)+1);
    threadContext->startRoutine = start_routine;
    threadContext->arguments = arg;
    threadHandle = CreateThread(NULL,
        (SIZE_T)threadAttr->stackSize,
        (LPTHREAD_START_ROUTINE)os_startRoutineWrapper,
        (LPVOID)threadContext,
        (DWORD)0, &threadIdent);
    if (threadHandle == 0) {
        OS_DEBUG_1("os_threadCreate", "Failed with System Error Code: %i\n", os_getErrno ());
        return os_resultFail;
    }

    fflush(stdout);

    *threadId = (os_threadId)threadIdent;

   /*
    * Windows CE has a different thread priority model to Windows.
    * It should use CeSetThreadPriority which takes a priority value
    * 0 - 255 (0 being highest priority and 255 the lowest).  Most
    * applications should use the 248 - 255 range with our default
    * CE_THREAD_PRIO_256_NORMAL mapping to value 251.
    *
    * However we should not restrict the user to the 248 - 255 range
    */

    if (CeSetThreadPriority (threadHandle, threadAttr->schedPriority) == 0) {
      OS_DEBUG_1("os_threadCreate", "CeSetThreadPriority failed with %d", os_getErrno());
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
    return (os_ulong_int) id;
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
    return (os_threadId)GetCurrentThreadId();
}

/** \brief Wait for the termination of the identified thread
 *
 * \b os_threadWaitExit wait for the termination of the
 * thread \b threadId by calling \b pthread_join. The return
 * value of the thread is passed via \b thread_result.
 * thread_result needs to be set to NULL as no other value works.
 * dds2897/OSPL-429
 */
os_result
os_threadWaitExit(
    os_threadId threadId,
    void **thread_result)
{
    HANDLE threadHandle;
    DWORD tr;

    /* NULL is the only value that works, so we need to check that it is NULL dds2897/Jira OSPL-429 */
    assert (thread_result==NULL);

    threadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, (DWORD)threadId);

    if(threadHandle == NULL){
        OS_DEBUG_1("os_threadWaitExit", "OpenThread Failed %d", os_getErrno());
        return os_resultFail;
    }

    if (WaitForSingleObject(threadHandle, INFINITE) != WAIT_OBJECT_0) {
        OS_DEBUG_1("os_threadWaitExit", "WaitForSingleObject Failed %d", os_getErrno());
        CloseHandle(threadHandle);
        return os_resultFail;
    }

    if (!GetExitCodeThread(threadHandle, &tr)) {
        OS_DEBUG_1("os_threadWaitExit", "GetExitCodeThread Failed %d", os_getErrno());
        CloseHandle(threadHandle);
        return os_resultFail;
    }
    /** Uncomment this when the issue with thread_result is resolved dds2897/OSPL-429
    if (thread_result != NULL) {
        *thread_result = (VOID *)tr;
    }
    */
    CloseHandle(threadHandle);
    /* ES: dds2086: Perform a second close of the handle, this in effect closes
     * the handle opened by the thread creation in the os_threadCreate(...) call.
     */
    CloseHandle(threadHandle);

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

os_int32
os_threadGetThreadName(
    os_char *buffer,
    os_uint32 length)
{
    assert (buffer != NULL);

    return snprintf (buffer, length, "%u", GetCurrentThreadId ());
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
    os_size_t size)
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
        tlsMemArray = (void**)TlsGetValue(tlsIndex);
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

    pi = (os_threadProtectInfo*)os_threadMemGet(OS_THREAD_PROTECT);
    if (pi == NULL) {
        pi = (os_threadProtectInfo*)os_threadMemMalloc(OS_THREAD_PROTECT,
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

    pi = (os_threadProtectInfo*)os_threadMemGet(OS_THREAD_PROTECT);
    if (pi) {
        pi->protectCount--;
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }

    return result;
}
