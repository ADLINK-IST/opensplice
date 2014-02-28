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
#ifndef GAPI_TOPIC_H
#define GAPI_TOPIC_H

#include "gapi_common.h"

#include "u_user.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define _Topic(o) ((_Topic)(o))

#define U_TOPIC_GET(t) u_topic(U_ENTITY_GET(t))

#define gapi_topicClaim(h,r) \
        (_Topic(gapi_objectClaim(h,OBJECT_KIND_TOPIC,r)))

#define gapi_topicClaimNB(h,r) \
        (_Topic(gapi_objectClaimNB(h,OBJECT_KIND_TOPIC,r)))

#define _TopicFromHandle(h) \
        (_Topic(gapi_objectPeek(h,OBJECT_KIND_TOPIC)))

#define _TopicAlloc() \
        (_Topic(_ObjectAlloc(OBJECT_KIND_TOPIC, \
                              C_SIZEOF(_Topic), \
                              NULL)))

#define _TopicPrepareDelete(_this) \
        _TopicDescriptionPrepareDelete(_TopicDescription(_this))

#define _TopicMessageOffset(_this) \
        _TopicDescriptionMessageOffset(_TopicDescription(_this))

#define _TopicUserdataOffset(_this) \
        _TopicDescriptionUserdataOffset(_TopicDescription(_this))

#define _TopicAllocSize(_this) \
        _TopicDescriptionAllocSize(_TopicDescription(_this))

#define _TopicAllocBuffer(_this) \
        _TopicDescriptionAllocBuffer(_TopicDescription(_this))

#define _TopicGetName(_this) \
        _TopicDescriptionGetName(_TopicDescription(_this))

#define _TopicGetTypeName(_this) \
        _TopicDescriptionGetTypeName(_TopicDescription(_this))

_Topic
_TopicNew (
    const gapi_char *topic_name,
    const gapi_char *type_name,
    const _TypeSupport typesupport,
    const gapi_topicQos *qos,
    const struct gapi_topicListener *listener,
    const gapi_statusMask mask,
    const _DomainParticipant participant,
    const gapi_context *context);

_Topic
_TopicFromKernelTopic (
    u_topic uTopic,
    const gapi_char *topicName,
    const gapi_char *typeName,
    const _DomainParticipant participant,
    const gapi_context *context);

_Topic
_TopicFromUserTopic (
    u_topic uTopic,
    const _DomainParticipant participant,
    const gapi_context *context);

_Topic
_TopicFromTopic (
    _Topic _this,
    const _DomainParticipant participant,
    const gapi_context *context);

gapi_returnCode_t
_TopicFree (
    _Topic _this);

gapi_long
_TopicIncRef (
    _Topic _this);

gapi_long
_TopicDecRef (
    _Topic _this);

gapi_long
_TopicRefCount (
    _Topic _this);

OS_API u_topic
_TopicUtopic (
    _Topic _this);

gapi_topicQos *
_TopicGetQos (
    _Topic _this,
    gapi_topicQos *qos);

void
_TopicNotifyListener(
    _Topic _this,
    gapi_statusMask triggerMask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_TOPIC_H */
