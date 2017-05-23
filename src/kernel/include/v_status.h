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
#ifndef V_STATUS_H
#define V_STATUS_H

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
 * \brief The <code>v_....Status</code> cast methods.
 *
 * This method casts an object to a <code>v_....Status</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_....Status</code> or
 * one of its subclasses.
 */
#define v_status(s)            (C_CAST(s,v_status))
#define v_kernelStatus(s)      (C_CAST(s,v_kernelStatus))
#define v_partitionStatus(s)   (C_CAST(s,v_partitionStatus))
#define v_topicStatus(s)       (C_CAST(s,v_topicStatus))
#define v_writerStatus(s)      (C_CAST(s,v_writerStatus))
#define v_readerStatus(s)      (C_CAST(s,v_readerStatus))
#define v_subscriberStatus(s)  (C_CAST(s,v_subscriberStatus))

typedef v_result
(*v_statusAction) (
    c_voidp info, c_voidp arg);

OS_API v_status       v_statusNew       (v_entity e);
OS_API void           v_statusFree      (v_status s);

OS_API void           v_statusInit      (v_status s, const c_char *name);
OS_API void           v_statusDeinit    (v_status s);

OS_API void           v_statusReset     (v_status s, c_ulong mask);
OS_API c_ulong        v_statusGetMask   (v_status s);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
