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
/** \file os/win32/code/os_mutex.c
 *  \brief WIN32 mutual exclusion semaphore
 *
 * Implements mutual exclusion semaphore for WIN32
 */
#include "os_mutex.h"
#include "os_stdlib.h"
#include "os_init.h"
#include "os_heap.h"
#include "code/os__debug.h"
#include "code/os__service.h"
#include "code/os__sharedmem.h"
#include "code/os__mutex.h"

#include <stdio.h>
#include <assert.h>

#include "../common/code/os_mutex_attr.c"

#include "ut_collection.h"

static ut_table eventCache = NULL;
static os_mutex eventCacheMutex;

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

/* addCacheItem.  precondition : cache is locked, and the mutex does not exist in the cache */
static void addCacheItem (os_mutex * mutex, HANDLE h)
{
    struct cache_item * c = malloc (sizeof (struct cache_item));
    if (c)
    {
        c->h = h;
        c->lifecycleId = mutex->lifecycleId;
        c->id = mutex->id;
        (void) ut_tableInsert (eventCache, mutex, c);
    }
    else
    {
        fprintf (stdout, "error : addCacheItem unable to malloc\n");
    }
}

/** freeCachedItem. precondition : cache is locked */
static void freeCachedItem (os_mutex * mutex)
{
    struct cache_item * c;

    c = (struct cache_item*) ut_get (ut_collection(eventCache), mutex);
    if (c)
    {
        if (c->h)
        {
            CloseHandle (c->h);
        }

        ut_remove (ut_collection(eventCache), mutex);
        free (c);
    }
}

static HANDLE createNewHandle (os_mutex * mutex)
{
    HANDLE result = NULL;
    char name[OS_SERVICE_ENTITY_NAME_MAX];
    wchar_t* wStringName;

    _snprintf(name, OS_SERVICE_ENTITY_NAME_MAX, "%s%d%d",
                OS_SERVICE_EVENT_NAME_PREFIX, mutex->id, os_getShmBaseAddressFromPointer(mutex));
    wStringName = wce_mbtowc(name);
    result = OpenEvent(EVENT_ALL_ACCESS, FALSE, wStringName);
    os_free (wStringName);

    return result;
}

static void close_cached_item (void *obj, void *arg)
{
    struct cache_item *c = (struct cache_item*)obj;
    CloseHandle ((HANDLE) c->h);
    free (c);
}

HANDLE get_mutex_handle (os_mutex *mutex)
{
    struct cache_item * c;
    HANDLE h = NULL;

    /* lookup the mutex handle from the local cache.  If it doesnt exist
     * create it the expensive way but cache it for likely future calls
     */

    os_mutexLock(&eventCacheMutex);
    c = (struct cache_item*) ut_get (ut_collection(eventCache), mutex);

    if (c)
    {
        h = (HANDLE)c->h;

        /* check that the lifecycleIds of the item cached is the same as the
         * requested item.  If not, then we have a stale reference in the cache.
         * In this case, remove this item from the cache and create another
         * new handle that is current
         */
        if (c->id != mutex->id || c->lifecycleId != mutex->lifecycleId)
        {
            freeCachedItem (mutex);
            h = createNewHandle (mutex);
            addCacheItem (mutex, h);
        }
    }
    else
    {
        h = createNewHandle (mutex);
        addCacheItem (mutex, h);
    }

    os_mutexUnlock(&eventCacheMutex);
    assert (h != NULL);

    return h;
}

/** \brief Initialize mutex module
 */
void os_mutexModuleInit (void)
{
    os_mutexInit(&eventCacheMutex, NULL);

    eventCache = ut_tableNew (address_cmp, NULL, NULL, NULL, close_cached_item, NULL);
}

/** \brief Deinitialize mutex module
 */
void os_mutexModuleExit (void)
{
    ut_tableFree (eventCache);
    os_mutexDestroy(&eventCacheMutex);
}

/*** Public functions *****/

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call.
 *
 * Not (yet) supported on this platform
 */
os_result
os_mutexSetPriorityInheritanceMode(
    os_boolean enabled)
{
    /* Priority Inheritance is not supported on this platform (yet) */
    return os_resultSuccess;
}

/** \brief Initialize the mutex taking the mutex attributes
 *         into account
 *
 * \b os_mutexInit calls \b pthread_mutex_init to intialize the
 * posix \b mutex
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * mutex "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implictly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi process variables in
 * single process mode.
 */
os_result
os_mutexInit (
    os_mutex *mutex,
    const os_mutexAttr *mutexAttr)
{
    struct os_servicemsg request;
    struct os_servicemsg reply;
    os_result osr;
    char * writeQueueName;
    HANDLE h;

    assert(mutex != NULL);

    if(!mutexAttr) {
        os_mutexAttr defAttr;
        os_mutexAttrInit(&defAttr);
        mutex->scope = defAttr.scopeAttr;
    } else {
        mutex->scope = mutexAttr->scopeAttr;
    }
    mutex->lockCount = 0;

    if (mutex->scope == OS_SCOPE_SHARED)
    {
        request.kind = OS_SRVMSG_CREATE_EVENT;
        writeQueueName = os_createPipeNameFromMutex(mutex);
        osr = requestResponseFromServiceQueue (&request, &reply, writeQueueName);

        if (osr == os_resultSuccess)
        {
            if ((reply.result == os_resultSuccess) && (reply.kind == OS_SRVMSG_CREATE_EVENT))
            {
                mutex->id = reply._u.id;
                mutex->lifecycleId = reply.lifecycleId;

                /* Create and cache a local handle to the Event */
                h = createNewHandle (mutex);
                if (h)
                {
                    os_mutexLock(&eventCacheMutex);
                    /* Add to the cache, first removing any existing reference to the mutex */
                    freeCachedItem (mutex);
                    addCacheItem (mutex, h);
                    os_mutexUnlock(&eventCacheMutex);
                }
                osr = (h != NULL) ? os_resultSuccess : os_resultFail;
            }
            else
            {
               osr = os_resultFail;
            }
        }
        os_free (writeQueueName);
    }
    else
    {
        /* private so don't get one from pool */
        mutex->id = (long)CreateEvent(NULL, FALSE, FALSE, NULL);
        osr = os_resultSuccess;
    }

    return osr;
}

/** \brief Destroy the mutex
 *
 * \b os_mutexDestroy calls \b pthread_mutex_destroy to destroy the
 * posix \b mutex.
 */
void
os_mutexDestroy (
    os_mutex *mutex)
{
    struct os_servicemsg request;
    struct os_servicemsg reply;
    os_result osr;
    char * writeQueueName;
    HANDLE h = NULL;

    assert(mutex != NULL);

    if (mutex->scope == OS_SCOPE_SHARED)
    {
        request.kind = OS_SRVMSG_DESTROY_EVENT;
        request._u.id = mutex->id;
        writeQueueName = os_createPipeNameFromMutex(mutex);
        osr = requestResponseFromServiceQueue (&request, &reply, writeQueueName);

        if (osr == os_resultSuccess)
        {
            if ((reply.result == os_resultSuccess) && (reply.kind == OS_SRVMSG_DESTROY_EVENT))
            {
                /* Remove from the cache the local handle to the Event */
                os_mutexLock(&eventCacheMutex);
                freeCachedItem (mutex);
                os_mutexUnlock(&eventCacheMutex);
            }
            else
            {
                abort ();
            }
        }
        os_free (writeQueueName);
    }
    else
    {
        /* private so don't return to pool */
        CloseHandle((HANDLE)mutex->id);
    }
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result
os_mutexLock_s(
    os_mutex *mutex)
{
    HANDLE mutexHandle;
    DWORD result;
    os_result r;
    long lc;

    assert(mutex != NULL);

    r = os_resultSuccess;
    lc = InterlockedIncrement(&mutex->lockCount);
    if (lc > 1)
    {
        if (mutex->scope == OS_SCOPE_SHARED)
        {
            mutexHandle = get_mutex_handle (mutex);
        }
        else
        {
            mutexHandle = (HANDLE)mutex->id;
        }
        result = WaitForSingleObject(mutexHandle, INFINITE);
        assert(result == WAIT_OBJECT_0);
        if (result != WAIT_OBJECT_0)
        {
            r = os_resultFail;
        }
    }

    return r;
}

void
os_mutexLock(
    os_mutex *mutex)
{
    if (os_mutexLock_s (mutex) != os_resultSuccess) {
        abort ();
    }
}

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 *
 * \b os_mutexTryLock calls \b pthread_mutex_trylock to acquire
 * the posix \b mutex.
 */
os_result
os_mutexTryLock (
    os_mutex *mutex)
{
    os_result r;
    long lc;

    assert(mutex != NULL);

    r = os_resultSuccess;
    lc = InterlockedCompareExchange(&mutex->lockCount, 1, 0);
    if (lc > 0) {
        r = os_resultBusy;
    }
    return r;

}

/** \brief Release the acquired mutex
 *
 * \b os_mutexUnlock calls \b pthread_mutex_unlock to release
 * the posix \b mutex.
 */
void
os_mutexUnlock (
    os_mutex *mutex)
{
    HANDLE mutexHandle;
    long lc;
    BOOL r;

    assert(mutex != NULL);

    lc = InterlockedDecrement(&mutex->lockCount);
    if (lc > 0)
    {
        if (mutex->scope == OS_SCOPE_SHARED)
        {
            mutexHandle = get_mutex_handle (mutex);
        }
        else
        {
            mutexHandle = (HANDLE)mutex->id;
        }
        r = SetEvent(mutexHandle);
        if (r == FALSE)
        {
            abort ();
        }
    }
}
