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

/** \file os/vxworks5.5/code/os_mutex.c
 *  \brief vxWorks mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for vxWorks
 * by including the POSIX implementation
 */

/** \file os/posix/code/os_mutex.c
 *  \brief Posix mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for POSIX
 */

#include "os_atomics.h"
#include <vxWorks.h>
#include <objLib.h>
#include <fioLib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "code/os__mutex.h"
#include "os_mutex.h"
#include "os_heap.h"
#include "os_init.h"
#include <assert.h>
#include "os_errno.h"
#include "os_signature.h"
#include "ut_collection.h"
#include <stdio.h>
#include <taskLib.h>

static ut_collection cache = NULL;
static SEM_ID cache_lock;
static os_boolean ospl_mtx_prio_inherit = OS_FALSE;
static SEM_ID invalid_sem_id = (SEM_ID)1;

static void mutex_panic (const char *msg, int status)
{
    printf ("ERROR: os_mutex: %s (%d, %d) - suspending task\n", msg, status, os_getErrno ());
    fflush (stdout);
    taskSuspend (taskIdSelf ());
}

static void cas_mutex_info_id (os_mutex *mutex, SEM_ID expected, SEM_ID id)
{
    SEM_ID cached_id;
    do {
        cached_id = pa_ldvoidp ((void*) &mutex->info.id);
        if (cached_id != expected) {
            mutex_panic ("sem id mismatch in cas_mutex_info_id", 0);
        }
    } while (!pa_casvoidp ((void*) &mutex->info.id, expected, id));
}

static os_equality address_cmp (void *o1, void *o2, void *arg)
{
    os_equality result = OS_EQ;
    (void)arg;
    if (o1 < o2) {
        result = OS_LT;
    } else if (o1 > o2) {
        result = OS_GT;
    }
    return result;
}

static void free_sem (void *sem, void *arg)
{
    (void)arg;
    semClose ((SEM_ID) sem);
}

static SEM_ID get_mutex (os_mutex *mutex)
{
    SEM_ID result;
    int options;
    semTake (cache_lock, WAIT_FOREVER);
    result = (SEM_ID) ut_get (cache, mutex);

    if (!result)
    {
        /* In single process mode only "private" variables are required */
        options = (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
        /* Set option for priority inheritance if feature is enabled */
        if (ospl_mtx_prio_inherit) {
            options = options | SEM_INVERSION_SAFE;
        }
        result = semOpen(mutex->name, SEM_TYPE_MUTEX, SEM_EMPTY, options, 0, NULL);
        if (result) {
            (void) ut_tableInsert ((ut_table) cache, mutex, result);
        } else {
            mutex_panic ("get_mutex", 0);
        }
    }

    semGive (cache_lock);
    return result;
}

/** \brief Initialize mutex module
 */
void os_mutexModuleInit (void)
{
    cache = (ut_collection) ut_tableNew (address_cmp, NULL, NULL, NULL, free_sem, NULL);
    cache_lock = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
}

/** \brief Deinitialize mutex module
 */
void os_mutexModuleExit (void)
{
    ut_tableFree ((ut_table) cache);
    semDelete (cache_lock);
}

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call.
 *
 * Store the setting in the static variable to be used when MutexInit is called.
 */
os_result os_mutexSetPriorityInheritanceMode (os_boolean enabled)
{
    ospl_mtx_prio_inherit = enabled;
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
 */

os_result os_mutexInit (os_mutex *mutex, const os_mutexAttr *mutexAttr)
{
    os_mutexAttr defAttr;
    os_result rv;
    int options;
    SEM_ID id;

    assert (mutex != NULL);
    #ifdef OSPL_STRICT_MEM
    assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif

    if(!mutexAttr) {
        os_mutexAttrInit(&defAttr);
        mutexAttr = &defAttr;
    }

    options = (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
    /* Set option for priority inheritance if feature is enabled */
    if (ospl_mtx_prio_inherit) {
        options = options | SEM_INVERSION_SAFE;
    }
    if (mutexAttr->scopeAttr == OS_SCOPE_SHARED) {
        /* create named mutex using shared address as unique name */
        snprintf (mutex->name, sizeof (mutex->name), "/%lu", (os_uint32) mutex);
        id = semOpen (mutex->name, SEM_TYPE_MUTEX, SEM_EMPTY, options, OM_CREATE|OM_EXCL, NULL);
        /* As it may exist from a previous run. remove leftover semaphore object */
        if (id != NULL) {
            /* a fresh one, initialise it and cache the SEM_ID */
            pa_st32 ((void *) &mutex->info.id, 0);
            semTake (cache_lock, WAIT_FOREVER);
            (void) ut_tableInsert ((ut_table) cache, mutex, id);
            semGive (cache_lock);
        } else {
            /* re-using an old one: it should have been destroyed and hence should have a marker in info.id */
            id = get_mutex (mutex);
            if (id != NULL) {
                semTake (id, WAIT_FOREVER);
                semGive (id);
                /*cas_mutex_info_id ((void*)&mutex->info.id, invalid_sem_id, 0);*/
            } else {
                mutex_panic ("init: attempt to reuse semaphore currently in use", 0);
            }
        }

        rv = (id != NULL) ? os_resultSuccess : os_resultFail;
    } else {
        int result;
        mutex->name[0] = '\0';
        /* TODO attrs */
        result = pthread_mutex_init(&mutex->info.posix_mutex, NULL);
        rv = (result == 0) ? os_resultSuccess : os_resultFail;
    }

#ifdef OSPL_STRICT_MEM
    mutex->signature = OS_MUTEX_MAGIC_SIG;
#endif
    return rv;
}

/** \brief Destroy the mutex
 *
 * \b os_mutexDestroy calls \b pthread_mutex_destroy to destroy the
 * posix \b mutex.
 */
void os_mutexDestroy (os_mutex *mutex)
{
    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    if (mutex->name[0] != '\0') {
        SEM_ID id;

        /* shared mutex */
        id = ut_get (cache, mutex);
        cas_mutex_info_id (mutex, 0, invalid_sem_id);

        if (id) {
            semClose (id);
            semTake(cache_lock, WAIT_FOREVER);
            ut_remove (cache, mutex);
            semGive(cache_lock);
        }
    } else {
        int res = pthread_mutex_destroy (&mutex->info.posix_mutex);
        if (res != 0) {
            mutex_panic ("pthread_mutex_destroy failed", res);
        }
    }

#ifdef OSPL_STRICT_MEM
    mutex->signature = 0;
#endif
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result os_mutexLock_s (os_mutex *mutex)
{
    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
    if (mutex->name[0] != '\0') {
        STATUS result;
        SEM_ID id;
        id = get_mutex (mutex);
        result = semTake (id, WAIT_FOREVER);
        if (result == ERROR) {
            return os_resultFail;
        }
        cas_mutex_info_id (mutex, 0, id);
    } else {
        int res = pthread_mutex_lock (&mutex->info.posix_mutex);
        if (res != 0) {
            return os_resultFail;
        }
    }
    return os_resultSuccess;
}

void os_mutexLock (os_mutex *mutex)
{
    if (os_mutexLock_s (mutex) != os_resultSuccess) {
        mutex_panic ("mutexLock failed", 0);
    }
}

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 *
 * \b os_mutexTryLock calls \b pthread_mutex_trylock to acquire
 * the posix \b mutex.
 */
os_result os_mutexTryLock (os_mutex *mutex)
{
    os_result rv = os_resultFail;

    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    if (mutex->name[0] != '\0') {
        STATUS result;
        SEM_ID id;

        id = get_mutex (mutex);
        result = semTake(id, NO_WAIT);

        if (result == OK) {
            cas_mutex_info_id (mutex, 0, id);
            rv = os_resultSuccess;
        } else {
            if (os_getErrno() == S_objLib_OBJ_UNAVAILABLE) {
                rv = os_resultBusy;
            } else {
                mutex_panic ("trylock: semTake failed", result);
            }
        }
    } else {
        int res = pthread_mutex_trylock (&mutex->info.posix_mutex);

        if (res == 0) {
            rv = os_resultSuccess;
        } else if (res == EBUSY) {
            rv = os_resultBusy;
        } else {
            mutex_panic ("pthread_mutex_trylock failed", res);
        }
    }

    return rv;
}

/** \brief Release the acquired mutex
 *
 * \b os_mutexUnlock calls \b pthread_mutex_unlock to release
 * the posix \b mutex.
 */
void os_mutexUnlock (os_mutex *mutex)
{
    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    if (mutex->name[0] != '\0') {
        STATUS status;
        SEM_ID id;
#if 1
        /* NB: id lookup in cache is for debugging reasons */
        id = get_mutex (mutex);
#else
        id = pa_ldvoidp (&mutex->info.id);
#endif
        cas_mutex_info_id (mutex, id, 0);
        status = semGive (id);
        if (status != OK) {
            mutex_panic("unlock: semGive failed", status);
        }
    } else {
        int res = pthread_mutex_unlock (&mutex->info.posix_mutex);
        if (res != 0) {
            mutex_panic ("unlock: semGive failed", res);
        }
    }
}
#include "../common/code/os_mutex_attr.c"
