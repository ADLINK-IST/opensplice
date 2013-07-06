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

#include "u_scheduler.h"
#include "os_heap.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/

/*
 * Extend the os_threadAttr struct with scheduling
 * settings derived from the u_scheduler class.
 * All other attributes from the os_threadAttr struct
 * are not modified.
 */
u_result u_threadAttrInit(
    struct v_schedulePolicy *v_schedParam,
    os_threadAttr *os_threadParam
    )
{
    if (v_schedParam->kind == V_SCHED_DEFAULT) {
	os_threadParam->schedClass = os_procAttrGetClass ();
    } else if (v_schedParam->kind == V_SCHED_REALTIME) {
	os_threadParam->schedClass = OS_SCHED_REALTIME;
    } else if (v_schedParam->kind == V_SCHED_TIMESHARING) {
	os_threadParam->schedClass = OS_SCHED_TIMESHARE;
    }
    if (v_schedParam->priorityKind == V_SCHED_PRIO_RELATIVE) {
	os_threadParam->schedPriority = os_procAttrGetPriority () + v_schedParam->priority;
    } else {
	os_threadParam->schedPriority = v_schedParam->priority;
    }
    return U_RESULT_OK;
}

/*
 * Extend the os_procAttr struct with scheduling
 * settings derived from the u_scheduler class.
 * All other attributes from the os_procAttr struct
 * are not modified.
 */
u_result u_procAttrInit(
    struct v_schedulePolicy *v_schedParam,
    os_procAttr *os_procParam
    )
{
    if (v_schedParam->kind == V_SCHED_DEFAULT) {
	os_procParam->schedClass = os_procAttrGetClass ();
    } else if (v_schedParam->kind == V_SCHED_REALTIME) {
	os_procParam->schedClass = OS_SCHED_REALTIME;
    } else if (v_schedParam->kind == V_SCHED_TIMESHARING) {
	os_procParam->schedClass = OS_SCHED_TIMESHARE;
    }
    if (v_schedParam->priorityKind == V_SCHED_PRIO_RELATIVE) {
	os_procParam->schedPriority = os_procAttrGetPriority () + v_schedParam->priority;
    } else {
	os_procParam->schedPriority = v_schedParam->priority;
    }
    return U_RESULT_OK;
}
