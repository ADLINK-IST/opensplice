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
/** \file os/win32/code/os_semaphore.c
 *  \brief WIN32 semaphore
 *
 * Implements semaphore for WIN32
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
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
