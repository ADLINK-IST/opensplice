#include <gapi.h>
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
