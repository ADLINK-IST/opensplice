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
#ifndef V__THREADINFO_H
#define V__THREADINFO_H

#include "v_kernel.h"
#include "v_threadInfo.h"

#if defined (__cplusplus)
extern "C" {
#endif

_Check_return_
_Ret_notnull_
v_threadInfo
v_threadInfoNew(
    _In_ c_base base,
    _In_ c_ulong serial,
    _In_ c_ulonglong threadId);

void
v_threadInfoWake(
    _In_ v_threadInfo _this);

void
v_threadInfoReport(
    _In_ v_threadInfo _this);

void
v_threadInfoSetWaitInfo(
    _Inout_ v_threadInfo _this,
    _In_opt_ c_cond *cnd,
    _In_opt_ c_mutex *mtx);

c_mutex *
v_threadInfoGetAndClearWaitInfo(
    _Inout_ v_threadInfo _this);

void
v_threadInfoSetFlags(
    _Inout_opt_ v_threadInfo _this,
    _In_ c_ulong flags);

/**
 * Free's the threadInfo record.
 *
 * Should ONLY be invoked by the process that created the record. Other processes
 * should just invoke c_free.
 */
void
v_threadInfoFree(
    _Inout_opt_ _Post_invalid_ v_threadInfo _this);

#if defined (__cplusplus)
}
#endif

#endif
