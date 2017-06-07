/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
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
#define OSPL_SHM_PROCMON
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

const char*
os_getShmDomainKeyForPointer(void *ptr);

int
os_sharedMemIsGlobal(void);

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32__SHAREDMEM_H */
