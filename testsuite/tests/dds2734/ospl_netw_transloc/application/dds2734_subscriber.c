#include <string.h>
#include <time.h>

#include "dds_dcps.h"
#include "dds2734.h"

#define EXPECTED_COUNT 10
#define MAX_WAITING_TIME 60

char *RetCodeName[] =
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

DDS_DomainId_t                   domain         = DDS_DOMAIN_ID_DEFAULT;
DDS_DomainParticipantFactory     factory        = DDS_OBJECT_NIL;
DDS_DomainParticipant            participant    = DDS_OBJECT_NIL;
DDS_Topic                        testTopic      = DDS_OBJECT_NIL;
DDS_Subscriber                   subscriber     = DDS_OBJECT_NIL;
dds2734_TestTopicTypeSupport     type_support   = DDS_OBJECT_NIL;
dds2734_TestTopicDataReader      reader         = DDS_OBJECT_NIL;
DDS_sequence_dds2734_TestTopic*  test_topic_seq = DDS_OBJECT_NIL;
DDS_SampleInfoSeq*               info_seq       = DDS_OBJECT_NIL;
DDS_TopicQos                     topic_qos;
time_t                           startTime;
time_t                           endTime;

void check_status(DDS_ReturnCode_t status, const char *info);
void check_handle(void *handle, char *info);
void clean_all(void);

int main()
{
   DDS_ReturnCode_t status;
   unsigned int     i              = 0;
   int              counter        = 0;
   os_time          sleep_time     = {0, 100000000};
   
   startTime = time(NULL);
   endTime = time(NULL);

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

   status = DDS_Topic_enable(testTopic);
   check_status(status, "DDS_Topic_enable");

   subscriber = DDS_DomainParticipant_create_subscriber(
      participant,
      DDS_SUBSCRIBER_QOS_DEFAULT,
      DDS_OBJECT_NIL,
      DDS_STATUS_MASK_NONE);
   check_handle(subscriber, "DDS_DomainParticipant_create_subscriber");

   status = DDS_Subscriber_enable(subscriber);
   check_status(status, "DDS_Subscriber_enable");

   reader = DDS_Subscriber_create_datareader(
      subscriber,
      testTopic,
      DDS_DATAREADER_QOS_USE_TOPIC_QOS,
      DDS_OBJECT_NIL,
      DDS_STATUS_MASK_NONE);
   check_handle(reader, "DDS_subscriber_create_datawriter");

   status = DDS_DataReader_enable(reader);
   check_status(status, "DDS_DataReader_enable");

   printf("Subscriber started.\n");

   test_topic_seq = DDS_sequence_dds2734_TestTopic__alloc();
   check_handle(test_topic_seq, "DDS_sequence_dds2734_TestTopic__alloc");

   info_seq = DDS_SampleInfoSeq__alloc();
   check_handle(info_seq, "DDS_SampleInfoSeq__alloc");

   while (counter < EXPECTED_COUNT && (difftime(endTime, startTime) < MAX_WAITING_TIME) )
   {
      status = dds2734_TestTopicDataReader_take(
         reader,
         test_topic_seq,
         info_seq,
         1,
         DDS_NOT_READ_SAMPLE_STATE,
         DDS_ANY_VIEW_STATE,
         DDS_ANY_INSTANCE_STATE);
      check_status(status, "dds2734_TestTopicDataReader_take");

      for (i = 0; i < test_topic_seq->_length; i++)
      {
         printf ("Reading testtopic: %d.\n", test_topic_seq->_buffer[i].id);
         counter++;
      }
      status = dds2734_TestTopicDataReader_return_loan(reader, test_topic_seq, info_seq);
      check_status(status, "dds2734_TestTopicDataReader_return_loan");
      os_nanoSleep(sleep_time);
      endTime = time(NULL);
   }

   clean_all();

   exit(EXIT_SUCCESS);
}

void check_status(DDS_ReturnCode_t status, const char *info)
{ 
   if (status != DDS_RETCODE_OK && status != DDS_RETCODE_NO_DATA)
   {
      printf("Error in %s: %s\n", info, RetCodeName[status]);
      clean_all();
      exit (EXIT_FAILURE);
   }
}

void check_handle(void *handle, char *info)
{
    if (!handle)
    {
       printf("Error in %s: Creation failed: invalid handle\n", info);
       clean_all();
       exit(EXIT_FAILURE);
    }
}

void clean_all(void)
{
   if (reader != DDS_OBJECT_NIL)
   {
       if ((info_seq != DDS_OBJECT_NIL) && (test_topic_seq != DDS_OBJECT_NIL))
       {
           dds2734_TestTopicDataReader_return_loan(reader, test_topic_seq, info_seq);
       }
   }
   if (participant != DDS_OBJECT_NIL)
   {
       DDS_DomainParticipant_delete_contained_entities(participant);
       DDS_DomainParticipantFactory_delete_participant(factory, participant);
   }

   if (type_support != DDS_OBJECT_NIL)
   {
       DDS_free(type_support);
   }
}
