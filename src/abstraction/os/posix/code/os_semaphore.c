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

/** \file os/posix/code/os_semaphore.c
 *  \brief Semaphore - os_semaphore
 *
 * Implements os_semaphore for POSIX
 */

#include <semaphore.h>

os_result os_sem_init (sem_t * sem, os_uint32 value)
{
  return (sem_init (sem, 0, value) == 0) ? os_resultSuccess : os_resultFail;
}

os_result os_sem_destroy (sem_t *sem)
{
  return (sem_destroy (sem) == 0) ? os_resultSuccess : os_resultFail;
}

os_result os_sem_post (sem_t *sem)
{
  return (sem_post (sem) == 0) ? os_resultSuccess : os_resultFail;
}

os_result os_sem_wait (sem_t *sem)
{
  return (sem_wait (sem) == 0) ? os_resultSuccess : os_resultFail;
}
