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

#include "gapi.h"

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

