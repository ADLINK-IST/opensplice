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
/** \file os/win32/code/os__socket.h
 *  \brief WIN32 socket management
 */
#ifndef __OS_WIN32__SOCKET_H__
#define __OS_WIN32__SOCKET_H__

#define OS_SOCK_VERSION         2
#define OS_SOCK_REVISION        0
#include "os_report.h"
#include "os_if.h"
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef void (*reportfn_t) (os_reportType reportType,
                            const char *reportContext,
                            const char *fileName,
                            os_int32 lineNo,
                            os_int32 reportCode,
                            const char *description,
                            ...);

//typedef void *(*mallocfn_t) (os_size_t size);

OS_API void os_socketModuleInit();

OS_API void os_socketModuleExit();

#undef OS_API
#endif /* __OS_WIN32__SOCKET_H__ */
