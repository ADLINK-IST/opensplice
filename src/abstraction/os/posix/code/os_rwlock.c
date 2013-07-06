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

/** \file os/posix/code/os_rwlock.c
 *  \brief Posix multiple reader writer lock
 *
 * Implements multiple reader writer lock for POSIX
 */

#include "os_rwlock.h"
#include "os_init.h"
#include <errno.h>
#include <assert.h>

/** \brief Initialize the rwlock taking the rwlock attributes into account
 *
 * \b os_rwlockInit calls \b pthread_rwlock_init to intialize the posix
 * \b rwlock.
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * \b rwlock "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implictly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi process variables in
 * single process mode.
 */
os_result
os_rwlockInit (
    os_rwlock *rwlock, 
    const os_rwlockAttr *rwlockAttr)
{
    pthread_rwlockattr_t rwattr;
    int result;
    os_result rv;

    assert (rwlock != NULL);
    assert (rwlockAttr != NULL);
    pthread_rwlockattr_init (&rwattr);

    /* In single process mode only "private" variables are required */
    if (rwlockAttr->scopeAttr == OS_SCOPE_SHARED && !os_serviceGetSingleProcess ()) {
        result = pthread_rwlockattr_setpshared (&rwattr, PTHREAD_PROCESS_SHARED);
    } else {
        result = pthread_rwlockattr_setpshared (&rwattr, PTHREAD_PROCESS_PRIVATE);
    }
    if (result == 0) {
        pthread_rwlock_init (rwlock, &rwattr);
    }
    pthread_rwlockattr_destroy (&rwattr);
    if (result == 0) {
	rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Destroy the rwlock
 *
 * \b os_rwlockDestroy calls \b pthread_rwlock_destroy to destroy the
 * posix \b rwlock.
 */
os_result
os_rwlockDestroy (
    os_rwlock *rwlock)
{
    int result;
    os_result rv;

    assert (rwlock != NULL);
    result = pthread_rwlock_destroy (rwlock);
    if (result == 0) {
	rv = os_resultSuccess;
    } else if (result == EBUSY) {
        rv = os_resultBusy;
    } else {
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Acquire the rwlock while intending to read only
 *
 * \b os_rwlockRead calls \b pthread_rwlock_rdlock to acquire
 * the posix \b rwlock for reading purposes.
 */
os_result
os_rwlockRead (
    os_rwlock *rwlock)
{
    int result;
    os_result rv;

    assert (rwlock != NULL);
    result = pthread_rwlock_rdlock (rwlock);
    if (result == 0) {
	rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Acquire the rwlock while intending to write
 *
 * \b os_rwlockWrite calls \b pthread_rwlock_wrlock to acquire
 * the posix \b rwlock for writing purposes.
 */
os_result
os_rwlockWrite (
    os_rwlock *rwlock)
{
    int result;
    os_result rv;

    assert (rwlock != NULL);
    result = pthread_rwlock_wrlock (rwlock);
    if (result == 0) {
	rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Try to acquire the rwlock while intending to read only
 *
 * \b os_rwlockTryRead calls \b pthread_mutex_tryrdlock to acquire
 * the posix \b rwlock for reading purposes.
 */
os_result
os_rwlockTryRead (
    os_rwlock *rwlock)
{
    int result;
    os_result rv;

    assert (rwlock != NULL);
    result = pthread_rwlock_tryrdlock (rwlock);
    if (result == 0) {
	rv = os_resultSuccess;
    } else if (result == EBUSY) {
	rv = os_resultBusy;
    } else {
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Try to acquire the rwlock while intending to write
 *
 * \b os_rwlockTryWrite calls \b pthread_mutex_trywrlock to acquire
 * the posix \b rwlock for writing purposes.
 */
os_result
os_rwlockTryWrite (
    os_rwlock *rwlock)
{
    int result;
    os_result rv;

    assert (rwlock != NULL);
    result = pthread_rwlock_trywrlock (rwlock);
    if (result == 0) {
	rv = os_resultSuccess;
    } else if (result == EBUSY) {
	rv = os_resultBusy;
    } else {
        rv = os_resultFail;
    }
    return rv;
}

/** \brief Release the acquired rwlock
 *
 * \b os_rwlockUnlock calls \b pthread_rwlock_unlock to release
 * the posix \b rwlock.
 */
os_result
os_rwlockUnlock (
    os_rwlock *rwlock)
{
    int result;
    os_result rv;

    assert (rwlock != NULL);
    result = pthread_rwlock_unlock (rwlock);
    if (result == 0) {
	rv = os_resultSuccess;
    } else {
        rv = os_resultFail;
    }
    return rv;
}
