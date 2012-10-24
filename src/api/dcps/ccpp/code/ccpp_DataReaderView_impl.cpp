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
#include "ccpp_DataReaderView_impl.h"
#include "ccpp_Utils.h"
#include "ccpp_QosUtils.h"
#include "ccpp_ReadCondition_impl.h"
#include "ccpp_QueryCondition_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_TopicDescription_impl.h"
//RZ: not #include "ccpp_ListenerUtils.h"

DDS::DataReaderView_impl::DataReaderView_impl(gapi_dataReaderView handle) : DDS::Entity_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&drv_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::DataReaderView_impl::~DataReaderView_impl()
{
  if (os_mutexDestroy(&drv_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::ReadCondition_ptr DDS::DataReaderView_impl::create_readcondition (
  DDS::SampleStateMask sample_states,
  DDS::ViewStateMask view_states,
  DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
  gapi_readCondition handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::ReadCondition_ptr readCondition = NULL;

  handle = gapi_dataReaderView_create_readcondition( _gapi_self, sample_states, view_states, instance_states);
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

DDS::QueryCondition_ptr DDS::DataReaderView_impl::create_querycondition (
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
    handle = gapi_dataReaderView_create_querycondition(
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

DDS::ReturnCode_t DDS::DataReaderView_impl::delete_readcondition (
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
      result =  gapi_dataReaderView_delete_readcondition(_gapi_self, handle);
      if (result == DDS::RETCODE_OK)
      {
      }
      else
      {
          result = DDS::RETCODE_ERROR;
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete readcondition");
      }
      if (os_mutexUnlock(&(readCondition->rc_mutex)) != os_resultSuccess)
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

DDS::ReturnCode_t DDS::DataReaderView_impl::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
  return gapi_dataReaderView_delete_contained_entities(_gapi_self);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::set_qos (
  const DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;

  if (&qos == DDS::DefaultQos::DataReaderViewQosDefault)
  {
    result = gapi_dataReaderView_set_qos(_gapi_self, GAPI_DATAVIEW_QOS_DEFAULT);
  }
  else
  {
    gapi_dataReaderViewQos * gapi_drvqos = gapi_dataReaderViewQos__alloc();
    if (gapi_drvqos)
    {
      ccpp_DataReaderViewQos_copyIn(qos, *gapi_drvqos);
      result = gapi_dataReaderView_set_qos(_gapi_self, gapi_drvqos);
      gapi_free(gapi_drvqos);
    }
    else
    {
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::get_qos (
  DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
  gapi_dataReaderViewQos * gapi_drvqos = gapi_dataReaderViewQos__alloc();
  if (gapi_drvqos)
  {
    result = gapi_dataReaderView_get_qos(_gapi_self, gapi_drvqos);
    if(result == DDS::RETCODE_OK){
        ccpp_DataReaderViewQos_copyOut(*gapi_drvqos, qos);

    }
    else{
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to retrieve DataReaderViewQoS");
    }
    gapi_free(gapi_drvqos);
  }
  else
  {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
  return result;
}


DDS::ReturnCode_t DDS::DataReaderView_impl::read (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReaderView_read(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::take (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReaderView_take(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::read_w_condition (
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
    result = gapi_fooDataReaderView_read_w_condition(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::take_w_condition (
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
    result = gapi_fooDataReaderView_take_w_condition(
                _gapi_self,
                data_values,
                &info_seq,
                max_samples,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::read_next_sample (
    void * data_values,
    DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
  gapi_sampleInfo gapi_info;
  DDS::ReturnCode_t result;

  result = gapi_fooDataReaderView_read_next_sample(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &gapi_info);

  if (result == DDS::RETCODE_OK)
  {
    ccpp_SampleInfo_copyOut(gapi_info, sample_info);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::take_next_sample (
    void * data_values,
    DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
  gapi_sampleInfo gapi_info;
  DDS::ReturnCode_t result;

  result = gapi_fooDataReaderView_take_next_sample(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &gapi_info);

  if (result == DDS::RETCODE_OK)
  {
    ccpp_SampleInfo_copyOut(gapi_info, sample_info);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::read_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReaderView_read_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::take_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReaderView_take_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::read_next_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReaderView_read_next_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::take_next_instance (
    void * data_values,
    DDS::SampleInfoSeq & info_seq,
    CORBA::Long max_samples,
    DDS::InstanceHandle_t a_handle,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    return gapi_fooDataReaderView_take_next_instance(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                sample_states,
                view_states,
                instance_states);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::read_next_instance_w_condition (
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
    result = gapi_fooDataReaderView_read_next_instance_w_condition(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::take_next_instance_w_condition (
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
    result = gapi_fooDataReaderView_take_next_instance_w_condition(
                _gapi_self,
                static_cast<gapi_foo*>(data_values),
                &info_seq,
                max_samples,
                a_handle,
                readCondition->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::DataReaderView_impl::return_loan (
    void *dataBuf,
    void *infoBuf
) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataReaderView_return_loan(_gapi_self, dataBuf, infoBuf);
}

DDS::ReturnCode_t DDS::DataReaderView_impl::get_key_value (
    void * key_holder,
    DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataReaderView_get_key_value(_gapi_self, static_cast<gapi_foo*>(key_holder), handle);
}

DDS::InstanceHandle_t DDS::DataReaderView_impl::lookup_instance (
    const void * instance
) THROW_ORB_EXCEPTIONS
{
  return gapi_fooDataReaderView_lookup_instance(_gapi_self, static_cast<const gapi_foo*>(instance));
}

DDS::StatusCondition_ptr DDS::DataReaderView_impl::get_statuscondition()
THROW_ORB_EXCEPTIONS
{
    return (DDS::StatusCondition_ptr)
        gapi_dataReaderView_get_statuscondition((gapi_dataReaderView)_gapi_self);

    DDS::StatusCondition_ptr statuscondition;
    DDS::ccpp_UserData_ptr myUD;
    gapi_statusCondition gapi_sc;

    gapi_sc = gapi_dataReaderView_get_statuscondition((gapi_dataReaderView)_gapi_self);
    myUD = dynamic_cast<DDS::ccpp_UserData_ptr>
                          ((CORBA::Object *)gapi_object_get_user_data(
                                  gapi_sc));
    if (myUD)
    {
      statuscondition = dynamic_cast<DDS::StatusCondition_ptr>(myUD->ccpp_object);
      if (statuscondition)
      {
        DDS::StatusCondition::_duplicate(statuscondition);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Status Condition");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
    }

    return statuscondition;
}

DDS::StatusMask DDS::DataReaderView_impl::get_status_changes()
THROW_ORB_EXCEPTIONS
{
    return (DDS::StatusMask)
        gapi_dataReaderView_get_status_changes((gapi_dataReaderView)_gapi_self);
}

DDS::DataReader_ptr DDS::DataReaderView_impl::get_datareader()
THROW_ORB_EXCEPTIONS
{
    DataReader_ptr reader;
    DDS::ccpp_UserData_ptr myUD;
    gapi_dataReader gapi_dr;

    gapi_dr = gapi_dataReaderView_get_datareader((gapi_dataReaderView)_gapi_self);
    myUD = dynamic_cast<DDS::ccpp_UserData_ptr>
                          ((CORBA::Object *)gapi_object_get_user_data(
                                  gapi_dr));
    if (myUD)
    {
      reader = dynamic_cast<DDS::DataReader_ptr>(myUD->ccpp_object);
      if (reader)
      {
        DDS::DataReader::_duplicate(reader);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Data Reader");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
    }

    return reader;
}
