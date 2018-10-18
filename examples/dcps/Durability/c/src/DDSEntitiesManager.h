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

/************************************************************************
 * LOGICAL_NAME:    DDSEntitiesManager.h
 * FUNCTION:        Declaration of functions calling DDS OpenSplice API code using basic error handling.
 * MODULE:          OpenSplice Durability example for the C programming language.
 * DATE             September 2010.
 ************************************************************************/

#ifndef _DDSENTITIESMANAGER_H_
#define _DDSENTITIESMANAGER_H_

#include "dds_dcps.h"

// DDS entities and other variables
extern DDS_DomainId_t g_domainId;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern DDS_DomainParticipant g_domainParticipant;

extern const char* g_partitionName;

extern DDS_InstanceHandle_t g_userHandle;

extern DDS_Publisher* g_Publisher;
extern DDS_DataWriter* g_DataWriter;

extern DDS_Subscriber* g_Subscriber;
extern DDS_DataReader* g_DataReader;

extern DDS_Topic g_Topic;

extern char* g_MsgTypeName;
extern DDS_TypeSupport g_MsgTypeSupport;

// QosPolicy holders
extern DDS_TopicQos* g_TopicQos;

extern DDS_DataWriterQos* g_DataWriterQos;
extern DDS_DataReaderQos* g_DataReaderQos;

// Specific QoS settings
extern DDS_boolean g_autodispose_unregistered_instances;
extern const char* g_durability_kind;

// Error handling
extern DDS_ReturnCode_t g_status;

// Declare handy helper functions
void createParticipant(const char * partitiontName);

void deleteParticipant();

void registerType(DDS_TypeSupport typeSupport);

void createTopic(const char *topicName, const char *typeName);

void deleteTopic();

void createPublisher();

void deletePublisher();

void createWriter();

void deleteDataWriter();

void createSubscriber();

void deleteSubscriber();

void createReader();

void deleteDataReader();

void waitForHistoricalData();

#endif
