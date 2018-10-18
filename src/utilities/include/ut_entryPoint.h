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
#ifndef UT_ENTRYPOINT_H
#define UT_ENTRYPOINT_H

#include "vortex_os.h"

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

typedef int (*ut_entryPointFunc)(int,char **);

typedef struct ut_entryPointWrapperArg {
    ut_entryPointFunc entryPoint;
    os_int32 argc;
    os_char **argv;
} ut_entryPointWrapperArg;

OS_API void* ut_entryPointWrapper (void *arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_ENTRYPOINT_H */
