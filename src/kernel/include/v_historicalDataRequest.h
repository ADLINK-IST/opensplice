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
#ifndef V_HISTORICALDATAREQUEST_H
#define V_HISTORICALDATAREQUEST_H

/** \file kernel/include/v_historicalDataRequest.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define v_historicalDataRequest(o) (C_CAST(o,v_historicalDataRequest));

#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API v_historicalDataRequest
v_historicalDataRequestNew(
    v_kernel kernel,
    const c_char* filter,
    const c_char* params[],
    c_ulong nofParams,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    v_resourcePolicyI *resourceLimits,
    os_duration timeout);

OS_API c_bool
v_historicalDataRequestEquals(
    v_historicalDataRequest req1,
    v_historicalDataRequest req2);

OS_API c_bool
v_historicalDataRequestIsValid(
    v_historicalDataRequest request,
    v_reader reader);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
