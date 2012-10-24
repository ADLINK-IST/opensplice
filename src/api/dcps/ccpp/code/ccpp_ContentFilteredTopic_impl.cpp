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
#include "ccpp_ContentFilteredTopic_impl.h"
#include "ccpp_Utils.h"
#include "ccpp_Topic_impl.h"

DDS::ContentFilteredTopic_impl::ContentFilteredTopic_impl(
	    gapi_contentFilteredTopic handle
) : DDS::TopicDescription_impl(handle)
{
  os_mutexAttr mutexAttr = { OS_SCOPE_PRIVATE };
  if (os_mutexInit(&cft_mutex, &mutexAttr) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to create mutex");
  }
}

DDS::ContentFilteredTopic_impl::~ContentFilteredTopic_impl()
{
  if (os_mutexDestroy(&cft_mutex) != os_resultSuccess)
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to destroy mutex");
  }
}

char * DDS::ContentFilteredTopic_impl::get_filter_expression (
) THROW_ORB_EXCEPTIONS 
{
  return gapi_contentFilteredTopic_get_filter_expression(__gapi_self);
}


DDS::ReturnCode_t DDS::ContentFilteredTopic_impl::get_expression_parameters (
  DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
  gapi_stringSeq gapi_expressionParams;
  DDS::ReturnCode_t result;
  
  ccpp_sequenceInitialize<gapi_stringSeq>(gapi_expressionParams);
  result = gapi_contentFilteredTopic_get_expression_parameters(__gapi_self, &gapi_expressionParams);
  if (result == DDS::RETCODE_OK)
  {
      ccpp_sequenceCopyOut(gapi_expressionParams, expression_parameters);
  }
  return result;

}

DDS::ReturnCode_t DDS::ContentFilteredTopic_impl::set_expression_parameters (
  const DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS 
{
  DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;
  gapi_stringSeq * gapi_exprPar = gapi_stringSeq__alloc();
  
  if (gapi_exprPar)
  {
    ccpp_sequenceCopyIn( expression_parameters, *gapi_exprPar);
    result = gapi_contentFilteredTopic_set_expression_parameters(__gapi_self, gapi_exprPar);
    gapi_free(gapi_exprPar);
  }
  return result;
}

DDS::Topic_ptr DDS::ContentFilteredTopic_impl::get_related_topic (
) THROW_ORB_EXCEPTIONS 
{
  gapi_topic handle;
  DDS::Topic_ptr result = NULL;
  DDS::ccpp_UserData_ptr myUD;

  handle = gapi_contentFilteredTopic_get_related_topic(__gapi_self);
  if (handle)
  {
    if (os_mutexLock(&cft_mutex) == os_resultSuccess)
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
        OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to obtain userdata");
      }
      if (os_mutexUnlock(&cft_mutex) != os_resultSuccess)
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
