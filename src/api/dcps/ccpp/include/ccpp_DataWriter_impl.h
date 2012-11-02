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
#ifndef CCPP_DATAWRITER_H
#define CCPP_DATAWRITER_H

#include "ccpp.h"
#include "ccpp_QosUtils.h"
#include "ccpp_Publisher_impl.h"
#include "ccpp_Utils.h"
#include "os_mutex.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
  class OS_DCPS_API DataWriter_impl
    : public virtual ::DDS::DataWriter,
      public ::DDS::Entity_impl
  {
    friend class ::DDS::Publisher_impl;

    protected:
      os_mutex dw_mutex;
      DataWriter_impl(const gapi_dataWriter handle);
     ~DataWriter_impl();

        ::DDS::InstanceHandle_t lookup_instance (
            const void * instance_data) THROW_ORB_EXCEPTIONS;

        ::DDS::InstanceHandle_t register_instance(
            const void * instance_data) THROW_ORB_EXCEPTIONS;

        ::DDS::InstanceHandle_t register_instance_w_timestamp(
            const void * instance_data,
            const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t unregister_instance(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t unregister_instance_w_timestamp(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle,
            const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t write(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t write_w_timestamp(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle,
            const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t dispose(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t dispose_w_timestamp(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle,
            const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t writedispose(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t writedispose_w_timestamp(
            const void * instance_data,
            ::DDS::InstanceHandle_t handle,
            const ::DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS;

        ::DDS::ReturnCode_t get_key_value(
            void * key_holder,
            ::DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS;

    public:

      virtual ::DDS::ReturnCode_t set_qos (
        const ::DDS::DataWriterQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_qos (
        ::DDS::DataWriterQos & qos
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t set_listener(
        ::DDS::DataWriterListener_ptr a_listener,
        ::DDS::StatusMask mask) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::DataWriterListener_ptr get_listener() THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Topic_ptr get_topic() THROW_ORB_EXCEPTIONS;

      virtual ::DDS::Publisher_ptr get_publisher (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t wait_for_acknowledgments (
        const ::DDS::Duration_t & max_wait
      ) THROW_ORB_EXCEPTIONS;

       virtual ::DDS::ReturnCode_t get_liveliness_lost_status (
         ::DDS::LivelinessLostStatus & a_status
       ) THROW_ORB_EXCEPTIONS;

       virtual ::DDS::ReturnCode_t get_offered_deadline_missed_status (
         ::DDS::OfferedDeadlineMissedStatus & a_status
       ) THROW_ORB_EXCEPTIONS;

       virtual ::DDS::ReturnCode_t get_offered_incompatible_qos_status (
         ::DDS::OfferedIncompatibleQosStatus & a_status
       ) THROW_ORB_EXCEPTIONS;

       virtual ::DDS::ReturnCode_t get_publication_matched_status (
         ::DDS::PublicationMatchedStatus & a_status
       ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t assert_liveliness (
      ) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_matched_subscriptions(
        ::DDS::InstanceHandleSeq & subscription_handles) THROW_ORB_EXCEPTIONS;

      virtual ::DDS::ReturnCode_t get_matched_subscription_data(
        ::DDS::SubscriptionBuiltinTopicData & subscription_data,
        ::DDS::InstanceHandle_t subscription_handle
      ) THROW_ORB_EXCEPTIONS;
  };

  typedef DataWriter_impl* DataWriter_impl_ptr;
}

#endif /* DATAWRITER */
