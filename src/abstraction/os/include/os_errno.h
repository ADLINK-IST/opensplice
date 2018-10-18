/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef OS_ERRNO_H
#define OS_ERRNO_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"

#if (defined(WIN32) || defined(WINCE))
#include <winerror.h>
#endif

#include <errno.h> /* Required on Windows platforms too */

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/** \brief Get error code set by last operation that failed
 *
 * @return Error code
 */
OS_API os_int
os_getErrno (
    void);

/** \brief Set error code to specified value
 *
 * @return void
 * @param err Error code
 */
OS_API void
os_setErrno (
    os_int err);

/**
 * \brief Get string representation for specified error code
 *
 * @return Pointer to string allocated in thread specific memory
 * @param err Error number
 */
OS_API const os_char *
os_strError (
    os_int err);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_ERRNO_H */
