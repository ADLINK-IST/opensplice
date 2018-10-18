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
#ifndef U_STATUSCONDITION_H
#define U_STATUSCONDITION_H

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

#define u_statusCondition(o) \
        ((u_statusCondition)u_objectCheckType(u_object(o), U_STATUSCONDITION))

OS_API u_statusCondition
u_statusConditionNew(
    const u_entity entity);

OS_API u_result
u_statusCondition_set_mask(
    const u_statusCondition _this,
    u_eventMask eventMask);

OS_API u_result
u_statusCondition_get_triggerValue (
    const u_statusCondition _this,
    u_eventMask *triggerValue);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
