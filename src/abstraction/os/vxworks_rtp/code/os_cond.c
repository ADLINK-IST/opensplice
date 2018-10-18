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

/** \file os/vxworks5.5/code/os_cond.c
 *  \brief vxWorks condition variables
 *
 * Implements condition variables for vxWorks
 * by including the POSIX implementation
 */

/** \file os/posix/code/os_cond.c
 *  \brief Posix condition variables
 *
 * Implements condition variables for POSIX, the condition variable
 * is mapped on the posix condition variable
 */

#include <version.h>
#include <stdio.h>
#include <objLib.h>
#include <sysLib.h>
#include <tickLib.h>
#include <memLib.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <assert.h>
#include "os_errno.h"

#include "os_cond.h"
#include "os_mutex.h"
#include "os_heap.h"
#include "os_init.h"
#include "os_atomics.h"
#include "ut_collection.h"
#ifdef OSPL_STRICT_MEM
#include "os_signature.h"
#endif

#include <taskLib.h>
#if ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 6 )
/* Vxworks 6.5 doesn't support __thread so alterate implemtation using tlsLib */
#define USE_TLS 1
#include <tlsLib.h>
#endif

#define MAX_TICKS (0xffffffffUL)
#define MAX_THREAD_ID_SIZE 12

typedef struct thread_key {
    os_os_cond *cond;
    int id;
} thread_key;

static ut_collection cache = NULL;
static ut_collection thread_cache = NULL;
static SEM_ID cache_lock;
#if USE_TLS
static TLS_KEY os_cond_thread_id_tss_key = NULL;
static TLS_KEY os_cond_id_tss_key = NULL;
#endif
static pa_uint32_t os_cond_thread_id = PA_UINT32_INIT (0);

static void cond_panic (const char *msg, int status)
{
    printf ("ERROR: os_cond: %s (%d, %d) - suspending task\n", msg, status, os_getErrno ());
    fflush (stdout);
    taskSuspend (taskIdSelf ());
}

static void cond_queue_full (os_cond *cond)
{
    static pa_uint32_t count = PA_UINT32_INIT (0);
    static pa_uint32_t print = PA_UINT32_INIT (1);
    os_uint32 c, p = pa_ld32 (&print);
    if ((c = pa_inc32_nv (&count)) >= p) {
        os_uint32 np = 2 * p + 1;
        while (!pa_cas32 (&print, p, np) && pa_ld32 (&print) == p) {
            /* only ~double it once per printf, allowing for spurious failures of CAS */
        }
        printf ("ERROR: os_cond: 0x%" PA_PRIxADDR " - queue full, taskDelay(1) [%u]\n", (os_address) cond, c);
        fflush (stdout);
    }
    taskDelay (1);
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

static os_equality thread_key_cmp (void *o1, void *o2, void *arg)
{
    os_equality result = OS_EQ;
    thread_key *key1 = (thread_key *) o1;
    thread_key *key2 = (thread_key *) o2;
    (void)arg;
    if (key1->cond < key2->cond) {
        result = OS_LT;
    } else if (key1->cond > key2->cond) {
        result = OS_GT;
    } else {
        if (key1->id < key2->id) {
            result = OS_LT;
        } else if (key1->id > key2->id) {
            result = OS_GT;
        }
    }
    return result;
}

static void free_sem (void *sem, void *arg)
{
    (void)arg;
    semClose ((SEM_ID) sem);
}

static void free_key (void *key, void *arg)
{
    (void)arg;
    free (key);
}

static int self (os_os_cond *cond, SEM_ID *thread_sem_id)
{
#if USE_TLS
    void *tlsData;
    int thread_id = -1;
    SEM_ID id;
    STATUS vxRet;
    if ((tlsData = tlsValueGet (os_cond_thread_id_tss_key)) != 0) {
        thread_id = (int) ((os_address) tlsData);
        id = tlsValueGet (os_cond_id_tss_key);
    }
#else
    static __thread int thread_id = -1;
    static __thread SEM_ID id;
#endif

    if (thread_id == -1)
    {
        /* create public semaphore accessible between RTPs */
        char sem_id[MAX_THREAD_ID_SIZE + 5];
        int pid = getpid();
        thread_id = pid + pa_inc32_nv(&os_cond_thread_id);
        snprintf (sem_id, sizeof (sem_id), "/cv-%u", thread_id);
        id = semOpen (sem_id, SEM_TYPE_BINARY, SEM_EMPTY, SEM_Q_FIFO, OM_CREATE | OM_DELETE_ON_LAST_CLOSE, NULL);
        if (id == NULL) { cond_panic ("self: semOpen", 0); }

        /* current thread owns cond binary semaphore at this point */
#if USE_TLS
        vxRet = tlsValueSet (os_cond_id_tss_key, id);
        if (vxRet != OK) { cond_panic ("self: tlsValueSet(cond_id)", vxRet); }
        vxRet = tlsValueSet (os_cond_thread_id_tss_key, (void *) ((os_address) thread_id));
        if (vxRet != OK) { cond_panic ("self: tlsValueSet(thread_id)", vxRet); }
#endif
    }

    *thread_sem_id = id;
    return thread_id;
}

static SEM_ID get_sem (os_os_cond *cond, int thread_id)
{
    SEM_ID result;
    thread_key key;

    key.cond = cond;
    key.id = thread_id;
    semTake (cache_lock, WAIT_FOREVER);
    result = (SEM_ID) ut_get (thread_cache, &key);
    if (!result) {
        char id[MAX_THREAD_ID_SIZE + 5];
        thread_key *new_key = malloc (sizeof (thread_key));

        snprintf (id, sizeof (id), "/cv-%u", thread_id);
        result = semOpen (id, SEM_TYPE_BINARY, SEM_FULL, SEM_Q_FIFO, OM_CREATE | OM_DELETE_ON_LAST_CLOSE, NULL);
        if (result == NULL) { cond_panic ("get_sem: semOpen", 0); }

        *new_key = key;
        (void) ut_tableInsert ((ut_table) thread_cache, new_key, result);
    }
    semGive (cache_lock);

    return result;
}

static SEM_ID get_cond_mutex (os_os_cond *cond)
{
    SEM_ID result;

    semTake (cache_lock, WAIT_FOREVER);
    result = (SEM_ID) ut_get (cache, cond);

    if (!result) {
        result = semOpen(cond->name, SEM_TYPE_BINARY, SEM_FULL, SEM_Q_FIFO, 0, NULL);
        if (result) {
            (void) ut_tableInsert ((ut_table) cache, cond, result);
        } else {
            cond_panic ("get_cond_mutex: semOpen", 0);
        }
    }

    semGive (cache_lock);
    return result;
}

/** \brief Initialize cond module
 */
void os_condModuleInit (void)
{
    cache = (ut_collection) ut_tableNew (address_cmp, NULL, NULL, NULL, free_sem, NULL);
    thread_cache = (ut_collection) ut_tableNew (thread_key_cmp, NULL, free_key, NULL, free_sem, NULL);
    cache_lock = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
#if USE_TLS
    if (os_cond_thread_id_tss_key == NULL) {
        os_cond_thread_id_tss_key = tlsKeyCreate ();
        os_cond_id_tss_key = tlsKeyCreate ();
    }
#endif
}

/** \brief Deinitialize cond module
 */
void os_condModuleExit (void)
{
    ut_tableFree ((ut_table) cache);
    ut_tableFree ((ut_table) thread_cache);
    semDelete (cache_lock);
}

/** \brief Initialize the condition variable taking the conition
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
os_result os_condInit (os_cond *cond, os_mutex *mutex, const os_condAttr *condAttr)
{
    os_result rv;
    os_condAttr defAttr;

    /* Avoid unused error */
    (void) mutex;

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert(cond->signature != OS_COND_MAGIC_SIG);
#endif

    if(!condAttr) {
        os_condAttrInit(&defAttr);
        condAttr = &defAttr;
    }

    if (condAttr->scopeAttr == OS_SCOPE_SHARED)
    {
        /* use shared address as unique name */
        snprintf(cond->name, sizeof (cond->name), "/%lu", (os_uint32) cond);
        cond->info.queue.id = semOpen(cond->name, SEM_TYPE_BINARY, SEM_FULL, SEM_Q_FIFO, OM_CREATE, NULL);
        /* As it may exist from a previous run. remove leftover semaphore object */
        cond->info.queue.thread_count = 0;
        cond->info.queue.start = 0;
        cond->info.queue.len = 0;

        if (cond->info.queue.id)
        {
            semTake(cache_lock, WAIT_FOREVER);
            (void) ut_tableInsert ((ut_table) cache, cond, cond->info.queue.id);
            semGive (cache_lock);
        }

        rv = (cond->info.queue.id != NULL) ? os_resultSuccess : os_resultFail;
    }
    else
    {
        int res;

        cond->name[0] = '\0';
        /* TODO attr */
        res = pthread_cond_init (&cond->info.posix_cond, NULL);
        rv = (res == 0) ? os_resultSuccess : os_resultFail;
    }

    /* mixing shared and private mutex and conds is not allowed */
    assert (((cond->name[0] == '\0') && (mutex->name[0] == '\0')) || ((cond->name[0] != '\0') && (mutex->name[0] != '\0')));
#ifdef OSPL_STRICT_MEM
    cond->signature = OS_COND_MAGIC_SIG;
#endif

    return rv;
}

/** \brief Destory the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
void os_condDestroy (os_cond *cond)
{
    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

    if (cond->name[0] != '\0') {
        SEM_ID id;
        if (cond->info.queue.len != 0) { cond_panic ("destroy called while waiters exist", 0); }
        if ((id = ut_get (cache, cond)) != 0) {
            semClose (id);
            semTake(cache_lock, WAIT_FOREVER);
            ut_remove (cache, cond);
            semGive (cache_lock);
        }
#ifdef OSPL_STRICT_MEM
        cond->signature = 0;
#endif
    } else {
        int res = pthread_cond_destroy (&cond->info.posix_cond);
        if (res != 0) { cond_panic ("pthread_cond_destroy failed", res); }
    }
}

/** \brief Wait for the condition
 */
void os_condWait (os_cond *cond, os_mutex *mutex)
{
    assert (cond != NULL);
    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
    assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif

    if (cond->name[0] != '\0') {
        int thread;
        STATUS status;
        SEM_ID mutex_id;
        SEM_ID sem;

        mutex_id = get_cond_mutex (cond);
        status = semTake (mutex_id, WAIT_FOREVER);
        if (status != OK) { cond_panic ("wait: semTake failed", status); }

        thread = self (cond, &sem);
        if (cond->info.queue.len < OS_COND_MAX_THREADS) {
            cond->info.queue.ids[(cond->info.queue.start + cond->info.queue.len++) % OS_COND_MAX_THREADS] = thread;

            status = semGive (mutex_id);
            if (status != OK) { cond_panic ("wait: semGive failed", status); }

            os_mutexUnlock (mutex);

            status = semTake (sem, WAIT_FOREVER);
            if (status != OK) { cond_panic ("wait: semTake failed", status); }
        } else {
            /* thread queue full */
            status = semGive (mutex_id);
            if (status != OK) {
                cond_panic ("wait: semGive failed (queue full)", status);
            } else {
                os_mutexUnlock (mutex);
                cond_queue_full (cond);
            }
        }

        os_mutexLock (mutex);
    } else {
        int res = pthread_cond_wait (&cond->info.posix_cond, &mutex->info.posix_mutex);
        if (res != 0) { cond_panic("wait: pthread_cond_wait failed", res); }
    }
}

int calculateWaitTime (os_duration timeout)
{
    int tickRate;
    int wait;

    tickRate = sysClkRateGet();

    if (OS_DURATION_ISINFINITE(timeout)) {
        wait = WAIT_FOREVER;
    } else if (OS_DURATION_ISPOSITIVE(timeout)) {
        wait = timeout / (OS_DURATION_SECOND / tickRate);
        /* Workaround for the case when the clock resolution is so poor
         * that it cannot support a requested nanosecond value:
         * In this case set the wait value to the miminal value 1 - this will
         * cause the semTake call to actually halt rather than return
         * immeditaely, giving a yield point so other threads can have control.
         * Without this it is possible that the thread will continually spin.
         */

        if (wait == 0 && timeout != 0) {
            wait = 1;
        }
    } else {
        wait = 0;
    }

    return wait;
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
 */
os_result os_condTimedWait (os_cond *cond, os_mutex *mutex, os_duration timeout)
{
    os_result rv = os_resultTimeout;

    assert (cond != NULL);
    assert (mutex != NULL);
    assert (OS_DURATION_ISPOSITIVE(timeout));
#ifdef OSPL_STRICT_MEM
    assert (cond->signature == OS_COND_MAGIC_SIG);
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    if (cond->name[0] != '\0') {
        int thread;
        STATUS status;
        SEM_ID mutex_id;
        SEM_ID sem;
        os_timeW wakeup_time;
        os_timeW current_time;
        os_duration wait_time;
        int wait;
        int queue_pos;

        wakeup_time = os_timeWAdd (os_timeWGet(), timeout);
        wait = calculateWaitTime (timeout);

        mutex_id = get_cond_mutex (cond);
        status = semTake (mutex_id, wait);
        if (status != OK && os_getErrno() != S_objLib_OBJ_TIMEOUT) {
            cond_panic ("timedwait: semGive failed", status);
        }

        if (status == OK) {
            thread = self (cond, &sem);

            if (cond->info.queue.len < OS_COND_MAX_THREADS) {
                queue_pos = (cond->info.queue.start + cond->info.queue.len++) % OS_COND_MAX_THREADS;
                cond->info.queue.ids[queue_pos] = thread;

                status = semGive (mutex_id);
                if (status != OK) { cond_panic ("timedwait: semGive failed", status); }

                os_mutexUnlock (mutex);

                current_time = os_timeWGet ();

                if (os_timeWCompare (current_time, wakeup_time) == OS_LESS)
                {
                    wait_time = os_timeWDiff (wakeup_time, current_time);
                    wait = calculateWaitTime (wait_time);

                    if (wait > 0) {
                        status = semTake (sem, wait);
                        assert ((status == OK) ||
                                (os_getErrno() == S_objLib_OBJ_TIMEOUT) || (os_getErrno() == S_objLib_OBJ_UNAVAILABLE));
                        if (status == OK) {
                            rv = os_resultSuccess;
                        }
                    }
                }

                if (rv == os_resultTimeout) {
                    status = semTake (mutex_id, WAIT_FOREVER);
                    if (status != OK) { cond_panic ("timedwait: semTake failed", status); }

                    /* try one last time in case semGive occurred while
                     * obtaining the mutex */
                    status = semTake (sem, NO_WAIT);
                    assert ((status == OK) || (os_getErrno() == S_objLib_OBJ_UNAVAILABLE));

                    if (status == OK) {
                        rv = os_resultSuccess;
                    } else {
                        /* remove from queue */
                        int i;
#ifndef NDEBUG
                        if (cond->info.queue.len == 0) {
                            cond_panic ("timedwait: queue empty", 0);
                        }
#endif
                        /* queue position may have moved */
                        if (cond->name[0] != '\0') {
                            for (i = 0; i < cond->info.queue.len; i++) {
                                queue_pos = (cond->info.queue.start + i) % OS_COND_MAX_THREADS;
                                if (cond->info.queue.ids[queue_pos] == thread) {
                                    break;
                                }
                            }
                        }
#ifndef NDEBUG
                        if (i == cond->info.queue.len) {
                            cond_panic ("timedwait: thread not found in queue", 0);
                        }
#endif
                        while (i < cond->info.queue.len - 1) {
                            cond->info.queue.ids[(cond->info.queue.start + i) % OS_COND_MAX_THREADS]
                                = cond->info.queue.ids[(cond->info.queue.start + i + 1) % OS_COND_MAX_THREADS];
                            i++;
                        }
                        cond->info.queue.len--;
                    }

                    status = semGive (mutex_id);
                    if (status != OK) {
                        cond_panic ("timedwait: semGive failed (timeout)", status);
                    }
                }
            } else {
                /* thread queue full */
                status = semGive (mutex_id);
                if ( status != OK ) {
                    cond_panic ("timedwait: semTake failed (queue full)", status);
                } else {
                    os_mutexUnlock (mutex);
                    cond_queue_full (cond);
                }
                assert (status == OK);
                rv = os_resultSuccess;
            }
        }

        os_mutexLock (mutex);
    } else {
        struct timespec t;
        int result;
        os_timeW wakeup_time;

        wakeup_time = os_timeWAdd (os_timeWGet(), timeout);
        t.tv_sec = OS_TIMEW_GET_SECONDS(wakeup_time);
        t.tv_nsec = OS_TIMEW_GET_NANOSECONDS(wakeup_time);

        do {
            result = pthread_cond_timedwait (&cond->info.posix_cond, &mutex->info.posix_mutex, &t);
        } while (result == EINTR);

        if (result == 0) {
            rv = os_resultSuccess;
        } else if (result == ETIMEDOUT) {
            rv = os_resultTimeout;
        } else {
            cond_panic ("pthread_cond_timedwait failed", result);
        }
    }

    return rv;
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 */
void
os_condSignal(os_cond *cond)
{
    os_result rv;

    assert(cond);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

    if (cond->name[0] != '\0') {
        STATUS status;
        SEM_ID mutex;
        SEM_ID sem;

        mutex = get_cond_mutex (cond);
        status = semTake (mutex, WAIT_FOREVER);
        if (status != OK) { cond_panic ("signal: semTake failed", status); }

        if (cond->info.queue.len > 0) {
            sem = get_sem (cond, cond->info.queue.ids[cond->info.queue.start]);
            status = semGive (sem);
            if (status != OK) { cond_panic ("signal: semGive 1 failed", status); }
            cond->info.queue.ids[cond->info.queue.start] = -1;
            cond->info.queue.start = (cond->info.queue.start + 1) % OS_COND_MAX_THREADS;
            cond->info.queue.len--;
        }

        status = semGive (mutex);
        if (status != OK) { cond_panic ("signal: semGive 2 failed", status); }
    } else {
        int result;
        result = pthread_cond_signal (&cond->info.posix_cond);
        if (result == 0) {
            rv = os_resultSuccess;
        } else {
            cond_panic ("pthread_cond_signal failed", result);
        }
    }
}

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 */
void os_condBroadcast (os_cond *cond)
{
    os_result rv = os_resultSuccess;

    assert(cond);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

    if (cond->name[0] != '\0') {
        STATUS status;
        SEM_ID mutex;
        SEM_ID sem;

        mutex = get_cond_mutex (cond);
        status = semTake (mutex, WAIT_FOREVER);
        if (status != OK) { cond_panic ("broadcast: semTake failed", status); }

        while (cond->info.queue.len > 0) {
            sem = get_sem (cond, cond->info.queue.ids[cond->info.queue.start]);
            status = semGive (sem);
            if (status != OK) { cond_panic ("broadcast: semGive 1 failed", status); }
            cond->info.queue.start = (cond->info.queue.start + 1) % OS_COND_MAX_THREADS;
            cond->info.queue.len--;
        }

        cond->info.queue.start = 0;

        status = semGive (mutex);
        if (status != OK) { cond_panic ("broadcast: semGive 2 failed", status); }
    } else {
        int result = pthread_cond_broadcast (&cond->info.posix_cond);
        if (result == 0) {
            rv = os_resultSuccess;
        } else {
            cond_panic ("pthread_cond_broadcast failed", result);
        }
    }
}

#include "../common/code/os_cond_attr.c"
