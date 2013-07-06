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

/*     ReturnCode_t
 *     set_trigger_value(
 *         in boolean value);
 */
DDS_ReturnCode_t
DDS_GuardCondition_set_trigger_value (
    DDS_GuardCondition this,
    const DDS_boolean value
    )
{
    return (DDS_ReturnCode_t)
    gapi_guardCondition_set_trigger_value (
	(gapi_guardCondition)this,
	(gapi_boolean)value
    );
    
}

/*     GuardCondition
 *     GuardCondition__alloc (
 *         void);
 */
DDS_GuardCondition
DDS_GuardCondition__alloc (
    void
    )
{
    gapi_guardCondition guard_condition;

    guard_condition = gapi_guardCondition__alloc ();

    return guard_condition;
}
