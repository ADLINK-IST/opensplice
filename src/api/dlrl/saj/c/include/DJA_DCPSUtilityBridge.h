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
#ifndef DJA_DCPSUTILITY_BRIDGE_H
#define DJA_DCPSUTILITY_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

u_topic
DJA_DCPSUtilityBridge_us_createTopic(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    LOC_char* topicName,
    LOC_char* typeName,
    void** topicUserData,
    void** ls_topic,
    LOC_boolean isMainTopic);

u_reader
DJA_DCPSUtilityBridge_us_createDataReader(
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    void** ls_reader);

u_writer
DJA_DCPSUtilityBridge_us_createDataWriter(
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    void** ls_writer);

void
DJA_DCPSUtilityBridge_us_registerType(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_CacheAdmin* cache,
    LOC_char* topicName,
    LOC_char* typeName);

void
DJA_DCPSUtilityBridge_us_deleteDataReader(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    u_reader reader,
    DLRL_LS_object ls_reader);

void
DJA_DCPSUtilityBridge_us_deleteDataWriter(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    u_writer writer,
    DLRL_LS_object ls_writer);

void
DJA_DCPSUtilityBridge_us_deleteTopic(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    DK_TopicInfo* topicInfo);

void
DJA_DCPSUtilityBridge_us_releaseTopicUserData(
    void* userData,
    void* topicUserData);

void
DJA_DCPSUtilityBridge_us_enableEntity(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_entity);

#endif /* DJA_DCPSUTILITY_BRIDGE_H */
