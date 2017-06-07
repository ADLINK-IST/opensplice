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

#ifdef OSPL_BUILD_CORE
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
OS_API void
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
OS_API void
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
OS_API void
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
OS_API void
os_rwlockUnlock(
    os_rwlock *rwlock);

/** \brief Set the default rwlock attributes
 *
 * Postcondition:
 * - rwlock scope attribute is OS_SCOPE_PRIVATE
 */
_Post_satisfies_(rwlockAttr->scopeAttr == OS_SCOPE_PRIVATE)
OS_API void
os_rwlockAttrInit(
        _Out_ os_rwlockAttr *rwlockAttr)
    __nonnull_all__;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_RWLOCK_H */
