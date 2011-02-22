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
#include "ccpp_TopicDescription_impl.h"
#include "ccpp_DomainParticipantFactory.h"
#include "ccpp_Utils.h"

DDS::TopicDescription_impl::TopicDescription_impl(
    gapi_topicDescription handle
)  : DDS::Entity_impl (handle),
     __gapi_self(handle)
{
}

char * DDS::TopicDescription_impl::get_type_name (
) THROW_ORB_EXCEPTIONS
{
  char * gapi_result = gapi_topicDescription_get_type_name(__gapi_self);
  char * result = CORBA::string_dup(gapi_result);
  gapi_free(gapi_result);
  return result;
}

char * DDS::TopicDescription_impl::get_name (
) THROW_ORB_EXCEPTIONS
{
  char * gapi_result = gapi_topicDescription_get_name(__gapi_self);
  char * result = CORBA::string_dup(gapi_result);
  gapi_free(gapi_result);
  return result;
}

DDS::DomainParticipant_ptr DDS::TopicDescription_impl::get_participant (
) THROW_ORB_EXCEPTIONS
{
  gapi_domainParticipant handle;
  DDS::DomainParticipant_ptr Participant = NULL;

  handle = gapi_topicDescription_get_participant(__gapi_self);

  DDS::ccpp_UserData_ptr myUD;

  myUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
  if (myUD)
  {
    Participant = dynamic_cast<DDS::DomainParticipant_ptr>(myUD->ccpp_object);
    if (Participant)
    {
      DDS::DomainParticipant::_duplicate(Participant);
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

  return Participant;
}

