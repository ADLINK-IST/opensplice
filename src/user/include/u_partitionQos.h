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
#ifndef U_PARTITIONQOS_H
#define U_PARTITIONQOS_H

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

/** \brief This operation creates a Qos object.
 *
 * The Qos policy values are copied from the given template, if no template
 * is provided (i.e. NULL is passed as agrument) a Qos object with the default
 * values is created.
 * The Qos object including any containing referenced policy objects are allocated
 * on heap, it is the respocibility of the user to free any containing referenced
 * policy objects when overriding them by assignment of new policy objects.
 * The whole Qos object is freed by the following Free operation.
 */
OS_API u_partitionQos u_partitionQosNew (const u_partitionQos _template);

/** \brief This operation frees the emory resources claimed by the Qos object.
 *
 * The Qos policy resources including all contained referenced policy objects
 * are freed. Be aware that for this reason all contained policy objects MUST
 * be allocated on heap!
 */
OS_API void u_partitionQosFree (const u_partitionQos _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
