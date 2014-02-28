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
#include "ccpp_Utils.h"
#include "ccpp_QosUtils.h"
#include "gapi.h"
#include "os.h"
#include "ccpp_TypeSupport_impl.h"

void DDS::ccpp_AllocateGapiSeq(gapi_octet* *buffer, gapi_unsigned_long len)
{
  *buffer = gapi_octetSeq_allocbuf(len);
}

void DDS::ccpp_AllocateGapiSeq(gapi_string* *buffer, gapi_unsigned_long len)
{
  *buffer = gapi_stringSeq_allocbuf(len);
}

void DDS::ccpp_AllocateGapiSeq( gapi_instanceHandle_t* *buffer, gapi_unsigned_long len)
{
  *buffer = gapi_instanceHandleSeq_allocbuf(len);
}


void DDS::ccpp_AllocateDdsSeq(DDS::Octet * *buffer, DDS::ULong len)
{
  *buffer = reinterpret_cast<DDS::Octet *>(os_malloc(sizeof(DDS::Octet) * len));
}

void DDS::ccpp_AllocateDdsSeq(char** *buffer, DDS::ULong len)
{
  *buffer = reinterpret_cast<char**>(os_malloc(sizeof(char*) * len));
}

void DDS::ccpp_AllocateDdsSeq(DDS::InstanceHandle_t* *buffer, DDS::ULong len)
{
  *buffer = reinterpret_cast<DDS::InstanceHandle_t*>(os_malloc(sizeof(DDS::InstanceHandle_t) * len));
}

void DDS::ccpp_CopySeqElemIn(DDS::Octet & from, gapi_octet & to)
{
  to = from;
}

void DDS::ccpp_CopySeqElemIn(char* & from, gapi_string & to)
{
  if (from)
  {
    to = gapi_string_dup(from);
  }
  else
  {
    to = NULL;
  }
}

void DDS::ccpp_CopySeqElemIn(DDS::InstanceHandle_t & from, gapi_instanceHandle_t & to)
{
  to = from;
}


void DDS::ccpp_CopySeqElemOut(gapi_octet & from, DDS::Octet & to)
{
  to = from;
}

void DDS::ccpp_CopySeqElemOut(gapi_string & from, char* & to)
{
  if (from)
  {
    to = DDS::string_dup(from);
  }
  else
  {
    to = NULL;
  }
}

void DDS::ccpp_CopySeqElemOut(gapi_instanceHandle_t & from, DDS::InstanceHandle_t & to)
{
  to = from;
}

void DDS::ccpp_CopySeqElemOut(gapi_qosPolicyCount_s & from, DDS::QosPolicyCount & to)
{
  to.policy_id = from.policy_id;
  to.count = from.count;
}


void DDS::ccpp_sequenceCopyIn( const DDS::StringSeq &from, gapi_stringSeq &to)
{
    to._maximum = from.length();
    to._length = from.length();
    to._release = TRUE;
    if (to._maximum > 0){
      to._buffer = gapi_stringSeq_allocbuf(to._length);
      for (DDS::ULong i=0; i<to._length; i++)
      {
        const char *value = from[i];
        to._buffer[i] = gapi_string_dup(value);
      }
    } else {
      to._buffer = NULL;
    };
}

void DDS::ccpp_sequenceCopyOut( const gapi_stringSeq &from, DDS::StringSeq &to)
{
   to.length(from._length);
   for (DDS::ULong i=0; i<from._length; i++)
   {
     to[i] = DDS::string_dup(reinterpret_cast<const char *>(from._buffer[i]));
   }
}

void DDS::ccpp_Duration_copyIn( const DDS::Duration_t & from,
        gapi_duration_t & to)
{
    to.sec = from.sec;
    to.nanosec = from.nanosec;
}

void DDS::ccpp_Duration_copyOut( const gapi_duration_t & from,
        DDS::Duration_t & to)
{
    to.sec = from.sec;
    to.nanosec = from.nanosec;
}

void DDS::ccpp_BuiltinTopicKey_copyOut( const gapi_builtinTopicKey_t & from,
    DDS::BuiltinTopicKey_t &to)
{
  for (int i=0; i<3; i++)
  {
    to[i] = from[i];
  }
}

void DDS::ccpp_ParticipantBuiltinTopicData_copyOut(
    const gapi_participantBuiltinTopicData & from,
    DDS::ParticipantBuiltinTopicData & to)
{
    DDS::ccpp_BuiltinTopicKey_copyOut(from.key, to.key);
    DDS::ccpp_UserDataQosPolicy_copyOut(from.user_data, to.user_data);
}

void DDS::ccpp_TopicBuiltinTopicData_copyOut(
    const gapi_topicBuiltinTopicData & from,
    DDS::TopicBuiltinTopicData & to)
{
    DDS::ccpp_BuiltinTopicKey_copyOut(from.key, to.key);
    to.name = DDS::string_dup(from.name);
    to.type_name = DDS::string_dup(from.type_name);
    DDS::ccpp_DurabilityQosPolicy_copyOut(from.durability, to.durability);
    DDS::ccpp_DurabilityServiceQosPolicy_copyOut(from.durability_service, to.durability_service);
    DDS::ccpp_DeadlineQosPolicy_copyOut(from.deadline, to.deadline);
    DDS::ccpp_LatencyBudgetQosPolicy_copyOut(from.latency_budget, to.latency_budget);
    DDS::ccpp_LivelinessQosPolicy_copyOut(from.liveliness, to.liveliness);
    DDS::ccpp_ReliabilityQosPolicy_copyOut(from.reliability, to.reliability);
    DDS::ccpp_TransportPriorityQosPolicy_copyOut(from.transport_priority, to.transport_priority);
    DDS::ccpp_LifespanQosPolicy_copyOut(from.lifespan, to.lifespan);
    DDS::ccpp_DestinationOrderQosPolicy_copyOut(from.destination_order,to.destination_order);
    DDS::ccpp_HistoryQosPolicy_copyOut(from.history, to.history);
    DDS::ccpp_ResourceLimitsQosPolicy_copyOut(from.resource_limits, to.resource_limits);
    DDS::ccpp_OwnershipQosPolicy_copyOut(from.ownership, to.ownership);
    DDS::ccpp_TopicDataQosPolicy_copyOut(from.topic_data, to.topic_data);
}

void DDS::ccpp_SubscriptionBuiltinTopicData_copyOut(
    const gapi_subscriptionBuiltinTopicData & from,
    DDS::SubscriptionBuiltinTopicData & to)
{
    DDS::ccpp_BuiltinTopicKey_copyOut(from.key, to.key);
    DDS::ccpp_BuiltinTopicKey_copyOut(from.participant_key, to.participant_key);
    to.topic_name = DDS::string_dup(from.topic_name);
    to.type_name = DDS::string_dup(from.type_name);
    DDS::ccpp_DurabilityQosPolicy_copyOut(from.durability, to.durability);
    DDS::ccpp_DeadlineQosPolicy_copyOut(from.deadline, to.deadline);
    DDS::ccpp_LatencyBudgetQosPolicy_copyOut(from.latency_budget, to.latency_budget);
    DDS::ccpp_LivelinessQosPolicy_copyOut(from.liveliness, to.liveliness);
    DDS::ccpp_ReliabilityQosPolicy_copyOut(from.reliability, to.reliability);
    DDS::ccpp_OwnershipQosPolicy_copyOut(from.ownership, to.ownership);
    DDS::ccpp_DestinationOrderQosPolicy_copyOut(from.destination_order,to.destination_order);
    DDS::ccpp_UserDataQosPolicy_copyOut(from.user_data, to.user_data);
    DDS::ccpp_TimeBasedFilterQosPolicy_copyOut(from.time_based_filter, to.time_based_filter);
    DDS::ccpp_PresentationQosPolicy_copyOut(from.presentation, to.presentation);
    DDS::ccpp_PartitionQosPolicy_copyOut(from.partition, to.partition);
    DDS::ccpp_TopicDataQosPolicy_copyOut(from.topic_data, to.topic_data);
    DDS::ccpp_GroupDataQosPolicy_copyOut(from.group_data, to.group_data);
}

void DDS::ccpp_PublicationBuiltinTopicData_copyOut(
    const gapi_publicationBuiltinTopicData & from,
    DDS::PublicationBuiltinTopicData & to)
{
    DDS::ccpp_BuiltinTopicKey_copyOut(from.key, to.key);
    DDS::ccpp_BuiltinTopicKey_copyOut(from.participant_key, to.participant_key);
    to.topic_name = DDS::string_dup(from.topic_name);
    to.type_name = DDS::string_dup(from.type_name);
    DDS::ccpp_DurabilityQosPolicy_copyOut(from.durability, to.durability);
    DDS::ccpp_DeadlineQosPolicy_copyOut(from.deadline, to.deadline);
    DDS::ccpp_LatencyBudgetQosPolicy_copyOut(from.latency_budget, to.latency_budget);
    DDS::ccpp_LivelinessQosPolicy_copyOut(from.liveliness, to.liveliness);
    DDS::ccpp_ReliabilityQosPolicy_copyOut(from.reliability, to.reliability);
    DDS::ccpp_LifespanQosPolicy_copyOut(from.lifespan, to.lifespan);
    DDS::ccpp_DestinationOrderQosPolicy_copyOut(from.destination_order, to.destination_order);
    DDS::ccpp_UserDataQosPolicy_copyOut(from.user_data, to.user_data);
    DDS::ccpp_OwnershipQosPolicy_copyOut(from.ownership, to.ownership);
    DDS::ccpp_OwnershipStrengthQosPolicy_copyOut(from.ownership_strength, to.ownership_strength);
    DDS::ccpp_PresentationQosPolicy_copyOut(from.presentation, to.presentation);
    DDS::ccpp_PartitionQosPolicy_copyOut(from.partition, to.partition);
    DDS::ccpp_TopicDataQosPolicy_copyOut(from.topic_data, to.topic_data);
    DDS::ccpp_GroupDataQosPolicy_copyOut(from.group_data, to.group_data);
}

void ccpp_CallBack_DeleteUserData( void * userData, void * arg)
{
  if (userData)
  {
    DDS::Object_ptr cObject;
    DDS::LocalObject_ptr anObject;

    cObject = static_cast<DDS::Object_ptr>(userData);
    anObject = dynamic_cast<DDS::LocalObject_ptr>(cObject);
    DDS::release(anObject);
  }
}

void DDS::ccpp_TimeStamp_copyIn(const DDS::Time_t & from, gapi_time_t & to)
{
  to.sec = from.sec;
  to.nanosec = from.nanosec;
}

void DDS::ccpp_TimeStamp_copyOut(const gapi_time_t & from, DDS::Time_t & to)
{
  to.sec = from.sec;
  to.nanosec = from.nanosec;
}


void DDS::ccpp_SampleInfo_copyOut(const gapi_sampleInfo & from, DDS::SampleInfo & to)
{
    to.sample_state                = from.sample_state;
    to.view_state                  = from.view_state;
    to.instance_state              = from.instance_state;
    to.valid_data                  = from.valid_data != 0;
    ccpp_TimeStamp_copyOut(from.source_timestamp, to.source_timestamp);
    to.instance_handle             = from.instance_handle;
    to.publication_handle          = from.publication_handle;
    to.disposed_generation_count   = from.disposed_generation_count;
    to.no_writers_generation_count = from.no_writers_generation_count;
    to.sample_rank                 = from.sample_rank;
    to.generation_rank             = from.generation_rank;
    to.absolute_generation_rank    = from.absolute_generation_rank;
    ccpp_TimeStamp_copyOut(from.reception_timestamp, to.reception_timestamp);
}

