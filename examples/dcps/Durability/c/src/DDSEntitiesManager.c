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
 * LOGICAL_NAME:    DDSEntitiesManager.c
 * FUNCTION:        Implementation of functions calling DDS OpenSplice API code using basic error handling.
 * MODULE:          OpenSplice Durability example for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/
#include "DDSEntitiesManager.h"
#include "CheckStatus.h"
#ifdef _WIN32
#include "os_stdlib.h"
#endif
#include "DurabilityData.h"

// DDS entities and other variables
DDS_DomainId_t g_domainId = DDS_DOMAIN_ID_DEFAULT;
DDS_DomainParticipantFactory g_domainParticipantFactory = DDS_OBJECT_NIL;
DDS_DomainParticipant g_domainParticipant = DDS_OBJECT_NIL;

const char* g_partitionName = DDS_OBJECT_NIL;

DDS_InstanceHandle_t g_userHandle;

DDS_Publisher* g_Publisher = DDS_OBJECT_NIL;
DDS_DataWriter* g_DataWriter = DDS_OBJECT_NIL;

DDS_Subscriber* g_Subscriber = DDS_OBJECT_NIL;
DDS_DataReader* g_DataReader = DDS_OBJECT_NIL;

DDS_Topic g_Topic = DDS_OBJECT_NIL;

char* g_MsgTypeName = NULL;
DDS_TypeSupport g_MsgTypeSupport;

// Specific QoS settings
DDS_boolean g_autodispose_unregistered_instances = FALSE;
const char* g_durability_kind;

// Error handling
DDS_ReturnCode_t g_status;

void createParticipant(const char * partitionName)
{
   g_domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
   checkHandle(g_domainParticipantFactory, "DDS_DomainParticipantFactory_get_instance");

   g_domainParticipant = DDS_DomainParticipantFactory_create_participant(g_domainParticipantFactory, g_domainId, DDS_PARTICIPANT_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

   checkHandle(g_domainParticipant, "DDS_DomainParticipantFactory_create_participant");

   g_partitionName = partitionName;
}

void deleteParticipant()
{
   g_status = DDS_DomainParticipantFactory_delete_participant(g_domainParticipantFactory, g_domainParticipant);
   checkStatus(g_status, "DDS_DomainParticipantFactory_delete_participant");
}

void registerType(DDS_TypeSupport typeSupport)
{
   char* typeName = DurabilityData_MsgTypeSupport_get_type_name(typeSupport);

   g_status = DurabilityData_MsgTypeSupport_register_type(typeSupport,

   g_domainParticipant, typeName);
   checkStatus(g_status, "DurabilityData_MsgTypeSupport_register_type");

   DDS_free(typeName);
}

void createTopic(const char *topicName, const char *typeName)
{
   const char* messageFirstPart;
   char* message;
   size_t messageFirstPartLength, topicNameLength;
   DDS_TopicQos *topicQos = DDS_TopicQos__alloc();

   checkHandle(topicQos, "DDS_TopicQos__alloc");
   g_status = DDS_DomainParticipant_get_default_topic_qos(g_domainParticipant, topicQos);
   checkStatus(g_status, "DDS_DomainParticipant_get_default_topic_qos");
   topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

   if( strcmp(g_durability_kind, "transient") == 0 )
   {
       topicQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;
   }
   else if( strcmp(g_durability_kind, "persistent") == 0 )
   {
       topicQos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;
   }

   // Use the changed policy when defining the DurabilityData topic.
   g_Topic = DDS_DomainParticipant_create_topic(g_domainParticipant, topicName, typeName, topicQos, NULL, DDS_STATUS_MASK_NONE);

   //Format error message
   messageFirstPart = "DDS_DomainParticipant_create_topic ";
   messageFirstPartLength = strlen(messageFirstPart);
   topicNameLength = strlen(topicName);
   message = (char*) malloc(messageFirstPartLength + topicNameLength + 2);

   snprintf(message, messageFirstPartLength + topicNameLength + 1, "%s%s", messageFirstPart, topicName);
   checkHandle(g_Topic, message);

   DDS_free(topicQos);
   free(message);
}

void deleteTopic()
{
   g_status = DDS_DomainParticipant_delete_topic(g_domainParticipant, g_Topic);
   checkStatus(g_status, "DDS_DomainParticipant_delete_topic (nameServiceTopic)");
}

void createPublisher()
{
   DDS_PublisherQos* publisherQos = DDS_PublisherQos__alloc();
   checkHandle(publisherQos, "DDS_PublisherQos__alloc");
   g_status = DDS_DomainParticipant_get_default_publisher_qos(g_domainParticipant, publisherQos);
   checkStatus(g_status, "DDS_DomainParticipant_get_default_publisher_qos");
   publisherQos->partition.name._length = 1;
   publisherQos->partition.name._maximum = 1;
   publisherQos->partition.name._release = TRUE;
   publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
   checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
   publisherQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
   checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_dup");

   /* Create a Publisher for the application. */
   g_Publisher = DDS_DomainParticipant_create_publisher(g_domainParticipant, publisherQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(g_Publisher, "DDS_DomainParticipant_create_publisher");

   DDS_free(publisherQos);
}

void deletePublisher()
{
   g_status = DDS_DomainParticipant_delete_publisher(g_domainParticipant, g_Publisher);
   checkStatus(g_status, "DDS_DomainParticipant_delete_publisher");
}

void createWriter()
{
   DDS_DataWriterQos *dataWriterQos = DDS_DataWriterQos__alloc();
   DDS_TopicQos *topicQos = DDS_TopicQos__alloc();
   checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
   checkHandle(topicQos, "DDS_TopicQos__alloc");

   g_status = DDS_Publisher_get_default_datawriter_qos(g_Publisher, dataWriterQos);
   checkStatus(g_status, "DDS_Publisher_get_default_datawriter_qos");
   g_status = DDS_Topic_get_qos(g_Topic, topicQos);
   checkStatus(g_status, "DDS_Topic_get_qos");
   g_status = DDS_Publisher_copy_from_topic_qos(g_Publisher, dataWriterQos, topicQos);
   checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");

   // If autodispose_unregistered_instances is true, when the writer is stopped,
   // the instances of the topic will be suppressed from the persistence file;
   // If it is false the data will still be accessible after the Writer has stopped,
   // even if a Reader starts after the Writer's end and then reads the Topic's persistent samples.
   dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = g_autodispose_unregistered_instances;
   g_DataWriter = DDS_Publisher_create_datawriter(g_Publisher, g_Topic, dataWriterQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(g_DataWriter, "DDS_Publisher_create_datawriter (g_DataWriter)");

   DDS_free(dataWriterQos);
   DDS_free(topicQos);
}

void deleteDataWriter()
{
   g_status = DDS_Publisher_delete_datawriter(g_Publisher, g_DataWriter);
   checkStatus(g_status, "DDS_Publisher_delete_datawriter (g_DataWriter)");
}

void createSubscriber()
{
   // Adapt the default SubscriberQos to read from the Partition with the given name.
   DDS_SubscriberQos* subscriberQos = DDS_SubscriberQos__alloc();
   checkHandle(subscriberQos, "DDS_SubscriberQos__alloc");
   g_status = DDS_DomainParticipant_get_default_subscriber_qos(g_domainParticipant, subscriberQos);
   checkStatus(g_status, "DDS_DomainParticipant_get_default_subscriber_qos");
   subscriberQos->partition.name._length = 1;
   subscriberQos->partition.name._maximum = 1;
   subscriberQos->partition.name._release = TRUE;
   subscriberQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
   checkHandle(subscriberQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
   subscriberQos->partition.name._buffer[0] = DDS_string_dup(g_partitionName);
   checkHandle(subscriberQos->partition.name._buffer[0], "DDS_string_dup");

   // Create a Subscriber for the MsgBoard application.
   g_Subscriber = DDS_DomainParticipant_create_subscriber(g_domainParticipant, subscriberQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(g_Subscriber, "DDS_DomainParticipant_create_subscriber");

   DDS_free(subscriberQos);
}

void deleteSubscriber()
{
   g_status = DDS_DomainParticipant_delete_subscriber(g_domainParticipant, g_Subscriber);
   checkStatus(g_status, "DDS_DomainParticipant_delete_subscriber");
}

void createReader()
{
   DDS_DataReaderQos *dataReaderQos = DDS_DataReaderQos__alloc();
   DDS_TopicQos *topicQos = DDS_TopicQos__alloc();
   checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");
   checkHandle(topicQos, "DDS_TopicQos__alloc");

   g_status = DDS_Subscriber_get_default_datareader_qos(g_Subscriber, dataReaderQos);
   checkStatus(g_status, "DDS_Subscriber_get_default_datareader_qos");
   g_status = DDS_Topic_get_qos(g_Topic, topicQos);
   checkStatus(g_status, "DDS_Topic_get_qos");
   g_status = DDS_Subscriber_copy_from_topic_qos(g_Subscriber, dataReaderQos, topicQos);
   checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");

   g_DataReader = DDS_Subscriber_create_datareader(g_Subscriber, g_Topic, dataReaderQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(g_DataReader, "DDS_Subscriber_create_datareader");

   DDS_free(dataReaderQos);
   DDS_free(topicQos);
}

void deleteDataReader()
{
   g_status = DDS_Subscriber_delete_datareader(g_Subscriber, g_DataReader);
   checkStatus(g_status, "DDS_Subscriber_delete_datareader (g_DataReader)");
}

void waitForHistoricalData()
{
   // If the logic of your application requires it,
   // wait (block) until all historical data are received or
   // until the timeout has elapsed
   const DDS_Duration_t max_wait =
   { 1, 00000000 };
   g_status = DDS_DataReader_wait_for_historical_data(g_DataReader, &max_wait);
   checkStatus(g_status, "DDS_DataReader_wait_for_historical_data");
}
