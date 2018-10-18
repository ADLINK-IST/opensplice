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
#ifndef V_PARTICIPANTQOS_H
#define V_PARTICIPANTQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_participantQos</code> cast method.
 *
 * This method casts an object to a <code>v_participantQos</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_participantQos</code> or
 * one of its subclasses.
 */
#define v_participantQos(o) (C_CAST(o,v_participantQos))

OS_API v_participantQos
v_participantQosNew(
    v_kernel kernel,
    v_participantQos template);
    
OS_API void
v_participantQosFree(
    v_participantQos q);

OS_API v_result
v_participantQosCheck(
    v_participantQos _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_PARTICIPANTQOS_H */
