/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "v_scheduler.h"
#include "os_process.h"

v_scheduleKind
v_scheduleProcessCurrentKind (
    void)
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
    void)
{
    return (c_long)os_procAttrGetPriority ();
}
