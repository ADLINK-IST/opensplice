#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <os.h>

#include "dds_dcps.h"
#include "dds2734.h"

#define NUM_TOPICS            10

char *RetCodeName[13] =
{ 
   "DDS_RETCODE_OK",
   "DDS_RETCODE_ERROR",
   "DDS_RETCODE_UNSUPPORTED",
   "DDS_RETCODE_BAD_PARAMETER",
   "DDS_RETCODE_PRECONDITION_NOT_MET",
   "DDS_RETCODE_OUT_OF_RESOURCES",
   "DDS_RETCODE_NOT_ENABLED",
   "DDS_RETCODE_IMMUTABLE_POLICY",
   "DDS_RETCODE_INCONSISTENT_POLICY",
   "DDS_RETCODE_ALREADY_DELETED",
   "DDS_RETCODE_TIMEOUT",
   "DDS_RETCODE_NO_DATA",
   "DDS_RETCODE_ILLEGAL_OPERATION"
};

DDS_DomainParticipantFactory    factory       = DDS_OBJECT_NIL;
DDS_DomainParticipant           participant   = DDS_OBJECT_NIL;
DDS_Publisher                   publisher     = DDS_OBJECT_NIL;
DDS_DomainId_t                  domain        = DDS_DOMAIN_ID_DEFAULT;
dds2734_TestTopicTypeSupport    type_support  = DDS_OBJECT_NIL;
dds2734_TestTopicDataWriter     writer        = DDS_OBJECT_NIL;
dds2734_TestTopic*              testTopic     = DDS_OBJECT_NIL;
DDS_TopicQos                    topic_qos;

void check_status(DDS_ReturnCode_t status, const char *info);
void check_handle(void *handle, char *info);
void clean_all(void);

int main()
{
   os_time sleep_time = {60, 0};   
   DDS_ReturnCode_t                status;
   int i = 0;

   printf("Publisher started.\n");

   factory = DDS_DomainParticipantFactory_get_instance();
   check_handle(factory, "DDS_DomainParticipantFactory_get_instance");

   participant = DDS_DomainParticipantFactory_create_participant(
      factory,
      domain,
      DDS_PARTICIPANT_QOS_DEFAULT,
      DDS_OBJECT_NIL,
      DDS_STATUS_MASK_NONE);
   check_handle(participant, "DDS_DomainParticipantFactory_create_participant");

   type_support = dds2734_TestTopicTypeSupport__alloc();
   check_handle(type_support, "dds2734_TestTopicTypeSupport__alloc");

   status = dds2734_TestTopicTypeSupport_register_type(
      type_support,
      participant,
      dds2734_TestTopicTypeSupport_get_type_name(type_support));
   check_status(status, "dds2734_TestTopicTypeSupport_register_type");

   status = DDS_DomainParticipant_get_default_topic_qos(participant, &topic_qos);
   check_status(status, "DDS_DomainParticipant_get_default_topic_qos");
   topic_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;

   testTopic = DDS_DomainParticipant_create_topic(
      participant,
      "dds2734_TestTopic",
      dds2734_TestTopicTypeSupport_get_type_name(type_support),
      &topic_qos,
      DDS_OBJECT_NIL,
      DDS_STATUS_MASK_NONE);
    check_handle(testTopic, "DDS_DomainParticipant_create_topic");

   publisher = DDS_DomainParticipant_create_publisher(
      participant,
      DDS_PUBLISHER_QOS_DEFAULT,
      DDS_OBJECT_NIL,
      DDS_STATUS_MASK_NONE);
   check_handle(publisher, "DDS_DomainParticipant_create_publisher");

   writer = DDS_Publisher_create_datawriter(
      publisher,
      testTopic,
      DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
      DDS_OBJECT_NIL,
      DDS_STATUS_MASK_NONE);
   check_handle(writer, "DDS_Publisher_create_datawriter");

   for (i = 0; i < NUM_TOPICS; i++)
   {
      testTopic = dds2734_TestTopic__alloc();
      check_handle(testTopic, "dds2734_TestTopic__alloc");
      testTopic->id = i;
      printf("Writing test topic: %d.\n", testTopic->id);
      status = dds2734_TestTopicDataWriter_write(writer, testTopic, 0);
      check_status(status, "ddds2734_TestTopicDataWriter_write");
      DDS_free(testTopic);
   }

   os_nanoSleep(sleep_time);
   clean_all();

   exit(EXIT_SUCCESS);
}

void check_status(DDS_ReturnCode_t status, const char *info)
{ 
   if (status != DDS_RETCODE_OK && status != DDS_RETCODE_NO_DATA)
   {
      printf("Error in %s: %s\n", info, RetCodeName[status]);
      clean_all();
      exit(EXIT_FAILURE);
   }
}

void check_handle(void *handle, char *info)
{
   if (handle == DDS_OBJECT_NIL)
   {
      printf("Error in %s: Creation failed: invalid handle\n", info);
      clean_all();
      exit(EXIT_FAILURE);
   }
}

void clean_all(void)
{
   if (participant != DDS_OBJECT_NIL)
   {
      DDS_DomainParticipant_delete_contained_entities(participant);
      DDS_DomainParticipantFactory_delete_participant(factory, participant);
   }

   if (type_support != DDS_OBJECT_NIL)
   {
      DDS_free(type_support);
   }

   if (testTopic != DDS_OBJECT_NIL)
   {
      DDS_free(testTopic);
   }
}
