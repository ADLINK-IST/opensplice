/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
  #include "CheckStatus.h"

// DDS entities and other variables
extern DDS_DomainId_t g_domainId;
extern DDS_DomainParticipantFactory g_domainParticipantFactory;
extern DDS_DomainParticipant g_domainParticipant;

extern const char* g_partitionName;

// Examples's Topics
extern char* g_MessageTypeName;
extern DDS_TypeSupport g_MessageTypeSupport;
extern DDS_Topic g_MessageTopic;

// Error handling
extern DDS_ReturnCode_t g_status;

// Declare handy helper functions
void createParticipant(const char * partitiontName);

void deleteContainedEntities();

void deleteParticipant();

DDS_Topic createTopic(const char *topicName, const char *typeName);

void deleteTopic(DDS_Topic topic);

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic);

void waitForHistoricalData(DDS_DataReader dataReader);

void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader);

#endif
