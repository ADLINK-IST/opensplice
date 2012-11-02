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
#include "gapi.h"
#include "ccpp_DataWriter_impl.h" 
#include "ccpp_Utils.h"
#include "ccpp_ListenerUtils.h"
#include "ccpp_QosUtils.h"
#include "os_report.h"

DDS::DataWriter_impl::DataWriter_impl(const gapi_dataWriter handle) : DDS::Entity_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&dw_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::DataWriter_impl::~DataWriter_impl()
{
  if (os_mutexDestroy(&dw_mutex) != os_resultSuccess) {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::ReturnCode_t DDS::DataWriter_impl::set_qos (
  const DDS::DataWriterQos & qos) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

  if (&qos == DDS::DefaultQos::DataWriterQosDefault) {
    result = gapi_dataWriter_set_qos(_gapi_self, GAPI_DATAWRITER_QOS_DEFAULT);
  }
  else if (&qos == DDS::DefaultQos::DataWriterQosUseTopicQos) {
    result = gapi_dataWriter_set_qos(_gapi_self, GAPI_DATAWRITER_QOS_USE_TOPIC_QOS);
  } else {
    gapi_dataWriterQos * gapi_dwqos = gapi_dataWriterQos__alloc();
    if (gapi_dwqos) {
      ccpp_DataWriterQos_copyIn(qos, *gapi_dwqos);
      result = gapi_dataWriter_set_qos(_gapi_self, gapi_dwqos);
      gapi_free(gapi_dwqos);
    } else {
     OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
     result = DDS::RETCODE_OUT_OF_RESOURCES;
   }
  }
  return result;
}

DDS::ReturnCode_t DDS::DataWriter_impl::get_qos (
  DDS::DataWriterQos & qos) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataWriterQos * gapi_dwqos = gapi_dataWriterQos__alloc();
  if (gapi_dwqos) {
    result = gapi_dataWriter_get_qos(_gapi_self, gapi_dwqos);
    ccpp_DataWriterQos_copyOut(*gapi_dwqos, qos);
    gapi_free(gapi_dwqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t DDS::DataWriter_impl::set_listener(
  DDS::DataWriterListener_ptr a_listener,
  DDS::StatusMask mask) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataWriterListener gapi_listener;

  ccpp_DataWriterListener_copyIn(a_listener, gapi_listener);
  if (os_mutexLock(&dw_mutex) == os_resultSuccess) {
    result = gapi_dataWriter_set_listener(_gapi_self, &gapi_listener, mask);
    if (result == DDS::RETCODE_OK) {
      DDS::ccpp_UserData_ptr myUD;
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
      if (myUD) {
        myUD->setListener(a_listener);
      } else {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
    }
    if (os_mutexUnlock(&dw_mutex) != os_resultSuccess) {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
    }
  } else {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
  }
  return result;
}

DDS::DataWriterListener_ptr DDS::DataWriter_impl::get_listener() THROW_ORB_EXCEPTIONS
{
  DDS::DataWriterListener_ptr result;
  gapi_dataWriterListener gapi_listener;

  if (os_mutexLock(&dw_mutex) == os_resultSuccess) {
    gapi_listener = gapi_dataWriter_get_listener(_gapi_self);
    result = reinterpret_cast<DDS::DataWriterListener_ptr>(gapi_listener.listener_data);
    if (result) {
      DDS::DataWriterListener::_duplicate(result);
    }
    if (os_mutexUnlock(&dw_mutex) != os_resultSuccess) {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
    }
  } else {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
  }
  return result;
}

DDS::Topic_ptr DDS::DataWriter_impl::get_topic() THROW_ORB_EXCEPTIONS
{
  gapi_topic handle = NULL;
  DDS::Topic_ptr topic = NULL;

  handle = gapi_dataWriter_get_topic(_gapi_self);
  if (handle){
    ccpp_UserData_ptr drUD = NULL;
    if (os_mutexLock(&dw_mutex) == os_resultSuccess) {
      drUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (drUD) {
        topic = dynamic_cast<DDS::Topic_ptr>(drUD->ccpp_object);
        if (topic) {
          DDS::Topic::_duplicate(topic);
        } else {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Topic");
        }
      } else {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
      if (os_mutexUnlock(&dw_mutex) != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    } else {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return topic;
}

DDS::Publisher_ptr DDS::DataWriter_impl::get_publisher() THROW_ORB_EXCEPTIONS
{
  gapi_publisher handle = NULL;
  DDS::Publisher_ptr publisher = NULL;

  handle = gapi_dataWriter_get_publisher(_gapi_self);
  if (handle) {
    ccpp_UserData_ptr drUD = NULL;
    if (os_mutexLock(&dw_mutex) == os_resultSuccess) {
      drUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (drUD) {
        publisher = dynamic_cast<DDS::Publisher_ptr>(drUD->ccpp_object);
        if (publisher) {
          DDS::Publisher::_duplicate(publisher);
        } else {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Publisher");
        }
      } else {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
      if (os_mutexUnlock(&dw_mutex) != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    } else {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return publisher;
}

DDS::ReturnCode_t DDS::DataWriter_impl::wait_for_acknowledgments (
  const DDS::Duration_t & max_wait
) THROW_ORB_EXCEPTIONS
{
    gapi_duration_t gapi_max_wait;
    
    ccpp_Duration_copyIn(max_wait, gapi_max_wait);
    return gapi_dataWriter_wait_for_acknowledgments(_gapi_self, &gapi_max_wait);
}

DDS::ReturnCode_t DDS::DataWriter_impl::get_liveliness_lost_status (
    DDS::LivelinessLostStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_livelinessLostStatus gapi_status;
  DDS::ReturnCode_t result;
  
  result = gapi_dataWriter_get_liveliness_lost_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_LivelinessLostStatus_copyOut(gapi_status, a_status);
  }
  
  return result;
}

DDS::ReturnCode_t DDS::DataWriter_impl::get_offered_deadline_missed_status (
    DDS::OfferedDeadlineMissedStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_offeredDeadlineMissedStatus gapi_status;
  DDS::ReturnCode_t result;
  
  result = gapi_dataWriter_get_offered_deadline_missed_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_OfferedDeadlineMissedStatus_copyOut(gapi_status, a_status);
  }
  
  return result;
}


DDS::ReturnCode_t DDS::DataWriter_impl::get_offered_incompatible_qos_status (
    DDS::OfferedIncompatibleQosStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_offeredIncompatibleQosStatus* gapi_status;
  DDS::ReturnCode_t result;
  
  gapi_status = gapi_offeredIncompatibleQosStatus_alloc();
  
  result = gapi_dataWriter_get_offered_incompatible_qos_status( DDS::Entity_impl::_gapi_self,gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_OfferedIncompatibleQosStatus_copyOut(*gapi_status, a_status);
  }
  gapi_free(gapi_status);
  
  return result;
}

DDS::ReturnCode_t DDS::DataWriter_impl::get_publication_matched_status (
    DDS::PublicationMatchedStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_publicationMatchedStatus gapi_status;
  DDS::ReturnCode_t result;
  
  result = gapi_dataWriter_get_publication_matched_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_PublicationMatchedStatus_copyOut(gapi_status, a_status);
  }
  
  return result;
}

DDS::ReturnCode_t DDS::DataWriter_impl::assert_liveliness (
) THROW_ORB_EXCEPTIONS
{
  return gapi_dataWriter_assert_liveliness(_gapi_self);
}

DDS::ReturnCode_t DDS::DataWriter_impl::get_matched_subscriptions(
  DDS::InstanceHandleSeq & subscription_handles) THROW_ORB_EXCEPTIONS
{
  gapi_instanceHandleSeq gapi_seq;
  DDS::ReturnCode_t result;

  ccpp_sequenceInitialize<gapi_instanceHandleSeq>(gapi_seq);
  result = gapi_dataWriter_get_matched_subscriptions(_gapi_self, &gapi_seq);
  if (result == DDS::RETCODE_OK) {
    ccpp_sequenceCopyOut< gapi_instanceHandleSeq, gapi_instanceHandle_t, DDS::InstanceHandleSeq, 
                            DDS::InstanceHandle_t> (gapi_seq, subscription_handles);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataWriter_impl::get_matched_subscription_data (
  DDS::SubscriptionBuiltinTopicData & subscription_data,
  DDS::InstanceHandle_t subscription_handle) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;
  gapi_subscriptionBuiltinTopicData * gapi_data = gapi_subscriptionBuiltinTopicData__alloc();

  if (gapi_data) {
    result = gapi_dataWriter_get_matched_subscription_data(_gapi_self, gapi_data, subscription_handle);
    if (result == DDS::RETCODE_OK) {
      ccpp_SubscriptionBuiltinTopicData_copyOut(*gapi_data, subscription_data);
    }
    gapi_free(gapi_data);
  } else {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
  return result;
}

DDS::InstanceHandle_t DDS::DataWriter_impl::register_instance(
    const void * instance_data) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataWriter_register_instance(_gapi_self, 
            static_cast<const gapi_foo*>(instance_data));
}
            
DDS::InstanceHandle_t DDS::DataWriter_impl::register_instance_w_timestamp(
    const void * instance_data,
    const DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS
{
  gapi_time_t gapi_time_stamp;
  ccpp_TimeStamp_copyIn(source_timestamp, gapi_time_stamp);
  return gapi_fooDataWriter_register_instance_w_timestamp( 
        _gapi_self, 
        static_cast<const gapi_foo*>(instance_data), 
        &gapi_time_stamp);
}
            
DDS::ReturnCode_t DDS::DataWriter_impl::unregister_instance(
    const void * instance_data,
    DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataWriter_unregister_instance(_gapi_self, 
            static_cast<const gapi_foo*>(instance_data), 
            handle);
}
    
DDS::ReturnCode_t DDS::DataWriter_impl::unregister_instance_w_timestamp (
    const void * instance_data,
    DDS::InstanceHandle_t handle,
    const DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS
{
  gapi_time_t gapi_timestamp;
  ccpp_TimeStamp_copyIn(source_timestamp, gapi_timestamp);
  return gapi_fooDataWriter_unregister_instance_w_timestamp(
            _gapi_self, 
            static_cast<const gapi_foo*>(instance_data), 
            handle, 
            &gapi_timestamp);  
}
    
DDS::ReturnCode_t DDS::DataWriter_impl::write(
    const void * instance_data,
    DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataWriter_write(_gapi_self, 
        static_cast<const gapi_foo*>(instance_data), 
        handle);
}
    
DDS::ReturnCode_t DDS::DataWriter_impl::write_w_timestamp(
    const void * instance_data,
    DDS::InstanceHandle_t handle,
    const DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS
{
  gapi_time_t gapi_timestamp;
  ccpp_TimeStamp_copyIn(source_timestamp, gapi_timestamp);
  return gapi_fooDataWriter_write_w_timestamp(
            _gapi_self, 
            static_cast<const gapi_foo*>(instance_data), 
            handle, 
            &gapi_timestamp);
}
    
DDS::ReturnCode_t DDS::DataWriter_impl::dispose(
    const void * instance_data,
    DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataWriter_dispose(_gapi_self, static_cast<const gapi_foo*>(instance_data), handle);
}
    
DDS::ReturnCode_t DDS::DataWriter_impl::dispose_w_timestamp(
    const void * instance_data,
    DDS::InstanceHandle_t handle,
    const DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS
{
  gapi_time_t gapi_timestamp;
  ccpp_TimeStamp_copyIn(source_timestamp, gapi_timestamp);
  return gapi_fooDataWriter_dispose_w_timestamp(
        _gapi_self, 
        static_cast<const gapi_foo*>(instance_data), 
        handle, 
        &gapi_timestamp);
}

DDS::ReturnCode_t DDS::DataWriter_impl::writedispose(
    const void * instance_data,
    DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataWriter_writedispose(_gapi_self, 
        static_cast<const gapi_foo*>(instance_data), 
        handle);
}
    
DDS::ReturnCode_t DDS::DataWriter_impl::writedispose_w_timestamp(
    const void * instance_data,
    DDS::InstanceHandle_t handle,
    const DDS::Time_t & source_timestamp) THROW_ORB_EXCEPTIONS
{
  gapi_time_t gapi_timestamp;
  ccpp_TimeStamp_copyIn(source_timestamp, gapi_timestamp);
  return gapi_fooDataWriter_writedispose_w_timestamp(
            _gapi_self, 
            static_cast<const gapi_foo*>(instance_data), 
            handle, 
            &gapi_timestamp);
}
            
DDS::ReturnCode_t DDS::DataWriter_impl::get_key_value(
    void * key_holder,
    DDS::InstanceHandle_t handle) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataWriter_get_key_value(_gapi_self, static_cast<gapi_foo*>(key_holder), handle);
}

DDS::InstanceHandle_t DDS::DataWriter_impl::lookup_instance (
    const void * instance_data) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataWriter_lookup_instance(_gapi_self, 
                static_cast<const gapi_foo*>(instance_data));
}


