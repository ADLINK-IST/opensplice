/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef V__TOPIC_H
#define V__TOPIC_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_event.h"
#include "v_topic.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_topicKeyType(o) (c_type(c_keep(v_topic(o)->keyType)))

OS_API void
v_topicDeinit(
    v_topic topic);

OS_API v_topic
v__topicNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos,
    c_bool announce);

/**
 * for every notify method the observer lock must be locked!
 */
OS_API void
v_topicNotify (
    v_topic _this,
    v_event event,
    c_voidp userData);

OS_API void
v_topicNotifyInconsistentTopic (
    v_topic _this);

void
v_topicNotifyAllDataDisposed(
   v_topic topic);OS_API v_result
v_topicSetQos (
    v_topic _this,
    v_topicQos qos);

OS_API void
v_topicMessageCopyKeyValues (
    v_topic _this,
    v_message dst,
    v_message src);

OS_API c_type
v_topicKeyTypeCreate (
    v_topic _this,
    const c_char *keyExpr,
    c_array *keyListPtr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
