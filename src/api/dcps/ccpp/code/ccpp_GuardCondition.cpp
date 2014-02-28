/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "ccpp_GuardCondition.h"
#include "gapi.h"
#include "ccpp_Utils.h"
#include "os_report.h"

/* initially there is no gapi object and therefore no handle */
DDS::GuardCondition::GuardCondition( ) : DDS::Condition_impl(NULL)
{
  DDS::ccpp_UserData_ptr myUD;
  _gapi_self = gapi_guardCondition__alloc();
  if (_gapi_self)
  {
    myUD = new DDS::ccpp_UserData(this, NULL, NULL, true);
    if (myUD)
    {
        DDS::Object_ptr parent = dynamic_cast<DDS::Object_ptr>(myUD);
        gapi_object_set_user_data(_gapi_self, static_cast<void *>(parent),
                              ccpp_CallBack_DeleteUserData,NULL);
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    }
  }
}

DDS::GuardCondition::~GuardCondition( )
{
  DDS::ccpp_UserData_ptr myUD;
  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((DDS::Object *)gapi_object_get_user_data(_gapi_self));
  if (myUD)
  {
  /* avoid another last release of the reference to this WaitSet */
    myUD->ccpp_object = NULL;
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
  }
  gapi__free(_gapi_self);
}

DDS::ReturnCode_t DDS::GuardCondition::set_trigger_value (
  ::DDS::Boolean value
) THROW_ORB_EXCEPTIONS
{
    return gapi_guardCondition_set_trigger_value( _gapi_self, value);
}

