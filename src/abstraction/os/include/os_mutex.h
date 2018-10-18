/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
 * Interface definition for mutual exclusion semaphores         *
 ****************************************************************/

/** \file os_mutex.h
 *  \brief Critical section management - mutual exclusion semaphore
 */

#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#include "os_defs.h"

#include "os_if.h"
#include "os_stdlib.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Definition of the mutex
 *
 * os_mutex is a platform specific definition for a mutual exclusion
 * semaphore. The mutex sempahore can be defined in system shared memory
 * in order to synchronize processes and threads, or the mutex semaphore
 * can be placed in process shared memory in order to synchronize threads.
 */
typedef os_os_mutex os_mutex;

/** \brief Definition of the mutex attributes
 */
typedef struct os_mutexAttr {
    /**
     * - OS_SCOPE_SHARED The scope of the mutex
     *   is system wide
     * - OS_SCOPE_PRIVATE The scope of the mutex
     *   is process wide
     */
    os_scopeAttr	scopeAttr;

    /* - OS_ERRORCHECKING_DISABLED The mutex operations aren't checked
     * - OS_ERRORCHECKING_ENABLED The mutex operations are checked */
    os_errorCheckingAttr errorCheckingAttr;
} os_mutexAttr;

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call. (only effective on
 *   platforms that support priority inheritance)
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     mutex is successfuly initialized
 * - returns os_resultSuccess
 */
OS_API os_result
os_mutexSetPriorityInheritanceMode(
        os_boolean enabled);

/** \brief Initialize the mutex taking the mutex attributes
 *         into account
 *
 * Possible Results:
 * - assertion failure: mutex = NULL || mutexAttr = NULL
 * - returns os_resultSuccess if
 *     mutex is successfuly initialized
 * - returns os_resultFail if
 *     mutex is not initialized because of a failure
 */
_Check_return_
OS_API os_result
os_mutexInit(
        _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os_mutex *mutex,
        _In_opt_ const os_mutexAttr *mutexAttr)
    __nonnull((1));

/** \brief Destroy the mutex
 *
 * Never returns on failure
 */
OS_API void
os_mutexDestroy(
        _Inout_ _Post_invalid_ os_mutex *mutex)
    __nonnull_all__;

/** \brief Acquire the mutex.
 *
 * If you need to detect an error, use os_mutexLock_s instead.
 *
 * @see os_mutexLock_s
 */
_Acquires_nonreentrant_lock_(&mutex->lock)
OS_API void
os_mutexLock(
        _Inout_ os_mutex *mutex)
    __nonnull_all__;

/**
 * \brief Acquire the mutex. Returns whether the call succeeeded or an error
 * occurred.
 *
 * Precondition:
 * - mutex is not yet acquired by the calling thread
 *
 * Possible Results:
 * - assertion failure: mutex = NULL
 * - returns os_resultSuccess if
 *     mutex is acquired
 * - returns os_resultFail if
 *     mutex is not acquired because of a failure
 */
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(&mutex->lock))
OS_API os_result
os_mutexLock_s(
        _Inout_ os_mutex *mutex)
    __nonnull_all__
    __attribute_warn_unused_result__;

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 *
 * Precondition:
 * - mutex is not yet acquired by the calling thread
 *
 * Possible Results:
 * - assertion failure: mutex = NULL
 * - returns os_resultSuccess if
 *      mutex is acquired
 * - returns os_resultBusy if
 *      mutex is not acquired because it is already acquired
 *      by another thread
 * - returns os_resultFail if
 *      mutex is not acquired because of a failure
 */
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(&mutex->lock))
OS_API os_result
os_mutexTryLock (
        _Inout_ os_mutex *mutex)
    __nonnull_all__
    __attribute_warn_unused_result__;

/** \brief Release the acquired mutex
 */
_Releases_nonreentrant_lock_(&mutex->lock)
OS_API void
os_mutexUnlock (
        _Inout_ os_mutex *mutex)
    __nonnull_all__;

/** \brief Set the default mutex attributes
 *
 * Postcondition:
 * - mutex scope attribute is OS_SCOPE_PRIVATE
 * - mutex errorChecking attribute is OS_ERRORCHECKING_DISABLED
 *
 * Precondition:
 * - mutexAttr is a valid object
 */
_Post_satisfies_(mutexAttr->scopeAttr == OS_SCOPE_PRIVATE)
_Post_satisfies_(mutexAttr->errorCheckingAttr == OS_ERRORCHECKING_DISABLED)
OS_API void
os_mutexAttrInit(
        _Out_ os_mutexAttr *mutexAttr)
    __nonnull_all__;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_MUTEX_H */
