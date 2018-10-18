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
#ifndef V__PROCESSINFO_H
#define V__PROCESSINFO_H

#include "v_kernel.h"
#include "os_process.h"
#include "v_processInfo.h"
#include "os_abstract.h"

#if defined (__cplusplus)
extern "C" {
#endif

_Check_return_
_Ret_maybenull_
v_processInfo
v_processInfoNew(
    _In_ v_kernel kernel,
    _In_ os_procId processId);

/**
 * Frees the process info.
 *
 * This should ONLY be invoked by the owning process. If another process needs
 * to free the processInfo, c_free should be used instead.
 */
void
v_processInfoFree(
    _Inout_opt_ _Post_invalid_ v_processInfo _this);

#if defined (__cplusplus)
}
#endif

#endif
