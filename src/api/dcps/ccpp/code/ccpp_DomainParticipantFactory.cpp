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
#include "ccpp.h"
#include "ccpp_dds_dcps.h"
#include "ccpp_DomainParticipantFactory.h"
#include "ccpp_DomainParticipant_impl.h"
#include "ccpp_Domain_impl.h"
#include "ccpp_ListenerUtils.h"
#include "ccpp_QosUtils.h"

class LocalFactoryMutex
{
  public:
    os_mutex dpf_mutex;

    LocalFactoryMutex()
    {
      os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
      if (os_mutexInit(&dpf_mutex, &mutexAttr) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "DDS::LocalFactoryMutex", 0,
                  "Unable to create mutex");
      }
    }
};

static LocalFactoryMutex localMutex;

DDS::DomainParticipantFactory::DomainParticipantFactory()
{
}

DDS::DomainParticipantFactory::~DomainParticipantFactory()
{
}

DDS::DomainParticipantFactory_ptr
DDS::DomainParticipantFactory::get_instance(
) THROW_ORB_EXCEPTIONS
{

    ccpp_UserData_ptr myUD = NULL;
    DomainParticipantFactory_ptr singletonSelf = NULL;

    if (os_mutexLock(&(localMutex.dpf_mutex)) == os_resultSuccess)
    {
      if (_gapi_self != NULL)
      {
        if (os_mutexUnlock(&(localMutex.dpf_mutex)) == os_resultSuccess)
        {
          CORBA::Object * data = (CORBA::Object *)gapi_object_get_user_data(_gapi_self);
          myUD = dynamic_cast<ccpp_UserData_ptr>(data);
          if (myUD)
          {
             singletonSelf = dynamic_cast<DomainParticipantFactory_ptr>(myUD->ccpp_object);
             if (singletonSelf == NULL)
             {
               OS_REPORT(OS_ERROR,
                         "DDS::DomainParticipantFactory::get_instance", 0,
                         "Invalid Domain Participant Factory");
             }
          }
          else
          {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipantFactory::get_instance", 0,
                      "Unable to obtain userdata");
          }
        }
        else
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipantFactory::get_instance", 0,
                    "Unable to release mutex");
        }
      }
      else
      {
        _gapi_self = gapi_domainParticipantFactory_get_instance ();
        if (_gapi_self)
        {
          singletonSelf = new DomainParticipantFactory();
          if (singletonSelf)
          {
            myUD = new ccpp_UserData(singletonSelf);
            if (myUD)
            {
              gapi_object_set_user_data(_gapi_self, (CORBA::Object *)myUD,
                                        DDS::ccpp_CallBack_DeleteUserData,NULL);
            }
            else
            {
              OS_REPORT(OS_ERROR,
                        "DDS::DomainParticipantFactory::get_instance", 0,
                        "Unable to allocate memory");
            }
          }
          else
          {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipantFactory::get_instance", 0,
                      "Unable to allocate memory");
          }
        }
        if (os_mutexUnlock(&(localMutex.dpf_mutex)) != os_resultSuccess)
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipantFactory::get_instance", 0,
                    "Unable to release mutex");
        }
      }
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipantFactory::get_instance", 0,
                "Unable to obtain mutex");
    }
    return DomainParticipantFactoryInterface::_duplicate(singletonSelf);
}

DDS::DomainParticipant_ptr
DDS::DomainParticipantFactory::create_participant (
    const char * domainId,
    const DDS::DomainParticipantQos & qos,
    DDS::DomainParticipantListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    gapi_domainParticipant handle = NULL;
    DDS::DomainParticipant_impl_ptr myParticipant = NULL;
    gapi_domainParticipantQos * gapi_dpqos;
    gapi_domainParticipantListener * gapi_listener = NULL;
    CORBA::Boolean allocatedQos = false;

    if (a_listener)
    {
      gapi_listener = gapi_domainParticipantListener__alloc();
      if (gapi_listener)
      {
        ccpp_DomainParticipantListener_copyIn(a_listener, *gapi_listener);
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipantFactory::create_participant", 0,
                  "Unable to allocate memory");
      }
    }

    if (&qos == DDS::DefaultQos::ParticipantQosDefault)
    {
      gapi_dpqos = GAPI_PARTICIPANT_QOS_DEFAULT;
    }
    else
    {
      gapi_dpqos = gapi_domainParticipantQos__alloc();
      if (gapi_dpqos)
      {
        allocatedQos = true;
        ccpp_DomainParticipantQos_copyIn ( qos, *gapi_dpqos );
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipantFactory::create_participant", 0,
                  "Unable to allocate memory");
      }
    }

    handle = gapi_domainParticipantFactory_create_participant (
        _gapi_self,
        (gapi_domainId_t)domainId,
        gapi_dpqos,
        gapi_listener,
        mask,
        NULL, NULL, NULL);

    if (allocatedQos)
    {
      gapi_free(gapi_dpqos);
    }

    if (handle)
    {
        myParticipant = new DDS::DomainParticipant_impl(handle);
        if (myParticipant)
        {
          ccpp_UserData_ptr myUD = NULL;

          myUD = new ccpp_UserData(myParticipant, a_listener);
          if (myUD)
          {
            gapi_domainParticipantFactoryQos *dpfqos = gapi_domainParticipantFactoryQos__alloc();
            gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
            if(dpfqos){
                if(gapi_domainParticipantFactory_get_qos(_gapi_self, dpfqos) == GAPI_RETCODE_OK){
                    if(dpfqos->entity_factory.autoenable_created_entities) {
                        gapi_entity_enable(handle);
                    }
                }
                else
                {
                    OS_REPORT(OS_ERROR,
                              "DDS::DomainParticipantFactory::create_participant", 0,
                              "Unable to obtain domainParticipantFactoryQos");
                }
                gapi_free(dpfqos);
            }
            else
            {
                OS_REPORT(OS_ERROR,
                          "DDS::DomainParticipantFactory::create_participant", 0,
                          "Unable to allocate memory");
            }
          }
          else
          {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipantFactory::create_participant", 0,
                      "Unable to allocate memory");
          }
        }
        else
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipantFactory::create_participant", 0,
                    "Unable to allocate memory");
        }
    }

    if (myParticipant)
    {
      if ( myParticipant->initializeBuiltinTopics() != DDS::RETCODE_OK)
      {
        delete_participant(myParticipant);
        myParticipant = NULL;
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipantFactory::create_participant", 0,
                  "Unable to register TypeSupports for BuiltinTopics.");
      }
    }

    if (gapi_listener)
    {
      gapi_free(gapi_listener);
    }
    return myParticipant;
}



DDS::ReturnCode_t
DDS::DomainParticipantFactory::delete_participant (
    DDS::DomainParticipant_ptr a_participant
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t status = DDS::RETCODE_BAD_PARAMETER;
    DDS::DomainParticipant_impl_ptr myParticipant = NULL;

    myParticipant = dynamic_cast<DDS::DomainParticipant_impl_ptr>(a_participant);
    if (myParticipant)
    {
      gapi_domainParticipant handle = NULL;
      handle = myParticipant->_gapi_self;

      if (os_mutexLock(&(myParticipant->dp_mutex)) == os_resultSuccess)
      {
        status = gapi_domainParticipantFactory_delete_participant(_gapi_self, handle);
        if (status != DDS::RETCODE_OK)
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipantFactory::delete_participant", 0,
                    "Unable to delete Participant");
        }
        if (os_mutexUnlock(&(myParticipant->dp_mutex)) != os_resultSuccess)
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipantFactory::delete_participant", 0,
                    "Unable to release mutex");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR,
                  "DDS::DomainParticipantFactory::delete_participant", 0,
                  "Unable to obtain mutex");
        status = DDS::RETCODE_ERROR;
      }
    }
    return status;
}

DDS::DomainParticipant_ptr
DDS::DomainParticipantFactory::lookup_participant (
  const char * domainId
) THROW_ORB_EXCEPTIONS
{
    gapi_domainParticipant handle = NULL;
    DDS::DomainParticipant_impl_ptr myParticipant = NULL;

    handle = gapi_domainParticipantFactory_lookup_participant(
        _gapi_self,
        (gapi_domainId_t)domainId
     );
     if (handle)
     {
        ccpp_UserData_ptr myUD = NULL;

        myUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
        if (myUD)
        {
          myParticipant = dynamic_cast<DDS::DomainParticipant_impl_ptr>(myUD->ccpp_object);
          if (myParticipant)
          {
            DDS::DomainParticipant::_duplicate(myParticipant);
          }
          else
          {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipantFactory::lookup_participant", 0,
                      "Invalid Participant");
          }
        }
        else
        {
          OS_REPORT(OS_ERROR,
                    "DDS::DomainParticipantFactory::lookup_participant", 0,
                    "Unable to obtain userdata");
        }
     }
     return myParticipant;
}

DDS::ReturnCode_t DDS::DomainParticipantFactory::set_qos (
  const DDS::DomainParticipantFactoryQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    if (&qos == DDS::DefaultQos::ParticipantFactoryQosDefault)
    {
      result = gapi_domainParticipantFactory_set_qos(_gapi_self, GAPI_PARTICIPANTFACTORY_QOS_DEFAULT);
    }
    else
    {
      gapi_domainParticipantFactoryQos * gapi_dpfqos = gapi_domainParticipantFactoryQos__alloc();
      if (gapi_dpfqos)
      {
        ccpp_DomainParticipantFactoryQos_copyIn ( qos, *gapi_dpfqos );
        result = gapi_domainParticipantFactory_set_qos(_gapi_self, gapi_dpfqos);
        gapi_free(gapi_dpfqos);
      }
      else
      {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
      }
    }
    return result;
}


DDS::ReturnCode_t
DDS::DomainParticipantFactory::get_qos (
  DDS::DomainParticipantFactoryQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_domainParticipantFactoryQos * gapi_dpfqos = gapi_domainParticipantFactoryQos__alloc();
    if (gapi_dpfqos)
    {
      result = gapi_domainParticipantFactory_get_qos ( _gapi_self, gapi_dpfqos);
      ccpp_DomainParticipantFactoryQos_copyOut ( *gapi_dpfqos, qos );
      gapi_free(gapi_dpfqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipantFactory::get_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::Domain_ptr
DDS::DomainParticipantFactory::lookup_domain (
  const char * domainId
) THROW_ORB_EXCEPTIONS
{
    gapi_domainParticipant handle = NULL;
    DDS::Domain_impl_ptr myDomain = NULL;

    handle = gapi_domainParticipantFactory_lookup_domain (
        _gapi_self,
        (gapi_domainId_t)domainId
     );
     if (handle)
     {
        ccpp_UserData_ptr myUD = NULL;

        myUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
        if (myUD)
        {
          myDomain = dynamic_cast<DDS::Domain_impl_ptr>(myUD->ccpp_object);
          if (myDomain)
          {
            DDS::Domain::_duplicate(myDomain);
          }
          else
          {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipantFactory::lookup_domain", 0,
                      "Invalid Domain");
          }
        }
        else
        {
            myDomain = new DDS::Domain_impl(handle);
            if (myDomain)
            {
                ccpp_UserData_ptr myUD = NULL;

                myUD = new ccpp_UserData(myDomain);
                if (myUD)
                {
                    gapi_object_set_user_data(handle, (CORBA::Object *)myUD,
                                              DDS::ccpp_CallBack_DeleteUserData,NULL);
                } else
                {
                    OS_REPORT(OS_ERROR,
                              "DDS::DomainParticipantFactory::lookup_domain", 0,
                              "Unable to allocate memory");
                }
            } else
            {
                OS_REPORT(OS_ERROR,
                          "DDS::DomainParticipantFactory::lookup_domain", 0,
                          "Unable to allocate memory");
            }
        }
     }
     return myDomain;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::delete_domain (
    DDS::Domain_ptr a_domain
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t status = DDS::RETCODE_BAD_PARAMETER;
    DDS::Domain_impl_ptr myDomain = NULL;

    myDomain = dynamic_cast<DDS::Domain_impl_ptr>(a_domain);
    if (myDomain)
    {
        gapi_domain handle = myDomain->_gapi_self;
        status = gapi_domainParticipantFactory_delete_domain(_gapi_self, handle);
        if (status != DDS::RETCODE_OK)
        {
            OS_REPORT(OS_ERROR,
                      "DDS::DomainParticipantFactory::delete_domain", 0,
                      "Unable to delete Domain");
        }


    }
    return status;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::delete_contained_entities (
    ) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t status;

    status = gapi_domainParticipantFactory_delete_contained_entities(_gapi_self);

    return status;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::set_default_participant_qos (
    const DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_domainParticipantQos * gapi_dpqos = gapi_domainParticipantQos__alloc();
    if (gapi_dpqos)
    {
      ccpp_DomainParticipantQos_copyIn ( qos, *gapi_dpqos );
      result = gapi_domainParticipantFactory_set_default_participant_qos(
        _gapi_self,
        gapi_dpqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipantFactory::set_default_participant_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::get_default_participant_qos (
  DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    gapi_domainParticipantQos * gapi_dpqos = gapi_domainParticipantQos__alloc();
    if (gapi_dpqos)
    {
      result = gapi_domainParticipantFactory_get_default_participant_qos (
        _gapi_self,
        gapi_dpqos);
      ccpp_DomainParticipantQos_copyOut ( *gapi_dpqos, qos );
      gapi_free(gapi_dpqos);
    }
    else
    {
      OS_REPORT(OS_ERROR,
                "DDS::DomainParticipantFactory::get_default_participant_qos", 0,
                "Unable to allocate memory");
      result = DDS::RETCODE_OUT_OF_RESOURCES;
    }
    return result;
}

const DDS::DomainParticipantQos * const
DDS::DomainParticipantFactory::participant_qos_default (void)
{
    return DDS::DefaultQos::ParticipantQosDefault;
}

const DDS::TopicQos * const
DDS::DomainParticipantFactory::topic_qos_default (void)
{
    return DDS::DefaultQos::TopicQosDefault;
}

const DDS::PublisherQos * const
DDS::DomainParticipantFactory::publisher_qos_default (void)
{
    return DDS::DefaultQos::PublisherQosDefault;
}

const DDS::SubscriberQos * const
DDS::DomainParticipantFactory::subscriber_qos_default (void)
{
    return DDS::DefaultQos::SubscriberQosDefault;
}

const DDS::DataReaderQos * const
DDS::DomainParticipantFactory::datareader_qos_default (void)
{
    return DDS::DefaultQos::DataReaderQosDefault;
}

const DDS::DataReaderViewQos * const
DDS::DomainParticipantFactory::datareaderview_qos_default (void)
{
    return DDS::DefaultQos::DataReaderViewQosDefault;
}

const DDS::DataReaderQos * const
DDS::DomainParticipantFactory::datareader_qos_use_topic_qos (void)
{
    return DDS::DefaultQos::DataReaderQosUseTopicQos;
}

const DDS::DataWriterQos * const
DDS::DomainParticipantFactory::datawriter_qos_default (void)
{
    return DDS::DefaultQos::DataWriterQosDefault;
}

const DDS::DataWriterQos * const
DDS::DomainParticipantFactory::datawriter_qos_use_topic_qos (void)
{
    return DDS::DefaultQos::DataWriterQosUseTopicQos;
}


gapi_domainParticipantFactory DDS::DomainParticipantFactory::_gapi_self = NULL;
