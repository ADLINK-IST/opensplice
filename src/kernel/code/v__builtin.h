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

#define v_builtinTopicInfoData(_this,_msg) \
        ((struct v_topicInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_TOPICINFO_ID),_msg))

#define v_builtinParticipantCMInfoData(_this,_msg) \
        ((struct v_participantCMInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_CMPARTICIPANTINFO_ID),_msg))

#define v_builtinParticipantInfoData(_this,_msg) \
        ((struct v_participantInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_PARTICIPANTINFO_ID),_msg))

#define v_builtinPublicationInfoData(_this,_msg) \
        ((struct v_publicationInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_PUBLICATIONINFO_ID),_msg))

#define v_builtinSubscriptionInfoData(_this,_msg) \
        ((struct v_subscriptionInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_SUBSCRIPTIONINFO_ID),_msg))

#define v_builtinDeliveryInfoData(_this,_msg) \
        ((struct v_deliveryInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_DELIVERYINFO_ID),_msg))

#define v_builtinHeartbeatInfoData(_this,_msg) \
        ((struct v_heartbeatInfo *) \
         v_topicData(v_builtinTopicLookup(_this, V_HEARTBEATINFO_ID),_msg))

#define v_builtinControlAndMonitoringCommandData(_this,_msg) \
        ((v_controlAndMonitoringCommand *) \
         v_topicData(v_builtinTopicLookup(_this, V_C_AND_M_COMMAND_ID),_msg))

v_builtin
v_builtinNew(
    v_kernel kernel);

void
v_builtinWritersDisable(
    v_builtin _this);

v_message
v_builtinCreateParticipantInfo (
    v_builtin _this,
    v_participant p);

v_message
v_builtinCreateCMParticipantInfo (
    v_builtin _this,
    v_participant p);

v_message
v_builtinCreateTopicInfo (
    v_builtin _this,
    v_topic topic);

v_message
v_builtinCreatePublicationInfo (
    v_builtin _this,
    v_writer writer);

v_message
v_builtinCreateSubscriptionInfo (
    v_builtin _this,
    v_dataReader reader);

#if defined (__cplusplus)
}
#endif

#endif
