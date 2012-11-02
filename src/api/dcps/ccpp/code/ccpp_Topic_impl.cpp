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
#include "ccpp_Topic_impl.h"
#include "ccpp_ListenerUtils.h"

DDS::Topic_impl::Topic_impl(gapi_topic handle) : DDS::TopicDescription_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&t_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::Topic_impl::~Topic_impl()
{
  if (os_mutexDestroy(&t_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::ReturnCode_t DDS::Topic_impl::get_inconsistent_topic_status (
    DDS::InconsistentTopicStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_inconsistentTopicStatus gapi_status;
  DDS::ReturnCode_t result;
  
  result = gapi_topic_get_inconsistent_topic_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_InconsistentTopicStatus_copyOut(gapi_status, a_status);
  }
  
  return result;
}

DDS::ReturnCode_t DDS::Topic_impl::get_all_data_disposed_topic_status (
    DDS::AllDataDisposedTopicStatus & a_status
) THROW_ORB_EXCEPTIONS
{
  gapi_allDataDisposedTopicStatus gapi_status;
  DDS::ReturnCode_t result;
  
  result = gapi_topic_get_all_data_disposed_topic_status( DDS::Entity_impl::_gapi_self,&gapi_status);
  if (result == DDS::RETCODE_OK)
  {
    ccpp_AllDataDisposedTopicStatus_copyOut(gapi_status, a_status);
  }
  
  return result;
}

DDS::ReturnCode_t DDS::Topic_impl::get_qos (
  DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  gapi_topicQos * gapi_tqos = gapi_topicQos__alloc();
  if (gapi_tqos)
  {
    result = gapi_topic_get_qos(DDS::Entity_impl::_gapi_self, gapi_tqos);
    ccpp_TopicQos_copyOut( *gapi_tqos, qos);
    gapi_free(gapi_tqos);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    result = DDS::RETCODE_OUT_OF_RESOURCES;
  }
  return result;
}
    
DDS::ReturnCode_t DDS::Topic_impl::set_qos (
  const DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  if (&qos == DDS::DefaultQos::TopicQosDefault)
  {
    result = gapi_topic_set_qos( _gapi_self, GAPI_TOPIC_QOS_DEFAULT );
  }
  else
  {
    gapi_topicQos * gapi_tqos= gapi_topicQos__alloc();
    if (gapi_tqos)
    {
      ccpp_TopicQos_copyIn( qos, *gapi_tqos);
      result = gapi_topic_set_qos( _gapi_self, gapi_tqos);
      gapi_free(gapi_tqos);
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    }
  }
  return result;
}

DDS::TopicListener_ptr DDS::Topic_impl::get_listener (
) THROW_ORB_EXCEPTIONS
{
  DDS::TopicListener_ptr result;
  gapi_topicListener gapi_listener;
  if (os_mutexLock(&t_mutex) == os_resultSuccess)
  {
    gapi_listener = gapi_topic_get_listener(_gapi_self);
    result = reinterpret_cast<DDS::TopicListener_ptr>(gapi_listener.listener_data);
    if (result)
    {
      DDS::TopicListener::_duplicate(result);
    }
    if (os_mutexUnlock(&t_mutex) != os_resultSuccess)
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
    }
  }
  else
  { 
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain lock");
  }
  return result;
}

DDS::ReturnCode_t DDS::Topic_impl::set_listener (
DDS::TopicListener_ptr a_listener,
DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_topicListener * gapi_listener = NULL;
    if (mask & ALL_DATA_DISPOSED_TOPIC_STATUS
        && dynamic_cast<DDS::ExtTopicListener *>(a_listener) == NULL)
    {
       result = DDS::RETCODE_BAD_PARAMETER;
       OS_REPORT( OS_ERROR, "CCPP", 0,
                  "ExtTopicListener subclass must be used when the "
                     "ALL_DATA_DISPOSED_STATUS is set" );
    }
    else
    {
       gapi_listener = gapi_topicListener__alloc();
       if (gapi_listener)
       {
          ccpp_TopicListener_copyIn(a_listener, *gapi_listener);
          if (os_mutexLock(&t_mutex) == os_resultSuccess)
          {
             result = gapi_topic_set_listener(_gapi_self, gapi_listener, mask);
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
             if (os_mutexUnlock(&t_mutex) != os_resultSuccess)
             {
                OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
             }
          }
          else
          {
             OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain lock");
          }
          gapi_free(gapi_listener);
       }
       else
       {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
       }
    }
    return result;
}

DDS::ReturnCode_t DDS::Topic_impl::dispose_all_data (
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;
  
  result = gapi_topic_dispose_all_data(DDS::Entity_impl::_gapi_self);
  
  return result;
}
