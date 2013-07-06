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
#include <stddef.h>

#include "os_init.h"

#if HAVE_LKST
#include "lkst.h"
#include "mach/mach_time.h"
#endif

static os_boolean ospl_mtx_prio_inherit = OS_FALSE;
#if HAVE_LKST
int ospl_lkst_enabled;
#endif

#if defined __GLIBC_PREREQ
#if __GLIBC_PREREQ(2,5)
#define OSPL_PRIO_INHERIT_SUPPORTED
#endif
#endif

void
os_mutexModuleInit()
{
#if HAVE_LKST
  ospl_lkst_enabled = lkst_init (1);
#endif
  ospl_mtx_prio_inherit = 0;
}

void
os_mutexModuleExit()
{
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_fini ();
#endif
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
  int shared;
  assert (mutex != NULL);
  assert (mutexAttr != NULL);
#ifdef OSPL_STRICT_MEM
  assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif
  if (mutexAttr->scopeAttr != OS_SCOPE_SHARED || os_serviceGetSingleProcess ())
  {
    pthread_mutex_init (&mutex->mutex, NULL);
    shared = 0;
  }
  else
  {
    pthread_mutexattr_t ma;
    pthread_mutexattr_init (&ma);
    pthread_mutexattr_setpshared (&ma, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init (&mutex->mutex, &ma);
    pthread_mutexattr_destroy (&ma);
    shared = 1;
  }
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_init (mutex, shared ? LKST_MF_SHARED : 0);
#endif
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
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_destroy (mutex);
#endif
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  pthread_mutex_destroy (&mutex->mutex);
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
#if HAVE_LKST
  if (!ospl_lkst_enabled)
#endif
  {
    pthread_mutex_lock (&mutex->mutex);
  }
#if HAVE_LKST
  else
  {
    unsigned long long t = mach_absolute_time (), dt;
    if (pthread_mutex_trylock (&mutex->mutex) == 0)
      dt = 0;
    else
    {
      pthread_mutex_lock (&mutex->mutex);
      dt = 1 | (mach_absolute_time () - t);
    }
    lkst_track_op (mutex, LKST_LOCK, t, dt);
  }
#endif
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
  result = pthread_mutex_trylock (&mutex->mutex);
#if HAVE_LKST
  if (result == 0 && ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_LOCK, mach_absolute_time (), 0);
#endif
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
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_UNLOCK, mach_absolute_time (), 0);
#endif
  pthread_mutex_unlock (&mutex->mutex);
  return os_resultSuccess;
}

#include <../common/code/os_mutex_attr.c>
