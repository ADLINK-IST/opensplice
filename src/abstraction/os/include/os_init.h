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
 * Interface definition for OS layer initialization             *
 ****************************************************************/

/** \file os_init.h
 *  \brief OS layer initialization / deinitialization
 */

#ifndef OS_INIT_H
#define OS_INIT_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_defs.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * OS layer initialization. This call initializes the process-, shared-
 * memory-, mutex-, etc-administration for the os-abstraction. Its
 * antagonist is os_osExit(), which should both occur in pairs within
 * the context of a process. The first call actually performs the
 * administration. Consecutive calls will only increment a counter.
 *
 * @see os_osExit()
 */
OS_API void
os_osInit(void);

/**
 * OS layer de-initialization. This call de-initializes the process-,
 * shared-memory-, mutex-, etc-administration for the os-abstraction. Its
 * antagonist is os_osInit(), which should both occur in pairs. Every call
 * will decrement the initialization counter. The last call actually
 * performs the de-initialization. Any consecutive calls will generate a
 * warning (and further do nothing). In a debugging environment the call
 * will crash if called more often than os_osInit().
 *
 * @see os_osInit()
 */
OS_API void
os_osExit(void);

OS_API os_result
os_serviceStart(const char *name);

OS_API os_result
os_serviceStop(void);

OS_API const char *
os_serviceName(void);

/**
* Return the version string for OpenSplice DDS. Caller does not own.
*/
OS_API const char *
os_versionString(void);

OS_API void
os_createPipeNameFromDomainName(const char *name);

/**
 * Inform the os layer that this is a single process configuration
 */
OS_API void
os_serviceSetSingleProcess (void);

/**
 * Ask the os layer whether this is a single process configuration
 */
OS_API os_boolean
os_serviceGetSingleProcess (void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_INIT_H */
