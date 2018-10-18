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
#ifndef V__BUILTIN_H
#define V__BUILTIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_builtin.h"
#include "v_topic.h"
#include "v_writer.h"

#define v_builtinWriterLookup(_this, _id) \
        (_this == NULL ? NULL : \
        v_writer(v_builtin(_this)->writers[_id]))

#define v_builtinTopicInfoData(_msg) ((struct v_topicInfo *) (((v_message)_msg)+1))
#define v_builtinTypeInfoData(_msg) ((struct v_typeInfo *) (((v_message)_msg)+1))
#define v_builtinParticipantCMInfoData(_msg) ((struct v_participantCMInfo *) (((v_message)_msg)+1))
#define v_builtinParticipantInfoData(_msg) ((struct v_participantInfo *) (((v_message)_msg)+1))
#define v_builtinPublicationInfoData(_msg) ((struct v_publicationInfo *) (((v_message)_msg)+1))
#define v_builtinDataWriterCMInfoData(_msg) ((struct v_dataWriterCMInfo *) (((v_message)_msg)+1))
#define v_builtinSubscriptionInfoData(_msg) ((struct v_subscriptionInfo *) (((v_message)_msg)+1))
#define v_builtinDataReaderCMInfoData(_msg) ((struct v_dataReaderCMInfo *) (((v_message)_msg)+1))
#define v_builtinPublisherCMInfoData(_msg) ((struct v_publisherCMInfo *) (((v_message)_msg)+1))
#define v_builtinSubscriberCMInfoData(_msg) ((struct v_subscriberCMInfo *) (((v_message)_msg)+1))
#define v_builtinDeliveryInfoData(_msg) ((struct v_deliveryInfo *) (((v_message)_msg)+1))
#define v_builtinHeartbeatInfoData(_msg) ((struct v_heartbeatInfo *) (((v_message)_msg)+1))
#define v_builtinControlAndMonitoringCommandData(_msg) ((v_controlAndMonitoringCommand *) (((v_message)_msg)+1))

v_builtin
v_builtinNew(
    v_kernel kernel);

void
v_builtinWritersDisable(
    v_builtin _this);

v_message
v_builtinCreateTopicInfo (
    v_builtin _this,
    v_topic topic);

v_message
v_builtinCreateTypeInfo (
    v_builtin _this,
    v_typeRepresentation tr);

v_message
v_builtinCreatePublicationInfo (
    v_builtin _this,
    v_writer writer);

v_message
v_builtinCreateCMDataWriterInfo (
    v_builtin _this,
    v_writer writer);

v_message
v_builtinCreateSubscriptionInfo (
    v_builtin _this,
    v_reader reader);

v_message
v_builtinCreateCMDataReaderInfo (
    v_builtin _this,
    v_reader reader);

v_message
v_builtinCreateCMPublisherInfo (
    v_builtin _this,
    v_publisher writer);

v_message
v_builtinCreateCMSubscriberInfo (
    v_builtin _this,
    v_subscriber reader);

/* Returns TRUE if there is a partition match between
 * the given publication and subscription info.
 */
os_boolean
v_builtinTestPartitionMatch(
    const struct v_publicationInfo *pubInfo,
    const struct v_subscriptionInfo *subInfo);

/* Returns TRUE if there is a Qos match between
 * the given publication and subscription info.
 */
os_boolean
v_builtinTestQosMatch(
    const struct v_publicationInfo *pubInfo,
    const struct v_subscriptionInfo *subInfo);

void
v_builtinLogParticipant(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogPublication(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogSubscription(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogTopic(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogCMParticipant(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogCMDataWriter(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogCMDataReader(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogCMPublisher(
    v_builtin _this,
    const v_dataReaderSample sample);

void
v_builtinLogCMSubscriber(
    v_builtin _this,
    const v_dataReaderSample sample);

#if defined (__cplusplus)
}
#endif

#endif
