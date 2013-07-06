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
 *     wait(
 *         out ConditionSeq active_conditions,
 *         in Duration_t timeout);
 */
 
DDS_ReturnCode_t
DDS_WaitSet_wait (
    DDS_WaitSet this,
    DDS_ConditionSeq *active_conditions,
    const DDS_Duration_t *timeout
    )
{
    return (DDS_ReturnCode_t)
	gapi_waitSet_wait (
	    (gapi_waitSet)this,
	    (gapi_conditionSeq *)active_conditions,
	    (const gapi_duration_t *)timeout
	);
}

/*     ReturnCode_t
 *     attach_condition(
 *         in Condition cond);
 */
DDS_ReturnCode_t
DDS_WaitSet_attach_condition (
    DDS_WaitSet this,
    const DDS_Condition cond
    )
{
    return (DDS_ReturnCode_t)
	gapi_waitSet_attach_condition (
	    (gapi_waitSet)this,
	    (gapi_condition)cond
	);
}

/*     ReturnCode_t
 *     detach_condition(
 *         in Condition cond);
 */
DDS_ReturnCode_t
DDS_WaitSet_detach_condition(
    DDS_WaitSet this,
    const DDS_Condition cond
    )
{
    return (DDS_ReturnCode_t)
	gapi_waitSet_detach_condition (
	    (gapi_waitSet)this,
	    (gapi_condition)cond
	);
}

/*     ReturnCode_t
 *     get_conditions(
 *         out ConditionSeq attached_conditions);
 */
DDS_ReturnCode_t
DDS_WaitSet_get_conditions(
        DDS_WaitSet this,
        DDS_ConditionSeq *attached_conditions
        )
{
    return (DDS_ReturnCode_t)
	gapi_waitSet_get_conditions (
	    (gapi_waitSet)this,
	    (gapi_conditionSeq *)attached_conditions
	);
}

/*     WaitSet
 *     WaitSet__alloc (
 *         void);
 */
DDS_WaitSet
DDS_WaitSet__alloc (
    void
    )
{
    gapi_waitSet wait_set;

    wait_set = gapi_waitSet__alloc ();

    return wait_set;
}
