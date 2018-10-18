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
#ifndef V_TOPIC_ADAPTER_H
#define V_TOPIC_ADAPTER_H

#include "v_kernel.h"
#include "v_entity.h"
#include "v_status.h"

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
 * \brief The <code>v_topicAdapter</code> cast methods.
 *
 * This method casts an object to a <code>v_topicAdapter</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_topicAdapter</code> or
 * one of its subclasses.
 */
#define v_topicAdapter(o) (C_CAST(o,v_topicAdapter))

#define v_topicAdapterTopic(_this)\
        (v_topic(v_topicAdapter(_this)->topic))

#define v_topicAdapterGetQos(_this)\
        v_topicGetQos(v_topicAdapter(_this)->topic)

#define v_topicAdapterSetQos(_this, qos)\
        v_topicSetQos(v_topicAdapter(_this)->topic, qos)

#define v_topicAdapterName(_this) \
        (v_entityName(v_topicAdapter(_this)))

#define v_topicAdapterMessageType(_this) \
        (v_topicAdapter(_this)->topic->messageType)

#define v_topicAdapterMessageKeyList(_this) \
        (v_topicAdapter(_this)->topic->messageKeyList)

#define v_topicAdapterDataType(_this) \
        (v_topicAdapter(_this)->topic->dataField->type)

#define v_topicAdapterQosRef(_this) \
        (v_topicAdapter(_this)->topic->qos)

#define v_topicAdapterKeyExpr(_this) \
        (v_topicAdapter(_this)->topic->keyExpr)

#define v_topicAdapterDataOffset(_this) \
        (v_topicAdapter(_this)->topic->dataField->offset)

#define v_topicAdapterData(_this,_msg) \
        ((c_voidp)C_DISPLACE(_msg,v_topicDataOffset(v_topicAdapter(_this))->topic))

#define v_topicAdapterAccessMode(_this)\
        (v_topicAdapter(_this)->topic->accessMode)

#define v_topicAdapterDisposeAllData(_this)\
        v_topicDisposeAllData(v_topicAdapter(_this)->topic)

#define v_topicAdapterMetaDescriptor(_this)\
        v_topicMetaDescriptor(v_topicAdapter(_this)->topic)

#define v_topicAdapterParticipant(_this)\
        v_participant(v_topic(_this)->owner)

OS_API v_topicAdapter
v_topicAdapterNew(
    v_participant p,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos);

OS_API v_topicAdapter
v_topicAdapterNewFromTopicInfo (
    v_participant p,
    const struct v_topicInfo *info,
    c_bool announce);

OS_API void
v_topicAdapterFree(
    v_topicAdapter adapter);

OS_API v_topicAdapter
v_topicAdapterWrap(
    v_participant p,
    v_topic topic);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
