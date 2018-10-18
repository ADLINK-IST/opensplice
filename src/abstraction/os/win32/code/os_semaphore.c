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
/** \file os/win32/code/os_semaphore.c
 *  \brief WIN32 semaphore
 *
 * Implements semaphore for WIN32
 */

#include "os_win32incs.h"
#include "os_stdlib.h"
#include "os_semaphore.h"

os_result os_sem_init (os_sem_t * sem, os_uint32 value)
{
  *sem = CreateSemaphore (NULL, value, 32767, NULL);
  return (*sem) ? os_resultSuccess : os_resultFail;
}

os_result os_sem_destroy (os_sem_t * sem)
{
  return (CloseHandle (*sem)) ? os_resultSuccess : os_resultFail;
}

os_result os_sem_post (os_sem_t * sem)
{
  return (ReleaseSemaphore (*sem, 1, NULL)) ? os_resultSuccess : os_resultFail;
}

os_result os_sem_wait (os_sem_t * sem)
{
  return (WaitForSingleObject (*sem, INFINITE) == 0) ? os_resultSuccess : os_resultFail;
}
