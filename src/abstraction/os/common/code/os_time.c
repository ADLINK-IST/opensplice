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
/** \file os/common/code/os_time.c
 *  \brief Common time management services
 *
 * Implements os_timeAdd, os_timeSub, os_timeCompare,
 * os_timeAbs, os_timeMulReal, os_timeToReal, os_realToTime
 * which are platform independent
 */

#include <stdlib.h>
#include <assert.h>
#include "os_report.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "os_atomics.h"

/* double type definition for calculations in os_timeMulReal 	*/
/* Can be adapted to available and required accuracy		*/
typedef double os_cdouble;

os_duration
os_durationAdd(
    os_duration d1,
    os_duration d2)
{
    os_duration dr;

    if (OS_DURATION_ISINVALID(d1) || OS_DURATION_ISINVALID(d2)) {
        dr = OS_DURATION_INVALID;
    } else if ((OS_DURATION_ISINFINITE(d1) && OS_DURATION_ISMIN_INFINITE(d2)) ||
               (OS_DURATION_ISMIN_INFINITE(d1) && OS_DURATION_ISINFINITE(d2))) {
        dr = OS_DURATION_INVALID;
    } else if (OS_DURATION_ISINFINITE(d1) || OS_DURATION_ISINFINITE(d2)) {
        dr = OS_DURATION_INFINITE;
    } else if (OS_DURATION_ISMIN_INFINITE(d1) || OS_DURATION_ISMIN_INFINITE(d2)) {
        dr = OS_DURATION_MIN_INFINITE;
    } else {
        if (((d1 <= 0) && (d2 >= 0)) ||
            ((d1 >= 0) && (d2 <= 0))) {
            /* Never an under or overflow. */
            dr = d1 + d2;
        } else {
            /* Use os_uint64 to check for overflows. */
            if (((os_uint64)os_durationAbs(d1) + (os_uint64)os_durationAbs(d2)) > ((os_uint64)OS_DURATION_INFINITE)) {
                /* Overflow (or underflow). */
                dr = (d1 < 0) ? OS_DURATION_MIN_INFINITE : OS_DURATION_INFINITE;
            } else {
                dr = d1 + d2;
            }
        }
    }
    return dr;
}

os_duration
os_durationSub(
    os_duration d1,
    os_duration d2)
{
    os_duration dr;

    if (OS_DURATION_ISINVALID(d1) || OS_DURATION_ISINVALID(d2)) {
        dr = OS_DURATION_INVALID;
    } else if ((OS_DURATION_ISINFINITE(d1)     && OS_DURATION_ISINFINITE(d2)    ) ||
               (OS_DURATION_ISMIN_INFINITE(d1) && OS_DURATION_ISMIN_INFINITE(d2))) {
        dr = OS_DURATION_INVALID;
    } else if (OS_DURATION_ISMIN_INFINITE(d1) || OS_DURATION_ISINFINITE(d2)) {
        dr = OS_DURATION_MIN_INFINITE;
    } else if (OS_DURATION_ISINFINITE(d1) || OS_DURATION_ISMIN_INFINITE(d2)) {
        dr = OS_DURATION_INFINITE;
    } else {
        if (((d1 >= 0) && (d2 >= 0)) ||
            ((d1 <= 0) && (d2 <= 0))) {
            /* Never an under or overflow. */
            dr = d1 - d2;
        } else {
            /* One value is positive while the other is negative. */
            if (((os_uint64)os_durationAbs(d1) + (os_uint64)os_durationAbs(d2)) > ((os_uint64)OS_DURATION_INFINITE)) {
                /* Overflow (or underflow). */
                dr = (d1 < 0) ? OS_DURATION_MIN_INFINITE : OS_DURATION_INFINITE;
            } else {
                dr = d1 - d2;
            }
        }
    }
    return dr;
}

os_duration
os_durationMul(
    os_duration d,
    double multiply)
{
    os_duration dr;

    if (OS_DURATION_ISINVALID(d)) {
        dr = OS_DURATION_INVALID;
    } else if (multiply == 0.0) {
        dr = OS_DURATION_ZERO;
    } else if ((OS_DURATION_ISINFINITE(d)) || (OS_DURATION_ISMIN_INFINITE(d))) {
        dr = (multiply < 0.0) ? -d : d;
    } else {
        double dur = (double)d;
        double product = dur * multiply;
        if (product > (double)OS_DURATION_INFINITE) {
            dr = OS_DURATION_INFINITE;
        } else if (product < (double)OS_DURATION_MIN_INFINITE) {
            dr = OS_DURATION_MIN_INFINITE;
        } else {
            dr = (os_duration)product;
        }
    }
    return dr;
}

os_duration
os_durationAbs(
    os_duration d)
{
    os_duration dr = d;

    /* Note: OS_DURATION_INVALID remains unchanged */
    if (os_durationCompare(d, OS_DURATION_ZERO) == OS_LESS) {
        if (d == OS_DURATION_MIN_INFINITE) {
            dr = OS_DURATION_INFINITE;
        } else {
            dr = -d;
        }
    }
    return dr;
}

os_compare
os_durationCompare(
    os_duration d1,
    os_duration d2)
{
    os_compare result;

    if (OS_DURATION_ISINVALID(d1) || OS_DURATION_ISINVALID(d2)) {
        result = OS_INVALID;
    } else if (d1 > d2) {
        result = OS_MORE;
    } else if (d1 < d2) {
        result = OS_LESS;
    } else {
        result = OS_EQUAL;
    }
    return result;
}

/** \brief Add time t1 to time t2
 *
 * Possible Results:
 * - returns t1 + t2 when
 *     the result fits within the time structure
 * - returns an unspecified value when
 *     the result does not fit within the time structure
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

/** \brief Adds a duration to a time value
 *
 * Returns:
 * - returns t+d
 * - returns OS_TIME_INFINITE if (d == OS_DURATION_INFINITE)
 * - returns OS_TIME_INFINITE if (t+d > OS_TIME_INFINITE)
 */
static os_uint64
timeAdd(
    os_uint64 t,
    os_duration d)
{
    os_uint64 tr;

    assert(t != OS_TIME_INVALID);
    assert(d != OS_DURATION_INVALID);

    if (t == OS_TIME_INVALID || d == OS_DURATION_INVALID) {
        tr = OS_TIME_INVALID;
    } else if (t == OS_TIME_INFINITE || d == OS_DURATION_INFINITE) {
        if (d == OS_DURATION_MIN_INFINITE) {
            tr = OS_TIME_INVALID;
        } else {
            tr = OS_TIME_INFINITE;
        }
    } else if (d < 0) {
        if (((os_uint64)(-d) > t) || d == OS_DURATION_MIN_INFINITE) {
            tr = OS_TIME_INVALID;
        } else {
            tr = (os_uint64)((os_duration)t + d);
        }
    } else {
        tr = t+(os_uint64)d;
        tr = OS_TIME_NORMALIZE(tr);
    }
    return tr;
}

os_timeW
os_timeWAdd(
    os_timeW t,
    os_duration d)
{
    os_timeW tr;
    tr.wt = timeAdd(t.wt, d);
    return tr;
}

os_timeM
os_timeMAdd(
    os_timeM t,
    os_duration d)
{
    os_timeM tr;
    tr.mt = timeAdd(t.mt, d);
    return tr;
}

os_timeE
os_timeEAdd(
    os_timeE t,
    os_duration d)
{
    os_timeE tr;
    tr.et = timeAdd(t.et, d);
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

/** \brief Subtract time t2 from time t1
 *
 * Results:
 * - returns the duration (t1 - t2)
 * - returns OS_TIME_INFINITE if (t1 == OS_TIME_INFINITE)
 * - returns OS_TIME_INVALID if (t2 == OS_TIME_INFINITE)
 */
static os_uint64
timeSub (
    os_uint64 t,
    os_duration d)
{
    os_uint64 tr;

    assert(t != OS_TIME_INVALID);
    assert(d != OS_DURATION_INVALID);

    if (t == OS_TIME_INVALID || d == OS_DURATION_INVALID || d == OS_DURATION_INFINITE) {
        tr = OS_TIME_INVALID;
    } else if (t == OS_TIME_INFINITE) {
        tr = OS_TIME_INFINITE;
    } else if (d<0) {
        tr = t+(os_uint64)(0-d);
        tr = OS_TIME_NORMALIZE(tr);
    } else {
        tr = t >= (os_uint64)d ? t-(os_uint64)d : OS_TIME_INVALID;
    }

    return tr;
}

os_timeW
os_timeWSub(
    os_timeW t,
    os_duration d)
{
    os_timeW tr;
    tr.wt = timeSub(t.wt, d);
    return tr;
}

os_timeM
os_timeMSub(
    os_timeM t,
    os_duration d)
{
    os_timeM tr;
    tr.mt = timeSub(t.mt, d);
    return tr;
}

os_timeE
os_timeESub(
    os_timeE t,
    os_duration d)
{
    os_timeE tr;
    tr.et = timeSub(t.et, d);
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

static os_compare
timeCompare(
    os_uint64 t1,
    os_uint64 t2)
{
    os_compare rv;

    assert(t1 != OS_TIME_INVALID);
    assert(t2 != OS_TIME_INVALID);

    t1 = OS_TIME_NORMALIZE(t1);
    t2 = OS_TIME_NORMALIZE(t2);

    if (t1 < t2) {
        rv = OS_LESS;
    } else if (t1 > t2) {
        rv = OS_MORE;
    } else {
        rv = OS_EQUAL;
    }

    return rv;
}

os_compare
os_timeWCompare(
    os_timeW t1,
    os_timeW t2)
{
    return timeCompare(t1.wt, t2.wt);
}

os_compare
os_timeMCompare(
    os_timeM t1,
    os_timeM t2)
{
    return timeCompare(t1.mt, t2.mt);
}

os_compare
os_timeECompare(
    os_timeE t1,
    os_timeE t2)
{
    return timeCompare(t1.et, t2.et);
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

/** \brief Determine the duration difference between two times
 *
 * Results:
 * - returns the duration (t1 - t2)
 * - returns OS_DURATION_INFINITE if (t1 == OS_TIME_INFINITE)
 * - returns OS_DURATION_MIN_INFINITE if (t2 == OS_TIME_INFINITE)
 */
static os_duration
timeDiff (
    os_uint64 t1,
    os_uint64 t2)
{
    os_duration dr;

    assert(t1 != OS_TIME_INVALID);
    assert(t2 != OS_TIME_INVALID);

    t1 = OS_TIME_NORMALIZE(t1);
    t2 = OS_TIME_NORMALIZE(t2);

    if (OS_TIME_ISINFINITE(t1)) {
        dr = OS_DURATION_INFINITE;
    } else if (OS_TIME_ISINFINITE(t2)) {
        dr = OS_DURATION_MIN_INFINITE;
    } else {
        dr = (os_duration)(t1 - t2);
    }
    return dr;
}

os_duration
os_timeWDiff(
    os_timeW t1,
    os_timeW t2)
{
    return timeDiff(t1.wt, t2.wt);
}

os_duration
os_timeMDiff(
    os_timeM t1,
    os_timeM t2)
{
    return timeDiff(t1.mt, t2.mt);
}

os_duration
os_timeEDiff(
    os_timeE t1,
    os_timeE t2)
{
    return timeDiff(t1.et, t2.et);
}

/** \brief conversion routines to convert old time to new time representations.
 */
os_timeW
os_timeToTimeW(
    os_time t)
{
    os_timeW tr = OS_TIMEW_INVALID;

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);

    if (os_timeIsInfinite(t)) {
        tr = OS_TIMEW_INFINITE;
    } else if (t.tv_sec >= 0) {
        tr.wt = (os_uint64)t.tv_sec*OS_TIME_SECOND + (os_uint64)t.tv_nsec;
    }
    return tr;
}

os_timeM
os_timeToTimeM(
    os_time t)
{
    os_timeM tr = OS_TIMEM_INVALID;

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);

    if (os_timeIsInfinite(t)) {
        tr = OS_TIMEM_INFINITE;
    } else if (t.tv_sec >= 0) {
        tr.mt = (os_uint64)t.tv_sec*OS_TIME_SECOND + (os_uint64)t.tv_nsec;
    }
    return tr;
}

os_timeE
os_timeToTimeE(
    os_time t)
{
    os_timeE tr = OS_TIMEE_INVALID;

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);

    if (os_timeIsInfinite(t)) {
        tr = OS_TIMEE_INFINITE;
    } else if (t.tv_sec >= 0) {
        tr.et = (os_uint64)t.tv_sec*OS_TIME_SECOND + (os_uint64)t.tv_nsec;
    }
    return tr;
}

os_duration
os_timeToDuration(
    os_time t)
{
    os_duration dr;

    assert (t.tv_nsec >= 0);
    assert (t.tv_nsec < 1000000000);

    if (os_timeIsInfinite(t)) {
        dr = OS_DURATION_INFINITE;
    } else {
        dr = (os_duration)t.tv_sec*OS_DURATION_SECOND + (os_duration)t.tv_nsec;
    }
    return dr;
}

/** \brief conversion routines to convert new time representations to old time.
 */
os_time
os_timeWToTime(
    os_timeW t)
{
    os_time tr = {OS_TIME_INFINITE_SEC, OS_TIME_INFINITE_NSEC};

    assert(t.wt != OS_TIME_INVALID);

    if (!OS_TIMEW_ISINFINITE(t)) {
        tr.tv_sec = (os_timeSec)(t.wt / OS_TIME_SECOND);
        tr.tv_nsec = (os_int32)(t.wt % OS_TIME_SECOND);
    }
    return tr;
}

os_time
os_timeMToTime(
    os_timeM t)
{
    os_time tr = {OS_TIME_INFINITE_SEC, OS_TIME_INFINITE_NSEC};

    assert(t.mt != OS_TIME_INVALID);

    if (!OS_TIMEM_ISINFINITE(t)) {
        tr.tv_sec = (os_timeSec)(t.mt / OS_TIME_SECOND);
        tr.tv_nsec = (os_int32)(t.mt % OS_TIME_SECOND);
    }
    return tr;
}

os_time
os_timeEToTime(
    os_timeE t)
{
    os_time tr = {OS_TIME_INFINITE_SEC, OS_TIME_INFINITE_NSEC};

    assert(t.et != OS_TIME_INVALID);

    if (!OS_TIMEE_ISINFINITE(t)) {
        tr.tv_sec = (os_timeSec)(t.et / OS_TIME_SECOND);
        tr.tv_nsec = (os_int32)(t.et % OS_TIME_SECOND);
    }
    return tr;
}

os_time
os_durationToTime(
    os_duration d)
{
    os_time tr = {OS_TIME_INFINITE_SEC, OS_TIME_INFINITE_NSEC};

    assert(d != OS_DURATION_INVALID);

    if (d != OS_DURATION_INFINITE) {
        tr.tv_sec = (os_timeSec)(d / OS_DURATION_SECOND);
        tr.tv_nsec = (os_int32)(d % OS_DURATION_SECOND);
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

os_timeReal
os_durationToReal(
    os_duration d)
{
    volatile os_timeReal tr;

    assert(d != OS_DURATION_INVALID);
    tr = (os_timeReal)d / (os_timeReal)OS_DURATION_SECOND;

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
    assert(tr.tv_nsec >= 0 && tr.tv_nsec < 1000000000);
    return tr;
}

os_duration
os_realToDuration(
    os_timeReal t)
{
    os_duration dr;
    dr = (os_duration)(t*(os_timeReal)OS_DURATION_SECOND);
    return dr;
}

#ifndef LITE
os_result
os_timeGetPowerEvents(
    os_timePowerEvents *events,
    os_duration maxBlockingTime)
{
    static os_duration mt_el_offset = 0; /* offset of os_timeGetElapsed() to os_timeGetMonotonic */
    static int mt_el_offset_isset = 0;
    const os_duration maxPowerEventDiffTime = 3*OS_DURATION_SECOND; /* use 3 seconds as the threshold to determine whether a power event has occurred or not */
    const os_duration delay = 100*OS_DURATION_MILLISECOND;
    os_timeE el;
    os_timeM mt, mtn;
    os_duration toBlock, delta;
    os_boolean stateEqualsEvents;
    os_boolean timeoutDetected = OS_FALSE;
    static os_timePowerEvents state;

    assert(events);

    toBlock = maxBlockingTime;

    do {
        /* TODO: When OSPL-4394 (clock-property querying) is done, this loop can
         * be skipped when either of the clocks isn't available with the right
         * properties. Perhaps the call should even return something to signal
         * that this call isn't supported. */
        mt = os_timeMGet();
        el = os_timeEGet();

        /* This isn't thread-safe, but since syncing should be more or less
         * idempotent, this doesn't matter functionally. */
        if (!mt_el_offset_isset) {
            mt_el_offset = (os_duration)(mt.mt - el.et);
            mt_el_offset_isset = 1;
        }

        /* A resume event is detected when the elapsed time differs from the
         * monotonic time (expressed in elapsed-time) > maxPowerEventDiffTime. */
        mtn = os_timeMSub(mt, mt_el_offset);
        delta = (os_duration)(el.et - mtn.mt);
        if (delta > maxPowerEventDiffTime) {
            pa_inc32_nv(&state.resumeCount);
            /* Updating state.resumeLastDetected is NOT thread-safe! Consequently,
             * these could be assigned an incorrect time value that does not
             * reflect the actual time of occurrence of a power event when
             * different threads are setting this value concurrently. The time
             * value is (according to the interface) supposed to be used for
             * logging only. */
            state.resumeLastDetected = os_timeWGet();
        }

        /* In all cases after the above check, the clocks can be re-synced. This
         * isn't thread-safe, but since re-syncing should be more or less
         * idempotent, this doesn't matter functionally. */
        mt_el_offset = (os_duration)(mt.mt - el.et);

        /* If maxBlockingTime == 0, events is not an in-parameter, so its
         * contents shouldn't be inspected. Furthermore, the function should
         * never return os_resultTimeOut in this case, so break out of the loop. */
        if (maxBlockingTime == 0) {
            break;
        }

        stateEqualsEvents = (memcmp(&state, events, sizeof state) == 0) ? OS_TRUE : OS_FALSE;

        if (stateEqualsEvents == OS_TRUE) {
            if (toBlock <= 0) {
                /* maxBlockingTime reached, break the loop */
                timeoutDetected = OS_TRUE;
            } else if (delay < toBlock) {
                /* It is safe to sleep for delay */
                os_sleep(delay);
                /* Set the new max blocking time and redo the loop. */
                toBlock -= delay;
            } else {
                /* The time to block is less than delay. */
                os_sleep(toBlock);
                /* Set the resulting max blocking time zero to check for power
                 * events one more time. */
                toBlock = 0;
            }
        }
    } while ( (stateEqualsEvents == OS_TRUE) && (timeoutDetected == OS_FALSE) );

    /* Store current state in events */
    *events = state;

    return timeoutDetected ? os_resultTimeout : os_resultSuccess;
}
#endif

/* Internal userclock type */
struct dds_userclock_t {
    os_int64 seconds;
    os_int32 nanoseconds;
};

static os_timeW os__timeWGet();
static os_time (*_userClockFunc)(void) = NULL;
static struct dds_userclock_t (*_userClock64Func)(void) = NULL;

/** \brief Set the user clock
 *
 * \b os_timeSetUserClock sets the current time source
 * get function.
 */
void
os_timeSetUserClock(
    os_time (*userClockFunc)(void))
{
    _userClockFunc = userClockFunc;
}

void
os_timeSetUserClock64(
    os_fptr userClock64Func)
{
    _userClock64Func = (struct dds_userclock_t (*)(void))userClock64Func;
}

/** \brief Get the current time
 *
 * This common wrapper implements the user-clock overloading.
 */
os_time
os_timeGet (
    void)
{
    os_time t;
    os_timeW w;

    w = os_timeWGet();
    t.tv_sec = (os_timeSec)OS_TIMEW_GET_SECONDS(w);
    t.tv_nsec = (os_int32)OS_TIMEW_GET_NANOSECONDS(w);

    return t;
}

os_timeW
os_timeWGet(void)
{
    os_timeW t;

    if (_userClock64Func) {
        struct dds_userclock_t dt = _userClock64Func();
#if 0
        /* Currently disabled as it's not possible to generate an error report
         * from within this function as the OS_REPORT will call this function.
         */
        if ((dt.seconds > OS_TIME_MAX_VALID_SECONDS) ||
            (dt.seconds < 0) ||
            (dt.nanoseconds >= (os_int32)OS_TIME_SECOND) ||
            (dt.nanoseconds < 0)) {
            OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
                      "UserClock return an invalid time (seconds=%" PA_PRId64 ", nanoseconds=%d)\n",
                      dt.seconds, dt.nanoseconds);
        }
#endif
        t = OS_TIMEW_INIT(dt.seconds, dt.nanoseconds);
    } else if (_userClockFunc) {
        os_time ot = _userClockFunc();
#if 0
        /* Currently disabled as it's not possible to generate an error report
         * from within this function as the OS_REPORT will call this function.
         */
        if ((ot.tv_sec < 0) ||
            (ot.tv_nsec >= (os_int32)OS_TIME_SECOND) ||
            (ot.tv_nsec < 0)) {
            OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
                      "UserClock return an invalid time (tv_sec=%d, tv_nsec=%d)\n",
                      ot.tv_sec, ot.tv_nsec);
        }
#endif
        t = OS_TIMEW_INIT(ot.tv_sec, ot.tv_nsec);
    } else {
        t = os__timeWGet();
    }
    return t;
}

os_time
os_timeGetMonotonic(void)
{
    os_time t;
    os_timeM m;

    m = os_timeMGet();
    t.tv_sec = (os_timeSec)OS_TIMEM_GET_SECONDS(m);
    t.tv_nsec = (os_int32)OS_TIMEM_GET_NANOSECONDS(m);

    return t;
}

os_time
os_timeGetElapsed(void)
{
    os_time t;
    os_timeE e;

    e = os_timeEGet();
    t.tv_sec = (os_timeSec)OS_TIMEE_GET_SECONDS(e);
    t.tv_nsec = (os_int32)OS_TIMEE_GET_NANOSECONDS(e);

    return t;
}

os_result
os_nanoSleep(
    os_time delay)
{
    return os_sleep(OS_DURATION_INIT(delay.tv_sec, delay.tv_nsec));
}

os_timeW
os_timeWInit(
    os_uint64 value)
{
    os_timeW t;
    t.wt = value;
    return t;
}

os_timeM
os_timeMInit(
    os_uint64 value)
{
    os_timeM t;
    t.mt = value;
    return t;
}

os_timeE
os_timeEInit(
    os_uint64 value)
{
    os_timeE t;
    t.et = value;
    return t;
}

char *
os_durationImage (
    os_duration d,
    char *buf,
    os_size_t bufsz)
{
    char *b = NULL;
    os_size_t index = 0;

    assert(buf);
    assert(bufsz >= OS_DURATION_TO_STRING_BUFSIZE);

    if (buf && bufsz >= OS_DURATION_TO_STRING_BUFSIZE) {
        if (OS_DURATION_ISINVALID(d)) {
            strncpy(buf, "INVALID", bufsz);
        } else if (OS_DURATION_ISINFINITE(d)) {
            strncpy(buf, "INFINITE", bufsz);
        } else if (OS_DURATION_ISMIN_INFINITE(d)) {
            strncpy(buf, "-INFINITE", bufsz);
        } else {
            if (d < 0) {
                strncpy(buf, "-", bufsz);
                index++;
                d = os_durationAbs(d);
            }
            snprintf(&buf[index], bufsz-index, "%" PA_PRId64 ".%09u", d/OS_DURATION_SECOND, (os_uint32)(d%OS_DURATION_SECOND));
        }
        b = buf;
    }
    return b;
}

static char *
timeImage (
    os_uint64 t,
    char *buf,
    os_size_t bufsz)
{
    char *b = NULL;

    assert(buf);
    assert(bufsz >= OS_TIME_TO_STRING_BUFSIZE);

    if (buf && bufsz >= OS_TIME_TO_STRING_BUFSIZE) {
        if (OS_TIME_ISINVALID(t)) {
            strncpy(buf, "INVALID", bufsz);
        } else if (OS_TIME_ISINFINITE(t)) {
            strncpy(buf, "INFINITE", bufsz);
        } else {
            snprintf(buf, bufsz, "%" PA_PRIu64 ".%09u", t/OS_TIME_SECOND, (os_uint32)(t%OS_TIME_SECOND));
        }
        b = buf;
    }
    return b;
}

char *
os_timeWImage (
    os_timeW t,
    char *buf,
    os_size_t bufsz)
{
    return timeImage(t.wt, buf, bufsz);
}

char *
os_timeMImage (
    os_timeM t,
    char *buf,
    os_size_t bufsz)
{
    return timeImage(t.mt, buf, bufsz);
}

char *
os_timeEImage (
    os_timeE t,
    char *buf,
    os_size_t bufsz)
{
    return timeImage(t.et, buf, bufsz);
}
