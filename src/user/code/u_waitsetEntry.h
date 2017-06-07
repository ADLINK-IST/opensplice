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

#ifndef U__WAISETENTRY_H
#define U__WAISETENTRY_H

#include "u_user.h"

#define u_waitsetEntry(o) ((u_waitsetEntry)(o))

C_CLASS(u_waitsetEntry);

u_waitsetEntry
u_waitsetEntryNew(
    const u_waitset waitset,
    const u_domain domain,
    const c_ulong eventMask);

u_result
u_waitsetEntrySetMode(
    u_waitsetEntry _this,
    u_bool multimode);

u_result
u_waitsetEntryAttach(
    const u_waitsetEntry _this,
    const u_observable observable,
    c_voidp context);

u_result
u_waitsetEntryDetach (
    const u_waitsetEntry _this,
    const u_observable observable);

u_result
u_waitsetEntryWait(
    const u_waitsetEntry _this,
    u_waitsetAction action,
    void *arg,
    const os_duration timeout);

u_result
u_waitsetEntryWait2(
    const u_waitsetEntry _this,
    u_waitsetAction2 action,
    void *arg,
    const os_duration timeout);

u_result
u_waitsetEntryTrigger(
    const u_waitsetEntry _this,
    c_voidp eventArg);

u_result
u_waitsetEntryGetEventMask(
    const u_waitsetEntry _this,
    c_ulong *eventMask);

u_result
u_waitsetEntrySetEventMask(
    const u_waitsetEntry _this,
    c_ulong eventMask);

#endif
