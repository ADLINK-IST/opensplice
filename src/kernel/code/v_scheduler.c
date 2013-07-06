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

#include "v_scheduler.h"
#include "os_process.h"

v_scheduleKind
v_scheduleProcessCurrentKind (
    void
    )
{
    os_schedClass class;
    v_scheduleKind kind;

    class = os_procAttrGetClass ();
    switch (class) {
	case OS_SCHED_REALTIME:
	    kind = V_SCHED_REALTIME;
	    break;
	case OS_SCHED_TIMESHARE:
	    kind = V_SCHED_TIMESHARING;
	    break;
	case OS_SCHED_DEFAULT:
	    kind = V_SCHED_DEFAULT;
	    break;
	default:
        kind = V_SCHED_DEFAULT;
	    assert (0);
	    break;
    }
    return kind;
}

c_long
v_scheduleProcessCurrentPriority (
    void
    )
{
    return (c_long)os_procAttrGetPriority ();
}
