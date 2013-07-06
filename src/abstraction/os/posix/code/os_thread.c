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
/** \file os/posix/code/os_thread.c
 *  \brief Posix thread management
 *
 * Implements thread management for POSIX
 */

#include "os_thread.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_process.h"
#include "os_abstract.h"
#include "os_stdlib.h"

#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#ifndef PIKEOS_POSIX
#include <strings.h>
#endif
#include <string.h>
#include <stdio.h>
#ifndef INTEGRITY
#include <signal.h>
#ifndef OS_HAS_NO_SET_NAME_PRCTL /* Define in os_defs.h iff there's no prctl()
                                    on this platform or PR_SET_NAME does not exist */
#include <sys/prctl.h>
#endif
#endif
#include <limits.h>

typedef struct {
    char *threadName;
    void *arguments;
    void *(*startRoutine)(void *);
} os_threadContext;

typedef struct {
#ifndef INTEGRITY
    sigset_t oldMask;
#endif
    os_uint  protectCount;
} os_threadProtectInfo;

static pthread_key_t os_threadNameKey;
static pthread_key_t os_threadMemKey;

static os_threadHook os_threadCBs;

#ifndef INTEGRITY
static sigset_t os_threadBlockAllMask;
#endif

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
 *       an appropriate error report is generated
 */
static void
os_threadMemInit (
    void)
{
    void *pthreadMemArray;

    pthreadMemArray = os_malloc (sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
    if (pthreadMemArray != NULL) {
        memset (pthreadMemArray, 0, sizeof(void *) * OS_THREAD_MEM_ARRAY_SIZE);
        if (pthread_setspecific (os_threadMemKey, pthreadMemArray) == EINVAL) {
            OS_REPORT_1 (OS_ERROR, "os_threadMemInit", 4, "pthread_setspecific failed with error %d", EINVAL);
        }
    } else {
        OS_REPORT (OS_ERROR, "os_threadMemInit", 3, "Out of heap memory");
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
 *       an appropriate error report is generated
 */
static void
os_threadMemExit(
    void)
{
    void **pthreadMemArray;
    os_int32 i;

    pthreadMemArray = pthread_getspecific (os_threadMemKey);
    if (pthreadMemArray != NULL) {
        for (i = 0; i < OS_THREAD_MEM_ARRAY_SIZE; i++) {
            if (pthreadMemArray[i] != NULL) {
                os_free (pthreadMemArray[i]);
            }
        }
        os_free (pthreadMemArray);
        pthreadMemArray = NULL;
        if (pthread_setspecific (os_threadMemKey, pthreadMemArray) == EINVAL) {
            OS_REPORT_1 (OS_ERROR, "os_threadMemExit", 4, "pthread_setspecific failed with error %d", EINVAL);
        }
    }
}

static int
os_threadStartCallback(
    os_threadId id,
    void *arg)
{
    OS_UNUSED_ARG(id);
    OS_UNUSED_ARG(arg);
    return 0;
}

static int
os_threadStopCallback(
    os_threadId id,
    void *arg)
{
    OS_UNUSED_ARG(id);
    OS_UNUSED_ARG(arg);
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

/** \brief Initialize the thread module
 *
 * \b os_threadModuleInit initializes the thread module for the
 *    calling process
 */
void
os_threadModuleInit (
    void)
{
    pthread_key_create (&os_threadNameKey, NULL);
    pthread_key_create (&os_threadMemKey, NULL);

    pthread_setspecific (os_threadNameKey, "main thread");

#ifndef INTEGRITY
    sigfillset(&os_threadBlockAllMask);
#endif

    os_threadMemInit();

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
    os_threadHookExit();
    os_threadMemExit();

    pthread_key_delete(os_threadNameKey);
    pthread_key_delete(os_threadMemKey);
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

    os_threadMemExit ();

    pthread_exit (thread_result);
    return;
}

/** \brief Wrap thread start routine
 *
 * \b os_startRoutineWrapper wraps a threads starting routine.
 * before calling the user routine, it sets the threads name
 * in the context of the thread. With \b pthread_getspecific,
 * the name can be retreived for different purposes.
 */
static void *
os_startRoutineWrapper (
    void *threadContext)
{
    os_threadContext *context = threadContext;
    void *resultValue;
    os_threadId id;

    resultValue = NULL;

#ifdef INTEGRITY
    SetTaskName(CurrentTask(), context->threadName, strlen(context->threadName));
#else
#ifndef OS_HAS_NO_SET_NAME_PRCTL
    prctl(PR_SET_NAME, context->threadName);
#endif
#endif

    /* store the thread name with the thread via thread specific data */
    pthread_setspecific (os_threadNameKey, context->threadName);

    /* allocate an array to store thread private memory references */
    os_threadMemInit ();

    id = pthread_self();
    /* Call the start callback */
    if (os_threadCBs.startCb(id, os_threadCBs.startArg) == 0) {
        /* Call the user routine */
        resultValue = context->startRoutine (context->arguments);
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
 * \b os_threadCreate creates a thread by calling \b pthread_create.
 * But first it processes all thread attributes in \b threadAttr and
 * sets the scheduling properties with \b pthread_attr_setscope
 * to create a bounded thread, \b pthread_attr_setschedpolicy to
 * set the scheduling class and \b pthread_attr_setschedparam to
 * set the scheduling priority.
 * \b pthread_attr_setdetachstate is called with parameter
 * \PTHREAD_CREATE_JOINABLE to make the thread joinable, which
 * is needed to be able to wait for the threads termination
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
    pthread_attr_t attr;
    struct sched_param sched_param;
    os_result rv = os_resultSuccess;
    os_threadContext *threadContext;
    os_threadAttr tattr;
    int result, create_ret;
    int policy;

    assert (threadId != NULL);
    assert (name != NULL);
    assert (threadAttr != NULL);
    assert (start_routine != NULL);
    tattr = *threadAttr;

    if (tattr.schedClass == OS_SCHED_DEFAULT) {
#ifndef PIKEOS_POSIX
#ifndef VXWORKS_RTP
        tattr.schedClass = os_procAttrGetClass ();
#endif
        tattr.schedPriority = os_procAttrGetPriority ();
#endif
    }
    if (pthread_attr_init (&attr) != 0)
    {
       rv = os_resultFail;
    }
    else
    {
       if (pthread_getschedparam(pthread_self(), &policy, &sched_param) != 0 ||
#if !defined (OS_RTEMS_DEFS_H) && !defined (PIKEOS_POSIX)
           pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM) != 0 ||
#endif
           pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE) != 0 ||
           pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED) != 0)
       {
          rv = os_resultFail;
       }
       else
       {
          if (tattr.stackSize != 0) {
#ifdef PTHREAD_STACK_MIN
             if ( tattr.stackSize < PTHREAD_STACK_MIN ) {
                tattr.stackSize = PTHREAD_STACK_MIN;
             }
#endif
#ifdef OSPL_STACK_MAX
             if ( tattr.stackSize > OSPL_STACK_MAX ) {
                tattr.stackSize = OSPL_STACK_MAX;
             }
#endif
             if (pthread_attr_setstacksize (&attr, tattr.stackSize) != 0) {
                rv = os_resultFail;
             }
          }
       }

       if (rv == os_resultSuccess) {
          if (tattr.schedClass == OS_SCHED_REALTIME) {
             result = pthread_attr_setschedpolicy (&attr, SCHED_FIFO);

             if (result != 0) {
                OS_REPORT_3 (OS_WARNING, "os_threadCreate", 2,
                             "pthread_attr_setschedpolicy failed for SCHED_FIFO with "\
                             "error %d (%s) for thread '%s', reverting to SCHED_OTHER.",
                             result, strerror(result), name);

                result = pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
                if (result != 0) {
                   OS_REPORT_2 (OS_WARNING, "os_threadCreate", 2, "pthread_attr_setschedpolicy failed with error %d (%s)", result, name);
                }
             }
          } else {
             result = pthread_attr_setschedpolicy (&attr, SCHED_OTHER);

             if (result != 0) {
                OS_REPORT_2 (OS_WARNING, "os_threadCreate", 2,
                             "pthread_attr_setschedpolicy failed with error %d (%s)",
                             result, name);
             }
          }
          pthread_attr_getschedpolicy(&attr, &policy);

          if ((tattr.schedPriority < sched_get_priority_min(policy)) ||
              (tattr.schedPriority > sched_get_priority_max(policy))) {
             OS_REPORT_1 (OS_WARNING, "os_threadCreate", 2,
                          "scheduling priority outside valid range for the policy "\
                          "reverted to valid value (%s)", name);
             sched_param.sched_priority = (sched_get_priority_min(policy) +
                                           sched_get_priority_max(policy)) / 2;
          } else {
             sched_param.sched_priority = tattr.schedPriority;
          }
          /* Take over the thread context: name, start routine and argument */
          threadContext = os_malloc (sizeof (os_threadContext));
          threadContext->threadName = os_malloc (strlen (name)+1);
          os_strncpy (threadContext->threadName, name, strlen (name)+1);
          threadContext->startRoutine = start_routine;
          threadContext->arguments = arg;

          /* start the thread */
          result = pthread_attr_setschedparam (&attr, &sched_param);
          if (result != 0) {
             OS_REPORT_2 (OS_WARNING, "os_threadCreate", 2,
                          "pthread_attr_setschedparam failed with error %d (%s)",
                          result, name);
          } 

          create_ret = pthread_create(threadId, &attr, os_startRoutineWrapper,
                                      threadContext);
          if (create_ret != 0) {
             /* In case real-time thread creation failed due to a lack
              * of permissions, try reverting to time-sharing and continue.
              */
             if((create_ret == EPERM) && (tattr.schedClass == OS_SCHED_REALTIME))
             {
                OS_REPORT_1 (OS_WARNING, "os_threadCreate", 2,
                             "pthread_create failed with SCHED_FIFO "     \
                             "for thread '%s', reverting to SCHED_OTHER.",
                             name);
                pthread_attr_setschedpolicy (&attr, SCHED_OTHER);
                pthread_attr_getschedpolicy(&attr, &policy);
              
                if ((tattr.schedPriority < sched_get_priority_min(policy)) ||
                    (tattr.schedPriority > sched_get_priority_max(policy)))
                {
                   OS_REPORT_1 (OS_WARNING, "os_threadCreate", 2,
                                "scheduling priority outside valid range for the " \
                                "policy reverted to valid value (%s)", name);
                   sched_param.sched_priority =
                   (sched_get_priority_min(policy) +
                    sched_get_priority_max(policy)) / 2;
                } else {
                   sched_param.sched_priority = tattr.schedPriority;
                }

                result = pthread_attr_setschedparam (&attr, &sched_param);
                if (result != 0) {
                   OS_REPORT_2 (OS_WARNING, "os_threadCreate", 2,
                                "pthread_attr_setschedparam failed "      \
                                "with error %d (%s)", result, name);
                } else {
                   create_ret = pthread_create(threadId, &attr,
                                               os_startRoutineWrapper, threadContext);
                }
             }
          } else {
             rv = os_resultSuccess;
          }
          if(create_ret != 0){
             os_free (threadContext->threadName);
             os_free (threadContext);
             OS_REPORT_2 (OS_WARNING, "os_threadCreate", 2, "pthread_create failed with error %d (%s)", create_ret, name);
             rv = os_resultFail;
          }
       }
       pthread_attr_destroy (&attr);
    }
    return rv;
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
 * calling \b pthread_self.
 */
os_threadId
os_threadIdSelf (
    void)
{
    return pthread_self ();
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
    char *threadName;

    threadName = pthread_getspecific (os_threadNameKey);
    if (threadName != NULL) {
        size = snprintf (threadIdentity, threadIdentitySize, "%s "PA_ADDRFMT, threadName, (PA_ADDRCAST)pthread_self ());
    } else {
        size = snprintf (threadIdentity, threadIdentitySize, PA_ADDRFMT, (PA_ADDRCAST)pthread_self ());
    }
    return size;
}

#ifndef VXWORKS_RTP
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
    int result;

    assert (threadId);
    result = pthread_join (threadId, thread_result);
    if (result != 0) {
        OS_REPORT_1 (OS_ERROR, "os_threadWaitExit", 2, "pthread_join failed with error %d", result);
        rv = os_resultFail;
    } else {
        rv = os_resultSuccess;
    }
    return rv;
}
#endif

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
os_threadMemMalloc (
    os_int32 index,
    os_size_t size)
{
    void **pthreadMemArray;
    void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        pthreadMemArray = pthread_getspecific (os_threadMemKey);
        if (pthreadMemArray == NULL) {
	        os_threadMemInit ();
            pthreadMemArray = pthread_getspecific (os_threadMemKey);
	    }
        if (pthreadMemArray != NULL) {
            if (pthreadMemArray[index] == NULL) {
                threadMemLoc = os_malloc (size);
	            if (threadMemLoc != NULL) {
	                pthreadMemArray[index] = threadMemLoc;
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
    void **pthreadMemArray;
    void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        pthreadMemArray = pthread_getspecific (os_threadMemKey);
        if (pthreadMemArray != NULL) {
            threadMemLoc = pthreadMemArray[index];
            if (threadMemLoc != NULL) {
                pthreadMemArray[index] = NULL;
                os_free (threadMemLoc);
            }
        }
    }
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
    void **pthreadMemArray;
    void *threadMemLoc = NULL;

    if ((0 <= index) && (index < OS_THREAD_MEM_ARRAY_SIZE)) {
        pthreadMemArray = pthread_getspecific (os_threadMemKey);
        if (pthreadMemArray != NULL) {
            threadMemLoc = pthreadMemArray[index];
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
#ifndef INTEGRITY
    if ((result == os_resultSuccess) && (pi->protectCount == 1)) {
        if (pthread_sigmask(SIG_SETMASK,
                         &os_threadBlockAllMask,
                         &pi->oldMask) != 0) {
            result = os_resultFail;
        }
    }
#endif
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
#ifndef INTEGRITY
        if (pi->protectCount == 0) {
            if (pthread_sigmask(SIG_SETMASK,&pi->oldMask,NULL) != 0) {
                result = os_resultFail;
            } else {
                result = os_resultSuccess;
            }
        } else {
#endif
            result = os_resultSuccess;
#ifndef INTEGRITY
        }
#endif
    } else {
        result = os_resultFail;
    }

    return result;
}
