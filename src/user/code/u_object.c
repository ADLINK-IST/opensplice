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
#include "u__user.h"
#include "u__types.h"
#include "u__object.h"
#include "os_report.h"
#include "os_atomics.h"

void *
u_objectAlloc(
    size_t allocSize,
    u_kind kind,
    u_deinitFunction_t deinit,
    u_freeFunction_t free)
{
    u_object _this;
    assert(kind != U_UNDEFINED && kind != U_COUNT);
    assert(deinit != 0);
    assert(free != 0);
    _this = os_malloc(allocSize);
    memset(_this, 0, allocSize);
    _this->kind = kind;
    pa_stvoidp (&_this->deinit, (void *) deinit);
    _this->free = free;
    return _this;
}

void
u__objectFreeW(
    void *_vthis)
{
    u_object _this = _vthis;
    _this->kind = U_UNDEFINED;
    _this->free = 0;
    os_free(_this);
}

u_result
u__objectDeinitW(
    void *_vthis)
{
    u_object _this = _vthis;
    pa_stvoidp (&_this->deinit, 0);
    return U_RESULT_OK;
}

#ifndef NDEBUG
u_object
u__objectCheckType(
    const void *_vthis,
    const u_kind kind)
{
    const struct u_object_s *_this = _vthis;
    const struct u_object_s *result = NULL;

    if ((_this) && ((((os_address)_this) % sizeof(os_address)) == 0)) {
        if (_this->kind == kind) {
            result = _this;
        } else if ((_this->kind == U_SERVICE) && (kind == U_PARTICIPANT)) {
            result = _this;
        } else if ((_this->kind == U_SPLICED) && (kind == U_PARTICIPANT)) {
            result = _this;
        } else {
            OS_REPORT(OS_ERROR, "u_objectCheckType", U_RESULT_INTERNAL_ERROR,
                      "User layer Entity type check failed, type = %s but expected %s",
                      u_kindImage(_this->kind), u_kindImage(kind));
        }
    }
    return (u_object) result;
}
#else
u_object
u__objectCheckType(
    const void *_vthis,
    const u_kind kind)
{
    OS_UNUSED_ARG(kind);
    return (u_object)_vthis;
}
#endif

u_kind
u_objectKind(
    const void *_vthis)
{
    const struct u_object_s *_this = _vthis;
    return _this->kind;
}

/* Generic deinit, free functions: demultiplexes based on kind. */
u_result
u_objectClose(
    void *_vthis)
{
    struct u_object_s *_this = _vthis;
    u_result r;
    void *deinit;

    do {
        deinit = pa_ldvoidp (&_this->deinit);
    } while (!pa_casvoidp (&_this->deinit, deinit, 0));

    if (deinit) {
        u_deinitFunction_t deinitFun = (u_deinitFunction_t) deinit;
        r = deinitFun(_this);
    } else {
        r = U_RESULT_ALREADY_DELETED;
    }
    return r;
}

u_result
u_objectFree_s(
    void *_vthis)
{
    struct u_object_s *_this = _vthis;
    u_result r;
    if ((r = u_objectClose (_this)) == U_RESULT_ALREADY_DELETED) {
        r = U_RESULT_OK;
    }
    _this->free(_this);
    return r;
}

void
u_objectFree (
    void *_this)
{
  (void) u_objectFree_s (_this);
}
