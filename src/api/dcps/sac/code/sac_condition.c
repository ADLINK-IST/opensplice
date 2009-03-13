
#include <gapi.h>

#include "dds_dcps.h"

/*
 *     boolean
 *     get_trigger_value();
 */
DDS_boolean
DDS_Condition_get_trigger_value (
    DDS_Condition this
    )
{
    return (DDS_boolean)
	gapi_condition_get_trigger_value (
	    (gapi_condition)this
	);
}

