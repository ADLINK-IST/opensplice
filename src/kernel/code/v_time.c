/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "os_time.h"
#include "c_time.h"
#include "v_time.h"

c_time
v_timeGet()
{
    c_time t;
    os_time tb;

    tb = os_timeGet();
    t.seconds = (c_long) tb.tv_sec;
    t.nanoseconds = (c_ulong) tb.tv_nsec;

    C_TIME_SET_KIND(t, C_TIME_REALTIME);

    return t;
}

c_time
v_timeGetMonotonic()
{
    c_time t;
    os_time tb;

    tb = os_timeGetMonotonic();
    t.seconds = (c_long) tb.tv_sec;
    t.nanoseconds = (c_ulong) tb.tv_nsec;

    C_TIME_SET_KIND(t, C_TIME_MONOTONIC);

    return t;
}

c_time
v_timeGetElapsed()
{
    c_time t;
    os_time tb;

    tb = os_timeGetElapsed();
    t.seconds = (c_long) tb.tv_sec;
    t.nanoseconds = (c_ulong) tb.tv_nsec;

    C_TIME_SET_KIND(t, C_TIME_ELAPSED);

    return t;
}


