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
#ifndef CCPP_TOPIC_H
#define CCPP_TOPIC_H

#include "ccpp.h"
#include "ccpp_QosUtils.h"
#include "ccpp_Entity_impl.h"
#include "ccpp_TopicDescription_impl.h"
#include "ccpp_Publisher_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API Topic_impl
    : public virtual ::DDS::Topic,
      public ::DDS::TopicDescription_impl
  {
    friend class ::DDS::DomainParticipant_impl;
    friend class ::DDS::Publisher_impl;

    private:
        os_mutex t_mutex;
        Topic_impl(gapi_topic handle);
        ~Topic_impl();
    public:
      virtual ::DDS::ReturnCode_t get_inconsistent_topic_status (
        ::DDS::InconsistentTopicStatus & a_status
      ) THROW_ORB_EXCEPTIONS;

      virtual DDS::ReturnCode_t get_all_data_disposed_topic_status (
        ::DDS::AllDataDisposedTopicStatus & a_status
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_qos (
        ::DDS::TopicQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_qos (
        const ::DDS::TopicQos & qos
      ) THROW_ORB_EXCEPTIONS;

     virtual ::DDS::TopicListener_ptr get_listener (
      ) THROW_ORB_EXCEPTIONS;

    virtual ::DDS::ReturnCode_t set_listener (
        ::DDS::TopicListener_ptr a_listener,
        ::DDS::StatusMask mask
      ) THROW_ORB_EXCEPTIONS;

    virtual ::DDS::ReturnCode_t dispose_all_data (
      ) THROW_ORB_EXCEPTIONS;

  };
  typedef Topic_impl * Topic_impl_ptr;
}

#endif /* TOPIC */
