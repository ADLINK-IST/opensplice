/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/** \file os/win32/code/os_semaphore.c
 *  \brief WIN32 semaphore
 *
 * Implements semaphore for WIN32
 */
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
