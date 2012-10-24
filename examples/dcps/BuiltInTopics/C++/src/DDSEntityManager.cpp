
#include "DDSEntityManager.h"


void DDSEntityManager::createParticipant()
{
  domain = NULL;
  dpf = DomainParticipantFactory::get_instance();
  checkHandle(dpf.in(), "DDS::DomainParticipantFactory::get_instance");
  participant = dpf->create_participant(domain, PARTICIPANT_QOS_DEFAULT, NULL,
    STATUS_MASK_NONE);
  checkHandle(participant.in(),
    "DDS::DomainParticipantFactory::create_participant");
}

void DDSEntityManager::deleteContainedEntities()
{
  /* Free all resources */
  status = dpf->delete_contained_entities();
  checkStatus(status, "DDS::DomainParticipant::delete_contained_entities");
}

void DDSEntityManager::deleteParticipant()
{
    status = dpf->delete_participant(participant);
	checkStatus(status, "DDS::DomainParticipantFactory::delete_participant ");
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
  reliable_topic_qos.history.kind = KEEP_LAST_HISTORY_QOS;
  reliable_topic_qos.history.depth = 2;

  /* Make the tailored QoS the new default. */
  status = participant->set_default_topic_qos(reliable_topic_qos);
  checkStatus(status, "DDS::DomainParticipant::set_default_topic_qos");

  /* Use the changed policy when defining the BuiltInTopics topic */
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

  publisher = participant->create_publisher(pub_qos, NULL, STATUS_MASK_NONE);
  checkHandle(publisher.in(), "DDS::DomainParticipant::create_publisher");
}

void DDSEntityManager::deletePublisher()
{
	status = participant->delete_publisher(publisher);
    checkStatus(status, "DDS::DomainParticipant::delete_publisher ");
}


void DDSEntityManager::createWriter()
{
    DataWriterQos dw_qos;
    status = publisher->get_default_datawriter_qos(dw_qos);
    checkStatus(status, "DDS::DomainParticipant::get_default_publisher_qos");
    status = publisher->copy_from_topic_qos(dw_qos, reliable_topic_qos);
    checkStatus(status, "DDS::Publisher::copy_from_topic_qos");
    dw_qos.writer_data_lifecycle.autodispose_unregistered_instances = false;
    writer = publisher->create_datawriter(topic.in(),
          dw_qos, NULL, STATUS_MASK_NONE);
    checkHandle(writer, "DDS::Publisher::create_datawriter (chatMessage)");
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
  subscriber = participant->create_subscriber(sub_qos, NULL, STATUS_MASK_NONE);
  checkHandle(subscriber.in(), "DDS::DomainParticipant::create_subscriber");
}

void DDSEntityManager::deleteSubscriber()
{
  participant->delete_subscriber(subscriber);
}

void DDSEntityManager::createReader()
{
  reader = subscriber->create_datareader(topic.in(),
    DATAREADER_QOS_USE_TOPIC_QOS, NULL, STATUS_MASK_NONE);
  checkHandle(reader, "DDS::Subscriber::create_datareader ()");
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
