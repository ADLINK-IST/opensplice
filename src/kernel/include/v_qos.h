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
#ifndef V_QOS_H
#define V_QOS_H

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
 * \brief The <code>v_qos</code> cast method.
 *
 * This method casts an object to a <code>qos</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>qos</code> or
 * one of its subclasses.
 */
#define v_qos(o) (C_CAST(o,v_qos))

typedef c_ulong v_qosChangeMask; /* mask indicating, which policies have changed */

_Check_return_
_Pre_satisfies_(kind >= V_PARTITION_QOS && kind < V_COUNT_QOS)
OS_API v_qos
v_qosCreate(
    _In_ c_base base,
    _In_ v_qosKind kind);

OS_API void
v_qosFree (
    v_qos _this);

OS_API const c_char *
v_qosKindImage (
    v_qosKind kind);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_QOS_H */
