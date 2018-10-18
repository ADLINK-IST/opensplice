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
#ifndef V_GROUPSTORE_H
#define V_GROUPSTORE_H

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

C_CLASS(v_groupStoreQuery);

OS_API v_groupStoreQuery
v_groupStoreQueryNew(
    const v_groupStore _this,
    const os_char *expression,
    const c_value params[],
    const os_uint32 nrOfParams);

OS_API v_groupStoreQuery
v_groupStoreQueryNew2(
    const v_groupStore _this,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams);

OS_API void
v_groupStoreQueryFree(
    v_groupStoreQuery _this);

typedef os_boolean (*v_groupStoreAction)(const v_groupSample sample, const void *actionArg);

OS_API v_result
v_groupStoreRead(
    const v_groupStore _this,
    const v_groupStoreQuery query,
    const v_groupStoreAction action,
    const void *actionArg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
