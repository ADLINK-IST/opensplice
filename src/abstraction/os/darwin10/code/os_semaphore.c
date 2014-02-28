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

/** \file os/posix/code/os_semaphore.c
 *  \brief Semaphore - os_semaphore
 *
 * Implements os_semaphore for POSIX
 */

#include "os_semaphore.h"

os_result os_sem_init (os_sem_t * sem, os_uint32 value)
{
  sem->value = value;
  pthread_mutex_init (&sem->mtx, NULL);
  pthread_cond_init (&sem->cv, NULL);
  return os_resultSuccess;
}

os_result os_sem_destroy (os_sem_t *sem)
{
  pthread_mutex_destroy (&sem->mtx);
  pthread_cond_destroy (&sem->cv);
  return os_resultSuccess;
}

os_result os_sem_post (os_sem_t *sem)
{
  pthread_mutex_lock (&sem->mtx);
  if (sem->value++ == 0)
    pthread_cond_signal (&sem->cv);
  pthread_mutex_unlock (&sem->mtx);
  return os_resultSuccess;
}

os_result os_sem_wait (os_sem_t *sem)
{
  pthread_mutex_lock (&sem->mtx);
  while (sem->value == 0)
    pthread_cond_signal (&sem->cv);
  pthread_mutex_unlock (&sem->mtx);
  return os_resultSuccess;
}
