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
#include "ccpp_dds_dcps.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_Topic_impl.h"
#include "ccpp_DataReader_impl.h"
#include "ccpp_ListenerUtils.h"
#include "ccpp_QosUtils.h"
#include "ccpp_Utils.h"
#include "ccpp_TypeSupport_impl.h"

DDS::Subscriber_impl::Subscriber_impl(gapi_subscriber handle) : DDS::Entity_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&s_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::Subscriber_impl::~Subscriber_impl()
{
  if (os_mutexDestroy(&s_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::DataReader_ptr DDS::Subscriber_impl::create_datareader (
  DDS::TopicDescription_ptr a_topic,
  const DDS::DataReaderQos & qos,
  DDS::DataReaderListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
  gapi_topic topic_handle;
  gapi_dataReaderQos * gapi_drqos;
  gapi_dataReaderListener * gapi_listener = NULL;
  DDS::TopicDescription_impl_ptr cfTopicImpl;
  DDS::DataReader_ptr DataReader = NULL;
  gapi_dataReader reader_handle;
  CORBA::Boolean allocatedQos = false;
  CORBA::Boolean proceed = true;

  cfTopicImpl = dynamic_cast<DDS::TopicDescription_impl_ptr>(a_topic);
  if (!cfTopicImpl)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid TopicDescription");
  }
  else
  {
    topic_handle = cfTopicImpl->__gapi_self;

    if (a_listener)
    {
      gapi_listener = gapi_dataReaderListener__alloc();
      if (gapi_listener)
      {
        ccpp_DataReaderListener_copyIn(a_listener, *gapi_listener);
      }
      else
      {
        proceed = false;
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      }
    }

    if (proceed)
    {
      if (&qos == DDS::DefaultQos::DataReaderQosDefault)
      {
        gapi_drqos = GAPI_DATAREADER_QOS_DEFAULT;
      }
      else if (&qos == DDS::DefaultQos::DataReaderQosUseTopicQos)
      {
        gapi_drqos = GAPI_DATAREADER_QOS_USE_TOPIC_QOS;
      }
      else
      {
        gapi_drqos = gapi_dataReaderQos__alloc();
        if (gapi_drqos)
        {
          allocatedQos = true;
          ccpp_DataReaderQos_copyIn(qos, *gapi_drqos);
        }
        else
        {
          proceed = false;
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
        }
      }
    }

    if (proceed)
    {
      reader_handle = gapi_subscriber_create_datareader(
                _gapi_self,
                topic_handle,
                gapi_drqos,
                gapi_listener,
                mask);

      if (reader_handle)
      {
        DDS::ccpp_UserData_ptr myUD;
        char * typeName = gapi_topic_get_type_name(topic_handle);

        if (typeName)
        {
          gapi_domainParticipant dp_handle = gapi_subscriber_get_participant(_gapi_self);

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
                DataReader = factory->create_datareader(reader_handle);

                if (DataReader)
                {
                  myUD = new DDS::ccpp_UserData(DataReader,  a_listener);
                  if (myUD)
                  {
                      gapi_subscriberQos *sqos = gapi_subscriberQos__alloc();
                      gapi_object_set_user_data(reader_handle, (CORBA::Object *)myUD,
                                                DDS::ccpp_CallBack_DeleteUserData,NULL);
                      if(sqos){
                          if(gapi_subscriber_get_qos(_gapi_self, sqos) == GAPI_RETCODE_OK){
                              if(sqos->entity_factory.autoenable_created_entities) {
                                  gapi_entity_enable(reader_handle);
                              }
                          }
                          else
                          {
                              OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain subscriber_qos");
                          }
                          gapi_free(sqos);
                      }
                      else
                      {
                          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
                      }

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
              OS_REPORT(OS_ERROR, "CCPP", 0, "Type Support information not available for create_datareader");
            }
          }
          gapi_free(typeName);
        }
      }
    }
  }
  if (gapi_listener)
  {
    gapi_free(gapi_listener);
  }
  if (allocatedQos)
  {
    gapi_free(gapi_drqos);
  }
  return DataReader;
}

DDS::ReturnCode_t DDS::Subscriber_impl::delete_datareader (
  DDS::DataReader_ptr a_datareader
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  DDS::DataReader_impl_ptr dataReader;

  dataReader = dynamic_cast<DDS::DataReader_impl_ptr>(a_datareader);
  if (dataReader)
  {
    if (os_mutexLock(&(dataReader->dr_mutex)) == os_resultSuccess)
    {
      result = gapi_subscriber_delete_datareader(_gapi_self, dataReader->_gapi_self);
      if (result == DDS::RETCODE_OK)
      {
        dataReader->_gapi_self = NULL;
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete datareader");
      }
      if (os_mutexUnlock(&(dataReader->dr_mutex)) != os_resultSuccess)
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

DDS::ReturnCode_t DDS::Subscriber_impl::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
  return gapi_subscriber_delete_contained_entities(_gapi_self);
}

DDS::DataReader_ptr DDS::Subscriber_impl::lookup_datareader (
const char * topic_name
) THROW_ORB_EXCEPTIONS
{
  gapi_dataReader handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::DataReader_ptr dataReader = NULL;

  handle = gapi_subscriber_lookup_datareader(_gapi_self, topic_name);
  if (handle)
  {
    if (os_mutexLock(&s_mutex) == os_resultSuccess)
    {
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (myUD)
      {
        dataReader = dynamic_cast<DDS::DataReader_ptr>(myUD->ccpp_object);
        if (dataReader)
        {
          DDS::DataReader::_duplicate(dataReader);
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
      if (os_mutexUnlock(&s_mutex) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return dataReader;
}

DDS::ReturnCode_t DDS::Subscriber_impl::get_datareaders (
  DDS::DataReaderSeq & readers,
  DDS::SampleStateMask sample_states,
  DDS::ViewStateMask view_states,
  DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
  gapi_dataReaderSeq * gapi_readers;
  DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;

  gapi_readers = gapi_dataReaderSeq__alloc();
  if (gapi_readers)
  {
    result = gapi_subscriber_get_datareaders( _gapi_self, gapi_readers, sample_states,
                                                view_states, instance_states);
    if (result == DDS::RETCODE_OK)
    {
      CORBA::ULong l = static_cast<CORBA::ULong>(gapi_readers->_length);
      readers.length(l);
      for (CORBA::ULong i=0; i<l; i++)
      {
        DDS::ccpp_UserData_ptr myUD;
        myUD = dynamic_cast<DDS::ccpp_UserData_ptr>
                              ((CORBA::Object *)gapi_object_get_user_data(gapi_readers->_buffer[i]));
        if (myUD)
        {
          readers[i] = dynamic_cast<DDS::DataReader_ptr>(myUD->ccpp_object);
          if (readers[i])
          {
            DDS::DataReader::_duplicate(readers[i]);
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
      }
    }
    gapi_sequence_free(gapi_readers);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
  return result;
}

DDS::ReturnCode_t DDS::Subscriber_impl::notify_datareaders (
) THROW_ORB_EXCEPTIONS
{
  return gapi_subscriber_notify_datareaders(_gapi_self);
}

DDS::ReturnCode_t DDS::Subscriber_impl::set_qos (
  const DDS::SubscriberQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  if (&qos == DDS::DefaultQos::SubscriberQosDefault)
  {
    result = gapi_subscriber_set_qos(_gapi_self, GAPI_SUBSCRIBER_QOS_DEFAULT );
  }
  else
  {
    gapi_subscriberQos * gapi_sqos = gapi_subscriberQos__alloc();
    if (gapi_sqos)
    {
      ccpp_SubscriberQos_copyIn( qos, *gapi_sqos);
      result = gapi_subscriber_set_qos(_gapi_self, gapi_sqos);
      gapi_free(gapi_sqos);
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
  }
  return result;
}

DDS::ReturnCode_t DDS::Subscriber_impl::get_qos (
  DDS::SubscriberQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_subscriberQos * gapi_sqos = gapi_subscriberQos__alloc();
  if (gapi_sqos)
  {
    result = gapi_subscriber_get_qos(_gapi_self, gapi_sqos);
    ccpp_SubscriberQos_copyOut(*gapi_sqos, qos);
    gapi_free(gapi_sqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t DDS::Subscriber_impl::set_listener (
  DDS::SubscriberListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_subscriberListener gapi_listener;
    ccpp_SubscriberListener_copyIn(a_listener, gapi_listener);
    result = gapi_subscriber_set_listener(_gapi_self, &gapi_listener, mask);
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
    return result;
}

DDS::SubscriberListener_ptr DDS::Subscriber_impl::get_listener (
) THROW_ORB_EXCEPTIONS
{
  DDS::SubscriberListener_ptr result;
  gapi_subscriberListener gapi_listener;

  gapi_listener = gapi_subscriber_get_listener(_gapi_self);
  result = reinterpret_cast<DDS::SubscriberListener_ptr>(gapi_listener.listener_data);
  if (result)
  {
    DDS::SubscriberListener::_duplicate(result);
  }
  return result;
}

DDS::ReturnCode_t DDS::Subscriber_impl::begin_access (
) THROW_ORB_EXCEPTIONS
{
  return gapi_subscriber_begin_access(_gapi_self);
}

DDS::ReturnCode_t DDS::Subscriber_impl::end_access (
) THROW_ORB_EXCEPTIONS
{
  return gapi_subscriber_end_access(_gapi_self);
}

DDS::DomainParticipant_ptr DDS::Subscriber_impl::get_participant (
) THROW_ORB_EXCEPTIONS
{
  gapi_domainParticipant handle;
  DDS::DomainParticipant_ptr domainParticipant = NULL;

  handle = gapi_subscriber_get_participant(_gapi_self);
  if (handle)
  {
    DDS::ccpp_UserData_ptr myUD;
    myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));

    if (myUD)
    {
      domainParticipant = dynamic_cast<DDS::DomainParticipant_ptr>(myUD->ccpp_object);
      if (domainParticipant)
      {
        DDS::DomainParticipant::_duplicate(domainParticipant);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Participant");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
    }
  }
  return domainParticipant;
}

DDS::ReturnCode_t DDS::Subscriber_impl::set_default_datareader_qos (
  const DDS::DataReaderQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataReaderQos * gapi_drqos = gapi_dataReaderQos__alloc();
  if (gapi_drqos)
  {
    ccpp_DataReaderQos_copyIn( qos, *gapi_drqos);
    result = gapi_subscriber_set_default_datareader_qos(_gapi_self, gapi_drqos);
    gapi_free(gapi_drqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t DDS::Subscriber_impl::get_default_datareader_qos (
  DDS::DataReaderQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataReaderQos * gapi_drqos = gapi_dataReaderQos__alloc();
  if (gapi_drqos)
  {
    result = gapi_subscriber_get_default_datareader_qos(_gapi_self, gapi_drqos);
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

DDS::ReturnCode_t DDS::Subscriber_impl::copy_from_topic_qos (
  DDS::DataReaderQos & a_datareader_qos,
  const DDS::TopicQos & a_topic_qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    gapi_dataReaderQos * gapi_drqos = gapi_dataReaderQos__alloc();
    gapi_topicQos * gapi_tqos = gapi_topicQos__alloc();

    if (gapi_drqos && gapi_tqos)
    {
      ccpp_TopicQos_copyIn(a_topic_qos, *gapi_tqos);
      ccpp_DataReaderQos_copyIn(a_datareader_qos, *gapi_drqos);
      result = gapi_subscriber_copy_from_topic_qos(_gapi_self, gapi_drqos, gapi_tqos);
      if (result == DDS::RETCODE_OK)
      {
        ccpp_DataReaderQos_copyOut(*gapi_drqos, a_datareader_qos);
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    if (gapi_tqos)
    {
      gapi_free(gapi_tqos);
    }
    if (gapi_drqos)
    {
      gapi_free(gapi_drqos);
    }
    return result;
}

