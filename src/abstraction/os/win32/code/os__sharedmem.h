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

char *
os_getDomainNameforMutex(
        os_mutex *mutex);

char* os_getDomainNameforCond (os_cond* cond);

const char*
os_getShmDomainKeyForPointer(void *ptr);

int
os_sharedMemIsGlobal(void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32__SHAREDMEM_H */
