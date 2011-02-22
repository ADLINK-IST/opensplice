/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/** \file os/posix/code/os_mutex.c
 *  \brief Posix mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for POSIX
 */

#include <../posix/code/os__mutex.h>
#include <os_report.h>
#include <os_signature.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

static os_boolean ospl_mtx_prio_inherit = OS_FALSE;

#if defined __GLIBC_PREREQ
#if __GLIBC_PREREQ(2,5)
#define OSPL_PRIO_INHERIT_SUPPORTED
#endif
#endif

void
os_mutexModuleInit()
{
//    ospl_mtx_prio_inherit = 0;
}

void
os_mutexModuleExit()
{
}

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call.
 *
 * Store the setting in the Static variable to be used when MutexInit is called.
 */
os_result
os_mutexSetPriorityInheritanceMode(
    os_boolean enabled)
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
os_result
os_mutexInit (
    os_mutex *mutex, 
    const os_mutexAttr *mutexAttr)
{
    pthread_mutexattr_t mattr;
    int result = 0;
    os_result rv;

    assert (mutex != NULL);
    assert (mutexAttr != NULL);
#ifdef OSPL_STRICT_MEM
    assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif

    pthread_mutexattr_init (&mattr);
    if (mutexAttr->scopeAttr == OS_SCOPE_SHARED) {
        result = pthread_mutexattr_setpshared (&mattr, PTHREAD_PROCESS_SHARED);
    } else {
        result = pthread_mutexattr_setpshared (&mattr, PTHREAD_PROCESS_PRIVATE);
    }
#ifdef OSPL_PRIO_INHERIT_SUPPORTED
/* only if priority inheritance is supported in the pthread lib */
    if ((result == 0) && ospl_mtx_prio_inherit) {
        result = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);
    }
#endif
    if (result == 0) {
#ifdef OSPL_STRICT_MEM
        result = pthread_mutex_init (&mutex->mutex, &mattr);
#else
        result = pthread_mutex_init (mutex, &mattr);
        if (result == EBUSY) {
            os_mutexDestroy (mutex);
            result = pthread_mutex_init (mutex, &mattr);
        }
#endif
    }
    pthread_mutexattr_destroy (&mattr);
    if (result == 0) {
        rv = os_resultSuccess;
#ifdef OSPL_STRICT_MEM
        mutex->signature = OS_MUTEX_MAGIC_SIG;
#endif

    } else {
        OS_REPORT_2(OS_ERROR,"os_mutexInit",0,
                    "Operation failed: mutex 0x%x, result = %s",
                    mutex, strerror(result));
        assert(OS_FALSE);
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Destroy the mutex
 *
 * \b os_mutexDestroy calls \b pthread_mutex_destroy to destroy the
 * posix \b mutex.
 */
os_result
os_mutexDestroy (
    os_mutex *mutex)
{
    int result;
    os_result rv;

    assert (mutex != NULL);

#ifdef OSPL_STRICT_MEM
    assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
    result = pthread_mutex_destroy (&mutex->mutex);
#else
    result = pthread_mutex_destroy (mutex);
#endif

    if (result == 0) {
        rv=  os_resultSuccess;
#ifdef OSPL_STRICT_MEM
        mutex->signature = 0;
#endif
    } else if (result == EBUSY) {
        rv = os_resultBusy;
    } else {
        OS_REPORT_2(OS_ERROR,"os_mutexDestroy",0,
                    "Operation failed: mutex 0x%x, result = %s",
                    mutex, strerror(result));
        assert(OS_FALSE);
        rv=  os_resultFail;
    }
    return rv;
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result
os_mutexLock (
    os_mutex *mutex)
{
    int result;
    os_result rv;

    assert (mutex != NULL);

#ifdef OSPL_STRICT_MEM
    assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
    result = pthread_mutex_lock (&mutex->mutex);
#else
    result = pthread_mutex_lock (mutex);
#endif

    if (result == 0) {
        rv=  os_resultSuccess;
    } else {
        OS_REPORT_2(OS_ERROR,"os_mutexLock",0,
                    "Operation failed: mutex 0x%x, result = %s",
                    mutex, strerror(result));
        assert(OS_FALSE);
        rv=  os_resultFail;
    }
    return rv;
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
    assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
    result = pthread_mutex_trylock (&mutex->mutex);
#else
    result = pthread_mutex_trylock (mutex);
#endif

    if (result == 0) {
        rv=  os_resultSuccess;
    } else if (result == EBUSY) {
        rv=  os_resultBusy;
    } else {
        OS_REPORT_2(OS_ERROR,"os_mutexTryLock",0,
                    "Operation failed: mutex 0x%x, result = %s",
                    mutex, strerror(result));
        assert(OS_FALSE);
        rv=  os_resultFail;
    }
    return rv;
}

/** \brief Release the acquired mutex
 *
 * \b os_mutexUnlock calls \b pthread_mutex_unlock to release
 * the posix \b mutex.
 */
os_result
os_mutexUnlock (
    os_mutex *mutex)
{
    int result;
    os_result rv;

    assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
    assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
    result = pthread_mutex_unlock (&mutex->mutex);
#else
    result = pthread_mutex_unlock (mutex);
#endif

    if (result == 0) {
        rv=  os_resultSuccess;
    } else {
        OS_REPORT_2(OS_ERROR,"os_mutexUnlock",0,
                    "Operation failed: mutex 0x%x, result = %s",
                    mutex, strerror(result));
        assert(OS_FALSE);
        rv=  os_resultFail;
    }
    return rv;
}
