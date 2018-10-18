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
#ifndef V__DEADLINEINSTANCELIST_H
#define V__DEADLINEINSTANCELIST_H

#include "v_leaseManager.h"
#include "v_status.h"
#include "v_public.h"
#include "v_instance.h"

#define v_deadLineInstanceList(o) \
        (C_CAST(o,v_deadLineInstanceList))

#define v_deadLineInstanceListHead(_this) \
        (v_deadLineInstanceList(_this)->head)

v_deadLineInstanceList
v_deadLineInstanceListNew(
    c_base base,
    v_leaseManager leaseManager,
    os_duration leaseDuration,
    v_leaseActionId actionId,
    v_public o);

void
v_deadLineInstanceListFree(
    v_deadLineInstanceList _this);

void
v_deadLineInstanceListSetDuration(
    v_deadLineInstanceList _this,
    os_duration duration);

void
v_deadLineInstanceListInsertInstance(
    v_deadLineInstanceList _this,
    v_deadLineInstance instance);

void
v_deadLineInstanceListRemoveInstance(
    v_deadLineInstanceList _this,
    v_deadLineInstance instance) __nonnull_all__;

void
v_deadLineInstanceListUpdate(
    v_deadLineInstanceList _this,
    v_deadLineInstance instance,
    os_timeE timestamp) __nonnull_all__;

c_iter
v_deadLineInstanceListCheckDeadlineMissed(
    v_deadLineInstanceList _this,
    os_duration deadlineTime,
    os_timeE now);

c_bool
v_deadLineInstanceListEmpty(
    v_deadLineInstanceList _this);

#endif
