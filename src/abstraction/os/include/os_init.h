/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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

#ifdef OSPL_BUILD_OS
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
 * administration. Consecutive calls will only increment a counter and
 * an info-log will be written to notify that the initialization has
 * been called multiple times.
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


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_INIT_H */
