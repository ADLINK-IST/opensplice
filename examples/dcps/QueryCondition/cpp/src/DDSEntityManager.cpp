
#include "DDSEntityManager.h"


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
  status = participant->get_default_topic_qos(reliable_topic_qos);
  checkStatus(status, "DDS::DomainParticipant::get_default_topic_qos");
  reliable_topic_qos.reliability.kind = RELIABLE_RELIABILITY_QOS;
  reliable_topic_qos.durability.kind = TRANSIENT_DURABILITY_QOS;
  reliable_topic_qos.destination_order.kind = BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;

  /* Make the tailored QoS the new default. */
  status = participant->set_default_topic_qos(reliable_topic_qos);
  checkStatus(status, "DDS::DomainParticipant::set_default_topic_qos");

  topic = participant->create_topic(topicName, typeName, reliable_topic_qos,
    NULL, STATUS_MASK_NONE);
  checkHandle(topic.in(), "DDS::DomainParticipant::create_topic ()");
}

void DDSEntityManager::createContentFilteredTopic(const char *topicName, const
  char *filter, const StringSeq &expr)
{
  filteredTopic = participant->create_contentfilteredtopic(topicName, topic,
    filter, expr);
  checkHandle(filteredTopic.in(),
    "DomainParticipant::create_contentfilteredtopic");
}

void DDSEntityManager::deleteFilteredTopic()
{
  status = participant->delete_contentfilteredtopic(filteredTopic);
  checkStatus(status, "DDS.DomainParticipant.delete_topic");
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

void DDSEntityManager::deleteWriter(DDS::DataWriter_ptr dataWriter)
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

void DDSEntityManager::createReader(bool filtered)
{
    if (!filtered)
    {
        reader = subscriber->create_datareader(topic.in(),
          DATAREADER_QOS_USE_TOPIC_QOS, NULL, STATUS_MASK_NONE);
        checkHandle(reader, "DDS::Subscriber::create_datareader ()");
    }
    else
    {
        reader = subscriber->create_datareader(filteredTopic.in(),
          DATAREADER_QOS_USE_TOPIC_QOS, NULL, STATUS_MASK_NONE);
        checkHandle(reader, "DDS::Subscriber::create_datareader ()");
    }
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
