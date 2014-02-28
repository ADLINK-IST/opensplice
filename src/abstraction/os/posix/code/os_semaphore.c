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
