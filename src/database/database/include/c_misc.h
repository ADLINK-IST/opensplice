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
#ifndef C_MISC_H
#define C_MISC_H

#include "c_typebase.h"
#include "c_metabase.h"

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

#ifdef _TYPECHECK_
#define C_CAST(o,t) ((t)c_checkType((c_object)(o),#t))
#else
#define C_CAST(o,t) ((t)(o))
#endif

#define C_TYPECHECK(o,t) (o == c_checkType(o,#t))

/* This function verifies the object type.
 * The type of the object must be the same as or be derived from the specified type.
 * If the type is correct the function will return the given object otherwise it will
 * return NULL, so it can simply be added into existing code.
 * If a type mismatch occures an error report is generated.
 * If the standard assert functionality is enabled a type mismatch will result in
 * an abort signal.
 */
OS_API c_object c_checkType     (c_object o, const c_char *typeName);
OS_API c_bool   c_instanceOf    (c_object o, const c_char *typeName);
OS_API void     c_copyIn        (c_type type, const void *data, c_voidp *dest);
OS_API void     c_copyOut       (c_type type, c_object object, c_voidp *data);
OS_API void     c_cloneIn       (c_type type, const void *data, c_voidp *dest);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
