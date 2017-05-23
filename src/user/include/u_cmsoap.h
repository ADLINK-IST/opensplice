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
#ifndef U_CMSOAP_H
#define U_CMSOAP_H

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

/**
 * \brief The <code>u_cmsoap</code> cast method.
 *
 * This method casts an object to a <code>u_cmsoap</code> object.
 * Since objects of the user layer are allocated on heap, no type checking
 * is performed.
 */
#define u_cmsoap(o) ((u_cmsoap)(o))

OS_API u_service
u_cmsoapNew (
    const os_char *uri,
    const u_domainId_t id,
    const os_int32 timeout, /* in seconds */
    const os_char *name,
    const u_participantQos qos,
    c_bool enable);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
