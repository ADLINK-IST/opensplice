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
#include "ccpp_GuardCondition.h"
#include "ccpp_Utils.h"

/* initially there is no gapi object and therefore no handle */
DDS::GuardCondition::GuardCondition( ) : DDS::Condition_impl(NULL)
{ 
  DDS::ccpp_UserData_ptr myUD;
  _gapi_self = gapi_guardCondition__alloc();
  if (_gapi_self)
  {
    myUD = new DDS::ccpp_UserData(this);
    /* remove the count of the reference to the object 
       such that it is deleted when the user releases it. 
     */
    CORBA::release(this);
    gapi_object_set_user_data(_gapi_self, (CORBA::Object *)myUD,
                              DDS::ccpp_CallBack_DeleteUserData,NULL);
  }
}
  
DDS::GuardCondition::~GuardCondition( ) 
{ 
  DDS::ccpp_UserData_ptr myUD;
  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
  /* avoid another last release of the reference to this WaitSet */
  myUD->ccpp_object = NULL;
//  delete myUD;
  
  gapi__free(_gapi_self);
}  

DDS::ReturnCode_t DDS::GuardCondition::set_trigger_value (
  ::CORBA::Boolean value
) THROW_ORB_EXCEPTIONS
{
    return gapi_guardCondition_set_trigger_value( _gapi_self, value);
}

