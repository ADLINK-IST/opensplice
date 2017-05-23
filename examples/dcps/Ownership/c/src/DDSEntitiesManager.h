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
#include "OwnershipData.h"

// DDS entities and other variables
extern DDS_DomainId_t g_domainId;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern DDS_DomainParticipant g_domainParticipant;

extern const char* g_partitionName;

// Examples's Topics
extern char* g_StockTypeName;
extern DDS_TypeSupport g_StockTypeSupport;
extern DDS_Topic g_StockTopic;

// Error handling
extern DDS_ReturnCode_t g_status;

// Declare handy helper functions
void createParticipant(const char * partitiontName);

void deleteParticipant();

void registerStockType(DDS_TypeSupport typeSupport);

DDS_Topic createTopic(const char *topicName, const char *typeName);

void deleteTopic(DDS_Topic topic);

DDS_Publisher createPublisher();

void deletePublisher(DDS_Publisher publisher);

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic, DDS_boolean isAutodispose, DDS_long strength);

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter);

DDS_Subscriber createSubscriber();

void deleteSubscriber(DDS_Subscriber subscriber);

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic);

void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader);

void writeStockSample(DDS_DataWriter dataWriter, DDS_InstanceHandle_t userHandle, OwnershipData_Stock* OwnershipDataSample);

#endif
