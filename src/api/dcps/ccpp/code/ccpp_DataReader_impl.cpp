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
#include "ccpp_DataReader_impl.h"
#include "ccpp_Utils.h"
#include "ccpp_QosUtils.h"
#include "ccpp_ReadCondition_impl.h"
#include "ccpp_QueryCondition_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_TopicDescription_impl.h"
#include "ccpp_ListenerUtils.h"
#include "ccpp_DataReaderView_impl.h"

DDS::DataReader_impl::DataReader_impl(gapi_dataReader handle) : DDS::Entity_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&dr_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::DataReader_impl::~DataReader_impl()
{
  if (os_mutexDestroy(&dr_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::ReadCondition_ptr DDS::DataReader_impl::create_readcondition (
  DDS::SampleStateMask sample_states,
  DDS::ViewStateMask view_states,
  DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
  gapi_readCondition handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::ReadCondition_ptr readCondition = NULL;

  handle = gapi_dataReader_create_readcondition( _gapi_self, sample_states, view_states, instance_states);
  if (handle)
  {
    readCondition = new DDS::ReadCondition_impl(handle);
    if (readCondition)
    {
      myUD = new ccpp_UserData(readCondition);
      if (myUD)
      {
        gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                  DDS::ccpp_CallBack_DeleteUserData, NULL);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      }
      DDS::ReadCondition::_duplicate(readCondition);
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    }
  }
  return readCondition;
}

DDS::QueryCondition_ptr DDS::DataReader_impl::create_querycondition (
  DDS::SampleStateMask sample_states,
  DDS::ViewStateMask view_states,
  DDS::InstanceStateMask instance_states,
  const char * query_expression,
  const DDS::StringSeq & query_parameters
) THROW_ORB_EXCEPTIONS
{
  gapi_stringSeq * gapi_query_parameters;
  gapi_queryCondition handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::QueryCondition_impl_ptr queryCondition = NULL;

  gapi_query_parameters = gapi_stringSeq__alloc();
  if (gapi_query_parameters)
  {
    ccpp_sequenceCopyIn(query_parameters, *gapi_query_parameters);
    handle = gapi_dataReader_create_querycondition(
      _gapi_self,
      sample_states,
      view_states,
      instance_states,
      query_expression,
      gapi_query_parameters);
    gapi_free(gapi_query_parameters);

    if (handle)
    {
      queryCondition = new DDS::QueryCondition_impl(handle);
      if (queryCondition)
      {
        myUD = new DDS::ccpp_UserData(queryCondition);
        if (myUD)
        {
	       gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                         DDS::ccpp_CallBack_DeleteUserData, NULL);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
        }
        DDS::QueryCondition::_duplicate(queryCondition);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      }
    }
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }

  return queryCondition;
}

DDS::ReturnCode_t DDS::DataReader_impl::delete_readcondition (
  DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  DDS::ReadCondition_impl_ptr readCondition;
  gapi_readCondition handle;

  readCondition = dynamic_cast<DDS::ReadCondition_impl_ptr>(a_condition);
  if (readCondition)
  {
    handle = readCondition->_gapi_self;
    if (os_mutexLock(&(readCondition->rc_mutex)) == os_resultSuccess)
    {
      result =  gapi_dataReader_delete_readcondition(_gapi_self, handle);
      if (result != DDS::RETCODE_OK)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete readcondition");
      }
      if (os_mutexUnlock(&(readCondition->rc_mutex)) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
  return gapi_dataReader_delete_contained_entities(_gapi_self);
}

DDS::ReturnCode_t DDS::DataReader_impl::set_qos (
  const DDS::DataReaderQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;

  if (&qos == DDS::DefaultQos::DataReaderQosDefault)
  {
    result = gapi_dataReader_set_qos(_gapi_self, GAPI_DATAREADER_QOS_DEFAULT);
  }
  else if (&qos == DDS::DefaultQos::DataReaderQosUseTopicQos)
  {
    result = gapi_dataReader_set_qos(_gapi_self, GAPI_DATAREADER_QOS_USE_TOPIC_QOS);
  }
  else
  {
    gapi_dataReaderQos * gapi_drqos = gapi_dataReaderQos__alloc();
    if (gapi_drqos)
    {
      ccpp_DataReaderQos_copyIn(qos, *gapi_drqos);
      result = gapi_dataReader_set_qos(_gapi_self, gapi_drqos);
      gapi_free(gapi_drqos);
    }
    else
    {
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_qos (
  DDS::DataReaderQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataReaderQos * gapi_drqos = gapi_dataReaderQos__alloc();
  if (gapi_drqos)
  {
    result = gapi_dataReader_get_qos(_gapi_self, gapi_drqos);
    ccpp_DataReaderQos_copyOut(*gapi_drqos, qos);
    gapi_free(gapi_drqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::set_listener (
  DDS::DataReaderListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
  gapi_dataReaderListener gapi_listener;

  ccpp_DataReaderListener_copyIn(a_listener, gapi_listener);
  if (os_mutexLock(&dr_mutex) == os_resultSuccess)
  {
    result = gapi_dataReader_set_listener(_gapi_self, &gapi_listener, mask);
    if (result == DDS::RETCODE_OK)
    {
      DDS::ccpp_UserData_ptr myUD;
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
      if (myUD)
      {
        myUD->setListener(a_listener);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
    }
    if (os_mutexUnlock(&dr_mutex) != os_resultSuccess)
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
    }
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
  }
  return result;
}

DDS::DataReaderListener_ptr DDS::DataReader_impl::get_listener (
) THROW_ORB_EXCEPTIONS
{
  DDS::DataReaderListener_ptr result = NULL;
  gapi_dataReaderListener gapi_listener;

  if (os_mutexLock(&dr_mutex) == os_resultSuccess)
  {
    gapi_listener = gapi_dataReader_get_listener(_gapi_self);
    result = reinterpret_cast<DDS::DataReaderListener_ptr>(gapi_listener.listener_data);
    if (result)
    {
      DDS::DataReaderListener::_duplicate(result);
    }
    if (os_mutexUnlock(&dr_mutex) != os_resultSuccess)
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
    }
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
  }
  return result;
}

DDS::TopicDescription_ptr DDS::DataReader_impl::get_topicdescription (
) THROW_ORB_EXCEPTIONS
{
  gapi_topicDescription handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::TopicDescription_ptr topicDescription = NULL;

  handle = gapi_dataReader_get_topicdescription(_gapi_self);
  if (handle)
  {
    if (os_mutexLock(&dr_mutex) == os_resultSuccess)
    {
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (myUD)
      {
        topicDescription = dynamic_cast<DDS::TopicDescription_impl_ptr>(myUD->ccpp_object);
        if (topicDescription)
        {
          DDS::TopicDescription::_duplicate(topicDescription);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Topic Description");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
      if (os_mutexUnlock(&dr_mutex) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return topicDescription;
}

DDS::Subscriber_ptr DDS::DataReader_impl::get_subscriber (
) THROW_ORB_EXCEPTIONS
{
  gapi_subscriber handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::Subscriber_ptr subscriber = NULL;

  handle = gapi_dataReader_get_subscriber(_gapi_self);
  if (handle)
  {
    if (os_mutexLock(&dr_mutex) == os_resultSuccess)
    {
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (myUD)
      {
        subscriber = dynamic_cast<DDS::Subscriber_impl_ptr>(myUD->ccpp_object);
        if (subscriber)
        {
          DDS::Subscriber::_duplicate(subscriber);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Subscriber");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
      if (os_mutexUnlock(&dr_mutex) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return subscriber;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_sample_rejected_status (
    DDS::SampleRejectedStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_sampleRejectedStatus gapi_status;
  DDS::ReturnCode_t result;

  result = gapi_dataReader_get_sample_rejected_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_SampleRejectedStatus_copyOut(gapi_status, a_status);
  }

  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_liveliness_changed_status (
    DDS::LivelinessChangedStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_livelinessChangedStatus gapi_status;
  DDS::ReturnCode_t result;

  result = gapi_dataReader_get_liveliness_changed_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_LivelinessChangedStatus_copyOut(gapi_status, a_status);
  }

  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_requested_deadline_missed_status (
    DDS::RequestedDeadlineMissedStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_requestedDeadlineMissedStatus gapi_status;
  DDS::ReturnCode_t result;

  result = gapi_dataReader_get_requested_deadline_missed_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_RequestedDeadlineMissedStatus_copyOut(gapi_status, a_status);
  }

  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_requested_incompatible_qos_status (
    DDS::RequestedIncompatibleQosStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_requestedIncompatibleQosStatus* gapi_status;
  DDS::ReturnCode_t result;

  gapi_status = gapi_requestedIncompatibleQosStatus_alloc();
  result = gapi_dataReader_get_requested_incompatible_qos_status( DDS::Entity_impl::_gapi_self,gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_RequestedIncompatibleQosStatus_copyOut(*gapi_status, a_status);
  }
  gapi_free(gapi_status);

  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_subscription_matched_status (
    DDS::SubscriptionMatchedStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_subscriptionMatchedStatus gapi_status;
  DDS::ReturnCode_t result;

  result = gapi_dataReader_get_subscription_matched_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_SubscriptionMatchedStatus_copyOut(gapi_status, a_status);
  }

  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_sample_lost_status (
    DDS::SampleLostStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_sampleLostStatus gapi_status;
  DDS::ReturnCode_t result;

  result = gapi_dataReader_get_sample_lost_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_SampleLostStatus_copyOut(gapi_status, a_status);
  }

  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::wait_for_historical_data (
  const DDS::Duration_t & max_wait
) THROW_ORB_EXCEPTIONS
{
  gapi_duration_t gapi_max_wait;

  ccpp_Duration_copyIn(max_wait, gapi_max_wait);
  return gapi_dataReader_wait_for_historical_data(_gapi_self, &gapi_max_wait);
}


DDS::ReturnCode_t DDS::DataReader_impl::wait_for_historical_data_w_condition (
    const char * filter_expression,
    const DDS::StringSeq & filter_parameters,
    const DDS::Time_t & min_source_timestamp,
    const DDS::Time_t & max_source_timestamp,
    const DDS::ResourceLimitsQosPolicy & resource_limits,
    const DDS::Duration_t & max_wait
) THROW_ORB_EXCEPTIONS
{
    gapi_duration_t gapi_max_wait;
    gapi_time_t gapi_min_source_timestamp, gapi_max_source_timestamp;
    gapi_resourceLimitsQosPolicy gapi_resource_limits;
    gapi_stringSeq *gapi_filter_parameters;
    DDS::ReturnCode_t result;

    gapi_filter_parameters = gapi_stringSeq__alloc();

    if (gapi_filter_parameters){
        ccpp_sequenceCopyIn(filter_parameters, *gapi_filter_parameters);
        ccpp_Duration_copyIn(max_wait, gapi_max_wait);
        ccpp_TimeStamp_copyIn(min_source_timestamp, gapi_min_source_timestamp);
        ccpp_TimeStamp_copyIn(max_source_timestamp, gapi_max_source_timestamp);
        ccpp_ResourceLimitsQosPolicy_copyIn(resource_limits, gapi_resource_limits);

        result = gapi_dataReader_wait_for_historical_data_w_condition(
                    _gapi_self,
                    filter_expression,
                    gapi_filter_parameters,
                    &gapi_min_source_timestamp,
                    &gapi_max_source_timestamp,
                    &gapi_resource_limits,
                    &gapi_max_wait);

        gapi_free(gapi_filter_parameters);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_matched_publications (
  DDS::InstanceHandleSeq & publication_handles
) THROW_ORB_EXCEPTIONS
{
  gapi_instanceHandleSeq gapi_handles;
  DDS::ReturnCode_t result;

  ccpp_sequenceInitialize<gapi_instanceHandleSeq>(gapi_handles);
  result = gapi_dataReader_get_matched_publications(_gapi_self, &gapi_handles);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_sequenceCopyOut<gapi_instanceHandleSeq, gapi_instanceHandle_t, DDS::InstanceHandleSeq, DDS::InstanceHandle_t>
            (gapi_handles, publication_handles);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_matched_publication_data (
  DDS::PublicationBuiltinTopicData & publication_data,
  DDS::InstanceHandle_t publication_handle
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;
  gapi_publicationBuiltinTopicData * gapi_data = gapi_publicationBuiltinTopicData__alloc();

  if (gapi_data) {
      result = gapi_dataReader_get_matched_publication_data(_gapi_self, gapi_data, publication_handle);
      if (result == DDS::RETCODE_OK)
      {
        ccpp_PublicationBuiltinTopicData_copyOut(*gapi_data, publication_data);
      }
      gapi_free(gapi_data);
  } else {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
  return result;
}

//RZ: what to return if gapi_drvqos == nil?
DDS::DataReaderView_ptr DDS::DataReader_impl::create_view (
  const ::DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
    DataReaderView_ptr drvp;
    gapi_dataReaderView view_handle;
    gapi_dataReaderViewQos* gapi_drvqos;

    gapi_drvqos = gapi_dataReaderViewQos__alloc();

    if (gapi_drvqos)
    {
        ccpp_DataReaderViewQos_copyIn(qos, *gapi_drvqos);
        view_handle = gapi_dataReader_create_view(_gapi_self, gapi_drvqos);
        gapi_free(gapi_drvqos);
    }

    if (view_handle)
    {
        DDS::ccpp_UserData_ptr myUD;
        gapi_topicDescription topic_descr_handle = gapi_dataReader_get_topicdescription(_gapi_self);
        char * typeName = gapi_topicDescription_get_type_name(topic_descr_handle);

        if (typeName)
        {
          gapi_subscriber subscr_handle = gapi_dataReader_get_subscriber(_gapi_self);
          gapi_domainParticipant dp_handle = gapi_subscriber_get_participant(subscr_handle);

          if (dp_handle)
          {
            gapi_typeSupport ts_handle = gapi_domainParticipant_get_typesupport(dp_handle, typeName);
            void *tsf = gapi_object_get_user_data(ts_handle);

            if (tsf)
            {
              CORBA::Object_ptr anObject = static_cast<CORBA::Object_ptr>(tsf);
              DDS::TypeSupportFactory_impl_ptr factory = dynamic_cast<DDS::TypeSupportFactory_impl_ptr>(anObject);
              if (factory)
              {
                drvp = factory->create_view(view_handle);

                if (drvp)
                {
                  myUD = new DDS::ccpp_UserData(drvp,  NULL);
                  if (myUD)
                  {
                    gapi_object_set_user_data(view_handle, (CORBA::Object *)myUD,
                                              DDS::ccpp_CallBack_DeleteUserData, NULL);
                  }
                  else
                  {
                    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
                  }
                }
              }
              else
              {
                OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Type Support Factory");
              }
            }
            else
            {
              OS_REPORT(OS_ERROR, "CCPP", 0, "Type Support information not available for create_dataview");
            }
          }
          gapi_free(typeName);
        }
      }

    return drvp;
}

DDS::ReturnCode_t DDS::DataReader_impl::delete_view (
  ::DDS::DataReaderView_ptr a_view
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    DDS::DataReaderView_impl_ptr dataReaderView;

    dataReaderView = dynamic_cast<DDS::DataReaderView_impl_ptr>(a_view);
    if (dataReaderView)
    {
      if (os_mutexLock(&(dataReaderView->drv_mutex)) == os_resultSuccess)
      {
        result = gapi_dataReader_delete_view(_gapi_self, dataReaderView->_gapi_self);
        if (result == DDS::RETCODE_OK)
        {
          dataReaderView->_gapi_self = NULL;
        }
        else
        {
          result = DDS::RETCODE_ERROR;
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete view");
        }
        if (os_mutexUnlock(&(dataReaderView->drv_mutex)) != os_resultSuccess)
        {
            result = DDS::RETCODE_ERROR;
            OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
        }
      }
      else
      {
          result = DDS::RETCODE_ERROR;
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
      }
    }
    return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::get_default_datareaderview_qos (
  ::DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataReaderViewQos * gapi_drvqos = gapi_dataReaderViewQos__alloc();
  if (gapi_drvqos)
  {
    result = gapi_dataReader_get_default_datareaderview_qos(_gapi_self, gapi_drvqos);
    ccpp_DataReaderViewQos_copyOut(*gapi_drvqos, qos);
    gapi_free(gapi_drvqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::set_default_datareaderview_qos (
  const ::DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataReaderViewQos * gapi_drvqos = gapi_dataReaderViewQos__alloc();
  if (gapi_drvqos)
  {
    ccpp_DataReaderViewQos_copyIn( qos, *gapi_drvqos);
    result = gapi_dataReader_set_default_datareaderview_qos(_gapi_self, gapi_drvqos);
    gapi_free(gapi_drvqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::read (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReader_read(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReader_impl::take (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReader_take(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReader_impl::read_w_condition (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
  DDS::ReadCondition_impl_ptr readCondition;
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

  readCondition = dynamic_cast<DDS::ReadCondition_impl_ptr>(a_condition);
  if (readCondition)
  {
    result = gapi_fooDataReader_read_w_condition(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::take_w_condition (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
  DDS::ReadCondition_impl_ptr readCondition;
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

  readCondition = dynamic_cast<DDS::ReadCondition_impl_ptr>(a_condition);
  if (readCondition)
  {
    result = gapi_fooDataReader_take_w_condition(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::read_next_sample (
    void * data_values,
    DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
  gapi_sampleInfo gapi_info;
  DDS::ReturnCode_t result;

  result = gapi_fooDataReader_read_next_sample(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &gapi_info);

  if (result == DDS::RETCODE_OK)
  {
    ccpp_SampleInfo_copyOut(gapi_info, sample_info);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::take_next_sample (
    void * data_values,
    DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
  gapi_sampleInfo gapi_info;
  DDS::ReturnCode_t result;

  result = gapi_fooDataReader_take_next_sample(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &gapi_info);

  if (result == DDS::RETCODE_OK)
  {
    ccpp_SampleInfo_copyOut(gapi_info, sample_info);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::read_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReader_read_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReader_impl::take_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReader_take_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReader_impl::read_next_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReader_read_next_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReader_impl::take_next_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReader_take_next_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReader_impl::read_next_instance_w_condition (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
  DDS::ReadCondition_impl_ptr readCondition;
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

  readCondition = dynamic_cast<DDS::ReadCondition_impl_ptr>(a_condition);
  if (readCondition)
  {
    result = gapi_fooDataReader_read_next_instance_w_condition(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::take_next_instance_w_condition (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
  DDS::ReadCondition_impl_ptr readCondition;
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

  readCondition = dynamic_cast<DDS::ReadCondition_impl_ptr>(a_condition);
  if (readCondition)
  {
    result = gapi_fooDataReader_take_next_instance_w_condition(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReader_impl::return_loan (
    void *dataBuf,
    void *infoBuf
) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataReader_return_loan(_gapi_self, dataBuf, infoBuf);
}

DDS::ReturnCode_t DDS::DataReader_impl::get_key_value (
    void * key_holder,
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataReader_get_key_value(_gapi_self, static_cast<gapi_foo*>(key_holder), handle);
}

DDS::InstanceHandle_t DDS::DataReader_impl::lookup_instance (
    const void * instance
) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataReader_lookup_instance(_gapi_self, static_cast<const gapi_foo*>(instance));
}

