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
#ifndef V_PROCESSINFO_H
#define V_PROCESSINFO_H

#include "c_misc.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#define v_processInfo(p) (C_CAST(p,v_processInfo))


/**
 * Generates a spurious wakeup on all threads maintained by processInfo that are
 * waiting on a shared condition variable (v_condWait(...)) for which the mutex
 * can be obtained.
 *
 * An external mechanism must be used to ensure the wakeup is delivered to all
 * waiting threads if that is needed. This routine uses a try-lock to prevent a
 * possible deadlock or delay on mutexes (needed for the broadcast) that can't
 * be obtained.
 *
 * Please note that also threads that aren't maintained by processInfo may receive
 * the spurious wakeup!
 */
OS_API
void
v_processInfoWakeThreads(
    _Inout_ v_processInfo _this);

OS_API
void
v_processInfoReportThreads(
    _Inout_ v_processInfo _this);

/**
 * Retrieves the threadInfo record for the thread with threadId tid.
 *
 * @param _this the processInfo maintaining the threadInfo records for the current
 *              process/domain combination.
 * @param tid the threadId of the thread to retrieve the threadInfo record for.
 * @return the record for the thread. The reference must NOT be freed.
 */
_Ret_notnull_
OS_API
v_threadInfo
v_processInfoGetThreadInfo (
    _Inout_ v_processInfo _this,
    _In_ c_ulonglong tid);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
