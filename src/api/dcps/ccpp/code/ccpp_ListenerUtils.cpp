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

#include "ccpp_ListenerUtils.h"
#include "ccpp_Utils.h"
#include "ccpp_QosUtils.h"

static DDS::DataWriter_ptr DataWriter_Lookup(gapi_dataWriter handle)
{
  DDS::DataWriter_ptr dataWriter = NULL;
  DDS::ccpp_UserData_ptr myUD;

  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
  if (myUD)
  {
    dataWriter = dynamic_cast<DDS::DataWriter_ptr>(myUD->ccpp_object);
    if (dataWriter)
    {
      DDS::DataWriter::_duplicate(dataWriter);
    }
  }
  return dataWriter;
}

static void CallBackWrapper_DataWriterListener_OfferedDeadlineMissed(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredDeadlineMissedStatus *gapi_status
)
{
  DDS::DataWriterListener_ptr listener;
  DDS::OfferedDeadlineMissedStatus status;
  DDS::DataWriter_ptr dataWriter = NULL;

  dataWriter = DataWriter_Lookup(writer);

  status.total_count = gapi_status->total_count;
  status.total_count_change = gapi_status->total_count_change;
  status.last_instance_handle = gapi_status->last_instance_handle;

  listener = reinterpret_cast<DDS::DataWriterListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_offered_deadline_missed(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_DataWriterListener_OfferedIncompatibleQos(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredIncompatibleQosStatus *gapi_status
    )
{
  DDS::DataWriterListener_ptr listener;
  DDS::OfferedIncompatibleQosStatus status;
  DDS::DataWriter_ptr dataWriter;

  dataWriter = DataWriter_Lookup(writer);

  ccpp_OfferedIncompatibleQosStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataWriterListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_offered_incompatible_qos(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_DataWriterListener_LivelinessLost(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_livelinessLostStatus *gapi_status
    )
{
  DDS::DataWriterListener_ptr listener;
  DDS::LivelinessLostStatus status;
  DDS::DataWriter * dataWriter;

  dataWriter = DataWriter_Lookup(writer);

  status.total_count = gapi_status->total_count;
  status.total_count_change = gapi_status->total_count_change;

  listener = reinterpret_cast<DDS::DataWriterListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_liveliness_lost(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_DataWriterListener_PublicationMatched(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_publicationMatchedStatus *gapi_status
    )
{
  DDS::DataWriterListener_ptr listener;
  DDS::PublicationMatchedStatus status;
  DDS::DataWriter * dataWriter;

  dataWriter = DataWriter_Lookup(writer);

  status.total_count = gapi_status->total_count;
  status.total_count_change = gapi_status->total_count_change;
  status.current_count = gapi_status->current_count;
  status.current_count_change = gapi_status->current_count_change;
  status.last_subscription_handle = (DDS::InstanceHandle_t)gapi_status->last_subscription_handle;

  listener = reinterpret_cast<DDS::DataWriterListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_publication_matched(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}


void DDS::ccpp_DataWriterListener_copyIn(const DDS::DataWriterListener_ptr & from,
        gapi_dataWriterListener & to)
{
  to.listener_data = from;

  to.on_offered_deadline_missed  = CallBackWrapper_DataWriterListener_OfferedDeadlineMissed;
  to.on_offered_incompatible_qos = CallBackWrapper_DataWriterListener_OfferedIncompatibleQos;
  to.on_liveliness_lost          = CallBackWrapper_DataWriterListener_LivelinessLost;
  to.on_publication_match        = CallBackWrapper_DataWriterListener_PublicationMatched;
}





static DDS::DataReader_ptr DataReader_Lookup(gapi_dataReader handle)
{
  DDS::DataReader_ptr dataReader = NULL;
  DDS::ccpp_UserData_ptr myUD;

  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
  if (myUD)
  {
    dataReader = dynamic_cast<DDS::DataReader_ptr>(myUD->ccpp_object);
    if (dataReader)
    {
      DDS::DataReader::_duplicate(dataReader);
    }
  }
  return dataReader;
}



static void CallBackWrapper_DataReaderListener_RequestedDeadlineMissed
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedDeadlineMissedStatus *gapi_status
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::RequestedDeadlineMissedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_RequestedDeadlineMissedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_requested_deadline_missed(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}


static void CallBackWrapper_DataReaderListener_RequestedIncompatibleQos
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedIncompatibleQosStatus *gapi_status
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::RequestedIncompatibleQosStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_RequestedIncompatibleQosStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_requested_incompatible_qos(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DataReaderListener_SampleRejected
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleRejectedStatus *gapi_status
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::SampleRejectedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SampleRejectedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_sample_rejected(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DataReaderListener_LivelinessChanged
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_livelinessChangedStatus *gapi_status
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::LivelinessChangedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_LivelinessChangedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_liveliness_changed(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DataReaderListener_DataAvailable
(
    void *listener_data,
    gapi_dataReader reader
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_data_available(dataReader);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DataReaderListener_SubscriptionMatched
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_subscriptionMatchedStatus *gapi_status
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::SubscriptionMatchedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SubscriptionMatchedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_subscription_matched(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DataReaderListener_SampleLost
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleLostStatus *gapi_status
)
{
  DDS::DataReaderListener_ptr listener;
  DDS::SampleLostStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SampleLostStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DataReaderListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_sample_lost(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

void DDS::ccpp_DataReaderListener_copyIn(
        const DDS::DataReaderListener_ptr & from,
        gapi_dataReaderListener & to)
{
    to.listener_data = from;

    to.on_requested_deadline_missed  = CallBackWrapper_DataReaderListener_RequestedDeadlineMissed;
    to.on_requested_incompatible_qos = CallBackWrapper_DataReaderListener_RequestedIncompatibleQos;
    to.on_sample_rejected            = CallBackWrapper_DataReaderListener_SampleRejected;
    to.on_liveliness_changed         = CallBackWrapper_DataReaderListener_LivelinessChanged;
    to.on_data_available             = CallBackWrapper_DataReaderListener_DataAvailable;
    to.on_subscription_match         = CallBackWrapper_DataReaderListener_SubscriptionMatched;
    to.on_sample_lost                = CallBackWrapper_DataReaderListener_SampleLost;
}



static void CallBackWrapper_Publisher_OfferedDeadlineMissedListener(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredDeadlineMissedStatus *gapi_status
)
{
  DDS::PublisherListener_ptr listener;
  DDS::OfferedDeadlineMissedStatus status;
  DDS::DataWriter_ptr dataWriter = NULL;

  dataWriter = DataWriter_Lookup(writer);

  status.total_count = gapi_status->total_count;
  status.total_count_change = gapi_status->total_count_change;
  status.last_instance_handle = gapi_status->last_instance_handle;

  listener = reinterpret_cast<DDS::PublisherListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_offered_deadline_missed(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_Publisher_OfferedIncompatibleQosListener(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredIncompatibleQosStatus *gapi_status
    )
{
  DDS::PublisherListener_ptr listener;
  DDS::OfferedIncompatibleQosStatus status;
  DDS::DataWriter_ptr dataWriter;

  dataWriter = DataWriter_Lookup(writer);

  ccpp_OfferedIncompatibleQosStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::PublisherListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_offered_incompatible_qos(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_Publisher_LivelinessLostListener(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_livelinessLostStatus *gapi_status
    )
{
  DDS::PublisherListener_ptr listener;
  DDS::LivelinessLostStatus status;
  DDS::DataWriter * dataWriter;

  dataWriter = DataWriter_Lookup(writer);

  status.total_count = gapi_status->total_count;
  status.total_count_change = gapi_status->total_count_change;

  listener = reinterpret_cast<DDS::PublisherListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_liveliness_lost(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_Publisher_PublicationMatchedListener(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_publicationMatchedStatus *gapi_status
    )
{
  DDS::PublisherListener_ptr listener;
  DDS::PublicationMatchedStatus status;
  DDS::DataWriter * dataWriter;

  dataWriter = DataWriter_Lookup(writer);

  status.total_count = gapi_status->total_count;
  status.total_count_change = gapi_status->total_count_change;
  status.last_subscription_handle = (DDS::InstanceHandle_t)gapi_status->last_subscription_handle;

  listener = reinterpret_cast<DDS::PublisherListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_publication_matched(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}


void DDS::ccpp_PublisherListener_copyIn( const DDS::PublisherListener_ptr & from,
        gapi_publisherListener & to)
{
  to.listener_data = from;

  to.on_offered_deadline_missed = CallBackWrapper_Publisher_OfferedDeadlineMissedListener;
  to.on_offered_incompatible_qos = CallBackWrapper_Publisher_OfferedIncompatibleQosListener;
  to.on_liveliness_lost = CallBackWrapper_Publisher_LivelinessLostListener;
  to.on_publication_match = CallBackWrapper_Publisher_PublicationMatchedListener;
}





static DDS::Subscriber_ptr Subscriber_Lookup(gapi_subscriber handle)
{
  DDS::Subscriber_ptr subscriber = NULL;
  DDS::ccpp_UserData_ptr myUD;

  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
  if (myUD)
  {
    subscriber = dynamic_cast<DDS::Subscriber_ptr>(myUD->ccpp_object);
    if (subscriber)
    {
      DDS::Subscriber::_duplicate(subscriber);
    }
  }
  return subscriber;
}

static void CallBackWrapper_SubscriberListener_RequestedDeadlineMissed
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedDeadlineMissedStatus *gapi_status
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::RequestedDeadlineMissedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_RequestedDeadlineMissedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_requested_deadline_missed(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}


static void CallBackWrapper_SubscriberListener_RequestedIncompatibleQos
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedIncompatibleQosStatus *gapi_status
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::RequestedIncompatibleQosStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_RequestedIncompatibleQosStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_requested_incompatible_qos(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_SubscriberListener_SampleRejected
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleRejectedStatus *gapi_status
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::SampleRejectedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SampleRejectedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_sample_rejected(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_SubscriberListener_LivelinessChanged
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_livelinessChangedStatus *gapi_status
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::LivelinessChangedStatus status;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_LivelinessChangedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_liveliness_changed(dataReader,status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_SubscriberListener_DataAvailable
(
    void *listener_data,
    gapi_dataReader reader
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::DataReader * dataReader;

  dataReader = DataReader_Lookup(reader);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_data_available(dataReader);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_SubscriberListener_SubscriptionMatched
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_subscriptionMatchedStatus *gapi_status
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::SubscriptionMatchedStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SubscriptionMatchedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_subscription_matched(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}


static void CallBackWrapper_SubscriberListener_SampleLost
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleLostStatus *gapi_status
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::SampleLostStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SampleLostStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_sample_lost(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_SubscriberListener_DataOnReaders
(
    void *listener_data,
    gapi_subscriber subscriber
)
{
  DDS::SubscriberListener_ptr listener;
  DDS::Subscriber_ptr Subscriber;

  Subscriber = Subscriber_Lookup(subscriber);

  listener = reinterpret_cast<DDS::SubscriberListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_data_on_readers(Subscriber);
  }
  if (Subscriber)
  {
    CORBA::release(Subscriber);
  }
}


void DDS::ccpp_SubscriberListener_copyIn(
        const DDS::SubscriberListener_ptr & from,
        gapi_subscriberListener & to)
{
    to.listener_data = from;

    to.on_requested_deadline_missed  = CallBackWrapper_SubscriberListener_RequestedDeadlineMissed;
    to.on_requested_incompatible_qos = CallBackWrapper_SubscriberListener_RequestedIncompatibleQos;
    to.on_sample_rejected            = CallBackWrapper_SubscriberListener_SampleRejected;
    to.on_liveliness_changed         = CallBackWrapper_SubscriberListener_LivelinessChanged;
    to.on_data_available             = CallBackWrapper_SubscriberListener_DataAvailable;
    to.on_subscription_match         = CallBackWrapper_SubscriberListener_SubscriptionMatched;
    to.on_sample_lost                = CallBackWrapper_SubscriberListener_SampleLost;
    to.on_data_on_readers            = CallBackWrapper_SubscriberListener_DataOnReaders;
}





static DDS::Topic_ptr Topic_Lookup(gapi_topic handle)
{
  DDS::Topic_ptr topic = NULL;
  DDS::ccpp_UserData_ptr myUD;

  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
  if (myUD)
  {
    topic = dynamic_cast<DDS::Topic_ptr>(myUD->ccpp_object);
    if (topic)
    {
      DDS::Topic::_duplicate(topic);
    }
  }
  return topic;
}

static void CallBackWrapper_TopicListener_InconsistentTopic
(
    void *listener_data,
    gapi_topic a_gapi_topic,
    const gapi_inconsistentTopicStatus *gapi_status
)
{
  DDS::TopicListener_ptr listener;
  DDS::InconsistentTopicStatus status;
  DDS::Topic_ptr a_topic;

  a_topic = Topic_Lookup(a_gapi_topic);

  ccpp_InconsistentTopicStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::TopicListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_inconsistent_topic(a_topic, status);
  }
  if (a_topic)
  {
    CORBA::release(a_topic);
  }
}

static void CallBackWrapper_TopicListener_AllDataDisposed
(
    void *listener_data,
    gapi_topic a_gapi_topic
)
{
  DDS::TopicListener_ptr listener;
  DDS::ExtTopicListener_ptr extListener;
  DDS::Topic_ptr a_topic;

  a_topic = Topic_Lookup(a_gapi_topic);

  listener = reinterpret_cast<DDS::TopicListener_ptr>(listener_data);
  if (listener)
  {
     extListener = DDS::ExtTopicListener::_narrow(listener);
     assert( extListener != NULL );
     extListener->on_all_data_disposed(a_topic);
  }
  if (a_topic)
  {
     CORBA::release(a_topic);
  }
}

void DDS::ccpp_TopicListener_copyIn(
        const DDS::TopicListener_ptr & from,
        gapi_topicListener & to)
{
   to.listener_data = from;
   to.on_inconsistent_topic = CallBackWrapper_TopicListener_InconsistentTopic;
   to.on_all_data_disposed = CallBackWrapper_TopicListener_AllDataDisposed;
}





static void CallBackWrapper_DomainParticipantListener_InconsistentTopic
(
    void *listener_data,
    gapi_topic a_gapi_topic,
    const gapi_inconsistentTopicStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::InconsistentTopicStatus status;
  DDS::Topic_ptr a_topic;

  a_topic = Topic_Lookup(a_gapi_topic);

  ccpp_InconsistentTopicStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_inconsistent_topic(a_topic, status);
  }
  if (a_topic)
  {
    CORBA::release(a_topic);
  }
}

static void CallBackWrapper_DomainParticipantListener_AllDataDisposed
(
    void *listener_data,
    gapi_topic a_gapi_topic
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::Topic_ptr a_topic;
  DDS::ExtTopicListener *extListener;

  a_topic = Topic_Lookup(a_gapi_topic);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
     extListener = DDS::ExtTopicListener::_narrow(listener);
     assert( extListener != NULL );
     extListener->on_all_data_disposed(a_topic);
  }
  if (a_topic)
  {
     CORBA::release(a_topic);
  }
}

static void CallBackWrapper_DomainParticipantListener_OfferedDeadlineMissed
(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredDeadlineMissedStatus *gapi_status
)
{
   DDS::DomainParticipantListener_ptr listener;
   DDS::OfferedDeadlineMissedStatus status;
   DDS::DataWriter_ptr dataWriter;

   dataWriter = DataWriter_Lookup(writer);

   ccpp_OfferedDeadlineMissedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_offered_deadline_missed(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_DomainParticipantListener_OfferedIncompatibleQos
(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_offeredIncompatibleQosStatus *gapi_status
)
{
   DDS::DomainParticipantListener_ptr listener;
   DDS::OfferedIncompatibleQosStatus status;
   DDS::DataWriter_ptr dataWriter;

   dataWriter = DataWriter_Lookup(writer);

   ccpp_OfferedIncompatibleQosStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_offered_incompatible_qos(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_DomainParticipantListener_LivelinessLost
(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_livelinessLostStatus *gapi_status
)
{
   DDS::DomainParticipantListener_ptr listener;
   DDS::LivelinessLostStatus status;
   DDS::DataWriter_ptr dataWriter;

   dataWriter = DataWriter_Lookup(writer);

   ccpp_LivelinessLostStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_liveliness_lost(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}

static void CallBackWrapper_DomainParticipantListener_PublicationMatched
(
    void *listener_data,
    gapi_dataWriter writer,
    const gapi_publicationMatchedStatus *gapi_status
)
{
   DDS::DomainParticipantListener_ptr listener;
   DDS::PublicationMatchedStatus status;
   DDS::DataWriter_ptr dataWriter;

   dataWriter = DataWriter_Lookup(writer);

   ccpp_PublicationMatchedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_publication_matched(dataWriter, status);
  }
  if (dataWriter)
  {
    CORBA::release(dataWriter);
  }
}


static void CallBackWrapper_DomainParticipantListener_RequestedDeadlineMissed
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedDeadlineMissedStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::RequestedDeadlineMissedStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_RequestedDeadlineMissedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_requested_deadline_missed(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}


static void CallBackWrapper_DomainParticipantListener_RequestedIncompatibleQos
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_requestedIncompatibleQosStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::RequestedIncompatibleQosStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_RequestedIncompatibleQosStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_requested_incompatible_qos(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DomainParticipantListener_SampleRejected
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleRejectedStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::SampleRejectedStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SampleRejectedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_sample_rejected(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DomainParticipantListener_LivelinessChanged
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_livelinessChangedStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::LivelinessChangedStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_LivelinessChangedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_liveliness_changed(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DomainParticipantListener_DataAvailable
(
    void *listener_data,
    gapi_dataReader reader
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_data_available(dataReader);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DomainParticipantListener_SubscriptionMatched
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_subscriptionMatchedStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::SubscriptionMatchedStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SubscriptionMatchedStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_subscription_matched(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DomainParticipantListener_SampleLost
(
    void *listener_data,
    gapi_dataReader reader,
    const gapi_sampleLostStatus *gapi_status
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::SampleLostStatus status;
  DDS::DataReader_ptr dataReader;

  dataReader = DataReader_Lookup(reader);

  ccpp_SampleLostStatus_copyOut(*gapi_status, status);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_sample_lost(dataReader, status);
  }
  if (dataReader)
  {
    CORBA::release(dataReader);
  }
}

static void CallBackWrapper_DomainParticipantListener_DataOnReaders
(
    void *listener_data,
    gapi_subscriber a_gapi_subscriber
)
{
  DDS::DomainParticipantListener_ptr listener;
  DDS::Subscriber_ptr a_subscriber;

  a_subscriber = Subscriber_Lookup(a_gapi_subscriber);

  listener = reinterpret_cast<DDS::DomainParticipantListener_ptr>(listener_data);
  if (listener)
  {
    listener->on_data_on_readers(a_subscriber);
  }
  if (a_subscriber)
  {
    CORBA::release(a_subscriber);
  }
}

void DDS::ccpp_DomainParticipantListener_copyIn(
        const DDS::DomainParticipantListener_ptr & from,
        gapi_domainParticipantListener & to)
{
    to.listener_data = from;

    to.on_inconsistent_topic         = CallBackWrapper_DomainParticipantListener_InconsistentTopic;
    to.on_offered_deadline_missed    = CallBackWrapper_DomainParticipantListener_OfferedDeadlineMissed;
    to.on_offered_incompatible_qos   = CallBackWrapper_DomainParticipantListener_OfferedIncompatibleQos;
    to.on_liveliness_lost            = CallBackWrapper_DomainParticipantListener_LivelinessLost;
    to.on_publication_match          = CallBackWrapper_DomainParticipantListener_PublicationMatched;
    to.on_requested_deadline_missed  = CallBackWrapper_DomainParticipantListener_RequestedDeadlineMissed;
    to.on_requested_incompatible_qos = CallBackWrapper_DomainParticipantListener_RequestedIncompatibleQos;
    to.on_sample_rejected            = CallBackWrapper_DomainParticipantListener_SampleRejected;
    to.on_liveliness_changed         = CallBackWrapper_DomainParticipantListener_LivelinessChanged;
    to.on_data_available             = CallBackWrapper_DomainParticipantListener_DataAvailable;
    to.on_subscription_match         = CallBackWrapper_DomainParticipantListener_SubscriptionMatched;
    to.on_sample_lost                = CallBackWrapper_DomainParticipantListener_SampleLost;
    to.on_data_on_readers            = CallBackWrapper_DomainParticipantListener_DataOnReaders;
    to.on_all_data_disposed          = CallBackWrapper_DomainParticipantListener_AllDataDisposed;
}



