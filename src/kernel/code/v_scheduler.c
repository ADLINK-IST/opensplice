
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
	    assert (1);
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
