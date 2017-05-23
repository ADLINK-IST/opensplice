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
/****************************************************************
 * Interface definition for semaphores                          *
 ****************************************************************/

/** \file os_semaphore.h
 *  \brief Thread synchronisation - semaphores
 */

#ifndef OS_SEMAPHORE_H
#define OS_SEMAPHORE_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API os_result os_sem_init (os_sem_t * sem, os_uint32 value);
OS_API os_result os_sem_destroy (os_sem_t * sem);
OS_API os_result os_sem_post (os_sem_t * sem);
OS_API os_result os_sem_wait (os_sem_t * sem);

#undef OS_API
#if defined (__cplusplus)
}
#endif
#endif /* OS_SEMAPHORE_H */
