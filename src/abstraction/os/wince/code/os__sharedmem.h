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

#ifndef OS_WIN32__SHAREDMEM_H
#define OS_WIN32__SHAREDMEM_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_mutex.h"
#include "os_cond.h"

/** \brief Initialize shared memory module
 */
void
os_sharedMemoryInit (
    void);

/** \brief Deinitialize shared memory module
 */
void
os_sharedMemoryExit (
    void);

const char *
os_getDomainNameforMutex(
        os_mutex *mutex);

const char*
os_getDomainNameforCond (
        os_cond* cond);

os_address
os_getShmBaseAddressFromPointer(
    void *vpointer);


#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32__SHAREDMEM_H */
