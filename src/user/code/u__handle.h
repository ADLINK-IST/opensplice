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
#ifndef U__HANDLE_H
#define U__HANDLE_H

#include "u__types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u__handleResult(result) \
        ((result == V_HANDLE_OK) ? U_RESULT_OK : \
         (result == V_HANDLE_EXPIRED) ? U_RESULT_HANDLE_EXPIRED : \
                                        U_RESULT_ILL_PARAM)

extern const u_handle U_HANDLE_NIL;

u_handle
u_handleNew(
    const v_public object);

u_result
u_handleClaim (
    const u_handle _this,
    const c_voidp instance);

u_result
u_handleRelease (
    const u_handle _this);

u_bool
u_handleIsNil (
    const u_handle _this);

u_bool
u_handleIsEqual (
    const u_handle h1,
    const u_handle h2);

void
u_handleSetNil (
    u_handle *_this);


#undef OS_API

#endif

