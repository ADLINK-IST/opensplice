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
#include "ccpp_Condition_impl.h"

DDS::Condition_impl::Condition_impl(
    gapi_condition handle
) : _gapi_self(handle)
{
}

CORBA::Boolean DDS::Condition_impl::get_trigger_value (
) THROW_ORB_EXCEPTIONS 
{
    return gapi_condition_get_trigger_value( _gapi_self);
}
