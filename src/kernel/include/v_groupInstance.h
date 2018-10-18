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

#ifndef V_GROUPINSTANCE_H
#define V_GROUPINSTANCE_H

#include "v_kernel.h"
#include "v_group.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API v_message
v_groupInstanceCreateMessage(
    v_groupInstance _this);

OS_API v_message
v_groupInstanceCreateTypedInvalidMessage(
    v_groupInstance _this,
    v_message untypedMsg);

OS_API v_registration
v_groupInstanceGetRegistration(
    v_groupInstance _this,
    v_gid gidTemplate,
    v_matchIdentityAction predicate);

OS_API void
v_groupInstanceUnregisterByTime (
    v_groupInstance _this,
    os_timeW time);

OS_API void
v_groupInstanceKeyToString(
    v_groupInstance _this,
    char *keystr,
    size_t keystr_size);

#undef OS_API

#endif
