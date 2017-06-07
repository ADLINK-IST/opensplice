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
#include "v__topic.h"
#include "v__topicQos.h"
#include "v_projection.h"
#include "v__kernel.h"
#include "v__entity.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_public.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v__builtin.h"
#include "v__status.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "ut_crc.h"
#include "v_event.h"
#include "v_policy.h"
#include "v__partition.h"
#include "v_configuration.h"
#include "v__policy.h"
#include "os_heap.h"
#include "v_topic.h"
#include "v__topicImpl.h"
#include "v__topicAdapter.h"
#include "v__participant.h"
#include "v_policy.h"

#include "sd_serializer.h"
#include "sd_serializerXMLTypeinfo.h"

#include "c_stringSupport.h"
#include "vortex_os.h"
#include "os_report.h"


void
v_topicFree(
    v_topic _this)
{
    if (v_objectKind(_this) == K_TOPIC) {
        v_topicImplFree(v_topicImpl(_this));
    } else {
        v_topicAdapterFree(v_topicAdapter(_this));
    }
}

v_result
v_topicEnable(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplEnable(ti);
}

v_topicQos
v_topicGetQos(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplGetQos(ti);
}

v_result
v_topicSetQos (
    v_topic _this,
    v_topicQos qos)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplSetQos(ti, qos);
}

void
v_topicAnnounce(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    v_topicImplAnnounce(ti);
}

v_message
v_topicMessageNew(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplMessageNew(ti);
}

v_message
v_topicMessageNew_s(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplMessageNew_s(ti);
}

c_char *
v_topicMessageKeyExpr(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplMessageKeyExpr(ti);
}

c_type
v_topicKeyType(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return (c_type(c_keep(ti->keyType)));
}

c_iter
v_topicLookupWriters(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplLookupWriters(ti);
}

c_iter
v_topicLookupReaders(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplLookupReaders(ti);
}

v_result
v_topicGetInconsistentTopicStatus(
    v_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;

    if (v_objectKind(_this) == K_TOPIC_ADAPTER) {
        result = v_topicAdapterGetInconsistentTopicStatus(v_topicAdapter(_this), reset, action, arg);
    } else {
        result = v_topicImplGetInconsistentTopicStatus(v_topicImpl(_this), reset, action, arg);
    }
    return result;
}

v_result
v_topicDisposeAllData(
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplDisposeAllData(ti);
}

v_result
v_topicGetAllDataDisposedStatus(
   v_topic _this,
   c_bool reset,
   v_statusAction action,
   c_voidp arg)
{
    v_result result;

    if (v_objectKind(_this) == K_TOPIC_ADAPTER) {
        result = v_topicAdapterGetAllDataDisposedStatus(v_topicAdapter(_this), reset, action, arg);
    } else {
        result = v_topicImplGetAllDataDisposedStatus(v_topicImpl(_this), reset, action, arg);
    }
    return result;
}

os_char *
v_topicMetaDescriptor (
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplMetaDescriptor(ti);
}


void
v_topicNotify (
    v_topic _this,
    v_event event,
    c_voidp userData)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    v_topicImplNotify(ti, event, userData);
}

void
v_topicNotifyInconsistentTopic (
    v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    v_topicImplNotifyInconsistentTopic(ti);
}

void
v_topicNotifyAllDataDisposed(
   v_topic _this)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    v_topicImplNotifyAllDataDisposed(ti);
}

void
v_topicMessageCopyKeyValues (
    v_topic _this,
    v_message dst,
    v_message src)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    v_topicImplMessageCopyKeyValues(ti, dst, src);
}

c_type
v_topicKeyTypeCreate (
    v_topic _this,
    const c_char *keyExpr,
    c_array *keyListPtr)
{
    v_topicImpl ti;

    if (v_objectKind(_this) == K_TOPIC) {
        ti = v_topicImpl(_this);
    } else {
        ti = v_topicAdapter(_this)->topic;
    }
    return v_topicImplKeyTypeCreate(ti, keyExpr, keyListPtr);
}

v_result
v_topicFillTopicInfo (
    struct v_topicInfo *info,
    v_topic topic)
{
    v_topicImpl ti;

    if (v_objectKind(topic) == K_TOPIC) {
        ti = v_topicImpl(topic);
    } else {
        ti = v_topicAdapter(topic)->topic;
    }
    return v_topicImplFillTopicInfo(info, ti);
}
