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

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "os_abstract.h"
#include <tlsLib.h>

#define VXWORKS_FIX_MAX_TSS_KEYS 16

int vxworks_fix_pthread_key_create (pthread_key_t * key, void *destructor)
{
   int res = 0;
   pthread_key_t val;

   val = tlsKeyCreate();

   if (val < VXWORKS_FIX_MAX_TSS_KEYS)
   {
      *key = val;
   }
   else
   {
      res = -1;
   }

   return res;
}

int vxworks_fix_pthread_key_delete (pthread_key_t key)
{
   return 0;
}

int vxworks_fix_pthread_setspecific (pthread_key_t key, void *data)
{
   STATUS vxRet = tlsValueSet(key, data);
   assert (vxRet == OK);
   return 0;
}

void *vxworks_fix_pthread_getspecific (pthread_key_t key)
{
   return tlsValueGet(key);
}

#define pthread_key_create vxworks_fix_pthread_key_create
#define pthread_key_delete vxworks_fix_pthread_key_delete
#define pthread_setspecific vxworks_fix_pthread_setspecific
#define pthread_getspecific vxworks_fix_pthread_getspecific

/* PR_SET_NAME doesn't seem to be available on VxWorks */
#define OS_HAS_NO_SET_NAME_PRCTL

#include "../posix/code/os_thread.c"
#include "os_thread_attr.c"
#include "os_threadWaitExit.c"
