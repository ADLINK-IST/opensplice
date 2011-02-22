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
#include "ccpp_QueryCondition_impl.h"
#include "ccpp_Utils.h"
#include "os_report.h"

DDS::QueryCondition_impl::QueryCondition_impl(
    gapi_queryCondition handle
) : DDS::ReadCondition_impl(handle)
{
}

char * DDS::QueryCondition_impl::get_query_expression (
) THROW_ORB_EXCEPTIONS 
{
  char * gapi_result = gapi_queryCondition_get_query_expression(_gapi_self);
  char * result = CORBA::string_dup(gapi_result);
  gapi_free(gapi_result);
  return result;
}


DDS::ReturnCode_t DDS::QueryCondition_impl::get_query_parameters (
    DDS::StringSeq & query_parameters
) THROW_ORB_EXCEPTIONS 
{
  gapi_stringSeq gapi_queryParams;
  DDS::ReturnCode_t result;
  
  ccpp_sequenceInitialize<gapi_stringSeq>(gapi_queryParams);
  result = gapi_queryCondition_get_query_parameters(_gapi_self, &gapi_queryParams);
  if (result == DDS::RETCODE_OK)
  {
      ccpp_sequenceCopyOut(gapi_queryParams, query_parameters);
  }
  return result;
}
 
DDS::ReturnCode_t DDS::QueryCondition_impl::set_query_parameters (
  const DDS::StringSeq & query_parameters
) THROW_ORB_EXCEPTIONS 
{
  DDS::ReturnCode_t result;

  gapi_stringSeq * gapi_queryArg = gapi_stringSeq__alloc();
  if (gapi_queryArg)
  {
    ccpp_sequenceCopyIn(query_parameters, *gapi_queryArg);
    result = gapi_queryCondition_set_query_parameters(_gapi_self, gapi_queryArg);
    gapi_free(gapi_queryArg);
  }
  else
  {
    result = DDS::RETCODE_OUT_OF_RESOURCES;
    OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate memory");
  }
  return result;
}

