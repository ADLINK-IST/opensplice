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

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )

#include <vxWorks.h>
#include "os_mutex.h"
#include <assert.h>
#include "os_errno.h"
#include <objLib.h>
#include <taskLib.h>

#ifdef OSPL_STRICT_MEM
#include "os_signature.h"
#endif

static void mutex_panic (const char *msg, int status)
{
    printf ("ERROR: os_mutex: %s (%d, %d) - suspending task\n", msg, status, os_getErrno ());
    fflush (stdout);
    taskSuspend (taskIdSelf ());
}

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call.
 *
 * Not (yet) supported on this platform
 */
os_result
os_mutexSetPriorityInheritanceMode(
    os_boolean enabled)
{
  (void) enabled;
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
 */
os_result
os_mutexInit (
    os_mutex *mutex,
    const os_mutexAttr *mutexAttr)
{
    os_result rv;

    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif

    taskLock();
    mutex->os_sem = semMCreate (SEM_Q_PRIORITY | SEM_DELETE_SAFE);
    mutex->os_countLock = 0;
    if (mutex->os_sem != NULL) {
        rv=  os_resultSuccess;
    } else {
        rv=  os_resultFail;
    }
    taskUnlock();
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
void
os_mutexDestroy (
    os_mutex *mutex)
{
    int result;
    int idList[4];
    int maxtask = 4;

    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    taskLock ();
    result = semInfo (mutex->os_sem, idList, maxtask);
    if (result != 0) {
        mutex_panic ("destroy: not in unlocked state", result);
    } else {
        result = semDelete (mutex->os_sem);
        if (result == 0) {
            rv = os_resultSuccess;
        } else {
            mutex_panic ("destroy: semDelete failed", result);
        }
    }
    taskUnlock ();

#ifdef OSPL_STRICT_MEM
    if (rv == os_resultSuccess)
    {
       mutex->signature = 0;
    }
#endif
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result
os_mutexLock_s (
    os_mutex *mutex)
{
    int result;
    os_result rv;

    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
    result = semTake (mutex->os_sem, WAIT_FOREVER);
    if (result == 0) {
        if (mutex->os_countLock != 0) {
            semGive(mutex->os_sem);
            rv = os_resultFail;
        } else {
            mutex->os_countLock = mutex->os_countLock + 1;
            rv = os_resultSuccess;
        }
    } else {
        rv = os_resultFail;
    }
    return rv;
}

void
os_mutexLock (
    os_mutex *mutex)
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
os_result
os_mutexTryLock (
    os_mutex *mutex)
{
    int result;
    os_result rv;

    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
    result = semTake (mutex->os_sem, NO_WAIT);
    if (result == 0) {
        if (mutex->os_countLock != 0) {
            semGive(mutex->os_sem);
            return os_resultBusy;
        } else {
            mutex->os_countLock = mutex->os_countLock + 1;
            return os_resultSuccess;
        }
    } else if (os_getErrno() == S_objLib_OBJ_UNAVAILABLE) {
        return os_resultBusy;
    } else {
        mutex_panic ("trylock failed", result);
        return os_resultBusy;
    }
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
    int result;
    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
    mutex->os_countLock = 0;
    result = semGive (mutex->os_sem);
    if (result != 0) {
        mutex_panic ("unlock: semGive failed");
    }
}
#include "../common/code/os_mutex_attr.c"

#else

#define OS_OMIT_PROCESS_SHARED_MUTEX_SETUP
#include "../posix/code/os_mutex.c"
#include "../common/code/os_mutex_attr.c"

#endif
