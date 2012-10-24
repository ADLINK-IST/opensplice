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
#include "ccpp_Domain_impl.h"

DDS::Domain_impl::Domain_impl(gapi_domain handle) :  _gapi_self(handle)
{
}

DDS::Domain_impl::~Domain_impl()
{

}

DDS::ReturnCode_t DDS::Domain_impl::create_persistent_snapshot (
    const char * partition_expression,
    const char * topic_expression,
    const char * URI
) THROW_ORB_EXCEPTIONS
{
  DDS::ReturnCode_t result;

  result = gapi_domain_create_persistent_snapshot(
               _gapi_self,
               partition_expression,
               topic_expression,
               URI);

  return result;
}
