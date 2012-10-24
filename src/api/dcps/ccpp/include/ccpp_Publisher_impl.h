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
#ifndef CCPP_PUBLISHER_H
#define CCPP_PUBLISHER_H

#include "ccpp.h"
#include "ccpp_Entity_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API DomainParticipant_impl;

  class OS_DCPS_API Publisher_impl
    : public virtual ::DDS::Publisher,
      public ::DDS::Entity_impl
  {
    friend class ::DDS::DomainParticipant_impl;

    private:
        os_mutex p_mutex;
        Publisher_impl(gapi_publisher a_handle);
       ~Publisher_impl();

    public:
      virtual ::DDS::DataWriter_ptr create_datawriter (
        ::DDS::Topic_ptr a_topic,
        const ::DDS::DataWriterQos & qos,
        ::DDS::DataWriterListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_datawriter (
        ::DDS::DataWriter_ptr a_datawriter
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DataWriter_ptr lookup_datawriter (
        const char * topic_name
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t delete_contained_entities (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_qos (
        const ::DDS::PublisherQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_qos (
        ::DDS::PublisherQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_listener (
        ::DDS::PublisherListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::PublisherListener_ptr get_listener (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t suspend_publications (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t resume_publications (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t begin_coherent_changes (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t end_coherent_changes (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t wait_for_acknowledgments (
        const ::DDS::Duration_t & max_wait
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DomainParticipant_ptr get_participant (

      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_default_datawriter_qos (
        const ::DDS::DataWriterQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_default_datawriter_qos (
        ::DDS::DataWriterQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t copy_from_topic_qos (
        ::DDS::DataWriterQos & a_datawriter_qos,
        const ::DDS::TopicQos & a_topic_qos
      ) THROW_ORB_EXCEPTIONS;
  };
  typedef Publisher_impl *Publisher_impl_ptr;
}

#endif /* PUBLISHER */
