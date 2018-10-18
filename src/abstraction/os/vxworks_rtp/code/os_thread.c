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

/** \file os/vxworks6.6/code/os_thread.c
 *  \brief VxWorks RTP thread management
 *
 * Implements thread management for VxWorks RTP
 * by including the POSIX implementation
 */
#include <version.h>
#if ( _WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 6 )
/* Vxworks 6.5 doesn't support __thread so alterate implemtation using tlsLib*/
/* Would be nice to abstract tls facility, but VxWorks 6.5 support will likely
   be dropped soon in anycase */
#include "os_thread_vx6_5.c"
#else

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "os_report.h"
#include "os_abstract.h"
#include "os_atomics.h"

#define VXWORKS_FIX_MAX_TSS_KEYS 16

static __thread void *os_tss_data[VXWORKS_FIX_MAX_TSS_KEYS];

static pa_uint32_t os_tss_key = PA_UINT32_INIT(0);

int vxworks_fix_pthread_key_create (pthread_key_t * key, void *destructor)
{
   int res = 0;
   pthread_key_t val;
   (void)destructor;

   val = (pa_inc32_nv (&os_tss_key)) - 1;

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
   (void)key;
   return 0;
}

int vxworks_fix_pthread_setspecific (pthread_key_t key, void *data)
{
   os_tss_data[key] = data;
   return 0;
}

void *vxworks_fix_pthread_getspecific (pthread_key_t key)
{
   return os_tss_data[key];
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
#endif
