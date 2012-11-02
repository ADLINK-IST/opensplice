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
#include "ccpp_DomainParticipant_impl.h"
#include "ccpp_QosUtils.h"
#include "ccpp_Utils.h"
#include "ccpp_Publisher_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_Topic_impl.h"
#include "ccpp_TopicDescription_impl.h"
#include "ccpp_ContentFilteredTopic_impl.h"
#include "ccpp_MultiTopic_impl.h"
#include "ccpp_ListenerUtils.h"
#include "os_report.h"
#include "dds_builtinTopicsSplDcps.h"

extern "C" {
#include "v_public.h"
#include "v_dataReaderInstance.h"
#include "u_instanceHandle.h"
}

DDS::DomainParticipant_impl::DomainParticipant_impl(gapi_domainParticipant handle) : DDS::Entity_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&dp_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::DomainParticipant_impl::~DomainParticipant_impl()
{
  if (os_mutexDestroy(&dp_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::Publisher_ptr DDS::DomainParticipant_impl::create_publisher (
  const DDS::PublisherQos & qos,
  DDS::PublisherListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    gapi_publisherQos * gapi_pqos;
    gapi_publisher handle;
    DDS::Publisher_ptr publisher = NULL;
    DDS::ccpp_UserData_ptr myUD;
    gapi_publisherListener * gapi_listener = NULL;
    CORBA::Boolean allocatedQos = false;
    CORBA::Boolean proceed = true;

    if (a_listener)
    {
      gapi_listener = gapi_publisherListener__alloc();
      if (gapi_listener)
      {
        ccpp_PublisherListener_copyIn(a_listener, *gapi_listener);
      }
      else
      {
        proceed = false;
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      }
    }

    if (proceed)
    {
      if (&qos == DDS::DefaultQos::PublisherQosDefault)
      {
        gapi_pqos = GAPI_PUBLISHER_QOS_DEFAULT;
      }
      else
      {
        gapi_pqos = gapi_publisherQos__alloc();
        if (gapi_pqos)
        {
          allocatedQos = true;
          ccpp_PublisherQos_copyIn(qos, *gapi_pqos);
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
      handle = gapi_domainParticipant_create_publisher(
                    _gapi_self,
                    gapi_pqos,
                    gapi_listener,
                    mask);
      if (handle)
      {
        publisher = new Publisher_impl(handle);
        if (publisher)
        {
          myUD = new ccpp_UserData(publisher, a_listener);
          if (myUD)
          {
            gapi_domainParticipantQos *dpqos = gapi_domainParticipantQos__alloc();
            gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
            if(dpqos){
                if(gapi_domainParticipant_get_qos(_gapi_self, dpqos) == GAPI_RETCODE_OK){
                    if(dpqos->entity_factory.autoenable_created_entities) {
                        gapi_entity_enable(handle);
                    }
                }
                else
                {
                    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain domainParticipantQos");
                }
                gapi_free(dpqos);
            }
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

    if (allocatedQos)
    {
      gapi_free(gapi_pqos);
    }

    if (gapi_listener)
    {
      gapi_free(gapi_listener);
    }

    return publisher;
}


DDS::ReturnCode_t DDS::DomainParticipant_impl::delete_publisher (
  DDS::Publisher_ptr p
 ) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    DDS::Publisher_impl_ptr pub;

    pub = dynamic_cast<DDS::Publisher_impl_ptr>(p);
    if (pub)
    {
      if (os_mutexLock(&(pub->p_mutex)) == os_resultSuccess)
      {
        result = gapi_domainParticipant_delete_publisher(_gapi_self, pub->_gapi_self);
        if (result == DDS::RETCODE_OK)
        {
          pub->_gapi_self = NULL;
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete publisher");
        }
        if (os_mutexUnlock(&(pub->p_mutex)) != os_resultSuccess)
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

DDS::Subscriber_ptr DDS::DomainParticipant_impl::create_subscriber (
  const DDS::SubscriberQos & qos,
  DDS::SubscriberListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    gapi_subscriberQos * gapi_sqos;
    gapi_subscriber handle;
    DDS::Subscriber_impl_ptr subscriber= NULL;
    DDS::ccpp_UserData_ptr myUD;
    gapi_subscriberListener * gapi_listener = NULL;
    CORBA::Boolean allocatedQos = false;
    CORBA::Boolean proceed = true;

    if (a_listener)
    {
      gapi_listener = gapi_subscriberListener__alloc();
      if (gapi_listener)
      {
        ccpp_SubscriberListener_copyIn(a_listener, *gapi_listener);
      }
      else
      {
        proceed = false;
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
      }
    }

    if (proceed)
    {
      if (&qos == DDS::DefaultQos::SubscriberQosDefault)
      {
        gapi_sqos = GAPI_SUBSCRIBER_QOS_DEFAULT;
      }
      else
      {
        gapi_sqos = gapi_subscriberQos__alloc();
        if (gapi_sqos)
        {
          allocatedQos = true;
          ccpp_SubscriberQos_copyIn(qos, *gapi_sqos);
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
      handle = gapi_domainParticipant_create_subscriber(
                _gapi_self,
                gapi_sqos,
                gapi_listener,
                mask);
      if (handle)
      {
        subscriber = new Subscriber_impl(handle);
        if (subscriber)
        {
          myUD = new ccpp_UserData(subscriber, a_listener);
          if (myUD)
          {
            gapi_domainParticipantQos *dpqos = gapi_domainParticipantQos__alloc();
            gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
            if(dpqos){
                if(gapi_domainParticipant_get_qos(_gapi_self, dpqos) == GAPI_RETCODE_OK){
                    if(dpqos->entity_factory.autoenable_created_entities) {
                        gapi_entity_enable(handle);
                    }
                }
                else
                {
                    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain domainParticipantQos");
                }
                gapi_free(dpqos);
            }
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

    if (allocatedQos)
    {
      gapi_free(gapi_sqos);
    }

    if (gapi_listener)
    {
      gapi_free(gapi_listener);
    }
    return subscriber;
}

DDS::ReturnCode_t DDS::DomainParticipant_impl::delete_subscriber (
  DDS::Subscriber_ptr s
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    DDS::Subscriber_impl_ptr sub;

    sub = dynamic_cast<DDS::Subscriber_impl_ptr>(s);
    if (sub)
    {
      if (os_mutexLock(&(sub->s_mutex)) == os_resultSuccess)
      {
        result = gapi_domainParticipant_delete_subscriber(_gapi_self, sub->_gapi_self);
        if (result == DDS::RETCODE_OK)
        {
          sub->_gapi_self = NULL;
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete subscriber");
        }
        if (os_mutexUnlock(&(sub->s_mutex)) != os_resultSuccess)
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

DDS::Subscriber_ptr DDS::DomainParticipant_impl::get_builtin_subscriber (
) THROW_ORB_EXCEPTIONS
{
    gapi_subscriber handle;
    DDS::ccpp_UserData_ptr myUD;
    DDS::Subscriber_impl_ptr subscriber;

    handle = gapi_domainParticipant_get_builtin_subscriber(_gapi_self);
    if (os_mutexLock(&dp_mutex) == os_resultSuccess)
    {
      myUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
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
        subscriber = new DDS::Subscriber_impl(handle);
        if (subscriber)
        {
          myUD = new ccpp_UserData(subscriber,NULL);
          if (myUD)
          {
            gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
            if ( !initializeBuiltinTopicEntities(handle) )
            {
              OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create BuiltinTopic entities");
              CORBA::release(subscriber);
              subscriber = NULL;
              delete myUD;
            }
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
      if (os_mutexUnlock(&dp_mutex) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }

    return subscriber;
}

DDS::Topic_ptr DDS::DomainParticipant_impl::create_topic (
  const char * topic_name,
  const char * type_name,
  const DDS::TopicQos & qos,
  DDS::TopicListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    gapi_topicQos * gapi_tqos;
    gapi_topic handle;
    DDS::Topic_ptr topic = NULL;
    gapi_topicListener * gapi_listener=NULL;
    CORBA::Boolean allocatedQos = false;
    CORBA::Boolean proceed = true;

    if (a_listener)
    {
       if (mask & DDS::ALL_DATA_DISPOSED_TOPIC_STATUS
           && dynamic_cast<DDS::ExtTopicListener *>(a_listener) == NULL)
       {
         proceed = false;
         OS_REPORT( OS_ERROR, "CCPP", ERRORCODE_INCONSISTENT_VALUE,
                     "ExtTopicListener subclass must be used when the "
                        "ALL_DATA_DISPOSED_STATUS is set" );
       }

       if (proceed)
       {
          gapi_listener = gapi_topicListener__alloc();
          if (gapi_listener)
          {
             ccpp_TopicListener_copyIn(a_listener, *gapi_listener);
          }
          else
          {
             proceed = false;
             OS_REPORT(OS_API_INFO, "CCPP", ERRORCODE_OUT_OF_RESOURCES,
                       "DomainParticipant::create_topic :"
                          " Unable to allocate memory for listener");
          }
       }
    }

    if (proceed)
    {
      if (&qos == DDS::DefaultQos::TopicQosDefault)
      {
        gapi_tqos = GAPI_TOPIC_QOS_DEFAULT;
      }
      else
      {
        gapi_tqos = gapi_topicQos__alloc();
        if (gapi_tqos)
        {
          allocatedQos = true;
          ccpp_TopicQos_copyIn(qos, *gapi_tqos);
        }
        else
        {
          proceed = false;
          OS_REPORT(OS_API_INFO, "CCPP", ERRORCODE_OUT_OF_RESOURCES ,
              "DomainParticipant::create_topic : Unable to allocate memory for QoS");
        }
      }
    }

    if (proceed)
    {
      handle = gapi_domainParticipant_create_topic(
                    _gapi_self,
                    topic_name,
                    type_name,
                    gapi_tqos,
                    gapi_listener,
                    mask);
      if (handle)
      {
        topic = new DDS::Topic_impl(handle);
        if (topic)
        {
          DDS::ccpp_UserData_ptr myUD;
          myUD = new DDS::ccpp_UserData(topic, a_listener);
          if (myUD)
          {
              gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                        DDS::ccpp_CallBack_DeleteUserData,NULL);
          }
          else
          {
            OS_REPORT(OS_API_INFO, "CCPP", ERRORCODE_OUT_OF_RESOURCES ,
              "DomainParticipant::create_topic : Unable to allocate memory");
          }
        }
        else
        {
            OS_REPORT(OS_API_INFO, "CCPP", ERRORCODE_OUT_OF_RESOURCES,
                "DomainParticipant::create_topic : Unable to allocate memory for topic");
        }
      }
    }

    if (allocatedQos)
    {
      gapi_free(gapi_tqos);
    }

    if (gapi_listener)
    {
      gapi_free(gapi_listener);
    }
    return topic;
}

DDS::ReturnCode_t DDS::DomainParticipant_impl::delete_topic (
  DDS::Topic_ptr a_topic
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  DDS::Topic_impl_ptr topic;

  topic = dynamic_cast<DDS::Topic_impl_ptr>(a_topic);
  if (topic)
  {
    if (os_mutexLock(&(topic->t_mutex)) == os_resultSuccess)
    {
      result = gapi_domainParticipant_delete_topic(_gapi_self, topic->_gapi_self);
      if (result == DDS::RETCODE_OK)
      {
//        topic->_gapi_self = NULL;
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to delete topic");
      }
      if (os_mutexUnlock(&(topic->t_mutex)) != os_resultSuccess)
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

DDS::Topic_ptr DDS::DomainParticipant_impl::find_topic (
  const char * topic_name,
  const DDS::Duration_t & timeout
) THROW_ORB_EXCEPTIONS
{
  gapi_duration_t gapi_timeout;
  gapi_topic handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::Topic_ptr result = NULL;

  ccpp_Duration_copyIn( timeout, gapi_timeout);
  handle = gapi_domainParticipant_find_topic(_gapi_self, topic_name, &gapi_timeout);

  if (handle)
  {
    if (os_mutexLock(&dp_mutex) == os_resultSuccess)
    {
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (myUD)
      {
        result = dynamic_cast<DDS::Topic_impl_ptr>(myUD->ccpp_object);
        if (result)
        {
          DDS::Topic::_duplicate(result);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Topic");
        }
      }
      else
      {
        result = new DDS::Topic_impl(handle);
        if (result)
        {
          DDS::ccpp_UserData_ptr myUD;
          myUD = new DDS::ccpp_UserData(result, NULL);
          if (myUD)
          {
            gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
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
      if (os_mutexUnlock(&dp_mutex) != os_resultSuccess)
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


DDS::TopicDescription_ptr DDS::DomainParticipant_impl::lookup_topicdescription (
  const char * name
) THROW_ORB_EXCEPTIONS
{
  DDS::TopicDescription_ptr result = NULL;

  if (os_mutexLock(&dp_mutex) == os_resultSuccess)
  {
    result = unprotected_lookup_topicdescription(name);

    if (os_mutexUnlock(&dp_mutex) != os_resultSuccess)
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


DDS::TopicDescription_ptr
DDS::DomainParticipant_impl::unprotected_lookup_topicdescription (
  const char * name
)
{
  gapi_topic handle;
  DDS::ccpp_UserData_ptr myUD;
  DDS::TopicDescription_ptr result = NULL;

  handle = gapi_domainParticipant_lookup_topicdescription(_gapi_self, name);

  if (handle)
  {
    myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
    if (myUD)
    {
      result = dynamic_cast<DDS::TopicDescription_impl_ptr>(myUD->ccpp_object);
      if (result)
      {
        DDS::TopicDescription::_duplicate(result);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Topic Description");
      }
    }
    else
    {
      // If handle is found in GAPI, but has no UserData, then it has to be a builtin Topic.
      // That's how we know that a Topic_impl wrapper needs to be instantiated in this case.
      result = new DDS::Topic_impl(handle);
      if (result)
      {
        DDS::ccpp_UserData_ptr myUD;
        myUD = new DDS::ccpp_UserData(result, NULL);
        if (myUD)
        {
          gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                    DDS::ccpp_CallBack_DeleteUserData,NULL);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
        }
      }
    }
  }
  return result;
}


DDS::ContentFilteredTopic_ptr DDS::DomainParticipant_impl::create_contentfilteredtopic (
  const char * name,
  DDS::Topic_ptr related_topic,
  const char * filter_expression,
  const DDS::StringSeq & filter_parameters
) THROW_ORB_EXCEPTIONS
{
  gapi_stringSeq *gapi_filter_parameters;
  DDS::ccpp_UserData_ptr myUD;
  gapi_contentFilteredTopic handle;
  DDS::ContentFilteredTopic_ptr result = NULL;
  DDS::Topic_impl_ptr topicImpl;

  topicImpl = dynamic_cast<DDS::Topic_impl_ptr>(related_topic);
  if (topicImpl) {
    gapi_filter_parameters = gapi_stringSeq__alloc();
    if (gapi_filter_parameters)
    {
      ccpp_sequenceCopyIn(filter_parameters, *gapi_filter_parameters);
      handle = gapi_domainParticipant_create_contentfilteredtopic(
          _gapi_self,
          name,
          topicImpl->__gapi_self,
          filter_expression,
          gapi_filter_parameters);
      gapi_free(gapi_filter_parameters);

      if (handle)
      {
        result = new DDS::ContentFilteredTopic_impl(handle);
        if (result)
        {
          myUD = new DDS::ccpp_UserData(result);
          if (myUD)
          {
            gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
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
    } else {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    }
  } else {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Topic parameter.");
  }
  return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::delete_contentfilteredtopic (
  DDS::ContentFilteredTopic_ptr a_contentfilteredtopic
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  gapi_contentFilteredTopic handle;
  DDS::ContentFilteredTopic_impl_ptr contentFilteredTopic;

  contentFilteredTopic = dynamic_cast<DDS::ContentFilteredTopic_impl_ptr>(a_contentfilteredtopic);

  if (contentFilteredTopic)
  {
    handle = contentFilteredTopic->__gapi_self;
    if (os_mutexLock(&(contentFilteredTopic->cft_mutex)) == os_resultSuccess )
    {
      result = gapi_domainParticipant_delete_contentfilteredtopic(_gapi_self, handle);
      if (result == DDS::RETCODE_OK)
      {
        contentFilteredTopic->__gapi_self = NULL;
      }
      else
      {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipant_impl::delete_contentfilteredtopic", 0,
                    "Unable to obtain userdata");
      }
      if ( os_mutexUnlock(&(contentFilteredTopic->cft_mutex)) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::delete_contentfilteredtopic", 0,
                  "Unable to release lock");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::delete_contentfilteredtopic", 0,
                "Unable to obtain lock");
    }
  }
  return result;
}

DDS::MultiTopic_ptr
DDS::DomainParticipant_impl::create_multitopic (
  const char * name,
  const char * type_name,
  const char * subscription_expression,
  const DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
  DDS::ccpp_UserData_ptr myUD;
  gapi_multiTopic handle;
  DDS::MultiTopic_ptr result = NULL;

  gapi_stringSeq * gapi_expression_parameters = gapi_stringSeq__alloc();
  if (gapi_expression_parameters)
  {
    ccpp_sequenceCopyIn(expression_parameters, *gapi_expression_parameters);
    handle = gapi_domainParticipant_create_multitopic(
                _gapi_self,
                name,
                type_name,
                subscription_expression,
                gapi_expression_parameters);

    if (handle)
    {
      result = new DDS::MultiTopic_impl(handle);
      if (result)
      {
        myUD = new DDS::ccpp_UserData(result);
        if (myUD)
        {
          gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                    DDS::ccpp_CallBack_DeleteUserData,NULL);
        }
        else
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipant_impl::create_multitopic", 0,
                    "Unable to allocate memory");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::create_multitopic", 0,
                  "Unable to allocate memory");
      }
    }
    gapi_free(gapi_expression_parameters);
  }
  else
  {
    OS_REPORT(OS_ERROR,
              "CCPP", 0,
              "Unable to allocate memory");
  }

  return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::delete_multitopic (
  DDS::MultiTopic_ptr a_multitopic
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  gapi_multiTopic handle;
  DDS::MultiTopic_impl_ptr multiTopic;

  multiTopic = dynamic_cast<DDS::MultiTopic_impl_ptr>(a_multitopic);
  if (multiTopic)
  {
    handle = multiTopic->__gapi_self;
    if (os_mutexLock(&(multiTopic->mt_mutex)) == os_resultSuccess)
    {
      result = gapi_domainParticipant_delete_multitopic(_gapi_self, handle);

      if (result == DDS::RETCODE_OK)
      {
        multiTopic->__gapi_self = NULL;
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::delete_multitopic", 0,
                  "Unable to delete multitopic");
      }
      if (os_mutexUnlock(&(multiTopic->mt_mutex)) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::delete_multitopic", 0,
                  "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::delete_multitopic", 0,
                "Unable to obtain lock");
    }
  }
  return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
  return gapi_domainParticipant_delete_contained_entities(_gapi_self);
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::set_qos (
  const DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    if (&qos == DDS::DefaultQos::ParticipantQosDefault)
    {
      result = gapi_domainParticipant_set_qos(_gapi_self, GAPI_PARTICIPANT_QOS_DEFAULT);
    }
    else
    {
      gapi_domainParticipantQos * gapi_dpqos = gapi_domainParticipantQos__alloc();
      if (gapi_dpqos)
      {
        ccpp_DomainParticipantQos_copyIn ( qos, *gapi_dpqos );
        result = gapi_domainParticipant_set_qos(_gapi_self, gapi_dpqos);
        gapi_free(gapi_dpqos);
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::set_qos", 0,
                  "Unable to allocate memory");
        result = DDS::RETCODE_OUT_OF_RESOURCES;
      }
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_qos (
  DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_domainParticipantQos * gapi_dpqos = gapi_domainParticipantQos__alloc();
    if (gapi_dpqos)
    {
      result = gapi_domainParticipant_get_qos ( _gapi_self, gapi_dpqos);
      ccpp_DomainParticipantQos_copyOut ( *gapi_dpqos, qos );
      gapi_free(gapi_dpqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::get_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;

}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::set_listener (
  DDS::DomainParticipantListener_ptr a_listener,
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
  DDS::ccpp_UserData_ptr myUD;
  gapi_domainParticipantListener gapi_listener;

  if (mask & ALL_DATA_DISPOSED_TOPIC_STATUS
      && dynamic_cast<DDS::ExtDomainParticipantListener *>(a_listener) != NULL)
  {
     result = DDS::RETCODE_BAD_PARAMETER;
     OS_REPORT(OS_ERROR,
               "DDS::DomainParticipant_impl::set_listener", 0,
               "ExtDomainParticipantListener must be used when"
               " the ALL_DATA_DISPOSED_STATUS bit is set" );
  }
  else
  {
     ccpp_DomainParticipantListener_copyIn(a_listener, gapi_listener);
     if (os_mutexLock(&dp_mutex) == os_resultSuccess)
     {
        result = gapi_domainParticipant_set_listener(_gapi_self, &gapi_listener, mask);
        if (result == DDS::RETCODE_OK)
        {
           myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
           if (myUD)
           {
              myUD->setListener(a_listener);
           }
           else
           {
              OS_REPORT(OS_ERROR,
                        "DDS::DomainParticipant_impl::set_listener", 0,
                        "Unable to obtain userdata");
           }
        }
        if (os_mutexUnlock(&dp_mutex) != os_resultSuccess)
        {
           OS_REPORT(OS_ERROR,
                     "DDS::DomainParticipant_impl::set_listener", 0,
                     "Unable to release mutex");
        }
     }
     else
     {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::set_listener", 0,
                  "Unable to obtain lock");
     }
  }
  return result;
}

DDS::DomainParticipantListener_ptr
DDS::DomainParticipant_impl::get_listener (
) THROW_ORB_EXCEPTIONS
{
  DDS::DomainParticipantListener_ptr result;
  gapi_domainParticipantListener gapi_listener;


  if (os_mutexLock(&dp_mutex) == os_resultSuccess)
  {
    gapi_listener = gapi_domainParticipant_get_listener(_gapi_self);
    result = reinterpret_cast<DDS::DomainParticipantListener_ptr>(gapi_listener.listener_data);
    if (result)
    {
      DDS::DomainParticipantListener::_duplicate(result);
    }
    if (os_mutexUnlock(&dp_mutex) != os_resultSuccess)
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::get_listener", 0,
                "Unable to release mutex");
    }
  }
  else
  {
    OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::get_listener", 0,
                "Unable to obtain lock");
  }
  return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::ignore_participant (
  DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
  return gapi_domainParticipant_ignore_participant(_gapi_self, handle);
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::ignore_topic (
  DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
  return gapi_domainParticipant_ignore_topic(_gapi_self, handle);
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::ignore_publication (
  DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
  return gapi_domainParticipant_ignore_publication(_gapi_self, handle);
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::ignore_subscription (
  DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
  return gapi_domainParticipant_ignore_subscription(_gapi_self, handle);
}

char *
DDS::DomainParticipant_impl::get_domain_id (
) THROW_ORB_EXCEPTIONS
{
  gapi_domainId_t di = gapi_domainParticipant_get_domain_id(_gapi_self);
  CORBA::String_var the_domain_id = CORBA::string_dup(di);
  gapi_free(di);
  return the_domain_id._retn ();
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::assert_liveliness (
) THROW_ORB_EXCEPTIONS
{
  return gapi_domainParticipant_assert_liveliness(_gapi_self);
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::set_default_publisher_qos (
  const DDS::PublisherQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_publisherQos * gapi_pqos = gapi_publisherQos__alloc();
    if (gapi_pqos)
    {
      ccpp_PublisherQos_copyIn(qos, *gapi_pqos);
      result = gapi_domainParticipant_set_default_publisher_qos(
        _gapi_self,
        gapi_pqos);
      gapi_free(gapi_pqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::set_default_publisher_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_default_publisher_qos (
  DDS::PublisherQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_publisherQos * gapi_pqos = gapi_publisherQos__alloc();
    if (gapi_pqos)
    {
      result = gapi_domainParticipant_get_default_publisher_qos(_gapi_self, gapi_pqos);
      ccpp_PublisherQos_copyOut(*gapi_pqos, qos);
      gapi_free(gapi_pqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::get_default_publisher_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::set_default_subscriber_qos (
  const DDS::SubscriberQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_subscriberQos * gapi_sqos = gapi_subscriberQos__alloc();
    if (gapi_sqos)
    {
      ccpp_SubscriberQos_copyIn(qos, *gapi_sqos);
      result = gapi_domainParticipant_set_default_subscriber_qos(
        _gapi_self,
        gapi_sqos);
      gapi_free(gapi_sqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::set_default_subscriber_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_default_subscriber_qos (
  DDS::SubscriberQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_subscriberQos * gapi_sqos = gapi_subscriberQos__alloc();
    if (gapi_sqos)
    {
      result = gapi_domainParticipant_get_default_subscriber_qos(_gapi_self, gapi_sqos);
      ccpp_SubscriberQos_copyOut(*gapi_sqos, qos);
      gapi_free(gapi_sqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::get_default_subscriber_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::set_default_topic_qos (
  const DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_topicQos * gapi_tqos = gapi_topicQos__alloc();
    if (gapi_tqos)
    {
      ccpp_TopicQos_copyIn(qos, *gapi_tqos);
      result = gapi_domainParticipant_set_default_topic_qos(
        _gapi_self,
        gapi_tqos);
      gapi_free(gapi_tqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::set_default_topic_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_default_topic_qos (
   DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_topicQos * gapi_tqos = gapi_topicQos__alloc();
    if (gapi_tqos)
    {
      result = gapi_domainParticipant_get_default_topic_qos(_gapi_self, gapi_tqos);
      ccpp_TopicQos_copyOut(*gapi_tqos, qos);
      gapi_free(gapi_tqos);
   }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipant_impl::get_default_topic_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

struct copyInstanceHandle {
    c_ulong index;
    DDS::InstanceHandleSeq *seq;
};

static c_bool
copyInstanceHandle(
    v_dataReaderInstance instance,
    c_voidp arg)
{
    c_bool result = TRUE;
    struct copyInstanceHandle *a = (struct copyInstanceHandle *)arg;
    DDS::InstanceHandle_t ghandle;
    os_uint32 length;
    if (a->index ==0) {
        length = c_count(v_dataReaderInstanceGetNotEmptyInstanceSet(instance));
        if (length > a->seq->maximum()) {
            a->seq->length(length); /* potentially reallocate */
        }
    }

    ghandle = u_instanceHandleNew((v_public)instance);
    if (a->index < a->seq->maximum()) {
        (*a->seq)[a->index++] = ghandle;

    } else {
        /* error index out of bounds */
    }

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_discovered_participants (
    DDS::InstanceHandleSeq & participant_handles
) THROW_ORB_EXCEPTIONS
{

    struct copyInstanceHandle cih;
    cih.index = 0;
    cih.seq = &participant_handles;

    DDS::ReturnCode_t result;

    result = gapi_domainParticipant_get_discovered_participants(_gapi_self, copyInstanceHandle, &cih);

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_discovered_participant_data (
    DDS::ParticipantBuiltinTopicData & participant_data,
    DDS::InstanceHandle_t participant_handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_participantBuiltinTopicData gapi_data;

    result = (DDS::ReturnCode_t)gapi_domainParticipant_get_discovered_participant_data(_gapi_self,
                                                                    &participant_data,
                                                                    participant_handle,
                                                                    __DDS_ParticipantBuiltinTopicData__copyOut);
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_discovered_topics (
    DDS::InstanceHandleSeq & topic_handles
) THROW_ORB_EXCEPTIONS
{

    struct copyInstanceHandle cih;
    cih.index = 0;
    cih.seq = &topic_handles;

    DDS::ReturnCode_t result;

    result = gapi_domainParticipant_get_discovered_topics(_gapi_self, copyInstanceHandle, &cih);

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_discovered_topic_data (
    DDS::TopicBuiltinTopicData & topic_data,
    DDS::InstanceHandle_t topic_handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_participantBuiltinTopicData gapi_data;

    result = (DDS::ReturnCode_t)gapi_domainParticipant_get_discovered_topic_data(_gapi_self,
                                                                    &topic_data,
                                                                    topic_handle,
                                                                    __DDS_TopicBuiltinTopicData__copyOut);
    return result;
}

CORBA::Boolean
DDS::DomainParticipant_impl::contains_entity (
    DDS::InstanceHandle_t a_handle
) THROW_ORB_EXCEPTIONS
{
    CORBA::Boolean result = false;

    if (gapi_domainParticipant_contains_entity(_gapi_self, a_handle))
    {
        result = true;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::get_current_time (
    DDS::Time_t & current_time
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_time_t gapi_time_stamp;

    result = gapi_domainParticipant_get_current_time(_gapi_self, &gapi_time_stamp);
    if (result == DDS::RETCODE_OK)
    {
        ccpp_TimeStamp_copyOut(gapi_time_stamp, current_time);
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipant_impl::initializeBuiltinTopics()
{
    DDS::ReturnCode_t status;
    ParticipantBuiltinTopicDataTypeSupport participantTS;
    TopicBuiltinTopicDataTypeSupport topicTS;
    PublicationBuiltinTopicDataTypeSupport publicationTS;
    SubscriptionBuiltinTopicDataTypeSupport subscriptionTS;

    status = participantTS.register_type(this, NULL);
    if (!status)
    {
        status = topicTS.register_type(this, NULL);
        if (!status)
        {
            status = publicationTS.register_type(this, NULL);
            if (!status)
            {
                status = subscriptionTS.register_type(this, NULL);
            }
        }
    }
    return status;
}

bool
DDS::DomainParticipant_impl::initializeBuiltinTopicEntities(
    gapi_subscriber handle
)
{
    bool status = false;
    DDS::TopicDescription_var a_topic;

    a_topic = unprotected_lookup_topicdescription(PARTICIPANT_BUILTINTOPIC_NAME);
    if ( a_topic.in() )
    {
        a_topic = unprotected_lookup_topicdescription(TOPIC_BUILTINTOPIC_NAME);
        if ( a_topic.in() )
        {
            a_topic = unprotected_lookup_topicdescription(PUBLICATION_BUILTINTOPIC_NAME);
            if ( a_topic.in() )
            {
                a_topic = unprotected_lookup_topicdescription(SUBSCRIPTION_BUILTINTOPIC_NAME);
                if ( a_topic.in() )
                {
                    status = initializeBuiltinReaders(handle);
                }
            }
        }
    }

    return status;
}

bool
DDS::DomainParticipant_impl::initializeBuiltinReaders(
    gapi_subscriber handle
)
{
    bool status = false;

    if ( createBuiltinReader(handle, PARTICIPANT_BUILTINTOPIC_NAME) )
    {
        if ( createBuiltinReader(handle, TOPIC_BUILTINTOPIC_NAME) )
        {
            if ( createBuiltinReader(handle, PUBLICATION_BUILTINTOPIC_NAME) )
            {
                status = createBuiltinReader(handle, SUBSCRIPTION_BUILTINTOPIC_NAME);
                if (!status)
                {
                    OS_REPORT(OS_ERROR,
                              "DDS::DomainParticipant_impl::initializeBuiltinReaders", 0,
                              "Create Builtin subscription reader Failed");
                }
            } else {
                OS_REPORT(OS_ERROR,
                          "DDS::DomainParticipant_impl::initializeBuiltinReaders", 0,
                          "Create Builtin publication reader Failed");
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipant_impl::initializeBuiltinReaders", 0,
                      "Create Builtin topic reader Failed");
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::initializeBuiltinReaders", 0,
                  "Create Builtin Participant Reader Failed");
    }

    return status;
}

bool
DDS::DomainParticipant_impl::createBuiltinReader(
    gapi_subscriber subscriber_handle,
    const char *name
)
{
    char *type_name;
    bool status = false;
    gapi_dataReader reader_handle = NULL;
    gapi_topic topic_handle;
    DDS::DataReader_ptr a_reader = NULL;
    DDS::ccpp_UserData_ptr myUD = NULL;
    gapi_typeSupport ts_handle = NULL;

    reader_handle = gapi_subscriber_lookup_datareader(subscriber_handle, name);
    if (reader_handle)
    {
        topic_handle = gapi_dataReader_get_topicdescription(reader_handle);
        if (topic_handle)
        {
            type_name = gapi_topicDescription_get_type_name(topic_handle);
            ts_handle = gapi_domainParticipant_get_typesupport(_gapi_self, type_name);
            gapi_free(type_name);
            if (ts_handle)
            {
                void *tsf = gapi_object_get_user_data(ts_handle);

                if (tsf)
                {
                    CORBA::Object_ptr anObject;
                    DDS::TypeSupportFactory_impl_ptr factory;

                    anObject = static_cast<CORBA::Object_ptr>(tsf);
                    factory = dynamic_cast<DDS::TypeSupportFactory_impl_ptr>(anObject);
                    if (factory)
                    {
                        a_reader = factory->create_datareader(reader_handle);
                    }

                    if (a_reader)
                    {
                        myUD = new ccpp_UserData(a_reader, NULL);
                        if (myUD)
                        {
                            gapi_object_set_user_data(reader_handle,
                                                      (CORBA::Object *)myUD,
                                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
                            status = true;
                        }
                    }
                }
            } else {
                OS_REPORT_1(OS_ERROR,
                            "DDS::DomainParticipant_impl::createBuiltinReader",
                            0, "Lookup TypeSupport Failed for DataReader <%s>",
                            name);
            }
        } else {
            OS_REPORT_1(OS_ERROR,
                        "DDS::DomainParticipant_impl::createBuiltinReader",
                        0, "Lookup Topic Failed for DataReader <%s>", name);
        }
    } else if (name) {
        OS_REPORT_1(OS_ERROR,
                    "DDS::DomainParticipant_impl::createBuiltinReader",
                    0, "Lookup DataReader <%s> Failed", name);
    } else {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipant_impl::createBuiltinReader",
                  0, "Lookup DataReader <NULL> Failed");
    }
    return status;
}

