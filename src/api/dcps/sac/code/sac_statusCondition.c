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

/*     StatusKindMask
 *     get_enabled_statuses();
 */
DDS_StatusMask
DDS_StatusCondition_get_enabled_statuses (
    DDS_StatusCondition this
    )
{
    return (DDS_StatusMask)
	gapi_statusCondition_get_enabled_statuses (
	    (gapi_statusCondition)this
	);
}

/*     ReturnCode_t
 *     set_enabled_statuses(
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_StatusCondition_set_enabled_statuses (
    DDS_StatusCondition this,
    const DDS_StatusMask mask
    )
{
    return (DDS_ReturnCode_t)
	gapi_statusCondition_set_enabled_statuses (
	    (gapi_statusCondition)this,
	    (gapi_statusMask)mask
	);
}

/*     Entity
 *     get_entity();
 */
DDS_Entity
DDS_StatusCondition_get_entity (
    DDS_StatusCondition this
    )
{
    return (DDS_Entity)
	gapi_statusCondition_get_entity (
	    (gapi_statusCondition)this
	);
}
