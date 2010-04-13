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

/** \file os/darwin/code/os_cond.c
 *  \brief Darwin condition variables
 *
 * Implements condition variables for Darwin 
 * by including the POSIX implementation
 */

#include <os_cond.h>
#include <assert.h>
#include <errno.h>
#include <os_signature.h>

struct __os_msem_cond_block
{
  struct __os_msem_cond_block *prev, *next;
  __msem_t sem;
  long int state;
  int caslock;
};

struct cond_to_list
{
  long long to;
  struct __os_msem_cond_block *cb;
  struct cond_to_list *prev, *next;
};

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
  assert (cond != NULL);
  assert (condAttr != NULL);
#ifdef OSPL_STRICT_MEM
  assert(cond->signature != OS_COND_MAGIC_SIG);
#endif
  if (condAttr->scopeAttr != OS_SCOPE_SHARED)
  {
    cond->shared = 0;
    pthread_cond_init (&cond->u.cond, NULL);
  }
  else
  {
    cond->shared = 1;
    __msem_cond_init (&cond->u.msem);
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
os_result os_condDestroy (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert(cond->signature == OS_COND_MAGIC_SIG);
#endif
  if (!cond->shared)
    pthread_cond_destroy (&cond->u.cond);
  else
    __msem_cond_destroy (&cond->u.msem);
#ifdef OSPL_STRICT_MEM
  cond->signature = 0;
#endif
  return os_resultSuccess;
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
os_result os_condWait (os_cond *cond, os_mutex *mutex)
{
  assert (cond != NULL);
  assert (mutex != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
  assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif
  assert ((cond->shared && mutex->shared) || (!cond->shared && !mutex->shared));
  if (!cond->shared)
    pthread_cond_wait (&cond->u.cond, &mutex->u.mutex);
  else
    __msem_cond_wait (&cond->u.msem, &mutex->u.msem);
  return os_resultSuccess;
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
os_result os_condTimedWait (os_cond *cond, os_mutex *mutex, const os_time *time)
{
  struct timespec t;
  int result;
  os_time wakeup_time;

  assert (cond != NULL);
  assert (mutex != NULL);
  assert (time != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
  assert( mutex->signature == OS_MUTEX_MAGIC_SIG );
#endif
  assert ((cond->shared && mutex->shared) || (!cond->shared && !mutex->shared));

  wakeup_time = os_timeAdd (os_timeGet(), *time);
  t.tv_sec = wakeup_time.tv_sec;
  t.tv_nsec = wakeup_time.tv_nsec;

  if (!cond->shared)
    result = pthread_cond_timedwait (&cond->u.cond, &mutex->u.mutex, &t);
  else
    result = __msem_cond_timedwait (&cond->u.msem, &mutex->u.msem, &t);
  return (result == ETIMEDOUT) ? os_resultTimeout : os_resultSuccess;
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * \b os_condSignal calls \b pthread_cond_signal to signal
 * the condition.
 */
os_result os_condSignal (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
#endif
  if (!cond->shared)
    pthread_cond_signal (&cond->u.cond);
  else
    __msem_cond_signal (&cond->u.msem);
  return os_resultSuccess;
}

/** \brief Signal the condition and wakeup all threads waiting
 *         for the condition
 *
 * \b os_condBroadcast calls \b pthread_cond_broadcast to broadcast
 * the condition.
 */
os_result os_condBroadcast (os_cond *cond)
{
  assert (cond != NULL);
#ifdef OSPL_STRICT_MEM
  assert( cond->signature == OS_COND_MAGIC_SIG );
#endif
  if (!cond->shared)
    pthread_cond_broadcast (&cond->u.cond);
  else
    __msem_cond_broadcast (&cond->u.msem);
  return os_resultSuccess;
}

#include <../common/code/os_cond_attr.c>
