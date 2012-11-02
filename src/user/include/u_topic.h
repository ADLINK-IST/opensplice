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
#ifndef U_TOPIC_H
#define U_TOPIC_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_status.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_topic(o) \
        ((u_topic)u_entityCheckType(u_entity(o), U_TOPIC))

#define u__topicName(o) u_topic(o)->name

typedef c_bool (*u_topicAction)(u_topic topic, c_voidp arg);

OS_API u_topic
u_topicNew (
    u_participant p,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos);

OS_API u_result
u_topicFree (
    u_topic _this);

OS_API c_char *
u_topicName (
    u_topic _this);

OS_API c_char *
u_topicTypeName (
    u_topic _this);

OS_API u_result
u_topicGetInconsistentTopicStatus (
    u_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_topicGetAllDataDisposedStatus (
    u_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_topicDisposeAllData (u_topic _this);

OS_API u_participant
u_topicParticipant (
    u_topic _this);

OS_API c_bool
u_topicContentFilterValidate (
    u_topic _this,
    q_expr expr,
    c_value params[]);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
