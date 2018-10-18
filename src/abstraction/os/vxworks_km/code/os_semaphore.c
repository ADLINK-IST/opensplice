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

/** \file os/vxworks5.5/code/os_semaphore.c
 *  \brief VxWorks semaphores
 *
 * Implements semaphores for VxWorks
 * by including the POSIX implementation
 */

/** \file os/posix/code/os_semaphore.c
 *  \brief Posix semaphores
 *
 * Implements semaphores for VxWorks
 */

#include <vxWorks.h>
#include "os_semaphore.h"
#include <assert.h>
#include "os_errno.h"
#include <objLib.h>
#include <taskLib.h>

os_result os_sem_init (os_sem_t * sem, os_uint32 value)
{
   *sem = semCCreate (SEM_Q_PRIORITY, value);
   return (*sem == 0 ? os_resultFail : os_resultSuccess);
}

os_result os_sem_post (os_sem_t * sem)
{
   STATUS status = semGive (*sem);
   return (status == OK ? os_resultSuccess : os_resultFail);
}

os_result os_sem_wait (os_sem_t * sem)
{
   STATUS status = semTake (*sem, WAIT_FOREVER);
   return (status == OK ? os_resultSuccess : os_resultFail);
}

os_result os_sem_destroy (os_sem_t * sem)
{
   STATUS status = semDelete (*sem);
   return (status == OK ? os_resultSuccess : os_resultFail);
}
