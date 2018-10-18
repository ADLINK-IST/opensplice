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
#ifndef V_TOPICQOS_H
#define V_TOPICQOS_H

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
 * \brief The <code>v_topicQos</code> cast methods.
 *
 * This method casts an object to a <code>v_topicQos</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_topicQos</code> or
 * one of its subclasses.
 */
#define v_topicQos(o) (C_CAST(o,v_topicQos))

OS_API v_topicQos
v_topicQosNew (
    v_kernel kernel,
    v_topicQos template);

OS_API void
v_topicQosFree (
    v_topicQos q);

OS_API v_result
v_topicQosCheck(
    v_topicQos _this);

OS_API v_topicQos
v_topicQosFromTopicInfo(
    c_base kernel,
    const struct v_topicInfo *info);

OS_API v_result
v_topicQosFillTopicInfo (
    struct v_topicInfo *info,
    const struct v_topicQos_s *q);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
