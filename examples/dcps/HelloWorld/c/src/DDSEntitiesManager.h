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
