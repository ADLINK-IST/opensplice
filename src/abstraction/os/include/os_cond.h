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
 * Interface definition for condition variables of SPLICE-DDS   *
 ****************************************************************/

/** \file os_cond.h
 *  \brief Event management - condition variable
 */

#ifndef OS_COND_H
#define OS_COND_H

#if defined (__cplusplus)
extern "C" {
#endif

/* Define all types used in this interface			*/
#include "os_defs.h"
#include "os_mutex.h"
#include "os_time.h"

/* Include OS specific header file				*/
#include "include/os_cond.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
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
     * - OS_SCOPE_PRIVATE The scope of the condition vaiable
     *   is process wide
     */
    os_scopeAttr	scopeAttr;
} os_condAttr;

/** \brief Initialize the condition variable taking the conition
 *         attributes into account
 * 
 * Possible Results:
 * - assertion failure: cond = NULL || condAttr = NULL
 * - returns os_resultSuccess if
 *     cond is successfuly initialized
 * - returns os_resultFail if
 *     cond is not initialized and can not be used
 */
OS_API os_result
os_condInit(
    os_cond *cond, 
    os_mutex *mutex,
    const os_condAttr *condAttr);

/** \brief Destory the condition variable
 *
 * Possible Results:
 * - assertion failure: cond = NULL
 * - returns os_resultSuccess if
 *     cond is successfuly destroyed
 * - returns os_reultBusy if
 *     cond is not destroyed because it is still referenced by a thread
 * - returns os_resultFail if
 *     cond is not destroyed
 */
OS_API os_result
os_condDestroy(
    os_cond *cond);

/** \brief Wait for the condition
 *
 * Precondition:
 * - mutex is acquired by the calling thread before calling
 *   os_condWait
 *
 * Postcondition:
 * - mutex is still acquired by the calling thread and should
 *   be released by it
 *
 * Possible Results:
 * - assertion failure: cond = NULL || mutex = NULL
 * - returns os_resultSuccess if
 *     cond is triggered
 * - returns os_resultFail
 *     cond is not triggered but os_condWait has returned
 *     because of a failure
 */
OS_API os_result
os_condWait(
    os_cond *cond,
    os_mutex *mutex);

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
    const os_time *time);

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * Precondition:
 * - the mutex used with the condition in general should be
 *   acquired by the calling thread before setting the
 *   condition state and signalling
 *
 * Postcondition:
 * - the mutex used with the condition must be released
 *
 * Possible Results:
 * - assertion failure: cond = NULL
 * - returns os_resultSuccess if
 *     cond is signalled
 * - returns os_resultFail if
 *     cond is not signalled because of a failure
 */
OS_API os_result
os_condSignal(
    os_cond *cond);

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 *
 * Precondition:
 * - the mutex used with the condition in general should be
 *   acquired by the calling thread before setting the
 *   condition state and signalling
 *
 * Postcondition:
 * - the mutex used with the condition must be released
 *
 * Possible Results:
 * - assertion failure: cond = NULL
 * - returns os_resultSuccess if
 *     cond is signalled
 * - returns os_resultFail if
 *     cond is not signalled because of a failure
 */
OS_API os_result
os_condBroadcast(
    os_cond *cond);

/** \brief Set the default condition variable attributes
 *
 * Postcondition:
 * - condition scope attribute is OS_SCOPE_SHARED
 *
 * Possible Results:
 * - assertion failure: condAttr = NULL
 * - returns os_resultSuccess
 */
OS_API os_result
os_condAttrInit(
    os_condAttr *condAttr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_COND_H */
