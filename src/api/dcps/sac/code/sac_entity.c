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
 *     ReturnCode_t
 *     enable();
 */
DDS_ReturnCode_t
DDS_Entity_enable (
    DDS_Entity this
    )
{
    return (DDS_ReturnCode_t)
        gapi_entity_enable (
            (gapi_entity)this
        );
}

/*
 *     StatusCondition
 *     get_statuscondition();
 */
DDS_StatusCondition
DDS_Entity_get_statuscondition (
    DDS_Entity this
    )
{
    return (DDS_StatusCondition)
        gapi_entity_get_statuscondition (
            (gapi_entity)this
        );
}

/*
 *     StatusKindMask
 *     get_status_changes();
 */
DDS_StatusMask
DDS_Entity_get_status_changes (
    DDS_Entity this
    )
{
    return (DDS_StatusMask)
        gapi_entity_get_status_changes (
            (gapi_entity)this
        );
}

/*
 *     InstanceHandle_t
 *     get_instance_handle();
 */
DDS_InstanceHandle_t
DDS_Entity_get_instance_handle (
    DDS_Entity this
    )
{
    return (DDS_InstanceHandle_t)
        gapi_entity_get_instance_handle (
            (gapi_entity)this
        );

}


