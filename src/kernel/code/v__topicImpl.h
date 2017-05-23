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

#ifndef V__TOPIC_IMPL_H
#define V__TOPIC_IMPL_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_event.h"
#include "v__topic.h"
#include "v_topicImpl.h"

void
v_topicImplDeinit(
    v_topicImpl topic);

v_topicImpl
v_topicImplNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos,
    c_bool announce);

v_topicImpl
v_topicImplNewFromTopicInfo (
    v_kernel kernel,
    const struct v_topicInfo *info,
    c_bool announce);

void
v_topicImplFree(
    v_topicImpl _this);

v_result
v_topicImplEnable(
    v_topicImpl topic);

v_topicQos
v_topicImplGetQos(
    v_topicImpl topic);

v_result
v_topicImplSetQos (
    v_topicImpl _this,
    v_topicQos qos);

void
v_topicImplAnnounce(
    v_topicImpl topic);

v_message
v_topicImplMessageNew(
    v_topicImpl topic);

v_message
v_topicImplMessageNew_s(
    v_topicImpl topic);

c_char *
v_topicImplMessageKeyExpr(
    v_topicImpl topic);

c_iter
v_topicImplLookupWriters(
    v_topicImpl topic);

c_iter
v_topicImplLookupReaders(
    v_topicImpl topic);

v_result
v_topicImplGetInconsistentTopicStatus(
    v_topicImpl _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

v_result
v_topicImplDisposeAllData(
    v_topicImpl topic);

v_result
v_topicImplGetAllDataDisposedStatus(
   v_topicImpl _this,
   c_bool reset,
   v_statusAction action,
   c_voidp arg);

os_char *
v_topicImplMetaDescriptor (
    v_topicImpl topic);

/**
 * for every notify method the observer lock must be locked!
 */
void
v_topicImplNotify (
    v_topicImpl _this,
    v_event event,
    c_voidp userData);

void
v_topicImplNotifyInconsistentTopic (
    v_topicImpl _this);

void
v_topicImplNotifyAllDataDisposed(
   v_topicImpl topic);

void
v_topicImplMessageCopyKeyValues (
    v_topicImpl _this,
    v_message dst,
    v_message src);

c_type
v_topicImplKeyTypeCreate (
    v_topicImpl _this,
    const c_char *keyExpr,
    c_array *keyListPtr);

v_result
v_topicImplFillTopicInfo (
    struct v_topicInfo *info,
    v_topicImpl topic);

#if defined (__cplusplus)
}
#endif

#endif
