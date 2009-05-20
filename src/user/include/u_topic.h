/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

#define u_topic(o) ((u_topic)(o))

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
u_topicTypeName (
    u_topic _this);

OS_API u_result
u_topicGetInconsistentTopicStatus (
    u_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

#undef OS_API 

#if defined (__cplusplus)
}
#endif

#endif
