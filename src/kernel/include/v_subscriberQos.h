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
#ifndef V_SUBSCRIBERQOS_H
#define V_SUBSCRIBERQOS_H

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_subscriberQos</code> cast methods.
 *
 * This method casts an object to a <code>v_subscriberQos</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_subscriberQos</code> or
 * one of its subclasses.
 */
#define v_subscriberQos(_this) (C_CAST(_this,v_subscriberQos))

OS_API v_subscriberQos
v_subscriberQosNew(
    v_kernel kernel,
    v_subscriberQos template);
    
OS_API void
v_subscriberQosFree(
    v_subscriberQos _this);

OS_API v_result
v_subscriberQosCheck(
    v_subscriberQos _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
