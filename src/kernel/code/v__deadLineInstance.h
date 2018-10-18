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
#ifndef V__DEADLINEINSTANCE_H
#define V__DEADLINEINSTANCE_H

#include "v_leaseManager.h"
#include "v_status.h"
#include "v_public.h"
#include "v_instance.h"

#define v_deadLineInstance(o) (C_CAST(o,v_deadLineInstance))
#define v_deadLineInstance_prev(_this) (_this == NULL ? NULL : v_deadLineInstance(_this)->prev)
#define v_deadLineInstance_next(_this) (_this == NULL ? NULL : v_deadLineInstance(_this)->next)
#define v_deadLineInstance_alone(_this) (v_deadLineInstance(_this)->next == _this ? TRUE : FALSE)

void
v_deadLineInstanceInit(
    v_deadLineInstance _this,
    v_entity entity);

void
v_deadLineInstanceDeinit(
    v_deadLineInstance _this);

void
v_deadLineInstanceRemove(
    v_deadLineInstance _this);

#endif
