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
#include "ccpp_MultiTopic_impl.h"
#include "ccpp_Utils.h"
#include "os_report.h"


DDS::MultiTopic_impl::MultiTopic_impl(
    gapi_multiTopic handle
) : DDS::TopicDescription_impl(handle)
{
}

DDS::MultiTopic_impl::~MultiTopic_impl()
{
}

char * DDS::MultiTopic_impl::get_subscription_expression (
) THROW_ORB_EXCEPTIONS 
{
  return gapi_multiTopic_get_subscription_expression(__gapi_self);
}

DDS::ReturnCode_t DDS::MultiTopic_impl::get_expression_parameters (
  DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS
{
    gapi_stringSeq gapi_expressionParams;
    DDS::ReturnCode_t result;
    
    ccpp_sequenceInitialize<gapi_stringSeq>(gapi_expressionParams);
    result = gapi_multiTopic_get_expression_parameters(__gapi_self, &gapi_expressionParams);
    if (result == DDS::RETCODE_OK)
    {
        ccpp_sequenceCopyOut(gapi_expressionParams, expression_parameters);
    }
    return result;
}
   
DDS::ReturnCode_t DDS::MultiTopic_impl::set_expression_parameters (
  const DDS::StringSeq & expression_parameters
) THROW_ORB_EXCEPTIONS 
{
  DDS::ReturnCode_t result = DDS::RETCODE_OUT_OF_RESOURCES;
  gapi_stringSeq * gapi_exprPar = gapi_stringSeq__alloc();
 
  if (gapi_exprPar)
  {
    ccpp_sequenceCopyIn( expression_parameters, *gapi_exprPar);
    result = gapi_multiTopic_set_expression_parameters(__gapi_self, gapi_exprPar);
  }
  else
  {
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
  return result;
}

