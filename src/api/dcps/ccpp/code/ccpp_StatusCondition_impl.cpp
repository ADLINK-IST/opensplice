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
#include "ccpp_StatusCondition_impl.h"
#include "ccpp_Utils.h"
#include "os_report.h"

DDS::StatusCondition_impl::StatusCondition_impl( 
  gapi_condition _gapi_handle
) : DDS::Condition_impl(_gapi_handle) 
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&sc_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  } 
}

DDS::StatusCondition_impl::~StatusCondition_impl( )
{
  if (os_mutexDestroy(&sc_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

DDS::StatusMask DDS::StatusCondition_impl::get_enabled_statuses (
) THROW_ORB_EXCEPTIONS
{
  return gapi_statusCondition_get_enabled_statuses( _gapi_self);
}

DDS::ReturnCode_t DDS::StatusCondition_impl::set_enabled_statuses (
  DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
  gapi_statusMask gapi_mask;
  gapi_mask = mask;
  return gapi_statusCondition_set_enabled_statuses(_gapi_self, gapi_mask);
}

DDS::Entity_ptr DDS::StatusCondition_impl::get_entity (
) THROW_ORB_EXCEPTIONS
{
  gapi_entity handle = NULL;
  DDS::Entity_ptr entity = NULL;

  handle = gapi_statusCondition_get_entity(_gapi_self);
  if (handle)
  {
    ccpp_UserData_ptr drUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
    if (drUD)
    {
      entity = dynamic_cast<DDS::Entity_ptr>(drUD->ccpp_object);
      if (entity)
      {
        DDS::Entity::_duplicate(entity);
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Entity");
     }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
    }
  }
  return entity;
}

