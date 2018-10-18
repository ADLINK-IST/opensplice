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

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )

#include <vxWorks.h>
#include "os_cond.h"
#include <assert.h>
#include "os_errno.h"
#include <objLib.h>

#include <taskLib.h>
#if ! defined ( VXWORKS_55 ) && ! defined ( VXWORKS_54 )
#include <sysLib.h>
#endif

#ifdef OSPL_STRICT_MEM
#include "os_signature.h"
#endif

static void cond_panic (const char *msg, int status)
{
    printf ("ERROR: os_cond: %s (%d, %d) - suspending task\n", msg, status, os_getErrno ());
    fflush (stdout);
    taskSuspend (taskIdSelf ());
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
 */
os_result
os_condInit (
    os_cond *cond,
    os_mutex *dummymtx,
    const os_condAttr *condAttr)
{
    os_result rv;

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert (cond->signature != OS_COND_MAGIC_SIG);
#endif

    cond->os_cond = semBCreate (SEM_Q_FIFO, SEM_EMPTY);
    if (cond->os_cond != NULL) {
        rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
#ifdef OSPL_STRICT_MEM
    cond->signature = OS_COND_MAGIC_SIG;
#endif
    return rv;
}

/** \brief Destroy the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
void
os_condDestroy (
    os_cond *cond)
{
    int result;
    int taskIdList[1];

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert (cond->signature == OS_COND_MAGIC_SIG);
#endif

    taskLock ();
    result = semInfo ((SEM_ID)cond->os_cond, taskIdList, 1);
    if (result == 0) {
        result = semDelete ((SEM_ID)cond->os_cond);
        if (result == 0) {
#ifdef OSPL_STRICT_MEM
            cond->signature = 0;
#endif
        } else {
            cond_panic ("destroy: semDelete failed", result);
        }
    } else if (result > 0) {
        cond_panic ("destroy: semDelete failed", result);
    }
    taskUnlock ();
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
    int result;

    assert (cond != NULL);
    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert (cond->signature == OS_COND_MAGIC_SIG);
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    taskLock ();
    os_mutexUnlock (mutex);
    result = semTake ((SEM_ID)cond->os_cond, WAIT_FOREVER);
    os_mutexLock (mutex);
    taskUnlock ();
    if (result != 0) {
        cond_panic ("wait: semTake failed", result);
    }
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
    int result;
    os_result rv = os_resultSuccess;
    int tickRate;
    int wait;

    assert (cond != NULL);
    assert (mutex != NULL);
    assert (OS_DURATION_ISPOSITIVE(timeout));
#ifdef OSPL_STRICT_MEM
    assert (cond->signature == OS_COND_MAGIC_SIG);
    assert (mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif

    if (OS_DURATION_ISINFINITE(timeout)) {
        wait = WAIT_FOREVER;
    } else if (OS_DURATION_ISPOSITIVE(timeout)) {
        tickRate = sysClkRateGet ();
        wait = (int)(timeout / (1000000000 / tickRate));

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

    taskLock ();
    os_mutexUnlock (mutex);
    result = semTake ((SEM_ID)cond->os_cond, wait);
    os_mutexLock (mutex);
    taskUnlock ();
    if (wait == WAIT_FOREVER) {
        if (result == ERROR) {
            cond_panic ("timedwait: semTake 1 failed", result);
        }
    } else if (wait == 0) {
        if (result == ERROR) {
            if (os_getErrno() == S_objLib_OBJ_UNAVAILABLE) {
                rv = os_resultTimeout;
            } else {
                cond_panic ("timedwait: semTake 2 failed", result);
            }
        }
    } else {
        if (result == ERROR) {
            if (os_getErrno() == S_objLib_OBJ_TIMEOUT) {
                rv = os_resultTimeout;
            } else {
                cond_panic ("timedwait: semTake 3 failed", result);
            }
        }
    }
    return rv;
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
    int result;
    int taskIdList[1];

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert (cond->signature == OS_COND_MAGIC_SIG);
#endif
    result = semInfo ((SEM_ID)cond->os_cond, taskIdList, 1);
    if (result == 0) {
        /* if there are tasks waiting retrun OK */
    } else if (result > 0) {
        result = semGive ((SEM_ID)cond->os_cond);
        if (result != 0) {
            cond_panic ("signal: semGive failed", result);
        }
    } else {
        cond_panic ("signal: semInfo failed", result);
    }
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
    int result;
    int taskIdList[1];

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert (cond->signature == OS_COND_MAGIC_SIG);
#endif

    taskLock();
    result = semInfo ((SEM_ID)cond->os_cond, taskIdList, 1);
    if (result == 0) {
        /* if there are tasks waiting retrun OK */
    } else if (result > 0) {
        result = semFlush ((SEM_ID)cond->os_cond);
        if (result != 0) {
            cond_panic ("broadcast: semFlush failed", result);
        }
    } else {
        cond_panic ("broadcast: semInfo failed", result);
    }
    taskUnlock();
}
#include "../common/code/os_cond_attr.c"

#else
#define OS_OMIT_PROCESS_SHARED_COND_SETUP
#include "../posix/code/os_cond.c"
#include "../common/code/os_cond_attr.c"
#endif
