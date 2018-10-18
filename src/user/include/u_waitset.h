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
#ifndef U_WAITSET_H
#define U_WAITSET_H

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_waitset(o) \
        ((u_waitset)u_objectCheckType(u_object(o), U_WAITSET))

typedef void (*u_waitsetAction)(u_waitsetEvent e, void *arg);
typedef os_boolean (*u_waitsetAction2)(void *context, void *arg);

OS_API u_waitset
u_waitsetNew(void);

OS_API u_waitset
u_waitsetNew2(void);

OS_API void
u_waitsetAnnounceDestruction(
    const u_waitset _this);

OS_API u_result
u_waitsetWaitAction(
    const u_waitset _this,
    u_waitsetAction action,
    void *arg,
    const os_duration timeout);

OS_API u_result
u_waitsetWaitAction2(
    const u_waitset _this,
    u_waitsetAction2 action,
    void *arg,
    const os_duration timeout);

OS_API u_result
u_waitsetNotify(
    const u_waitset _this,
    void *eventArg);

OS_API u_result
u_waitsetTrigger(
    const u_waitset _this);

OS_API u_result
u_waitsetAttach(
    const u_waitset _this,
    const u_observable observable,
    void *context);

OS_API void
u_waitsetDetach(
    const u_waitset _this,
    const u_observable observable);

OS_API u_result
u_waitsetDetach_s(
    const u_waitset _this,
    const u_observable observable);

OS_API u_result
u_waitsetGetEventMask(
    const u_waitset _this,
    u_eventMask *eventMask);

OS_API u_result
u_waitsetSetEventMask(
    const u_waitset _this,
    u_eventMask eventMask);

OS_API os_int32
u_waitsetGetDomainId(
    u_waitset _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
