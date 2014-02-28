
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
#ifndef OS_UNQIUENODEID_H
#define OS_UNQIUENODEID_H


#include <os_defs.h>

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif

OS_API os_uint32 os_uniqueNodeIdGet();

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
