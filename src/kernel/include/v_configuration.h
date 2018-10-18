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
#ifndef V_CONFIGURATION_H
#define V_CONFIGURATION_H

/** \file kernel/include/v_configuration.h
 *  \brief This file defines the interface
 *
 */

#include "kernelModuleI.h"
#include "v_kernel.h"
#include "v_cfElement.h"
#include "v_cfNode.h"
#include "v_cfData.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_configuration</code> cast method.
 *
 * This method casts an object to a <code>v_configuration</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_configuration</code> or
 * one of its subclasses.
 */
#define v_configuration(o) (C_CAST(o,v_configuration))

OS_API v_configuration
v_configurationNew (
    v_kernel kernel);

OS_API void
v_configurationFree (
    v_configuration config);

OS_API void
v_configurationSetRoot (
    v_configuration config,
    v_cfElement root);

OS_API v_cfElement
v_configurationGetRoot (
    v_configuration config);

OS_API void
v_configurationSetUri (
    v_configuration config,
    const c_char *uri);

OS_API const c_char *
v_configurationGetUri (
    v_configuration config);

OS_API c_ulong
v_configurationIdNew (
    v_configuration config);


OS_API v_cfNode
v_configurationGetNode (
    v_configuration config,
    c_ulong id);

OS_API void
v_configurationGetSchedulingPolicy (
    v_configuration config,
    const c_char* element,
    const c_char* name,
    v_schedulePolicyI *policy);

OS_API c_bool
v_configurationContainsService(
    v_configuration config,
    const char *serviceName);

#undef OS_API

#endif /* V_CONFIGURATION_H */
