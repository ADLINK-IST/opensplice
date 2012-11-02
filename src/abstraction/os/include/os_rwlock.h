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
/****************************************************************
 * Interface definition for multiple reader writer lock of      *
 * SPLICE-DDS                                                   *
 ****************************************************************/

#ifndef OS_RWLOCK_H
#define OS_RWLOCK_H

/** \file os_rwlock.h
 *  \brief Critical section management - multiple reader writer lock
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"

/* include OS specific header file				*/
#include "include/os_rwlock.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Definition of the multiple reader writer lock
 *
 * os_rwlock is a platform specific definition for a multiple reader writer lock.
 * The multiple reader writer lock can be defined in system shared memory in order
 * to synchronize processes and threads, or the multiple reader writer lock can
 * be placed in process shared memory in order to synchronize threads.
 */
typedef os_os_rwlock os_rwlock;

/** \brief Definition of the multiple reader writer lock attributes
 */
typedef struct os_rwlockAttr {
    /**
     * - OS_SCOPE_SHARED The scope of the multiple reader writer lock
     *   is system wide
     * - OS_SCOPE_PRIVATE The scope of the multiple reader writer lock
     *   is process wide
     */
    os_scopeAttr	scopeAttr;
} os_rwlockAttr;

/** \brief Initialize the rwlock taking the rwlock attributes into account
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL || rwlockAttr = NULL
 * - returns os_resultSuccess if
 *     rwlock is successfuly initialized
 * - returns os_resultFail
 *     rwlock is not initialized and can not be used
 */
OS_API os_result
os_rwlockInit(
    os_rwlock *rwlock,
    const os_rwlockAttr *rwlockAttr);

/** \brief Destroy the rwlock
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL
 * - returns os_resultSuccess if
 *     rwlock is successfuly destroyed
 * - returns os_resultBusy if
 *     rwlock is not destroyed because it is still claimed or referenced by a thread
 * - returns os_resultFail if
 *     rwlock is not destroyed
 */
OS_API os_result
os_rwlockDestroy(
    os_rwlock *rwlock);

/** \brief Acquire the rwlock while intending to read only
 *
 * Precondition:
 * - rwlock is not yet acquired by the calling thread
 *
 * Postcondition:
 * - The data related to the critical section is not changed
 *   by the calling thread
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL
 * - returns os_resultSuccess if
 *      rwlock is acquired
 * - returns os_resultFail if
 *      rwlock is not acquired because of a failure
 */
OS_API os_result
os_rwlockRead(
    os_rwlock *rwlock);

/** \brief Acquire the rwlock while intending to write
 *
 * Precondition:
 * - rwlock is not yet acquired by the calling thread
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL
 * - returns os_resultSuccess if
 *      rwlock is acquired
 * - returns os_resultFail if
 *      rwlock is not acquired because of a failure
 */
OS_API os_result
os_rwlockWrite(
    os_rwlock *rwlock);

/** \brief Try to acquire the rwlock while intending to read only
 *
 * Try to acquire the rwlock while intending to read only,
 * immediately return if the mutex is acquired by
 * another thread with the intention to write
 *
 * Precondition:
 * - rwlock is not yet acquired by the calling thread
 *
 * Postcondition:
 * - The data related to the critical section is not changed
 *   by the calling thread
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL
 * - returns os_resultSuccess if
 *      rwlock is acquired
 * - returns os_resultBusy if
 *      rwlock is not acquired because it is already
 *      acquired by another thread with the intention to write
 * - returns os_resultFail if
 *      rwlock is not acquired because of a failure
 */
OS_API os_result
os_rwlockTryRead(
    os_rwlock *rwlock);

/** \brief Try to acquire the rwlock while intending to write
 *
 * Try to acquire the rwlock while intending to write,
 * immediately return if the mutex is acquired by
 * another thread, either for read or for write
 *
 * Precondition:
 * - rwlock is not yet acquired by the calling thread
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL
 * - returns os_resultSuccess if
 *      rwlock is acquired
 * - returns os_resultBusy if
 *      rwlock is not acquired because it is already
 *      acquired by another thread
 * - returns os_resultFail if
 *      rwlock is not acquired because of a failure
 */
OS_API os_result
os_rwlockTryWrite(
    os_rwlock *rwlock);

/** \brief Release the acquired rwlock
 *
 * Precondition:
 * - rwlock is already acquired by the calling thread
 *
 * Possible Results:
 * - assertion failure: rwlock = NULL
 * - returns os_resultSuccess if
 *     rwlock is released
 * - returns os_resultFail if
 *     rwlock is not released because of a failure
 */
OS_API os_result
os_rwlockUnlock(
    os_rwlock *rwlock);

/** \brief Set the default rwlock attributes
 *
 * Postcondition:
 * - rwlock scope attribute is OS_SCOPE_SHARED
 *
 * Possible Results:
 * - assertion failure: rwlockAttr = NULL
 * - returns os_resultSuccess
 */
OS_API os_result
os_rwlockAttrInit(
    os_rwlockAttr *rwlockAttr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_RWLOCK_H */
