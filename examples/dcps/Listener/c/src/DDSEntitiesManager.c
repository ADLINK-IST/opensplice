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
 * LOGICAL_NAME:    DDSEntitiesManager.c
 * FUNCTION:        Implementation of functions calling DDS OpenSplice API code using basic error handling.
 * MODULE:          OpenSplice ContentFilteredTopic example for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/
#include "DDSEntitiesManager.h"
#include "ListenerData.h"
#include "CheckStatus.h"
#ifdef _WIN32
#include "os_stdlib.h"
#endif

// DDS entities and other variables
//
DDS_DomainId_t g_domainId = DDS_DOMAIN_ID_DEFAULT;
DDS_DomainParticipantFactory g_domainParticipantFactory = DDS_OBJECT_NIL;
DDS_DomainParticipant g_domainParticipant = DDS_OBJECT_NIL;

const char* g_partitionName = DDS_OBJECT_NIL;

// Examples's Topics
char* g_MessageTypeName;
DDS_TypeSupport g_MessageTypeSupport;
DDS_Topic g_MessageTopic;

// Error handling
DDS_ReturnCode_t g_status;

void createParticipant(const char * partitiontName)
{
   g_domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
   checkHandle(g_domainParticipantFactory, "DDS_DomainParticipantFactory_get_instance");

   g_domainParticipant = DDS_DomainParticipantFactory_create_participant(g_domainParticipantFactory, g_domainId, DDS_PARTICIPANT_QOS_DEFAULT, NULL, DDS_STATUS_MASK_NONE);

   checkHandle(g_domainParticipant, "DDS_DomainParticipantFactory_create_participant");

   g_partitionName = partitiontName;
}

void deleteParticipant()
{
   g_status = DDS_DomainParticipantFactory_delete_participant(g_domainParticipantFactory, g_domainParticipant);
   checkStatus(g_status, "DDS_DomainParticipantFactory_delete_participant");
}

void registerMessageType(DDS_TypeSupport typeSupport)
{
   char* typeName = ListenerData_MsgTypeSupport_get_type_name(typeSupport);

   g_status = ListenerData_MsgTypeSupport_register_type(typeSupport, g_domainParticipant, typeName);
   checkStatus(g_status, "ListenerData_MsgTypeSupport_register_type");

   DDS_free(typeName);
}

DDS_Topic createTopic(const char *topicName, const char *typeName)
{
   DDS_Topic topic;
   const char* messageFirstPart;
   char* message;
   size_t messageFirstPartLength, topicNameLength;
   DDS_TopicQos* topicQos = DDS_TopicQos__alloc();
   checkHandle(topicQos, "DDS_TopicQos__alloc");
   g_status = DDS_DomainParticipant_get_default_topic_qos(g_domainParticipant, topicQos);
   checkStatus(g_status, "DDS_DomainParticipant_get_default_topic_qos");
   topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
   topicQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

   // DeadlineQoSPolicy : period used to trigger the listener
   // (on_requested_deadline_missed)
   topicQos->deadline.period.nanosec = 0;
   topicQos->deadline.period.sec = 1;

   /* Use the changed policy when defining the Ownership topic */
   topic = DDS_DomainParticipant_create_topic(g_domainParticipant, topicName, typeName, topicQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(topic, "DDS::DomainParticipant::create_topic ()");

   //Format error message
   messageFirstPart = "DDS_DomainParticipant_create_topic";
   messageFirstPartLength = strlen(messageFirstPart);
   topicNameLength = strlen(topicName);
   message = (char*) malloc(messageFirstPartLength + topicNameLength + 2);

   snprintf(message, messageFirstPartLength + topicNameLength + 1, "%s %s", messageFirstPart, topicName);
   checkHandle(topic, message);

   free(message);
   DDS_free(topicQos);

   return topic;
}

void deleteTopic(DDS_Topic topic)
{
   g_status = DDS_DomainParticipant_delete_topic(g_domainParticipant, topic);
   checkStatus(g_status, "DDS_DomainParticipant_delete_topic");
}

DDS_Publisher createPublisher(char* publisherName)
{
   DDS_Publisher publisher;
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
   publisher = DDS_DomainParticipant_create_publisher(g_domainParticipant, publisherQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(publisher, "DDS_DomainParticipant_create_publisher");

   DDS_free(publisherQos);
   return publisher;
}

void deletePublisher(DDS_Publisher publisher)
{
   g_status = DDS_DomainParticipant_delete_publisher(g_domainParticipant, publisher);
   checkStatus(g_status, "DDS_DomainParticipant_delete_publisher");
}

DDS_DataWriter createDataWriter(DDS_Publisher publisher, DDS_Topic topic)
{
   DDS_DataWriter dataWriter;
   DDS_TopicQos *topicQos = DDS_TopicQos__alloc();
   DDS_DataWriterQos *dataWriterQos = DDS_DataWriterQos__alloc();
   checkHandle(dataWriterQos, "DDS_DataWriterQos__alloc");
   g_status = DDS_Publisher_get_default_datawriter_qos(publisher, dataWriterQos);
   checkStatus(g_status, "DDS_Publisher_get_default_datawriter_qos");
   g_status = DDS_Topic_get_qos(topic, topicQos);
   checkStatus(g_status, "DDS_Topic_get_qos");
   g_status = DDS_Publisher_copy_from_topic_qos(publisher, dataWriterQos, topicQos);
   checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");
   dataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;
   dataWriter = DDS_Publisher_create_datawriter(publisher, topic, dataWriterQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(dataWriter, "DDS_Publisher_create_datawriter");

   DDS_free(dataWriterQos);
   DDS_free(topicQos);

   return dataWriter;
}

void deleteDataWriter(DDS_Publisher publisher, DDS_DataWriter dataWriter)
{
   g_status = DDS_Publisher_delete_datawriter(publisher, dataWriter);
   checkStatus(g_status, "DDS_Publisher_delete_datawriter");
}

DDS_Subscriber createSubscriber()
{
   DDS_Subscriber subscriber;
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

   // Create a Subscriber for the MessageBoard application.
   subscriber = DDS_DomainParticipant_create_subscriber(g_domainParticipant, subscriberQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(subscriber, "DDS_DomainParticipant_create_subscriber");

   DDS_free(subscriberQos);

   return subscriber;
}

void deleteSubscriber(DDS_Subscriber subscriber)
{
   g_status = DDS_DomainParticipant_delete_subscriber(g_domainParticipant, subscriber);
   checkStatus(g_status, "DDS_DomainParticipant_delete_subscriber");
}

DDS_DataReader createDataReader(DDS_Subscriber subscriber, DDS_Topic topic)
{
   DDS_DataReader dataReader;
   DDS_TopicQos* topicQos;
   DDS_DataReaderQos* dataReaderQos;

   // Create a DataWriter for this Topic (using the appropriate QoS).
   dataReaderQos = DDS_DataReaderQos__alloc();
   checkHandle(dataReaderQos, "DDS_DataReaderQos__alloc");

   topicQos = DDS_TopicQos__alloc();
   checkHandle(topicQos, "DDS_TopicQos__alloc");

   g_status = DDS_Topic_get_qos(topic, topicQos);
   checkStatus(g_status, "DDS_Topic_get_qos");

   g_status = DDS_Subscriber_get_default_datareader_qos(subscriber, dataReaderQos);
   checkStatus(g_status, "DDS_Subscriber_get_default_datareader_qos");

   g_status = DDS_Subscriber_copy_from_topic_qos(subscriber, dataReaderQos, topicQos);
   checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");

   dataReader = DDS_Subscriber_create_datareader(subscriber, topic, dataReaderQos, NULL, DDS_STATUS_MASK_NONE);
   checkHandle(dataReader, "DDS_Subscriber_create_datareader");

   DDS_free(dataReaderQos);
   DDS_free(topicQos);

   return dataReader;
}

void deleteDataReader(DDS_Subscriber subscriber, DDS_DataReader dataReader)
{
   g_status = DDS_Subscriber_delete_datareader(subscriber, dataReader);
   checkStatus(g_status, "DDS_Subscriber_delete_datareader");
}
