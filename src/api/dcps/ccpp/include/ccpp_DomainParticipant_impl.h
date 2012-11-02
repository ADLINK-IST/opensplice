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
#ifndef CCPP_DOMAINPARTICIPANT_H
#define CCPP_DOMAINPARTICIPANT_H

#include "ccpp.h"
#include "dds_builtinTopicsDcps_impl.h"
#include "dds_dcps_builtintopicsDcps_impl.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_DomainParticipantFactory.h"
#include "ccpp_TypeSupport_impl.h"
#include "os_mutex.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define PARTICIPANT_BUILTINTOPIC_NAME "DCPSParticipant"
#define TOPIC_BUILTINTOPIC_NAME "DCPSTopic"
#define PUBLICATION_BUILTINTOPIC_NAME "DCPSPublication"
#define SUBSCRIPTION_BUILTINTOPIC_NAME "DCPSSubscription"


namespace DDS
{
  class OS_DCPS_API DomainParticipant_impl
    : public virtual ::DDS::DomainParticipant,
      public ::DDS::Entity_impl
  {
    friend class ::DDS::DomainParticipantFactory;
    friend class ::DDS::TypeSupport_impl;

    private:
        os_mutex dp_mutex;
        DomainParticipant_impl(gapi_domainParticipant handle);
       ~DomainParticipant_impl();

    public:
      virtual ::DDS::Publisher_ptr create_publisher (
        const ::DDS::PublisherQos & qos,
        ::DDS::PublisherListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_publisher (
        ::DDS::Publisher_ptr p
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Subscriber_ptr create_subscriber (
        const ::DDS::SubscriberQos & qos,
        ::DDS::SubscriberListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_subscriber (
        ::DDS::Subscriber_ptr s
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Subscriber_ptr get_builtin_subscriber (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Topic_ptr create_topic (
        const char * topic_name,
        const char * type_name,
        const ::DDS::TopicQos & qos,
        ::DDS::TopicListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_topic (
        ::DDS::Topic_ptr a_topic
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Topic_ptr find_topic (
        const char * topic_name,
        const ::DDS::Duration_t & timeout
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::TopicDescription_ptr lookup_topicdescription (
        const char * name
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ContentFilteredTopic_ptr create_contentfilteredtopic (
        const char * name,
        ::DDS::Topic_ptr related_topic,
        const char * filter_expression,
        const ::DDS::StringSeq & filter_parameters
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_contentfilteredtopic (
        ::DDS::ContentFilteredTopic_ptr a_contentfilteredtopic
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::MultiTopic_ptr create_multitopic (
        const char * name,
        const char * type_name,
        const char * subscription_expression,
        const ::DDS::StringSeq & expression_parameters
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_multitopic (
        ::DDS::MultiTopic_ptr a_multitopic
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_contained_entities (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_qos (
        const ::DDS::DomainParticipantQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_qos (
        ::DDS::DomainParticipantQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_listener (
        ::DDS::DomainParticipantListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DomainParticipantListener_ptr get_listener (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t ignore_participant (
        ::DDS::InstanceHandle_t handle
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t ignore_topic (
        ::DDS::InstanceHandle_t handle
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t ignore_publication (
        ::DDS::InstanceHandle_t handle
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t ignore_subscription (
        ::DDS::InstanceHandle_t handle
      ) THROW_ORB_EXCEPTIONS;

      virtual char * get_domain_id (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t assert_liveliness (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_default_publisher_qos (
        const ::DDS::PublisherQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_default_publisher_qos (
        ::DDS::PublisherQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_default_subscriber_qos (
        const ::DDS::SubscriberQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_default_subscriber_qos (
        ::DDS::SubscriberQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_default_topic_qos (
        const ::DDS::TopicQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_default_topic_qos (
        ::DDS::TopicQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_discovered_participants (
        ::DDS::InstanceHandleSeq & participant_handles
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_discovered_participant_data (
        ::DDS::ParticipantBuiltinTopicData & participant_data,
        ::DDS::InstanceHandle_t participant_handle
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_discovered_topics (
         ::DDS::InstanceHandleSeq & topic_handles
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_discovered_topic_data (
        ::DDS::TopicBuiltinTopicData & topic_data,
        ::DDS::InstanceHandle_t topic_handle
      ) THROW_ORB_EXCEPTIONS;

      virtual CORBA::Boolean contains_entity (
        ::DDS::InstanceHandle_t a_handle
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_current_time (
        ::DDS::Time_t & current_time
      ) THROW_ORB_EXCEPTIONS;

    private:
      ::DDS::TopicDescription_ptr unprotected_lookup_topicdescription (
        const char * name
      );

      ::DDS::ReturnCode_t initializeBuiltinTopics();

      bool initializeBuiltinTopicEntities(
        gapi_subscriber handle
      );

      bool initializeBuiltinReaders(
        gapi_subscriber handle
      );

      bool createBuiltinReader(
        gapi_subscriber subscriber_handle,
        const char *name
      );
  };
  typedef ::DDS::DomainParticipant_impl *DomainParticipant_impl_ptr;
}

#endif /* DOMAINPARTICIPANT */
