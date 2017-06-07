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
#ifndef U_PARTITION_H
#define U_PARTITION_H

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

#define u_partition(o) \
        ((u_partition)u_objectCheckType(u_object(o), U_PARTITION))

/* A u_partition object is a user proxy to the kernel partition object.
 * The constructor will lookup or else create a kernel partition object and
 * create a u_partition object as user proxy.
 */
OS_API u_partition
u_partitionNew (
    u_participant p,
    const os_char *name,
    u_partitionQos qos);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
