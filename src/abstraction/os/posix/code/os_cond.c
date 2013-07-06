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

/** \file os/posix/code/os_cond.c
 *  \brief Posix condition variables
 *
 * Implements condition variables for POSIX, the condition variable
 * is mapped onto the posix condition variable
 */

#include <time.h>
#include "os_cond.h"
#include <assert.h>
#include <errno.h>
#include "os_signature.h"
#include "os_init.h"
#include "os_report.h"

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
    pthread_condattr_t mattr;
    int result = 0;
    os_result rv;
    OS_UNUSED_ARG(dummymtx);

    assert (cond != NULL);
    assert (condAttr != NULL);

#ifdef OSPL_STRICT_MEM
   assert(cond->signature != OS_COND_MAGIC_SIG);
#endif

    pthread_condattr_init (&mattr);

#ifndef OS_OMIT_PROCESS_SHARED_COND_SETUP
    /* In single process mode only "private" variables are required */
    if (condAttr->scopeAttr == OS_SCOPE_SHARED && !os_serviceGetSingleProcess ()) {
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
       if (result == EBUSY) {
          os_condDestroy (cond);
          result = pthread_cond_init (cond, &mattr);
       }
#endif
    }
    pthread_condattr_destroy (&mattr);
    if (result == 0) {
#ifdef OSPL_STRICT_MEM
        cond->signature = OS_COND_MAGIC_SIG;
#endif
	rv = os_resultSuccess;
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
os_result
os_condDestroy (
    os_cond *cond)
{
    int result;
    os_result rv;

    assert (cond != NULL);

#ifdef OSPL_STRICT_MEM
   assert( cond->signature == OS_COND_MAGIC_SIG );
   result = pthread_cond_destroy (&cond->cond);
#else
   result = pthread_cond_destroy (cond);
#endif

    if (result == 0) {
	rv = os_resultSuccess;
#ifdef OSPL_STRICT_MEM
        cond->signature = 0;
#endif
    } else if (result == EBUSY) {
	rv = os_resultBusy;
    } else {
	rv = os_resultFail;
    }
    return rv;
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
os_result
os_condWait (
    os_cond *cond,
    os_mutex *mutex)
{
    int result;
    os_result rv;

    assert (cond != NULL);
    assert (mutex != NULL);
    
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
    assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
    result = pthread_cond_wait (&cond->cond, &mutex->mutex);
#else
    result = pthread_cond_wait (cond, mutex);
#endif

    if (result == 0) {
	rv = os_resultSuccess;
    } else {
	rv = os_resultFail;
    }
    return rv;
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
    const os_time *time)
{
    struct timespec t;
    int result;
    os_time wakeup_time;
    os_result rv;

    assert (cond != NULL);
    assert (mutex != NULL);
    assert (time != NULL);

#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
    assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif

    wakeup_time = os_timeAdd (os_timeGet(), *time);
    t.tv_sec = wakeup_time.tv_sec;
    t.tv_nsec = wakeup_time.tv_nsec;

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
        OS_REPORT_2(OS_ERROR,"os_condTimedWait",0,
                    "Operation failed: cond 0x%x, result = %s",
                    cond,
                    strerror(result));
#else
        OS_REPORT_2(OS_ERROR,"os_condTimedWait",0,
                    "Operation failed: cond 0x%x, result = %d",
                    cond,
                    result);
#endif
        assert(OS_FALSE);
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * \b os_condSignal calls \b pthread_cond_signal to signal
 * the condition.
 */
os_result
os_condSignal (
    os_cond *cond)
{
    int result;
    os_result rv;

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

#ifdef OSPL_STRICT_MEM
    result = pthread_cond_signal (&cond->cond);
#else
    result = pthread_cond_signal (cond);
#endif
    if (result == 0) {
	rv = os_resultSuccess;
    } else {
	rv = os_resultFail;
    }
    return rv;
}

/** \brief Signal the condition and wakeup all threads waiting
 *         for the condition
 *
 * \b os_condBroadcast calls \b pthread_cond_broadcast to broadcast
 * the condition.
 */
os_result
os_condBroadcast (
    os_cond *cond)
{
    int result;
    os_result rv;

    assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
    assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

#ifdef OSPL_STRICT_MEM
    result = pthread_cond_broadcast (&cond->cond);
#else
    result = pthread_cond_broadcast (cond);
#endif
    if (result == 0) {
	rv = os_resultSuccess;
    } else {
	rv = os_resultFail;
    }
    return rv;
}
