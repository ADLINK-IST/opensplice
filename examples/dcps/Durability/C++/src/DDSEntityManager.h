
#ifndef _DDSENTITYMGR_
  #define _DDSENTITYMGR_


  #include "ccpp_dds_dcps.h"
  #include "CheckStatus.h"
  using namespace DDS;

  class DDSEntityManager
  {
      /* Generic DDS entities */
      DomainParticipantFactory_var dpf;
      DomainParticipant_var participant;
      Topic_var topic;
      Publisher_var publisher;
      Subscriber_var subscriber;
      DataWriter_ptr writer;
      DataReader_ptr reader;

      /* QosPolicy holders */
      TopicQos topic_qos;
      TopicQos setting_topic_qos;
      PublisherQos pub_qos;
      DataWriterQos dw_qos;
      SubscriberQos sub_qos;
      DataReaderQos dr_qos;

      DomainId_t domain;
      InstanceHandle_t userHandle;
      ReturnCode_t status;

      CORBA::String_var partition;
      CORBA::String_var typeName;
      
    public:

      DDSEntityManager(std::string durability_kind);

      DDSEntityManager(std::string durabilty_kind, bool
        autodispose_unregistered_instances);

      void createParticipant(const char *partitiontName);

      void deleteParticipant();

      void registerType(TypeSupport *ts);

      void createTopic(char *topicName);

      void deleteTopic();

      void createPublisher();

      void deletePublisher();

      void createWriter();

      void deleteWriter(DDS::DataWriter_var dataWriter);

      void createSubscriber();

      void deleteSubscriber();

      void createReader();

      void deleteReader(DDS::DataReader_var dataReader);

      DataReader_ptr getReader();

      DataWriter_ptr getWriter();

      Publisher_ptr getPublisher();

      Subscriber_ptr getSubscriber();

      Topic_ptr getTopic();

      DomainParticipant_ptr getParticipant();

      ~DDSEntityManager();

    private:
      std::string m_durability_kind;
      bool m_autodispose_unregistered_instances;

  };

#endif
