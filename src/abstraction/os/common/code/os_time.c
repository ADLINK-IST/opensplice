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
/** \file os/common/code/os_time.c
 *  \brief Common time management services
 *
 * Implements os_timeAdd, os_timeSub, os_timeCompare,
 * os_timeAbs, os_timeMulReal, os_timeToReal, os_realToTime
 * which are platform independent
 */

#include <stdlib.h>
#include <assert.h>

/* double type definition for calculations in os_timeMulReal 	*/
/* Can be adapted to available and required accuracy		*/
typedef double os_cdouble;

/** \brief return value of \b t1 + \b t2
 *
 * If the value \b t1 + \b t2 does not fit in os_time the value
 * will be incorrect.
 */
os_time
os_timeAdd(
    os_time t1,
    os_time t2)
{
    os_time tr;

    assert (t1.tv_nsec >= 0);
    assert (t1.tv_nsec < 1000000000);
    assert (t2.tv_nsec >= 0);
    assert (t2.tv_nsec < 1000000000);
    
    tr.tv_nsec = t1.tv_nsec + t2.tv_nsec;
    tr.tv_sec = t1.tv_sec + t2.tv_sec;
    if (tr.tv_nsec >= 1000000000) {
        tr.tv_sec++;
        tr.tv_nsec = tr.tv_nsec - 1000000000;
    }
    return tr;
}

/** \brief return value of \b t1 - \b t2
 *
 * If the value \b t1 - \b t2 does not fit in os_time the value
 * will be incorrect.
 */
os_time
os_timeSub(
    os_time t1,
    os_time t2)
{
    os_time tr;

    assert (t1.tv_nsec >= 0);
    assert (t1.tv_nsec < 1000000000);
    assert (t2.tv_nsec >= 0);
    assert (t2.tv_nsec < 1000000000);
    
    if (t1.tv_nsec >= t2.tv_nsec) {
        tr.tv_nsec = t1.tv_nsec - t2.tv_nsec;
        tr.tv_sec = t1.tv_sec - t2.tv_sec;
    } else {
        tr.tv_nsec = t1.tv_nsec - t2.tv_nsec + 1000000000;
        tr.tv_sec = t1.tv_sec - t2.tv_sec - 1;
    }
    return tr;
}

/** \brief Compare \b t1 with \b t2
 *
 * - If the value of \b t1 < value of \b t2 return \b OS_LESS
 * - If the value of \b t1 > value of \b t2 return \b OS_MORE
 * - If the value of \b t1 equals the value of \b t2 return \b OS_EQUAL
 */
os_compare
os_timeCompare(
    os_time t1,
    os_time t2)
{
    os_compare rv;

    assert (t1.tv_nsec >= 0);
    assert (t1.tv_nsec < 1000000000);
    assert (t2.tv_nsec >= 0);
    assert (t2.tv_nsec < 1000000000);
    
    if (t1.tv_sec < t2.tv_sec) {
        rv = OS_LESS;
    } else if (t1.tv_sec > t2.tv_sec) {
        rv = OS_MORE;
    } else if (t1.tv_nsec < t2.tv_nsec) {
        rv = OS_LESS;
    } else if (t1.tv_nsec > t2.tv_nsec) {
        rv = OS_MORE;
    } else {
        rv = OS_EQUAL;
    }
    return rv;
}

/** \brief return absolute value \b t
 *
 * If the value |\b t| does not fit in os_time the value
 * will be incorrect.
 */
os_time
os_timeAbs(
    os_time t)
{
    os_time tr;

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);
    
    if (t.tv_sec < 0) {
        tr.tv_sec = -t.tv_sec - 1;
        tr.tv_nsec = 1000000000 - t.tv_nsec;
    } else {
        tr.tv_sec = t.tv_sec;
        tr.tv_nsec = t.tv_nsec;
    }
    return tr;
}

/** \brief return value \b t * \b multiply
 *
 * if the result value does not fit in os_time the value
 * will be incorrect.
 */
os_time
os_timeMulReal(
    os_time t,
    double multiply)
{
    os_time tr;
    os_cdouble trr;
    os_cdouble sec;
    os_cdouble nsec;

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);
    
    sec = (os_cdouble)t.tv_sec;
    nsec = (os_cdouble)t.tv_nsec / (os_cdouble)1000000000.0;
    trr = (sec + nsec) * multiply;
    if (trr >= 0.0) {
        tr.tv_sec = (os_timeSec)trr;
        tr.tv_nsec = (int)((trr-(os_cdouble)tr.tv_sec) * (os_cdouble)1000000000.0);
    } else {
        tr.tv_sec = (os_timeSec)trr - 1;
        tr.tv_nsec = (int)((trr-(os_cdouble)tr.tv_sec) * (os_cdouble)1000000000.0);
    }
    return tr;
}

/** \brief return floating point representation of \b t
 *
 * because of the limited floating point represenation (64 bits)
 * the value will be limited to a resoltion of about 1 us.
 */
os_timeReal
os_timeToReal(
    os_time t)
{
    volatile os_timeReal tr; /* This var is volatile to bypass a GCC 3.x bug on X86 */

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);
    
    tr = (os_timeReal)t.tv_sec + (os_timeReal)t.tv_nsec / (os_timeReal)1000000000.0;
    
    return tr;
}

/** \brief return os_time represenation of floating time \b t
 *
 * because of the limited floating point represenation (64 bits)
 * the value will be limited to a resoltion of about 1 us.
 */
os_time
os_realToTime(
    os_timeReal t)
{
    os_time tr;

    if (t >= 0.0) {
        tr.tv_sec = (os_timeSec)t;
        tr.tv_nsec = (int)((t-(os_timeReal)tr.tv_sec) * (os_timeReal)1000000000.0);
    } else {
        tr.tv_sec = (os_timeSec)t - 1;
        tr.tv_nsec = (int)((t-(os_timeReal)tr.tv_sec) * (os_timeReal)1000000000.0);
    }
    return tr;
}
