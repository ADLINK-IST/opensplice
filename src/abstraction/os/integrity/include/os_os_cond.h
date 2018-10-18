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
#ifndef OS_INTEGRITY_COND_H
#define OS_INTEGRITY_COND_H

#include <INTEGRITY.h>

#if defined (__cplusplus)
extern "C" {
#endif

   typedef struct
   {
#ifndef NDEBUG
      uint64_t signature; /* Used to identify initialized cond when memory is
                             freed - keep this first in the structure so its
                             so its address is the same as the os_cond */
#endif
      Condition localcond; /* Used only for process local conditions,
                             NULL for process shared conditions */
      int index;
      uint64_t uid; /* Unique ID for every cond to ensure flushing of stale */
                    /* entries in the per address space caches */
   } os_os_cond;

#if defined (__cplusplus)
}
#endif

#endif /* OS_INTEGRITY_COND_H */
