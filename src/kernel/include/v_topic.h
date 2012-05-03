/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V_TOPIC_H
#define V_TOPIC_H

#include "v_kernel.h"
#include "v_entity.h"
#include "v_status.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
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

#define v_topicMessageType(_this) \
        (v_topic(_this)->messageType)

#define v_topicMessageKeyList(_this) \
        (v_topic(_this)->messageKeyList)

#define v_topicDataType(_this) \
        (v_topic(_this)->dataField->type)

#define v_topicQosRef(_this) \
        (v_topic(_this)->qos)

#define v_topicKeyExpr(_this) \
        (v_topic(_this)->keyExpr)

#define v_topicDataOffset(_this) \
        (v_topic(_this)->dataField->offset)

#define v_topicData(_this,_msg) \
        ((c_voidp)C_DISPLACE(_msg,v_topicDataOffset(_this)))

#define v_topicAccessMode(_this)\
        (v_topic(_this)->accessMode)

OS_API v_topic
v_topicNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos);

OS_API void
v_topicFree(
    v_topic topic);

OS_API v_result
v_topicEnable(
    v_topic topic,
    v_topic* found_topic);

OS_API void
v_topicAnnounce(
    v_topic topic);

OS_API v_message
v_topicMessageNew(
    v_topic topic);

OS_API c_char *
v_topicMessageKeyExpr(
    v_topic topic);

OS_API c_iter
v_topicLookupWriters(
    v_topic topic);

OS_API c_iter
v_topicLookupReaders(
    v_topic topic);

OS_API v_topicQos
v_topicGetQos(
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

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
