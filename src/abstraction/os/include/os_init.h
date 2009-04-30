/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include <os_defs.h>
#include <os_if.h>

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief OS layer initialization */
OS_API void
os_osInit(void);

/** \brief OS layer deinitialization */
OS_API void
os_osExit(void);

OS_API os_result
os_serviceStart(const char *name);

OS_API os_result
os_serviceStop(void);

OS_API const char *
os_serviceName(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_INIT_H */
