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
#include "gapi_scheduler.h"
#include "u_scheduler.h"

void
gapi_scheduleToKernel (
    const gapi_schedulingQosPolicy *gapi_sched,
    struct v_schedulePolicy *v_sched
    )
{
    switch (gapi_sched->scheduling_class.kind) {
    case GAPI_SCHEDULE_DEFAULT:
	v_sched->kind = V_SCHED_DEFAULT;
	break;
    case GAPI_SCHEDULE_TIMESHARING:
	v_sched->kind = V_SCHED_TIMESHARING;
	break;
    case GAPI_SCHEDULE_REALTIME:
	v_sched->kind = V_SCHED_REALTIME;
	break;
    }
    switch (gapi_sched->scheduling_priority_kind.kind) {
    case V_SCHED_PRIO_RELATIVE:
	v_sched->priorityKind = V_SCHED_PRIO_RELATIVE;
	break;
    case V_SCHED_PRIO_ABSOLUTE:
	v_sched->priorityKind = V_SCHED_PRIO_ABSOLUTE;
	break;
    }
    v_sched->priority = (c_long)gapi_sched->scheduling_priority;
}

void
gapi_scheduleFromKernel (
    const struct v_schedulePolicy *v_sched,
    gapi_schedulingQosPolicy *gapi_sched
    )
{
    switch (v_sched->kind) {
    case V_SCHED_DEFAULT:
	gapi_sched->scheduling_class.kind = GAPI_SCHEDULE_DEFAULT;
	break;
    case V_SCHED_TIMESHARING:
	gapi_sched->scheduling_class.kind = GAPI_SCHEDULE_TIMESHARING;
	break;
    case V_SCHED_REALTIME:
	gapi_sched->scheduling_class.kind = GAPI_SCHEDULE_REALTIME;
	break;
    }
    switch (v_sched->priorityKind) {
    case V_SCHED_PRIO_RELATIVE:
	gapi_sched->scheduling_priority_kind.kind = GAPI_PRIORITY_RELATIVE;
	break;
    case V_SCHED_PRIO_ABSOLUTE:
	gapi_sched->scheduling_priority_kind.kind = GAPI_PRIORITY_ABSOLUTE;
	break;
    }
    gapi_sched->scheduling_priority = (gapi_long)v_sched->priority;
}

void
gapi_threadAttrInit (
    const gapi_schedulingQosPolicy *gapi_sched,
    os_threadAttr *threadAttr
    )
{
    struct v_schedulePolicy v_sched;

    gapi_scheduleToKernel (gapi_sched, &v_sched);
    u_threadAttrInit (&v_sched, threadAttr);
}
