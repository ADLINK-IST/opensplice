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
#include "DDSEntityManager.h"

DDSEntityManager::DDSEntityManager(std::string durability_kind)
{
  m_durability_kind = durability_kind;
}

DDSEntityManager::DDSEntityManager(std::string durability_kind, bool
  autodispose_unregistered_instances)
{
  m_durability_kind = durability_kind;
  m_autodispose_unregistered_instances = autodispose_unregistered_instances;
}


void DDSEntityManager::createParticipant(const char *partitiontName)
{
  domain = NULL;
  dpf = DomainParticipantFactory::get_instance();
  checkHandle(dpf.in(), "DDS::DomainParticipantFactory::get_instance");
  participant = dpf->create_participant(domain, PARTICIPANT_QOS_DEFAULT, NULL,
    STATUS_MASK_NONE);
  checkHandle(participant.in(), 
    "DDS::DomainParticipantFactory::create_participant");
  partition = partitiontName;
}

void DDSEntityManager::deleteParticipant()
{
  dpf->delete_participant(participant.in());
}

void DDSEntityManager::registerType(TypeSupport *ts)
{
  typeName = ts->get_type_name();
  status = ts->register_type(participant.in(), typeName);
  checkStatus(status, "register_type");
}

void DDSEntityManager::createTopic(char *topicName)
{
  status = participant->get_default_topic_qos(topic_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_topic_qos");
  topic_qos.reliability.kind = RELIABLE_RELIABILITY_QOS;
  if (m_durability_kind.compare("transient") == 0)
    topic_qos.durability.kind = TRANSIENT_DURABILITY_QOS;
  else if (m_durability_kind.compare("persistent") == 0)
    topic_qos.durability.kind = PERSISTENT_DURABILITY_QOS;

  /* Make the tailored QoS the new default. */
  status = participant->set_default_topic_qos(topic_qos);
  checkStatus(status, "DDS::DomainParticipant::set_default_topic_qos");

  /* Use the changed policy when defining the topic */
  topic = participant->create_topic(topicName, typeName, topic_qos, NULL,
    STATUS_MASK_NONE);
  checkHandle(topic.in(), "DDS::DomainParticipant::create_topic ()");
}

void DDSEntityManager::deleteTopic()
{
  status = participant->delete_topic(topic);
  checkStatus(status, "DDS.DomainParticipant.delete_topic");
}

void DDSEntityManager::createPublisher()
{
  status = participant->get_default_publisher_qos(pub_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_publisher_qos");
  pub_qos.partition.name.length(1);
  pub_qos.partition.name[0] = partition;

  publisher = participant->create_publisher(pub_qos, NULL, STATUS_MASK_NONE);
  checkHandle(publisher.in(), "DDS::DomainParticipant::create_publisher");
}

void DDSEntityManager::deletePublisher()
{
  participant->delete_publisher(publisher);
}

void DDSEntityManager::createWriter()
{
  status = publisher->get_default_datawriter_qos(dw_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_publisher_qos");
  status = publisher->copy_from_topic_qos(dw_qos, topic_qos);
  checkStatus(status, "DDS::Publisher::copy_from_topic_qos");
  // Set autodispose to false otherwise the instances of the 
  // topic will be suppressed from the persistence file when 
  // the writer is stopped
  dw_qos.writer_data_lifecycle.autodispose_unregistered_instances =
    m_autodispose_unregistered_instances;
  writer = publisher->create_datawriter(topic.in(), dw_qos, NULL,
    STATUS_MASK_NONE);
  checkHandle(writer, "DDS::Publisher::create_datawriter");
}

void DDSEntityManager::deleteWriter(DDS::DataWriter_var dataWriter)
{
   status = publisher->delete_datawriter(dataWriter);
   checkStatus(status, "DDS::Publisher::delete_datawriter ");
}

void DDSEntityManager::createSubscriber()
{
  int status = participant->get_default_subscriber_qos(sub_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_subscriber_qos");
  sub_qos.partition.name.length(1);
  sub_qos.partition.name[0] = partition;
  subscriber = participant->create_subscriber(sub_qos, NULL, STATUS_MASK_NONE);
  checkHandle(subscriber.in(), "DDS::DomainParticipant::create_subscriber");
}

void DDSEntityManager::deleteSubscriber()
{
  participant->delete_subscriber(subscriber);
}

void DDSEntityManager::createReader()
{
  status = subscriber->get_default_datareader_qos(dr_qos);
  checkStatus(status, "DDS::Subscriber::get_default_datareader_qos");
  status = subscriber->copy_from_topic_qos(dr_qos, topic_qos);
  checkStatus(status, "DDS::Subscriber::copy_from_topic_qos");
  reader = subscriber->create_datareader(topic.in(), dr_qos, NULL,
    STATUS_MASK_NONE);
  checkHandle(reader, "DDS::Subscriber::create_datareader ()");
  // If the logic of your application requires it
  // wait (block) until all historical data are received or
  // until the timeout has elapsed
  DDS::Duration_t a_timeout;
  a_timeout.sec = 2;
  a_timeout.nanosec = 10000000;
  status = reader->wait_for_historical_data(a_timeout);
}

void DDSEntityManager::deleteReader(DDS::DataReader_var dataReader)
{
   status = subscriber->delete_datareader(dataReader);
   checkStatus(status, "DDS::Subscriber::delete_datareader ");
}

DataReader_ptr DDSEntityManager::getReader()
{
  return reader;
}

DataWriter_ptr DDSEntityManager::getWriter()
{
  return writer;
}

Publisher_ptr DDSEntityManager::getPublisher()
{
  return publisher._retn();
}

Subscriber_ptr DDSEntityManager::getSubscriber()
{
  return subscriber._retn();
}

Topic_ptr DDSEntityManager::getTopic()
{
  return topic._retn();
}

DomainParticipant_ptr DDSEntityManager::getParticipant()
{
  return participant._retn();
}

DDSEntityManager::~DDSEntityManager(){

}
