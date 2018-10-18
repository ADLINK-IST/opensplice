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
#ifndef V_PROJECTION_H
#define V_PROJECTION_H

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
 * \brief The <code>v_projection</code> cast method.
 *
 * This method casts an object to a <code>v_projection</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_projection</code> or
 * one of its subclasses.
 */
#define v_projection(_this) (C_CAST(_this,v_projection))

/**
 * \brief The <code>v_mapping</code> cast method.
 *
 * This method casts an object to a <code>v_mapping</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_mapping</code> or
 * one of its subclasses.
 */
#define v_mapping(_this) (C_CAST(_this,v_mapping))

OS_API v_projection
v_projectionNew (
    v_dataReader reader,
    q_expr projection);

OS_API c_type
v_projectionType (
    v_projection _this);

OS_API c_field
v_projectionSource (
    v_projection _this,
    const c_char *fieldName);

OS_API c_array
v_projectionRules (
    v_projection _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
