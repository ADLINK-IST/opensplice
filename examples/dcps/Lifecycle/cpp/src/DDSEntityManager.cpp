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
#include "DDSEntityManager.h"


DDSEntityManager::DDSEntityManager()
{
  m_autodispose_unregistered_instances = false;
}

DDSEntityManager::DDSEntityManager(bool autodispose_unregistered_instances)
{
  m_autodispose_unregistered_instances = autodispose_unregistered_instances;
}

void DDSEntityManager::createParticipant(const char *partitiontName)
{
  domain = DOMAIN_ID_DEFAULT;
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
  status = dpf->delete_participant(participant.in());
  checkStatus(status, "DDS::DomainParticipantFactory::delete_participant");
}

void DDSEntityManager::registerType(TypeSupport *ts)
{
  typeName = ts->get_type_name();
  status = ts->register_type(participant.in(), typeName);
  checkStatus(status, "register_type");
}

void DDSEntityManager::createTopic(char *topicName)
{
  status = participant->get_default_topic_qos(reliable_topic_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_topic_qos");
  reliable_topic_qos.reliability.kind = RELIABLE_RELIABILITY_QOS;
  reliable_topic_qos.durability.kind = TRANSIENT_DURABILITY_QOS;
 
  /* Set a the history Policy */
  //reliable_topic_qos.history.kind = KEEP_LAST_HISTORY_QOS;
  //reliable_topic_qos.history.depth = 1;
 
  /* Make the tailored QoS the new default. */
  status = participant->set_default_topic_qos(reliable_topic_qos);
  checkStatus(status, "DDS::DomainParticipant::set_default_topic_qos");

  /* Use the changed policy when defining the Lifecycle topic */
  topic = participant->create_topic(topicName, typeName, reliable_topic_qos,
    NULL, STATUS_MASK_NONE);
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
  status = participant->delete_publisher(publisher);
   // TODO : uncomment checkStatus(status, "DDS::DomainParticipant::delete_publisher");
}


void DDSEntityManager::createWriters()
{
  status = publisher->get_default_datawriter_qos(dw_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_publisher_qos");
  status = publisher->copy_from_topic_qos(dw_qos, reliable_topic_qos);
  checkStatus(status, "DDS::Publisher::copy_from_topic_qos");
  // Set autodispose to false otherwise the instances of the 
  // topic will be suppressed from the persistence file when 
  // the writer is stopped
  dw_qos.writer_data_lifecycle.autodispose_unregistered_instances =
    m_autodispose_unregistered_instances;
  writer = publisher->create_datawriter(topic.in(), dw_qos, NULL,
    STATUS_MASK_NONE);
  writer_stopper = publisher->create_datawriter(topic.in(), dw_qos, NULL,
    STATUS_MASK_NONE);
  checkHandle(writer, "DDS::Publisher::create_datawriter");
}

void DDSEntityManager::deleteWriter(DDS::DataWriter_ptr dataWriter)
{
   status = publisher->delete_datawriter(dataWriter);
   checkStatus(status, "DDS::Publisher::delete_datawriter ");
}

void DDSEntityManager::createSubscriber()
{
  status = participant->get_default_subscriber_qos(sub_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_subscriber_qos");
  sub_qos.partition.name.length(1);
  sub_qos.partition.name[0] = partition;
  subscriber = participant->create_subscriber(sub_qos, NULL, STATUS_MASK_NONE);
  checkHandle(subscriber.in(), "DDS::DomainParticipant::create_subscriber");
}

void DDSEntityManager::deleteSubscriber()
{
  status = participant->delete_subscriber(subscriber);
  checkStatus(status, "DDS::DomainParticipant::delete_subscriber");
}

void DDSEntityManager::createReader()
{
  status = subscriber->get_default_datareader_qos(dr_qos);
  checkStatus(status, "DDS::Subscriber::get_default_datareader_qos");
  status = subscriber->copy_from_topic_qos(dr_qos, reliable_topic_qos);
  checkStatus(status, "DDS::Subscriber::copy_from_topic_qos");
  reader = subscriber->create_datareader(topic.in(), dr_qos, NULL,
    STATUS_MASK_NONE);
  checkHandle(reader, "DDS::Subscriber::create_datareader ()");
}

void DDSEntityManager::deleteReader(DDS::DataReader_ptr dataReader)
{
   status = subscriber->delete_datareader(dataReader);
   checkStatus(status, "DDS::Subscriber::delete_datareader ");
}

DataReader_ptr DDSEntityManager::getReader()
{
  return DataReader::_duplicate(reader.in());
}

DataWriter_ptr DDSEntityManager::getWriter()
{
  return DataWriter::_duplicate(writer.in());
}

DataWriter_ptr DDSEntityManager::getWriter_stopper()
{
  return DataWriter::_duplicate(writer_stopper.in());
}

Publisher_ptr DDSEntityManager::getPublisher()
{
  return Publisher::_duplicate(publisher.in());
}

Subscriber_ptr DDSEntityManager::getSubscriber()
{
  return Subscriber::_duplicate(subscriber.in());
}

Topic_ptr DDSEntityManager::getTopic()
{
  return Topic::_duplicate(topic.in());
}

DomainParticipant_ptr DDSEntityManager::getParticipant()
{
  return DomainParticipant::_duplicate(participant.in());
}

DDSEntityManager::~DDSEntityManager(){

}
