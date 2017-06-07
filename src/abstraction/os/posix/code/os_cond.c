/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/** \file os/posix/code/os_cond.c
 *  \brief Posix condition variables
 *
 * Implements condition variables for POSIX, the condition variable
 * is mapped onto the posix condition variable
 */

#include <time.h>
#include "os_cond.h"
#include <assert.h>
#include "os_errno.h"
#include "os_signature.h"
#include "os_init.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os__cond.h"


/** \brief The clock used in os_condTimedWait.
 *
 * The default clock is the realtime clock (CLOCK_REALTIME)
 * which is supported on all POSIX platforms.
 *
 * Some POSIX platforms support a monotonic clock. Only if a
 * monotonic clock is available it will be used. The
 * availability of the monotonic clock is checked in
 * os_timeModuleInit().
 */
static clockid_t OS_COND_TIMEDWAIT_CLOCK_ID = CLOCK_REALTIME;
static pthread_once_t os_condModuleInitialized = PTHREAD_ONCE_INIT;

static void
os__condModuleInit(void)
{
#if (!defined(OSPL_NO_POSIX_CLOCK_SELECTION) && defined(_POSIX_MONOTONIC_CLOCK) && defined(_POSIX_CLOCK_SELECTION) && ((_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 600))
    {
        pthread_condattr_t mattr;

        if(pthread_condattr_init (&mattr) == 0 && pthread_condattr_setclock(&mattr, CLOCK_MONOTONIC) == 0) {
            /* The monotonic clock can be used for pthread_cond_timedwait. */
            OS_COND_TIMEDWAIT_CLOCK_ID = CLOCK_MONOTONIC;
            return;
        }
    }
#else
#ifndef _WRS_KERNEL
#ifndef NDEBUG
    OS_REPORT(OS_WARNING, "os__condModuleInit", 0, "Time jumps are not supported on this platform.");
#endif /* NDEBUG */
#endif
#endif

}

void
os_condModuleInit(void)
{
    (void) pthread_once(&os_condModuleInitialized, os__condModuleInit);
}

void
os_condModuleExit(void)
{
    return;
}

/** \brief Initialize the condition variable taking the condition
 *         attributes into account
 *
 * \b os_condInit calls \b pthread_cond_init to intialize the posix condition
 * variable.
 *
 * If \b CLOCK_MONOTONIC is available the clock attribute of the condition
 * variable uses a monotonic clock. Otherwise it will use the realtime clock.
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * condition variable "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implicitly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi-process variables in
 * single process mode.
 */
os_result
os_condInit (
    os_cond *cond,
    os_mutex *dummymtx,
    const os_condAttr *condAttr)
{
    pthread_condattr_t mattr;
    int result;
    os_result rv;
    os_condAttr defAttr;

    OS_UNUSED_ARG(dummymtx);

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert(cond->signature != OS_COND_MAGIC_SIG);
#endif

    if(!condAttr){
        os_condAttrInit(&defAttr);
        condAttr = &defAttr;
    }

    result = pthread_condattr_init (&mattr);
    if (result != 0) {
        OS_REPORT(OS_FATAL, "os_condInit", 0,
                    "pthread_condattr_init failed (%u), insufficient memory",
                    result);
        return os_resultFail;
    }

    (void) pthread_once(&os_condModuleInitialized, os__condModuleInit);

#if (!defined(OSPL_NO_POSIX_CLOCK_SELECTION) && defined(_POSIX_CLOCK_SELECTION) && ((_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 600))
    /* If pthread_condattr_setclock fails this results in the default behaviour.
     * If this is the case, it is already reported in os_condModuleInit(), so the
     * result is ignored here. */
    (void)pthread_condattr_setclock(&mattr, OS_COND_TIMEDWAIT_CLOCK_ID);
#ifdef OSPL_STRICT_MEM
    cond->clockId = OS_COND_TIMEDWAIT_CLOCK_ID;
#endif /* OSPL_STRICT_MEM */
#endif

#ifndef OS_OMIT_PROCESS_SHARED_COND_SETUP
    if (condAttr->scopeAttr == OS_SCOPE_SHARED) {
        result = pthread_condattr_setpshared (&mattr, PTHREAD_PROCESS_SHARED);
    } else {
        result = pthread_condattr_setpshared (&mattr, PTHREAD_PROCESS_PRIVATE);
    }
#endif

    if (result == 0) {
#ifdef OSPL_STRICT_MEM
        result = pthread_cond_init (&cond->cond, &mattr);
#else
        result = pthread_cond_init (cond, &mattr);
#endif
    }

    (void)pthread_condattr_destroy (&mattr);

    if (result == 0) {
#ifdef OSPL_STRICT_MEM
        cond->signature = OS_COND_MAGIC_SIG;
#endif
        rv = os_resultSuccess;
    } else if (result == EBUSY){
        rv = os_resultBusy;
    } else {
        rv = os_resultFail;
    }
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

    assert (cond != NULL);

#ifdef OSPL_STRICT_MEM
   assert( cond->signature == OS_COND_MAGIC_SIG );
   result = pthread_cond_destroy (&cond->cond);
#else
   result = pthread_cond_destroy (cond);
#endif

    if (result == 0) {
#ifdef OSPL_STRICT_MEM
        cond->signature = 0;
#endif
    } else {
        OS_REPORT(OS_FATAL,"os_condDestroy",0,
                    "Operation failed: cond 0x%"PA_PRIxADDR", result = %s",
                    (os_address)cond, strerror(result));
        os_report_dumpStack(__FUNCTION__, __FILE__, __LINE__);
        abort ();
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
    int result;

    assert (cond != NULL);
    assert (mutex != NULL);

#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
    assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
    result = pthread_cond_wait (&cond->cond, &mutex->mutex);
#else
    result = pthread_cond_wait (cond, mutex);
#endif

    if (result != 0) {
        OS_REPORT(OS_FATAL,"os_condDestroy",0,
                    "Operation failed: cond 0x%"PA_PRIxADDR", result = %s",
                    (os_address)cond, strerror(result));
        os_report_dumpStack(__FUNCTION__, __FILE__, __LINE__);
        abort ();
    }
}

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 *
 * \b os_condTimedWait calls \b pthread_cond_timedwait to
 * wait for the condition with a timeout.
 *
 * \b os_condTimedWait provides an relative time to wait.
 * \b pthread_cond_timedwait uses a monotonic clock if available,
 * and the wall-clock otherwise. The deadline time to wakeup.
 * is calculated from the current time + the provided relative time
 * where the current time is based on monotonic time when available,
 * wall-clock time otherwise.
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
    struct timespec t;
    int result;
    os_result rv = os_resultFail;
    os_timeW wt;

    assert (cond != NULL);
    assert (mutex != NULL);
    assert (OS_DURATION_ISPOSITIVE(timeout));

#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
    assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
    assert( cond->clockId == OS_COND_TIMEDWAIT_CLOCK_ID);
#endif

    /* During initialisation we ensured that OS_COND_TIMEDWAIT_CLOCK_ID
     * is a supported clock_id. Also t is a valid timespec struct, so
     * no error checking is needed for clock_gettime. */
    (void)clock_gettime(OS_COND_TIMEDWAIT_CLOCK_ID, &t);
    wt = OS_TIMEW_INIT(t.tv_sec, t.tv_nsec);
    wt = os_timeWAdd(wt, timeout);
    t.tv_sec = (time_t) OS_TIMEW_GET_SECONDS(wt);
    t.tv_nsec = OS_TIMEW_GET_NANOSECONDS(wt);

    do {
#ifdef OSPL_STRICT_MEM
      result = pthread_cond_timedwait (&cond->cond, &mutex->mutex, &t);
#else
      result = pthread_cond_timedwait (cond, mutex, &t);
#endif
    } while (result == EINTR);

    if (result == 0) {
        rv = os_resultSuccess;
    } else if (result == ETIMEDOUT) {
        rv = os_resultTimeout;
    } else {
#if ! defined (OSPL_VXWORKS653)
        OS_REPORT(OS_FATAL,"os_condTimedWait",0,
                    "Operation failed: cond 0x%"PA_PRIxADDR", result = %s",
                    (os_address)cond,
                    os_strError(result));
#else
        OS_REPORT(OS_FATAL,"os_condTimedWait",0,
                    "Operation failed: cond 0x%"PA_PRIxADDR", result = %d",
                    (os_address)cond,
                    result);
#endif
        rv = os_resultFail;
        os_report_dumpStack(__FUNCTION__, __FILE__, __LINE__);
        abort ();
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

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

#ifdef OSPL_STRICT_MEM
    result = pthread_cond_signal (&cond->cond);
#else
    result = pthread_cond_signal (cond);
#endif
    if (result != 0) {
        OS_REPORT(OS_FATAL,"os_condSignal",0,
                    "Operation failed: cond 0x%"PA_PRIxADDR", result = %d",
                    (os_address)cond,
                    result);
    }
}

/** \brief Signal the condition and wakeup all threads waiting
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

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

#ifdef OSPL_STRICT_MEM
    result = pthread_cond_broadcast (&cond->cond);
#else
    result = pthread_cond_broadcast (cond);
#endif
    if (result != 0) {
        OS_REPORT(OS_FATAL,"os_condBroadcast",0,
                    "Operation failed: cond 0x%"PA_PRIxADDR", result = %d",
                    (os_address)cond,
                    result);
    }
}

