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

/** \file os/posix/code/os_semaphore.c
 *  \brief Semaphore - os_semaphore
 *
 * Implements os_semaphore for Darwin
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
    pthread_cond_wait (&sem->cv, &sem->mtx);
  pthread_mutex_unlock (&sem->mtx);
  return os_resultSuccess;
}
