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
#ifndef V_TOPIC_H
#define V_TOPIC_H

#include "v_kernel.h"
#include "v_entity.h"
#include "v_status.h"
#include "v_topicAdapter.h"
#include "v_topicImpl.h"

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
 * \brief The <code>v_topic</code> cast methods.
 *
 * This method casts an object to a <code>v_topic</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_topic</code> or
 * one of its subclasses.
 */
#define v_topic(o) (C_CAST(o,v_topic))

#define v_topicName(_this) \
        (v_entityName(v_topic(_this)))

#define v_topicIsAdapter(_this)\
        (v_objectKind(_this) == K_TOPIC_ADAPTER)

#define v_topicMessageExtType(o) \
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->messageExtType : v_topicImpl(o)->messageExtType)

#define v_topicMessageType(o) \
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->messageType : v_topicImpl(o)->messageType)

#define v_topicMessageKeyList(o) \
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->messageKeyList : v_topicImpl(o)->messageKeyList)

#define v_topicDataType(o) \
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->dataType : v_topicImpl(o)->dataType)

#define v_topicQosRef(o) \
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->qos : v_topicImpl(o)->qos)

#define v_topicKeyExpr(o) \
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->keyExpr : v_topicImpl(o)->keyExpr)

#define v_topicAccessMode(o)\
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->accessMode : v_topicImpl(o)->accessMode)

#define v_topicCrcOfName(o)\
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->crcOfName : v_topicImpl(o)->crcOfName)

#define v_topicCrcOfTypeName(o)\
        (v_topicIsAdapter(o) ? v_topicAdapter(o)->topic->crcOfTypeName : v_topicImpl(o)->crcOfTypeName)


OS_API void
v_topicFree(
    v_topic _this);

OS_API v_topicQos
v_topicGetQos(
    v_topic _this);

OS_API v_result
v_topicSetQos (
    v_topic _this,
    v_topicQos qos);

OS_API void
v_topicAnnounce(
    v_topic _this);

OS_API v_message
v_topicMessageNew(
    v_topic _this);

OS_API v_message
v_topicMessageNew_s(
    v_topic _this);

OS_API c_char *
v_topicMessageKeyExpr(
    v_topic _this);

OS_API c_iter
v_topicLookupWriters(
    v_topic _this);

OS_API c_iter
v_topicLookupReaders(
    v_topic topic);

OS_API v_result
v_topicGetInconsistentTopicStatus(
    v_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_topicDisposeAllData(v_topic topic);

OS_API v_result
v_topicGetAllDataDisposedStatus(
   v_topic _this,
   c_bool reset,
   v_statusAction action,
   c_voidp arg);

OS_API os_char *
v_topicMetaDescriptor (
    v_topic _this);

OS_API v_result
v_topicFillTopicInfo (
    struct v_topicInfo *info,
    v_topic topic);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
