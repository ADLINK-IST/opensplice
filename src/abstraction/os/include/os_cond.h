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
 * Interface definition for condition variables of SPLICE-DDS   *
 ****************************************************************/

/** \file os_cond.h
 *  \brief Event management - condition variable
 */

#ifndef OS_COND_H
#define OS_COND_H

/* Define all types used in this interface			*/
#include "os_defs.h"
#include "os_mutex.h"
#include "os_time.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Definition of the condition variable
 *
 * os_cond is a platform specific definition for a condition variable.
 * The condition variable can be defined in system shared memory in order
 * to synchronize processes and threads, or the condition variable can
 * be placed in process shared memory in order to synchronize threads.
 */
typedef os_os_cond os_cond;

/** \brief Definition of the condition variable attributes
 */
typedef struct os_condAttr {
    /**
     * - OS_SCOPE_SHARED The scope of the condition variable
     *   is system wide
     * - OS_SCOPE_PRIVATE The scope of the condition variable
     *   is process wide
     */
    os_scopeAttr    scopeAttr;
} os_condAttr;

/** \brief Initialize the condition variable taking the conition
 *         attributes into account
 * If condAttr == NULL, result is as if os_condInit was invoked
 * with the default os_condAttr as after os_condAttrInit.
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     cond is successfuly initialized
 * - returns os_resultFail if
 *     cond is not initialized and can not be used
 */
_Check_return_
_When_(condAttr == NULL, _Pre_satisfies_(mutex->scope == OS_SCOPE_PRIVATE))
_When_(condAttr != NULL, _Pre_satisfies_(mutex->scope == condAttr->scopeAttr))
OS_API os_result
os_condInit(
        _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os_cond *cond,
        _In_ os_mutex *mutex,
        _In_opt_ const os_condAttr *condAttr)
    __nonnull((1,2));

/** \brief Destroy the condition variable
 */
OS_API void
os_condDestroy(
        _Inout_ _Post_invalid_ os_cond *cond)
    __nonnull_all__;

/** \brief Wait for the condition
 *
 * Precondition:
 * - mutex is acquired by the calling thread before calling
 *   os_condWait
 *
 * Postcondition:
 * - mutex is still acquired by the calling thread and should
 *   be released by it
 */
OS_API void
os_condWait(
        os_cond *cond,
        os_mutex *mutex)
    __nonnull_all__;

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 *
 * Precondition:
 * - mutex is acquired by the calling thread before calling
 *   os_condTimedWait
 *
 * Postcondition:
 * - mutex is still acquired by the calling thread and should
 *   be released by it
 *
 * Possible Results:
 * - assertion failure: cond = NULL || mutex = NULL ||
 *     time = NULL
 * - returns os_resultSuccess if
 *     cond is triggered
 * - returns os_resultTimeout if
 *     cond is timed out
 * - returns os_resultFail if
 *     cond is not triggered nor is timed out but
 *     os_condTimedWait has returned because of a failure
 */
OS_API os_result
os_condTimedWait(
        os_cond *cond,
        os_mutex *mutex,
        os_duration time)
    __nonnull_all__;

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * Precondition:
 * - the mutex used with the condition in general should be
 *   acquired by the calling thread before setting the
 *   condition state and signalling
 */
OS_API void
os_condSignal(
        os_cond *cond)
    __nonnull_all__;

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 *
 * Precondition:
 * - the mutex used with the condition in general should be
 *   acquired by the calling thread before setting the
 *   condition state and signalling
 */
OS_API void
os_condBroadcast(
        os_cond *cond)
    __nonnull_all__;

/** \brief Set the default condition variable attributes
 *
 * Postcondition:
 * - condition scope attribute is OS_SCOPE_SHARED
 *
 * Precondition:
 * - condAttr is a valid object
 */
_Post_satisfies_(condAttr->scopeAttr == OS_SCOPE_PRIVATE)
OS_API void
os_condAttrInit(
        _Out_ os_condAttr *condAttr)
    __nonnull_all__;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_COND_H */
