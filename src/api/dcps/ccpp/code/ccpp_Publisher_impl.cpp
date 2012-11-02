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
#include "ccpp_Publisher_impl.h"
#include "ccpp_Topic_impl.h"
#include "ccpp_DataWriter_impl.h"
#include "ccpp_ListenerUtils.h"
#include "ccpp_QosUtils.h"
#include "ccpp_Utils.h"
#include "ccpp_TypeSupport_impl.h"

DDS::Publisher_impl::Publisher_impl(gapi_publisher handle) : DDS::Entity_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&p_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::Publisher_impl", 0,
              "Unable to create mutex");
  }
}

DDS::Publisher_impl::~Publisher_impl()
{
  if (os_mutexDestroy(&p_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::~Publisher_impl", 0,
              "Unable to destroy mutex");
  }
}

DDS::DataWriter_ptr
DDS::Publisher_impl::create_datawriter (
  DDS::Topic_ptr a_topic,
  const DDS::DataWriterQos & qos,
  DDS::DataWriterListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
  gapi_topic topic_handle;
  gapi_dataWriterQos * gapi_dwqos;
  gapi_dataWriterListener * gapi_listener = NULL;
  DDS::Topic_impl_ptr Topic;
  DDS::DataWriter_ptr DataWriter = NULL;
  gapi_dataWriter writer_handle;
  CORBA::Boolean allocatedQos = false;
  CORBA::Boolean proceed = true;

  Topic = dynamic_cast<DDS::Topic_impl_ptr>(a_topic);
  if (!Topic)
  {
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::create_datawriter", 0,
              "Invalid Topic");
  }
  else
  {
    topic_handle = Topic->__gapi_self;

    if (a_listener)
    {
      gapi_listener = gapi_dataWriterListener__alloc();
      if (gapi_listener)
      {
        ccpp_DataWriterListener_copyIn(a_listener, *gapi_listener);
      }
      else
      {
        proceed = false;
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::create_datawriter", 0,
                  "Unable to allocate memory");
      }
    }

    if (proceed)
    {
      if (&qos == DDS::DefaultQos::DataWriterQosDefault)
      {
        gapi_dwqos = GAPI_DATAWRITER_QOS_DEFAULT;
      }
      else if (&qos == DDS::DefaultQos::DataWriterQosUseTopicQos)
      {
        gapi_dwqos = GAPI_DATAWRITER_QOS_USE_TOPIC_QOS;
      }
      else
      {
        gapi_dwqos = gapi_dataWriterQos__alloc();
        if (gapi_dwqos)
        {
          allocatedQos = true;
          ccpp_DataWriterQos_copyIn(qos, *gapi_dwqos);
        }
        else
        {
          proceed = false;
          OS_REPORT(OS_ERROR,
                    "DDS::Publisher_impl::create_datawriter", 0,
                    "Unable to allocate memory");
        }
      }
    }

    if (proceed)
    {
      writer_handle = gapi_publisher_create_datawriter(
                _gapi_self,
                topic_handle,
                gapi_dwqos,
                gapi_listener,
                mask);


      if (writer_handle)
      {
        DDS::ccpp_UserData_ptr myUD;
        char * typeName = gapi_topic_get_type_name(topic_handle);

        if (typeName)
        {
          gapi_domainParticipant dp_handle = gapi_publisher_get_participant(_gapi_self);

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
                DataWriter = factory->create_datawriter(writer_handle);

                if (DataWriter)
                {
                  myUD = new DDS::ccpp_UserData(DataWriter,  a_listener);
                  if (myUD)
                  {
                    gapi_publisherQos *pqos = gapi_publisherQos__alloc();
                    gapi_object_set_user_data(writer_handle, (CORBA::Object *)myUD,
                                              DDS::ccpp_CallBack_DeleteUserData,NULL);
                    if(pqos){
                        if(gapi_publisher_get_qos(_gapi_self, pqos) == GAPI_RETCODE_OK){
                            if(pqos->entity_factory.autoenable_created_entities) {
                                gapi_entity_enable(writer_handle);
                            }
                        }
                        else
                        {
                            OS_REPORT(OS_ERROR,
                                      "DDS::Publisher_impl::create_datawriter", 0,
                                      "Unable to obtain publisher_qos");
                        }
                        gapi_free(pqos);
                    }
                    else
                    {
                        OS_REPORT(OS_ERROR,
                                  "DDS::Publisher_impl::create_datawriter", 0,
                                  "Unable to allocate memory");
                    }
                  }
                  else
                  {
                    OS_REPORT(OS_ERROR,
                              "DDS::Publisher_impl::create_datawriter", 0,
                              "Unable to allocate memory");
                  }
                }
              }
              else
              {
                OS_REPORT(OS_ERROR,
                          "DDS::Publisher_impl::create_datawriter", 0,
                          "Invalid Type Support Factory");
              }
            }
            else
            {
              OS_REPORT(OS_ERROR,
                        "DDS::Publisher_impl::create_datawriter", 0,
                        "Type Support information not available for create_datawriter");
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
    gapi_free(gapi_dwqos);
  }
  return DataWriter;
}


DDS::ReturnCode_t
DDS::Publisher_impl::delete_datawriter (
  DDS::DataWriter_ptr a_datawriter
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  DDS::DataWriter_impl_ptr dataWriter;

  dataWriter = dynamic_cast<DDS::DataWriter_impl_ptr>(a_datawriter);
  if (dataWriter)
  {
    if (os_mutexLock(&(dataWriter->dw_mutex)) == os_resultSuccess)
    {
      result = gapi_publisher_delete_datawriter(_gapi_self, dataWriter->_gapi_self);
      if (result == DDS::RETCODE_OK)
      {
        dataWriter->_gapi_self = NULL;
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::delete_datawriter", 0,
                  "Unable to delete datawriter");
      }
      if (os_mutexUnlock(&(dataWriter->dw_mutex)) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::delete_datawriter", 0,
                  "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::delete_datawriter", 0,
                "Unable to obtain mutex");
    }
  }
  return result;
}

DDS::DataWriter_ptr
DDS::Publisher_impl::lookup_datawriter (
  const char * topic_name
) THROW_ORB_EXCEPTIONS
{
  gapi_dataWriter handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::DataWriter_ptr dataWriter = NULL;

  handle = gapi_publisher_lookup_datawriter(_gapi_self, topic_name);
  if (handle)
  {
    if (os_mutexLock(&p_mutex) == os_resultSuccess)
    {
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (myUD)
      {
        dataWriter = dynamic_cast<DDS::DataWriter_ptr>(myUD->ccpp_object);
        if (dataWriter)
        {
          DDS::DataWriter::_duplicate(dataWriter);
        }
        else
        {
          OS_REPORT(OS_ERROR,
                    "DDS::Publisher_impl::lookup_datawriter", 0,
                    "Invalid Data Writer");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::lookup_datawriter", 0,
                  "Unable to obtain userdata");
      }
      if (os_mutexUnlock(&p_mutex) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::lookup_datawriter", 0,
                  "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::lookup_datawriter", 0,
                "Unable to obtain mutex");
    }
  }
  return dataWriter;
}

DDS::ReturnCode_t
DDS::Publisher_impl::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
  return gapi_publisher_delete_contained_entities(_gapi_self);
}

DDS::ReturnCode_t
DDS::Publisher_impl::set_qos (
  const DDS::PublisherQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;

  if (&qos == DDS::DefaultQos::PublisherQosDefault)
  {
    result = gapi_publisher_set_qos(_gapi_self, GAPI_PUBLISHER_QOS_DEFAULT );
  }
  else
  {
    gapi_publisherQos * gapi_pqos = gapi_publisherQos__alloc();
    if (gapi_pqos)
    {
      ccpp_PublisherQos_copyIn( qos, *gapi_pqos);
      result = gapi_publisher_set_qos(_gapi_self, gapi_pqos );
      gapi_free(gapi_pqos);
    }
    else
    {
      result = DDS::RETCODE_OUT_OF_RESOURCES;
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::set_qos", 0,
                "Unable to allocate memory");
    }
  }
  return result;
}

DDS::ReturnCode_t
DDS::Publisher_impl::get_qos (
  DDS::PublisherQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_publisherQos * gapi_pqos = gapi_publisherQos__alloc();
  if (gapi_pqos)
  {
    result = gapi_publisher_get_qos(_gapi_self, gapi_pqos);
    ccpp_PublisherQos_copyOut(*gapi_pqos, qos);
    gapi_free(gapi_pqos);
  }
  else
  {
    result = DDS::RETCODE_OUT_OF_RESOURCES;
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::get_qos", 0,
              "Unable to allocate memory");
  }
  return result;
}

DDS::ReturnCode_t
DDS::Publisher_impl::set_listener (
  DDS::PublisherListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_publisherListener * gapi_listener = NULL;
    gapi_listener = gapi_publisherListener__alloc();
    if (gapi_listener)
    {
      ccpp_PublisherListener_copyIn(a_listener, *gapi_listener);
      if (os_mutexLock(&p_mutex) == os_resultSuccess)
      {
        result = gapi_publisher_set_listener(_gapi_self, gapi_listener, mask);
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
            OS_REPORT(OS_ERROR,
                      "DDS::Publisher_impl::set_listener", 0,
                      "Unable to obtain userdata");
          }
        }
        if (os_mutexUnlock(&p_mutex) != os_resultSuccess)
        {
          OS_REPORT(OS_ERROR,
                    "DDS::Publisher_impl::set_listener", 0,
                    "Unable to release mutex");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::set_listener", 0,
                  "Unable to obtain lock");
      }
      gapi_free(gapi_listener);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::set_listener", 0,
                "Unable to allocate memory");
    }
    return result;
}

DDS::PublisherListener_ptr
DDS::Publisher_impl::get_listener (
) THROW_ORB_EXCEPTIONS
{
  DDS::PublisherListener_ptr result;
  gapi_publisherListener gapi_listener;

  if (os_mutexLock(&p_mutex) == os_resultSuccess)
  {
    gapi_listener = gapi_publisher_get_listener(_gapi_self);
    result = reinterpret_cast<DDS::PublisherListener_ptr>(gapi_listener.listener_data);
    if (result)
    {
      DDS::PublisherListener::_duplicate(result);
    }
    if (os_mutexUnlock(&p_mutex) != os_resultSuccess)
    {
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::get_listener", 0,
                "Unable to release mutex");
    }
  }
  else
  {
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::get_listener", 0,
              "Unable to obtain lock");
  }
  return result;
}

DDS::ReturnCode_t
DDS::Publisher_impl::suspend_publications (
) THROW_ORB_EXCEPTIONS
{
  return gapi_publisher_suspend_publications(_gapi_self);
}

DDS::ReturnCode_t
DDS::Publisher_impl::resume_publications (
) THROW_ORB_EXCEPTIONS
{
  return gapi_publisher_resume_publications(_gapi_self);
}

DDS::ReturnCode_t
DDS::Publisher_impl::begin_coherent_changes (
) THROW_ORB_EXCEPTIONS
{
  return gapi_publisher_begin_coherent_changes(_gapi_self);
}

DDS::ReturnCode_t
DDS::Publisher_impl::end_coherent_changes (
) THROW_ORB_EXCEPTIONS
{
  return gapi_publisher_end_coherent_changes(_gapi_self);
}

DDS::ReturnCode_t
DDS::Publisher_impl::wait_for_acknowledgments (
  const DDS::Duration_t & max_wait
) THROW_ORB_EXCEPTIONS
{
    gapi_duration_t gapi_max_wait;

    ccpp_Duration_copyIn(max_wait, gapi_max_wait);
    return gapi_publisher_wait_for_acknowledgments(_gapi_self, &gapi_max_wait);
}

DDS::DomainParticipant_ptr
DDS::Publisher_impl::get_participant (
) THROW_ORB_EXCEPTIONS
{
  gapi_domainParticipant handle;
  DDS::DomainParticipant_ptr domainParticipant = NULL;

  handle = gapi_publisher_get_participant(_gapi_self);
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
        OS_REPORT(OS_ERROR,
                  "DDS::Publisher_impl::get_participant", 0,
                  "Invalid Participant");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::get_participant", 0,
                "Unable to obtain userdata");
    }
  }
  return domainParticipant;
}

DDS::ReturnCode_t
DDS::Publisher_impl::set_default_datawriter_qos (
  const DDS::DataWriterQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataWriterQos * gapi_dwqos = gapi_dataWriterQos__alloc();
  if (gapi_dwqos)
  {
    ccpp_DataWriterQos_copyIn( qos, *gapi_dwqos);
    result = gapi_publisher_set_default_datawriter_qos(_gapi_self, gapi_dwqos);
    gapi_free(gapi_dwqos);
  }
  else
  {
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::set_default_datawriter_qos", 0,
              "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t
DDS::Publisher_impl::get_default_datawriter_qos (
  DDS::DataWriterQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_dataWriterQos * gapi_dwqos = gapi_dataWriterQos__alloc();
  if (gapi_dwqos)
  {
    result = gapi_publisher_get_default_datawriter_qos(_gapi_self, gapi_dwqos);
    ccpp_DataWriterQos_copyOut(*gapi_dwqos, qos);
    gapi_free(gapi_dwqos);
  }
  else
  {
    OS_REPORT(OS_ERROR,
              "DDS::Publisher_impl::get_default_datawriter_qos", 0,
              "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}

DDS::ReturnCode_t
DDS::Publisher_impl::copy_from_topic_qos (
  DDS::DataWriterQos & a_datawriter_qos,
  const DDS::TopicQos & a_topic_qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    gapi_dataWriterQos * gapi_dwqos = gapi_dataWriterQos__alloc();
    gapi_topicQos * gapi_tqos = gapi_topicQos__alloc();

    if (gapi_dwqos && gapi_tqos)
    {
      ccpp_TopicQos_copyIn(a_topic_qos, *gapi_tqos);
      ccpp_DataWriterQos_copyIn(a_datawriter_qos, *gapi_dwqos);
      result = gapi_publisher_copy_from_topic_qos(_gapi_self, gapi_dwqos, gapi_tqos);
      if (result == DDS::RETCODE_OK)
      {
        ccpp_DataWriterQos_copyOut(*gapi_dwqos, a_datawriter_qos);
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::Publisher_impl::copy_from_topic_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    if (gapi_dwqos)
    {
      gapi_free(gapi_dwqos);
    }
    if (gapi_tqos)
    {
      gapi_free(gapi_tqos);
    }
    return result;
}

