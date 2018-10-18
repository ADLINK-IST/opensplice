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
#ifndef U_TOPIC_H
#define U_TOPIC_H

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

#define u_topic(o) \
        ((u_topic)u_objectCheckType(u_object(o), U_TOPIC))

OS_API u_topic
u_topicNew (
    const u_participant p,
    const os_char *name,
    const os_char *typeName,
    const os_char *keyList,
    u_topicQos qos);

OS_API u_result
u_topicGetQos (
    const u_topic _this,
    u_topicQos *qos);

OS_API u_topic
u_topicNewFromTopicInfo(
    u_participant p,
    const struct v_topicInfo *info,
    c_bool announce);

OS_API u_result
u_topicSetQos (
    const u_topic _this,
    const u_topicQos qos);

OS_API os_char *
u_topicName (
    const u_topic _this);

OS_API os_char *
u_topicTypeName (
    const u_topic _this);

OS_API os_uint32
u_topicTypeSize(
    const u_topic _this);

OS_API os_char *
u_topicKeyExpr(
    const u_topic _this);

OS_API os_char *
u_topicMetaDescriptor (
    const u_topic _this);

OS_API u_result
u_topicGetInconsistentTopicStatus (
    const u_topic _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_topicGetAllDataDisposedStatus (
    const u_topic _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_topicDisposeAllData (
    const u_topic _this);

OS_API u_bool
u_topicContentFilterValidate (
    const u_topic _this,
    const q_expr expr,
    const c_value params[],
    os_uint32 nrOfParams);

OS_API u_bool
u_topicContentFilterValidate2 (
    const u_topic _this,
    const q_expr expr,
    const c_value params[],
    os_uint32 nrOfParams);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
