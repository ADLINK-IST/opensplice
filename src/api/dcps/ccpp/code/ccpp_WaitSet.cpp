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
#include "ccpp_WaitSet.h"
#include "ccpp_Condition_impl.h" 
#include "ccpp_Utils.h"
#include "os_report.h"

DDS::WaitSet::WaitSet( )
{
  DDS::ccpp_UserData_ptr myUD;
  _gapi_self = gapi_waitSet__alloc();
  if (_gapi_self)
  {
    myUD = new DDS::ccpp_UserData(this);
    /* remove the count of the reference to the object 
       such that it is deleted when the user releases it. 
     */
    if (myUD)
    {
      gapi_object_set_user_data(_gapi_self, (CORBA::Object *)myUD,
                                DDS::ccpp_CallBack_DeleteUserData, NULL);
    }
    else
    {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
    }
    CORBA::release(this);
  }
}

DDS::WaitSet::~WaitSet()
{
  DDS::ccpp_UserData_ptr myUD;
  myUD = dynamic_cast<DDS::ccpp_UserData_ptr>((CORBA::Object *)gapi_object_get_user_data(_gapi_self));
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

DDS::ReturnCode_t DDS::WaitSet::wait (
  DDS::ConditionSeq & active_conditions,
  const DDS::Duration_t & timeout
) THROW_ORB_EXCEPTIONS
{
  gapi_conditionSeq * gapi_conditions = gapi_conditionSeq__alloc();
  gapi_duration_t gapi_timeout;
  DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;

  ccpp_Duration_copyIn(timeout, gapi_timeout);
  result = gapi_waitSet_wait(_gapi_self, gapi_conditions, &gapi_timeout);
  if (result == DDS::RETCODE_OK || result == DDS::RETCODE_TIMEOUT)
  {
    CORBA::ULong l = static_cast<CORBA::ULong>(gapi_conditions->_length);
    active_conditions.length(l);
    for (CORBA::ULong i=0; i<l; i++)
    {
      DDS::ccpp_UserData_ptr myUD;
      myUD = dynamic_cast<DDS::ccpp_UserData_ptr>
             ((CORBA::Object *)gapi_object_get_user_data(gapi_conditions->_buffer[i]));
      if (myUD)
      {
        active_conditions[i] = dynamic_cast<DDS::Condition_ptr>(myUD->ccpp_object);
        if (active_conditions[i])
        {
          DDS::Condition::_duplicate(active_conditions[i]);
        }
        else
        {
          OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Condition");
        }
      }
      else
      {
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
    }
  }
  gapi_free(gapi_conditions);
  return result;
}

DDS::ReturnCode_t DDS::WaitSet::attach_condition (
  DDS::Condition_ptr cond
) THROW_ORB_EXCEPTIONS
{ 
  DDS::Condition_impl *cond_impl;
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
 
  cond_impl = dynamic_cast<DDS::Condition_impl *>(cond);
  if (cond_impl) 
  {
    result = gapi_waitSet_attach_condition(_gapi_self, cond_impl->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::WaitSet::detach_condition (
  DDS::Condition_ptr cond
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
  DDS::Condition_impl * cond_impl;

  cond_impl = dynamic_cast<DDS::Condition_impl *>(cond);
  if (cond_impl)
  {
    result = gapi_waitSet_detach_condition(_gapi_self, cond_impl->_gapi_self);
  }
  return result;
}

DDS::ReturnCode_t DDS::WaitSet::get_conditions (
  DDS::ConditionSeq & attached_conditions
) THROW_ORB_EXCEPTIONS
{
    gapi_conditionSeq * gapi_conditions = gapi_conditionSeq__alloc();
    DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;

    if (gapi_conditions)
    {
      result = gapi_waitSet_get_conditions(_gapi_self, gapi_conditions);
      if (result == DDS::RETCODE_OK)
      {
        CORBA::ULong l =  static_cast<CORBA::ULong>(gapi_conditions->_length);
        attached_conditions.length(l);
        for (CORBA::ULong i=0; i<l; i++)
        {
          DDS::ccpp_UserData_ptr myUD;
          myUD = dynamic_cast<DDS::ccpp_UserData_ptr>
                 ((CORBA::Object *)gapi_object_get_user_data(gapi_conditions->_buffer[i]));
          if (myUD)
          {
            attached_conditions[i] = dynamic_cast<DDS::Condition_ptr>(myUD->ccpp_object);
            
            if (attached_conditions[i])
            {
              DDS::Condition::_duplicate(attached_conditions[i]);
            }
            else
            {
              OS_REPORT(OS_ERROR, "CCPP", 0, "Invalid Condition");
            }
          }
          else
          {
            OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
          }
        }
      }
      gapi_sequence_free(gapi_conditions);
    }
  else
  {
      OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
    return result;
}

