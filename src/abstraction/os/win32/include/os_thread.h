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

#ifndef OS_WIN32_THREAD_H
#define OS_WIN32_THREAD_H


#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef unsigned long DWORD;
typedef void * HANDLE;

typedef struct os_threadInfo_s
{
    DWORD threadId;
    HANDLE handle;
} os_threadInfo;

typedef os_threadInfo os_os_threadId;

OS_API os_os_threadId id_none;

#define OS_THREAD_ID_NONE id_none


#if defined (__cplusplus)
}
#endif

#undef OS_API
#endif /* OS_WIN32_THREAD_H */
