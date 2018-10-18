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
/** \file os/win32/code/os_cond.c
 *  \brief WIN32 condition varaibles
 *
 * Implements condition variables
 */


#include "os_cond.h"
#include "os_stdlib.h"
#include "os_init.h"
#include "os_heap.h"
#include "os_mutex.h"
#include "code/os__debug.h"
#include "code/os__service.h"
#include "code/os__sharedmem.h"
#include "code/os__mutex.h"
#include "os_errno.h"

#include <stdio.h>
#include <assert.h>

#include "../common/code/os_cond_attr.c"

#define BROADCAST_BIT_MASK (0x40000000)
#define SIGNAL_BIT_MASK    (0x20000000)
#define WAITCOUNT_MASK     (0x9fffffff)

#define NOTIFIED_BY_BROADCAST(state) ((state&BROADCAST_BIT_MASK)==BROADCAST_BIT_MASK)
#define WAITCOUNT(state) (state&WAITCOUNT_MASK)

#include "ut_collection.h"

static ut_table semCache = NULL;
static os_mutex semCacheMutex;

struct cache_item {
    HANDLE h;
    long lifecycleId;
    long id;
};

static os_equality address_cmp (void *o1, void *o2, void *arg)
{
    os_equality result = OS_EQ;

    if (o1 < o2)
    {
        result = OS_LT;
    }
    else if (o1 > o2)
    {
        result = OS_GT;
    }
    return result;
}

/* addCacheItem.  precondition : cache is locked, and the cond does not exist in the cache */
static void addCacheItem (os_cond * cond, HANDLE h)
{
    struct cache_item * c = malloc (sizeof (struct cache_item));
    if (c)
    {
        c->h = h;
        c->lifecycleId = cond->lifecycleId;
        c->id = cond->qId;
        (void) ut_tableInsert (semCache, cond, c);
    }
    else
    {
        fprintf (stdout, "error : addCacheItem unable to malloc\n");
    }
}

/** freeCachedItem. precondition : cache is locked */
static void freeCachedItem (os_cond * cond)
{
    struct cache_item * c;

    c = (struct cache_item*) ut_get (ut_collection(semCache), cond);
    if (c)
    {
        if (c->h)
        {
            CloseHandle (c->h);
        }

        ut_remove (ut_collection(semCache), cond);
        free (c);
    }
}

static HANDLE createNewHandle (os_cond * cond)
{
    HANDLE result = NULL;
    char name[OS_SERVICE_ENTITY_NAME_MAX];
    wchar_t* wStringName;

    _snprintf(name, sizeof(name), "%s%d%d",
                OS_SERVICE_SEM_NAME_PREFIX, cond->qId, os_getShmBaseAddressFromPointer(cond));
    wStringName = wce_mbtowc(name);
    result = CreateSemaphore(NULL, 0, 0x7fffffff, wStringName);
    os_free (wStringName);

    return result;
}

static void close_cached_item (void *obj, void *arg)
{
    struct cache_item *c = (struct cache_item*)obj;
    CloseHandle ((HANDLE) c->h);
    free (c);
}

static HANDLE get_semaphore_handle (os_cond *cond)
{
    struct cache_item *c = NULL;
    HANDLE h = NULL;

    /* lookup the semaphore handle from the local cache.  If it doesnt exist
     * create it the expensive way but cache it for likely future calls
     */
    os_mutexLock(&semCacheMutex);

    c = (struct cache_item*) ut_get (ut_collection(semCache), cond);
    if (c)
    {
        h = (HANDLE)c->h;

        /* check that the lifecycleIds of the item cached is the same as the
         * requested item.  If not, then we have a stale reference in the cache.
         * In this case, remove this item from the cache and create another
         * new handle that is current
         */
        if (c->id != cond->qId  || c->lifecycleId != cond->lifecycleId)
        {
            freeCachedItem (cond);
            h = createNewHandle (cond);
            addCacheItem (cond, h);
        }
    }
    else
    {
        h = createNewHandle (cond);
        addCacheItem (cond, h);
    }

    os_mutexUnlock(&semCacheMutex);
    assert (h != NULL);

    return h;
}

/** \brief Initialize cond module
 */
void os_condModuleInit (void)
{
    os_mutexInit(&semCacheMutex, NULL);

    semCache = ut_tableNew (address_cmp, NULL, NULL, NULL, close_cached_item, NULL);
}

/** \brief Deinitialize cond module
 */
void os_condModuleExit (void)
{
    ut_tableFree (semCache);
    os_mutexDestroy(&semCacheMutex);
}

/************ PRIVATE ****************/

#if (_WIN32_WCE==0x600)
/* Windows CE 6.0 does not have its own InterlockedAnd or InterlockedOr */
static LONG
InterlockedAnd(
    __inout LONG volatile *t,
    __in LONG mask)
{
    LONG j;
    LONG i;
    j = *t;
    do {
        i = j;
        j = InterlockedCompareExchange(t,i&mask,i);
    } while (i != j);
    return j;
}

static LONG
InterlockedOr(
    __inout LONG volatile *t,
    __in LONG mask)
{
    LONG j;
    LONG i;
    j = *t;
    do {
        i = j;
        j = InterlockedCompareExchange(t,i|mask,i);
    } while (i != j);
    return j;
}
#endif

static os_result
getSem(
    os_cond* cond)
{
    struct os_servicemsg request;
    struct os_servicemsg reply;
    os_result osr;
    char * writeQueueName;
    HANDLE h;

    request.kind = OS_SRVMSG_CREATE_SEMAPHORE;
    writeQueueName = os_createPipeNameFromCond(cond);
    osr = requestResponseFromServiceQueue (&request, &reply, writeQueueName);

    if (osr == os_resultSuccess)
    {
        if ((reply.result == os_resultSuccess) && (reply.kind == OS_SRVMSG_CREATE_SEMAPHORE))
        {
            cond->qId = reply._u.id;
            cond->lifecycleId = reply.lifecycleId;

            /* Create and cache a local handle to the Semaphore */
            h = createNewHandle (cond);
            if (h)
            {
                os_mutexLock(&semCacheMutex);
                /* Add to the cache, first removing any existing reference to the cond */
                freeCachedItem (cond);
                addCacheItem (cond, h);
                os_mutexUnlock(&semCacheMutex);
            }
            osr = (h != NULL) ? os_resultSuccess : os_resultFail;
        }
        else
        {
            osr = os_resultFail;
        }
    }
    os_free (writeQueueName);

    return osr;
}

static os_result
returnSem(
    os_cond* cond)
{
    struct os_servicemsg request;
    struct os_servicemsg reply;
    os_result osr;
    char * writeQueueName;
    HANDLE h = NULL;

    request.kind = OS_SRVMSG_DESTROY_SEMAPHORE;
    request._u.id = cond->qId;
    writeQueueName = os_createPipeNameFromCond(cond);

    osr = requestResponseFromServiceQueue (&request, &reply, writeQueueName);

    if (osr == os_resultSuccess)
    {
        if ((reply.result == os_resultSuccess) && (reply.kind == OS_SRVMSG_DESTROY_SEMAPHORE))
        {
            /* Remove from the cache the local handle to the Semaphore */
            os_mutexLock(&semCacheMutex);
            freeCachedItem (cond);
            os_mutexUnlock(&semCacheMutex);
        }
        else
        {
            OS_DEBUG_4("returnSem", "Failure %d %d %d %d\n", result, os_getErrno(), nRead, reply.kind);
            osr = os_resultFail;
        }
    }

    os_free (writeQueueName);

    return osr;
}

static os_result
condTimedWait(
    os_cond *cond,
    os_mutex *mutex,
    DWORD timeout)
{
    HANDLE hQueue;
    HANDLE hMtx;
    DWORD wsr;
    LONG c;
    LONG lockCount;
    os_result osr;
    os_result result;

    assert(cond != NULL);
    assert(mutex != NULL);

    result = os_resultSuccess;
    if (cond->scope == OS_SCOPE_SHARED) {
        hQueue = get_semaphore_handle(cond);
        hMtx = get_mutex_handle (mutex);
    } else {
        hQueue  = (HANDLE)cond->qId;
        hMtx  = (HANDLE)mutex->id;
    }

    InterlockedIncrement(&cond->state);
    lockCount = InterlockedDecrement(&mutex->lockCount);
    if (lockCount > 0) {
        SetEvent(hMtx);
        wsr = WaitForSingleObject(hQueue, timeout);
    } else {
        wsr = WaitForSingleObject(hQueue, timeout);
    }
    assert((wsr == WAIT_OBJECT_0) || (wsr == WAIT_FAILED) || (wsr == WAIT_ABANDONED) || (wsr == WAIT_TIMEOUT));
    if (wsr == WAIT_TIMEOUT) {
        result = os_resultTimeout;
    } else if (wsr != WAIT_OBJECT_0) {
        result = os_resultFail;
        abort ();
    }

    c = InterlockedDecrement(&cond->state);
    os_mutexLock(mutex);

    return result;
}

static void
condSignal(
    os_cond *cond,
    long mask)
{
    HANDLE hQueue;
    DWORD result;
    long oldState;

    assert(cond != NULL);

    if (cond->scope == OS_SCOPE_SHARED) {
        hQueue = get_semaphore_handle(cond);
    } else {
        hQueue = (HANDLE)cond->qId;
    }

    oldState = InterlockedOr(&cond->state, mask);
    if (oldState == 0) { /* no waiters */
        InterlockedAnd(&cond->state, ~mask);
        return;
    }

    if (mask == BROADCAST_BIT_MASK) {
        result = ReleaseSemaphore(hQueue, oldState, 0);
    } else {
        result = ReleaseSemaphore(hQueue, 1, 0);
    }

    InterlockedAnd(&cond->state, ~mask);
}

/************ PUBLIC *****************/

/** \brief Initialize the condition variable taking the condition
 *         attributes into account
 *
 * \b os_condInit calls \b pthread_cond_init to intialize the posix condition
 * variable.
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * condition variable "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implictly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi process variables in
 * single process mode.
 */
os_result
os_condInit (
    os_cond *cond,
    os_mutex *dummymtx,
    const os_condAttr *condAttr)
{
    os_result result;

    assert (cond != NULL);

    if(!condAttr) {
        os_condAttr defAttr;

        os_condAttrInit(&defAttr);
        cond->scope = defAttr.scopeAttr;
    } else {
        cond->scope = condAttr->scopeAttr;
    }

    cond->state = 0;

    if (cond->scope == OS_SCOPE_SHARED) {
        return getSem(cond);
    } else {
        cond->qId = (long)CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
        if ((HANDLE)cond->qId == NULL) {
            return os_resultFail;
        } else {
            return os_resultSuccess;
        }
    }
}

/** \brief Destroy the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
void
os_condDestroy(
    os_cond *cond)
{
    os_result result;

    assert (cond != NULL);

    if (cond->scope == OS_SCOPE_SHARED) {
        result = returnSem(cond);
        if(result != os_resultSuccess) {
            abort();
        }
    } else {
        CloseHandle((HANDLE)cond->qId);
    }
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
void
os_condWait (
    os_cond *cond,
    os_mutex *mutex)
{
    (void) condTimedWait(cond, mutex, INFINITE);
}

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 *
 * \b os_condTimedWait calls \b pthread_cond_timedwait to
 * wait for the condition with a timeout.
 *
 * \b os_condTimedWait provides an relative time to wait while
 * \b pthread_cond_timedwait expects an absolute time to wakeup.
 * The absolute time is calculated from the current time + the
 * provided relative time.
 *
 * \b os_condTimedWait will repeat \b pthread_cond_timedwait in case of an
 * interrupted system call. Because the time which is passed onto
 * \b pthread_cond_timedwait is absolute, no remaining time must be
 * calculated.
 */
os_result
os_condTimedWait (
    os_cond *cond,
    os_mutex *mutex,
    os_duration timeout)
{
    DWORD wait_time;

    assert (cond != NULL);
    assert (mutex != NULL);
    assert (OS_DURATION_ISPOSITIVE(timeout));

    if (timeout < 0) {
        wait_time = 0;
    } else {
        wait_time = (DWORD)(timeout/1000000);
    }

    return condTimedWait(cond, mutex, wait_time);
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * \b os_condSignal calls \b pthread_cond_signal to signal
 * the condition.
 */
void
os_condSignal (
    os_cond *cond)
{
    condSignal(cond, SIGNAL_BIT_MASK);
}

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 *
 * \b os_condBroadcast calls \b pthread_cond_broadcast to broadcast
 * the condition.
 */
void
os_condBroadcast (
    os_cond *cond)
{
    condSignal(cond, BROADCAST_BIT_MASK);
}
