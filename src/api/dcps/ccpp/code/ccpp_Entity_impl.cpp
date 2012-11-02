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
#include "ccpp_Entity_impl.h"
#include "ccpp_Utils.h"
#include "ccpp_StatusCondition_impl.h"
#include "os_report.h"

DDS::Entity_impl::Entity_impl(
  gapi_entity handle
) : _gapi_self(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&e_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::Entity_impl::~Entity_impl()
{
  if (os_mutexDestroy(&e_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}


DDS::ReturnCode_t DDS::Entity_impl::enable (
) THROW_ORB_EXCEPTIONS
{
  return gapi_entity_enable( _gapi_self);
}

DDS::StatusCondition_ptr DDS::Entity_impl::get_statuscondition (
) THROW_ORB_EXCEPTIONS
{
  gapi_statusCondition handle;
  DDS::StatusCondition_ptr statusCondition = NULL;
  ccpp_UserData_ptr scUD;

  handle = gapi_entity_get_statuscondition(_gapi_self);
  if (handle)
  {
    if (os_mutexLock(&e_mutex) == os_resultSuccess)
    {
      scUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(handle));
      if (scUD)
      {
        statusCondition = dynamic_cast<DDS::StatusCondition_ptr>(scUD->ccpp_object);
        if (statusCondition)
        {
          DDS::StatusCondition::_duplicate(statusCondition);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Status Condition");
        }
      }
      else
      {
        statusCondition = new StatusCondition_impl(handle);
        if (statusCondition)
        {
          scUD = new ccpp_UserData(statusCondition, NULL, NULL);
          if (scUD)
          {
            ccpp_UserData_ptr ownUD;
            
            gapi_object_set_user_data(handle, (CORBA::Object *)scUD,
                                      DDS::ccpp_CallBack_DeleteUserData,NULL);
            ownUD = dynamic_cast<ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
            ownUD->ccpp_statusconditiondata = scUD;
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
      if (os_mutexUnlock(&e_mutex) != os_resultSuccess)
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to release mutex");
      }
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain mutex");
    }
  }
  return statusCondition;
}

DDS::StatusMask DDS::Entity_impl::get_status_changes (
) THROW_ORB_EXCEPTIONS
{
  return gapi_entity_get_status_changes( _gapi_self);
}

DDS::InstanceHandle_t DDS::Entity_impl::get_instance_handle (
) THROW_ORB_EXCEPTIONS
{
    return gapi_entity_get_instance_handle(_gapi_self);
}

// char * DDS::Entity_impl::get_name (
// ) THROW_ORB_EXCEPTIONS
// {
//   return gapi_entity_get_name( _gapi_self );
// }

// DDS::ReturnCode_t DDS::Entity_impl::set_name (
//     const char *name
// ) THROW_ORB_EXCEPTIONS
// {
//   return gapi_entity_set_name( _gapi_self, (gapi_string)name);
// }

gapi_entity DDS::Entity_impl::get_gapi_self()
{
  return _gapi_self;
}
