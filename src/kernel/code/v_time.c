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

#include "os_time.h"

#include "v_time.h"

c_time
v_timeGet()
{
    c_time t;
    os_time tb;

    tb = os_timeGet();
    t.seconds = tb.tv_sec;
    t.nanoseconds = tb.tv_nsec;
    return t;
}

c_equality
v_timeCompare(
    c_time t1,
    c_time t2)
{
    if (t1.seconds < t2.seconds) {
        return C_LT;
    }
    if (t1.seconds > t2.seconds) {
        return C_GT;
    }
    if (t1.nanoseconds < t2.nanoseconds) {
        return C_LT;
    }
    if (t1.nanoseconds > t2.nanoseconds) {
        return C_GT;
    }
    return C_EQ;
}


