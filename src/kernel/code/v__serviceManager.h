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
#ifndef V__SERVICEMANAGER_H
#define V__SERVICEMANAGER_H

#include "v_kernel.h"
#include "v_serviceManager.h"
#include "v_event.h"
/* This is only needed since cmxml component uses this protected header file! */

#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

v_serviceState
v_serviceManagerRegister(
    v_serviceManager _this,
    v_service service,
    const c_char *extStateName);

OS_API v_serviceState
v_serviceManagerGetServiceState(
    v_serviceManager _this,
    const c_char *serviceName);

#undef OS_API
 
#endif
