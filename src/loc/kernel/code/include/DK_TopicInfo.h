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
#ifndef DLRL_KERNEL_TOPIC_INFO_H
#define DLRL_KERNEL_TOPIC_INFO_H

/* kernel includes */
#include "v_entity.h"

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL MetaModel includes */
#include "DMM_DCPSTopic.h"

/* DLRL kernel includes */
#include "DK_Entity.h"
#include "DK_ObjectHomeAdmin.h"
#include "DLRL_Kernel.h"


#if defined (__cplusplus)
extern "C" {
#endif

struct DK_TopicInfo_s
{
    DK_Entity entity;
    LOC_boolean alive;
    DK_ObjectHomeAdmin* owner;
    DMM_DCPSTopic* metaTopic;
    u_topic topic;
    DLRL_LS_object ls_topic;/* not in design */
    void* topicUserData;
    c_object dataSample;
    c_long topicDataSampleOffset;
    v_message message;
};

/* topic user data may be null */
DK_TopicInfo*
DK_TopicInfo_new(
    DLRL_Exception* exception,
    u_topic topic,
    DLRL_LS_object ls_topic,
    DK_ObjectHomeAdmin* owner,
    DMM_DCPSTopic* metaTopic,
    void* topicUserData);

void
DK_TopicInfo_us_delete(
    DK_TopicInfo* _this,
    void* userData);

DMM_DCPSTopic*
DK_TopicInfo_us_getMetaTopic(
    DK_TopicInfo* _this);

LOC_string
DK_TopicInfo_us_getTopicType(
    DK_TopicInfo* _this);

u_topic
DK_TopicInfo_us_getTopic(
    DK_TopicInfo* _this);

void
DK_TopicInfo_us_setDataSample(
    DK_TopicInfo* _this,
    c_object dataSample);

void
DK_TopicInfo_us_setDataSampleOffset(
    DK_TopicInfo* _this,
    c_long dataSampleOffset);

void
DK_TopicInfo_us_setMessage(
    DK_TopicInfo* _this,
    v_message message);

/* NOT IN DESIGN - param added */
void
DK_TopicInfo_us_enable(
    DK_TopicInfo* _this,
    DLRL_Exception* exception,
    void* userData);

c_object
DK_TopicInfo_us_getDataSample(
    DK_TopicInfo* _this);

c_long
DK_TopicInfo_us_getDataSampleOffset(
    DK_TopicInfo* _this);

v_message
DK_TopicInfo_us_getMessage(
    DK_TopicInfo* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_TOPIC_INFO_H */
