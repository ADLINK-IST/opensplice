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

/** \file os/darwin/code/os_cond.c
 *  \brief Darwin condition variables
 *
 * Implements condition variables for Darwin
 * by including the POSIX implementation
 */

#include <os_cond.h>
#include <assert.h>
#include "os_errno.h"
#include <os_signature.h>

#include <time.h>
#include <sys/time.h>

#include "os_init.h"

#if HAVE_LKST
#include "lkst.h"
#include "mach/mach_time.h"

extern int ospl_lkst_enabled;
#endif

/** \brief Initialize the condition variable taking the condition
 *         attributes into account
 *
 * \b os_condInit calls \b pthread_cond_init to intialize the posix condition
 * variable.
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * condition variable "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 */
os_result os_condInit (os_cond *cond, os_mutex *dummymtx __attribute__ ((unused)), const os_condAttr *condAttr)
{
  os_condAttr defAttr;
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert(cond->signature != OS_COND_MAGIC_SIG);
#endif

  if(!condAttr) {
    os_condAttrInit(&defAttr);
    condAttr = &defAttr;
  }
  if (condAttr->scopeAttr != OS_SCOPE_SHARED)
  {
    pthread_cond_init (&cond->cond, NULL);
  }
  else
  {
    pthread_condattr_t ca;
    pthread_condattr_init (&ca);
    pthread_condattr_setpshared (&ca, PTHREAD_PROCESS_SHARED);
    pthread_cond_init (&cond->cond, &ca);
    pthread_condattr_destroy (&ca);
  }
#ifdef OSPL_STRICT_MEM
  cond->signature = OS_COND_MAGIC_SIG;
#endif
  return os_resultSuccess;
}

/** \brief Destroy the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
void os_condDestroy (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert(cond->signature == OS_COND_MAGIC_SIG);
#endif
  if (pthread_cond_destroy (&cond->cond) != 0)
    abort();
#ifdef OSPL_STRICT_MEM
  cond->signature = 0;
#endif
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
void os_condWait (os_cond *cond, os_mutex *mutex)
{
  assert (cond != NULL);
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
  assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_UNLOCK, mach_absolute_time (), 0);
#endif
  if (pthread_cond_wait (&cond->cond, &mutex->mutex) != 0)
    abort();
#if HAVE_LKST
  /* Have no way of determining whether it was uncontended or not, and
     if not, how long the wait was. */
  if (ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_LOCK, mach_absolute_time (), 0);
#endif
}

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 *
 * \b os_condTimedWait calls \b pthread_cond_timedwait to
 * wait for the condition with a timeout.
 *
 * \b os_condTimedWait provides an relative time to wait while
 * \b pthread_cond_timedwait expects an absolute time to wakeup.
 * The absolute time is calculated from the current time + the
 * provided relative time.
 *
 * \b os_condTimedWait will repeat \b pthread_cond_timedwait in case of an
 * interrupted system call. Because the time which is passed onto
 * \b pthread_cond_timedwait is absolute, no remaining time must be
 * calculated.
 */
os_result os_condTimedWait (os_cond *cond, os_mutex *mutex, os_duration timeout)
{
  struct timespec t;
  int result;
  os_timeW wt;
  struct timeval tv;

  assert (cond != NULL);
  assert (mutex != NULL);
  assert (OS_DURATION_ISPOSITIVE(timeout));
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
  assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif

  (void) gettimeofday (&tv, NULL);
  wt = OS_TIMEW_INIT(tv.tv_sec, 1000 * tv.tv_usec);
  wt = os_timeWAdd(wt, timeout);
  t.tv_sec = (time_t)OS_TIMEW_GET_SECONDS(wt);
  t.tv_nsec = OS_TIMEW_GET_NANOSECONDS(wt);
#if HAVE_LKST
  if (ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_UNLOCK, mach_absolute_time (), 0);
#endif
  /* By default Darwin uses the realtime clock in pthread_cond_timedwait().
   * Unfortunately Darwin has not (yet) implemented
   * pthread_condattr_setclock(), so we cannot tell it to use the
   * the monotonic clock. */
  result = pthread_cond_timedwait (&cond->cond, &mutex->mutex, &t);
  if (result != 0 && result != ETIMEDOUT)
    abort();
#if HAVE_LKST
  /* Have no way of determining whether it was uncontended or not, and
     if not, how long the wait was. */
  if (ospl_lkst_enabled)
    lkst_track_op (mutex, LKST_LOCK, mach_absolute_time (), 0);
#endif
  return (result == ETIMEDOUT) ? os_resultTimeout : os_resultSuccess;
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * \b os_condSignal calls \b pthread_cond_signal to signal
 * the condition.
 */
void os_condSignal (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
#endif
  if (pthread_cond_signal (&cond->cond) != 0)
    abort();
}

/** \brief Signal the condition and wakeup all threads waiting
 *         for the condition
 *
 * \b os_condBroadcast calls \b pthread_cond_broadcast to broadcast
 * the condition.
 */
void os_condBroadcast (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
#endif
  if (pthread_cond_broadcast (&cond->cond) != 0)
    abort();
}

#include <../common/code/os_cond_attr.c>
