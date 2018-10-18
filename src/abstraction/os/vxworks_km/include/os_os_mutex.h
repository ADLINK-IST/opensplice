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

#ifndef OS_VXWORKS_MUTEX_H
#define OS_VXWORKS_MUTEX_H

#include <vxWorks.h>
#include <semLib.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct _vxworks_mutex {   
#ifdef OSPL_STRICT_MEM
    uint64_t signature; /* Used to identify initialized cond when memory is
                           freed - keep this first in the structure so its
                           so its address is the same as the os_mutex */
#endif
    SEM_ID   os_sem;
    os_os_int32 os_countLock;
} vxworks_mutex_t;

typedef vxworks_mutex_t os_os_mutex;

#if defined (__cplusplus)
}
#endif

#endif /* OS_VXWORKS_MUTEX_H */
