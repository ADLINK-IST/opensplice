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
#include "u__types.h"
#include "u__domain.h"
#include "u__user.h"
#include "v_public.h"

const u_handle U_HANDLE_NIL;

static u_result u__handleResult (v_handleResult result)
{
    return ((result == V_HANDLE_OK) ? U_RESULT_OK :
            (result == V_HANDLE_EXPIRED) ? U_RESULT_ALREADY_DELETED :
            U_RESULT_ILL_PARAM);
}

u_handle
u_handleNew(
    const v_public object)
{
    v_handle handle;

    assert(object != NULL);

    handle = v_publicHandle(object);
    return handle;
}

u_result
u_handleClaim (
    const u_handle _this,
    const c_voidp  instance)
{
    u_result result;
    v_handleResult vresult;

    assert(instance);

    if (v_handleIsNil(_this)) {
        result = U_RESULT_ILL_PARAM;
    } else {
        vresult = v_handleClaim(_this,instance);
        result = u__handleResult(vresult);
    }

    return result;
}

u_result
u_handleRelease(
    const u_handle _this)
{
    u_result result;

    assert(!v_handleIsNil(_this));

    result = u__handleResult(v_handleRelease(_this));

    return result;
}

u_bool
u_handleIsEqual(
    const u_handle h1,
    const u_handle h2)
{
    return v_handleIsEqual(h1,h2);
}

u_bool
u_handleIsNil(
    const u_handle _this)
{
    return v_handleIsNil(_this);
}
