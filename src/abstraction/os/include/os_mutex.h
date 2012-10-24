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
 * Interface definition for mutual exclusion semaphores         *
 ****************************************************************/

/** \file os_mutex.h
 *  \brief Critical section management - mutual exclusion semaphore
 */

#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"

/* include OS specific header file				*/
#include "include/os_mutex.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
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
OS_API os_result
os_mutexInit(
    os_mutex *mutex,
    const os_mutexAttr *mutexAttr);

/** \brief Destroy the mutex
 *
 * Possible Results:
 * - assertion failure: mutex = NULL
 * - returns os_resultSuccess if
 *     mutex is successfuly destroyed
 * - returns os_resultBusy if
 *     mutex is not destroyed because it is still claimed or referenced by a thread
 * - returns os_resultFail if
 *     mutex is not destroyed because of a failure
 */
OS_API os_result
os_mutexDestroy(
    os_mutex *mutex);

/** \brief Acquire the mutex
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
OS_API os_result
os_mutexLock(
    os_mutex *mutex);

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
OS_API os_result
os_mutexTryLock(
    os_mutex *mutex);

/** \brief Release the acquired mutex
 *
 * Precondition:
 * - mutex is already acquired by the calling thread
 *
 * Possible Results:
 * - assertion failure: mutex = NULL
 * - returns os_resultSuccess if
 *     mutex is released
 * - returns os_resultFail if
 *     mutex is not released because of a failure
 */
OS_API os_result
os_mutexUnlock(
    os_mutex *mutex);

/** \brief Set the default mutex attributes
 *
 * Postcondition:
 * - mutex scope attribute is OS_SCOPE_SHARED
 *
 * Possible Results:
 * - assertion failure: mutexAttr = NULL
 * - returns os_resultSuccess
 */
OS_API os_result
os_mutexAttrInit(
    os_mutexAttr *mutexAttr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_MUTEX_H */
