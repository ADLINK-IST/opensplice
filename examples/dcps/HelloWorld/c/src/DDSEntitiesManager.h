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

/************************************************************************
 * LOGICAL_NAME:    DDSEntitiesManager.h
 * FUNCTION:        Declaration of functions calling DDS OpenSplice API code using basic error handling.
 * MODULE:          OpenSplice Lifecycle example for the C programming language.
 * DATE             September 2010.
 ************************************************************************/

#ifndef _DDSENTITIESMANAGER_H_
#define _DDSENTITIESMANAGER_H_

#include "dds_dcps.h"
#include "HelloWorldData.h"

// DDS entities and other variables
extern DDS_DomainId_t g_domainId;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern const char* g_partitionName;

// Error handling
extern DDS_ReturnCode_t g_status;

// Declare handy helper functions
DDS_DomainParticipant createParticipant(const char * partitionName);

void deleteParticipant(DDS_DomainParticipant domainParticipant);

void registerMessageType(DDS_DomainParticipant domainParticipant, DDS_TypeSupport typeSupport);

DDS_Topic createTopic(DDS_DomainParticipant domainParticipant, const char *topicName, const char *typeName);

void deleteTopic(DDS_DomainParticipant domainParticipant, DDS_Topic topic);

DDS_Publisher createPublisher(DDS_DomainParticipant domainParticipant);

void deletePublisher(DDS_DomainParticipant domainParticipant, DDS_Publisher publisher);

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic);

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter);

DDS_Subscriber createSubscriber(DDS_DomainParticipant domainParticipant);

void deleteSubscriber(DDS_DomainParticipant domainParticipant, DDS_Subscriber subscriber);

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic);

void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader);

#endif
