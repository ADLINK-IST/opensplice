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

/** \file os/darwin/code/os_mutex.c
 *  \brief Darwin mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for Darwin
 * by including the POSIX implementation
 */

#include <../posix/code/os__mutex.h>
#include <os_signature.h>
#include <assert.h>
#include "os_errno.h"
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
  os_mutexAttr defAttr;
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert (mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif
  if(!mutexAttr) {
      os_mutexAttrInit(&defAttr);
      mutexAttr = &defAttr;
  }
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
#else
    (void)shared;
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
void os_mutexDestroy (os_mutex *mutex)
{
  assert (mutex != NULL);
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_destroy (mutex);
#endif
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
  if (pthread_mutex_destroy (&mutex->mutex) != 0)
    abort();
#ifdef OSPL_STRICT_MEM
  mutex->signature = 0;
#endif
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
void os_mutexLock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
#if HAVE_LKST
  if (!ospl_lkst_enabled)
#endif
  {
    if (pthread_mutex_lock (&mutex->mutex) != 0)
      abort();
  }
#if HAVE_LKST
  else
  {
    unsigned long long t = mach_absolute_time (), dt;
    if (pthread_mutex_trylock (&mutex->mutex) == 0)
      dt = 0;
    else
    {
      if (pthread_mutex_lock (&mutex->mutex) != 0)
        abort();
      dt = 1 | (mach_absolute_time () - t);
    }
    lkst_track_op (mutex, LKST_LOCK, t, dt);
  }
#endif
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock_s calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result os_mutexLock_s (os_mutex *mutex)
{
  int result;
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
#if HAVE_LKST
  if (!ospl_lkst_enabled)
#endif
  {
    result = pthread_mutex_lock (&mutex->mutex);
  }
#if HAVE_LKST
  else
  {
    unsigned long long t = mach_absolute_time (), dt;
    if ((result = pthread_mutex_trylock (&mutex->mutex)) == 0)
      dt = 0;
    else
    {
      result = pthread_mutex_lock (&mutex->mutex);
      dt = 1 | (mach_absolute_time () - t);
    }
    lkst_track_op (mutex, LKST_LOCK, t, dt);
  }
#endif
  return (result == 0) ? os_resultSuccess : os_resultFail;
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
  if (result != 0 && result != EBUSY)
    abort();
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
void os_mutexUnlock (os_mutex *mutex)
{
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
#endif
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_UNLOCK, mach_absolute_time (), 0);
#endif
  if (pthread_mutex_unlock (&mutex->mutex) != 0)
    abort();
}

#include <../common/code/os_mutex_attr.c>
