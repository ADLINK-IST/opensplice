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

#ifndef OS_VXWORKS_KM_COND_H
#define OS_VXWORKS_KM_COND_H

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct _vxworks_cond {   
#ifdef OSPL_STRICT_MEM
    uint64_t signature; /* Used to identify initialized cond when memory is
                           freed - keep this first in the structure so its
                           so its address is the same as the os_cond */
#endif
    SEM_ID   os_cond;
    os_os_int32 os_countLock;
} vxworks_cond_t;

typedef vxworks_cond_t os_os_cond;

#if defined (__cplusplus)
}
#endif

#endif /* OS_VXWORKS_COND_H */
