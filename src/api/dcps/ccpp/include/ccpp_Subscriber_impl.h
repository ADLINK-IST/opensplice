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
#ifndef CCPP_SUBSCRIBER_H
#define CCPP_SUBSCRIBER_H

#include "ccpp.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_Utils.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API DomainParticipant_impl;

  class OS_DCPS_API Subscriber_impl
    : public virtual ::DDS::Subscriber,
      public ::DDS::Entity_impl
    {
    friend class ::DDS::DomainParticipant_impl;

    private:
        os_mutex s_mutex;
        Subscriber_impl(gapi_subscriber handle);
       ~Subscriber_impl();

    public:
      virtual ::DDS::DataReader_ptr create_datareader (
        ::DDS::TopicDescription_ptr a_topic,
        const ::DDS::DataReaderQos & qos,
        ::DDS::DataReaderListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_datareader (
        ::DDS::DataReader_ptr a_datareader
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_contained_entities (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DataReader_ptr lookup_datareader (
        const char * topic_name
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_datareaders (
        ::DDS::DataReaderSeq & readers,
        ::DDS::SampleStateMask sample_states,
        ::DDS::ViewStateMask view_states,
        ::DDS::InstanceStateMask instance_states
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t notify_datareaders (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_qos (
        const ::DDS::SubscriberQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_qos (
        ::DDS::SubscriberQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_listener (
        ::DDS::SubscriberListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::SubscriberListener_ptr get_listener (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t begin_access (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t end_access (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DomainParticipant_ptr get_participant (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_default_datareader_qos (
        const ::DDS::DataReaderQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_default_datareader_qos (
        ::DDS::DataReaderQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t copy_from_topic_qos (
        ::DDS::DataReaderQos & a_datareader_qos,
        const ::DDS::TopicQos & a_topic_qos
      ) THROW_ORB_EXCEPTIONS;
  };
  typedef Subscriber_impl * Subscriber_impl_ptr;
}

#endif /* SUBSCRIBER */
