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
 * LOGICAL_NAME:    DDSEntitiesManager.c
 * FUNCTION:        Implementation of functions calling DDS OpenSplice API code using basic error handling.
 * MODULE:          OpenSplice Durability example for the C programming language.
 * DATE             September 2010.
 ***********************************************************************/
#include "DDSEntitiesManager.h"

// DDS entities and other variables
DDS_DomainId_t g_domainId = DDS_OBJECT_NIL;
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

// QosPolicy holders
DDS_TopicQos* g_TopicQos = DDS_OBJECT_NIL;

DDS_DataWriterQos* g_DataWriterQos = DDS_OBJECT_NIL;
DDS_DataReaderQos* g_DataReaderQos = DDS_OBJECT_NIL;

// Specific QoS settings
DDS_boolean g_autodispose_unregistered_instances = FALSE;
char* g_durability_kind;

// Error handling
DDS_ReturnCode_t g_status;

void createParticipant(const char * partitiontName)
{
   g_domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
   checkHandle(g_domainParticipantFactory, "DDS_DomainParticipantFactory_get_instance");

   g_domainParticipant = DDS_DomainParticipantFactory_create_participant(g_domainParticipantFactory, g_domainId, DDS_PARTICIPANT_QOS_DEFAULT, NULL, DDS_ANY_STATUS);

   checkHandle(g_domainParticipant, "DDS_DomainParticipantFactory_create_participant");

   g_partitionName = partitiontName;
}

void deleteParticipant()
{
   g_status = DDS_DomainParticipantFactory_delete_participant(g_domainParticipantFactory, g_domainParticipant);
   checkStatus(g_status, "DDS_DomainParticipantFactory_delete_participant");
}

void registerType(DDS_TypeSupport typeSupport)
{
   char* typeName = (char*) DurabilityData_MsgTypeSupport_get_type_name(typeSupport);

   g_status = DurabilityData_MsgTypeSupport_register_type(typeSupport,

   g_domainParticipant, typeName);
   checkStatus(g_status, "DurabilityData_MsgTypeSupport_register_type");

   DDS_free(typeName);
}

void createTopic(const char *topicName, const char *typeName)
{
   const char* messageFirstPart;
   char* message;
   int messageFirstPartLength, topicNameLength;

   g_TopicQos = DDS_TopicQos__alloc();
   checkHandle(g_TopicQos, "DDS_TopicQos__alloc");
   g_status = DDS_DomainParticipant_get_default_topic_qos(g_domainParticipant, g_TopicQos);
   checkStatus(g_status, "DDS_DomainParticipant_get_default_topic_qos");
   g_TopicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

   if( strcmp(g_durability_kind, "transient") == 0 )
   {
      g_TopicQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;
   }
   else if( strcmp(g_durability_kind, "persistent") == 0 )
   {
      g_TopicQos->durability.kind = DDS_PERSISTENT_DURABILITY_QOS;
   }

   // Use the changed policy when defining the DurabilityData topic.
   g_Topic = DDS_DomainParticipant_create_topic(g_domainParticipant, topicName, typeName, g_TopicQos, NULL, DDS_ANY_STATUS);

   //Format error message
   messageFirstPart = "DDS_DomainParticipant_create_topic ";
   messageFirstPartLength = strlen(messageFirstPart);
   topicNameLength = strlen(topicName);
   message = (char*) DDS_string_alloc(messageFirstPartLength + topicNameLength);

   snprintf(message, messageFirstPartLength + topicNameLength + 1, "%s%s", messageFirstPart, topicName);
   checkHandle(g_Topic, message);

   DDS_free(message);
}

void deleteTopic()
{
   g_status = DDS_DomainParticipant_delete_topic(g_domainParticipant, g_Topic);
   //TODO
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
   publisherQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
   checkHandle(publisherQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
   publisherQos->partition.name._buffer[0] = DDS_string_alloc(strlen(g_partitionName));
   checkHandle(publisherQos->partition.name._buffer[0], "DDS_string_alloc");
   strcpy(publisherQos->partition.name._buffer[0], g_partitionName);

   /* Create a Publisher for the application. */
   g_Publisher = DDS_DomainParticipant_create_publisher(g_domainParticipant, publisherQos, NULL, DDS_ANY_STATUS);
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
   // Create a DataWriter for the Topic (using the appropriate QoS).
   g_DataWriterQos = DDS_DataWriterQos__alloc();
   checkHandle(g_DataWriterQos, "DDS_DataWriterQos__alloc");
   g_status = DDS_Publisher_get_default_datawriter_qos(g_Publisher, g_DataWriterQos);
   checkStatus(g_status, "DDS_Publisher_get_default_datawriter_qos");
   g_status = DDS_Publisher_copy_from_topic_qos(g_Publisher, g_DataWriterQos, g_TopicQos);
   checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");

   // If autodispose_unregistered_instances is true, when the writer is stopped,
   // the instances of the topic will be suppressed from the persistence file;
   // If it is false the data will still be accessible after the Writer has stopped,
   // even if a Reader starts after the Writer's end and then reads the Topic's persistent samples.
   g_DataWriterQos->writer_data_lifecycle.autodispose_unregistered_instances = g_autodispose_unregistered_instances;
   g_DataWriter = DDS_Publisher_create_datawriter(g_Publisher, g_Topic, g_DataWriterQos, NULL, DDS_ANY_STATUS);
   checkHandle(g_DataWriter, "DDS_Publisher_create_datawriter (g_DataWriter)");
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
   subscriberQos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
   checkHandle(subscriberQos->partition.name._buffer, "DDS_StringSeq_allocbuf");
   subscriberQos->partition.name._buffer[0] = DDS_string_alloc(strlen(g_partitionName));
   checkHandle(subscriberQos->partition.name._buffer[0], "DDS_string_alloc");
   strcpy(subscriberQos->partition.name._buffer[0], g_partitionName);

   // Create a Subscriber for the MsgBoard application.
   g_Subscriber = DDS_DomainParticipant_create_subscriber(g_domainParticipant, subscriberQos, NULL, DDS_ANY_STATUS);
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
   // Create a DataWriter for the NameService Topic (using the appropriate QoS).
   g_DataReaderQos = DDS_DataReaderQos__alloc();
   checkHandle(g_DataReaderQos, "DDS_DataReaderQos__alloc");
   g_status = DDS_Subscriber_get_default_datareader_qos(g_Subscriber, g_DataReaderQos);
   checkStatus(g_status, "DDS_Subscriber_get_default_datareader_qos");
   g_status = DDS_Subscriber_copy_from_topic_qos(g_Subscriber, g_DataReaderQos, g_TopicQos);
   checkStatus(g_status, "DDS_Publisher_copy_from_topic_qos");

   g_DataReader = DDS_Subscriber_create_datareader(g_Subscriber, g_Topic, g_DataReaderQos, NULL, DDS_ANY_STATUS);
   checkHandle(g_DataReader, "DDS_Subscriber_create_datareader");
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
