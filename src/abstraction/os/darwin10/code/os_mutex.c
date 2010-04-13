/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

/** \file os/darwin/code/os_mutex.c
 *  \brief Darwin mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for Darwin
 * by including the POSIX implementation
 */

#include <../posix/code/os__mutex.h>
#include <os_signature.h>
#include <assert.h>
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
    ospl_mtx_prio_inherit = 0;
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
os_result os_mutexInit (os_mutex *mutex, const os_mutexAttr *mutexAttr)
{
  assert (mutex != NULL);
  assert (mutexAttr != NULL);
#ifdef OSPL_STRICT_MEM
  assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif
  if (mutexAttr->scopeAttr != OS_SCOPE_SHARED)
  {
    mutex->shared = 0;
    pthread_mutex_init (&mutex->u.mutex, NULL);
  }
  else
  {
    mutex->shared = 1;
    __msem_mutex_init (&mutex->u.msem);
  }
#ifdef OSPL_STRICT_MEM
  mutex->signature = OS_MUTEX_MAGIC_SIG;
#endif
  return os_resultSuccess;
}

/** \brief Destroy the mutex
 *
 * \b os_mutexDestroy calls \b pthread_mutex_destroy to destroy the
 * posix \b mutex.
 */
os_result os_mutexDestroy (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  if (mutex->shared)
    __msem_mutex_destroy (&mutex->u.msem);
  else
    pthread_mutex_destroy (&mutex->u.mutex);
#ifdef OSPL_STRICT_MEM
  mutex->signature = 0;
#endif
  return os_resultSuccess;
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result os_mutexLock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  if (mutex->shared)
    __msem_mutex_lock (&mutex->u.msem);
  else
    pthread_mutex_lock (&mutex->u.mutex);
  return os_resultSuccess;
}

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 *
 * \b os_mutexTryLock calls \b pthread_mutex_trylock to acquire
 * the posix \b mutex.
 */
os_result os_mutexTryLock (os_mutex *mutex)
{
  int result;
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  if (mutex->shared)
    result = __msem_mutex_trylock (&mutex->u.msem);
  else
    result = pthread_mutex_trylock (&mutex->u.mutex);
  return (result == 0) ? os_resultSuccess : os_resultBusy;
}

/** \brief Release the acquired mutex
 *
 * \b os_mutexUnlock calls \b pthread_mutex_unlock to release
 * the posix \b mutex.
 */
os_result os_mutexUnlock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  if (mutex->shared)
    __msem_mutex_unlock (&mutex->u.msem);
  else
    pthread_mutex_unlock (&mutex->u.mutex);
  return os_resultSuccess;
}

#include <../common/code/os_mutex_attr.c>
