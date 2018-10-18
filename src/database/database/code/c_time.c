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

#include "c_time.h"
#include "os_time.h"
#include "os_report.h"

const c_time C_TIME_ZERO         = {0, 0};
const c_time C_TIME_INFINITE     = {0x7fffffff, 0x7fffffffU};
const c_time C_TIME_MIN_INFINITE = {-0x7fffffff,  0x7fffffffU};
const c_time C_TIME_INVALID      = {-1,  0xffffffffU};

c_time
c_timeNormalize(
    c_time t)
{
    if (t.seconds == C_TIME_INFINITE.seconds) {
        return C_TIME_INFINITE;
    } else  {
        if (t.seconds == C_TIME_MIN_INFINITE.seconds) {
            return C_TIME_MIN_INFINITE;
        }
    }

    while (t.nanoseconds >= 1000000000) {
        t.seconds += 1;
        if (t.seconds == C_TIME_INFINITE.seconds) {
            return C_TIME_INFINITE;
        }
        t.nanoseconds -= 1000000000;
    }

    return t;
}

c_equality
c_timeCompare(
    c_time t1,
    c_time t2)
{
    assert (CHECK_TIME(
                C_TIME_TIMEKIND_EQUALS(t1, t2) /* Equal times can always be compared */
                        || (C_TIME_ISRELATIVE(t1) && C_TIME_ISRELATIVE(t2)) /* Both relative */
                        || (C_TIME_ISABSOLUTE(t1) && C_TIME_ISINFINITE(t2))
                        || (C_TIME_ISINFINITE(t1) && C_TIME_ISABSOLUTE(t2))
                )
           );
    assert ((C_TIME_NANOS(t1.nanoseconds) < 1000000000) ||
            (t1.nanoseconds == C_TIME_INFINITE.nanoseconds));
    assert ((C_TIME_NANOS(t2.nanoseconds) < 1000000000) ||
            (t2.nanoseconds == C_TIME_INFINITE.nanoseconds));

    if ((C_TIME_NANOS(t1.nanoseconds) >= 1000000000) &&
        ((t1.nanoseconds != C_TIME_INFINITE.nanoseconds) ||
         ((t1.seconds != C_TIME_INFINITE.seconds) &&
          (t1.seconds != C_TIME_MIN_INFINITE.seconds) )))
    {
        OS_REPORT(OS_ERROR,
                    "c_timeCompare",0,"Illegal time t1; <%d.%u>",
                    t1.seconds,
                    t1.nanoseconds);
    }
    if ((C_TIME_NANOS(t2.nanoseconds) >= 1000000000) &&
        ((t2.nanoseconds != C_TIME_INFINITE.nanoseconds) ||
         ((t2.seconds != C_TIME_INFINITE.seconds) &&
          (t2.seconds != C_TIME_MIN_INFINITE.seconds)) ))
    {
        OS_REPORT(OS_ERROR,
                    "c_timeCompare",0,"Illegal time t2; <%d.%u>",
                    t2.seconds,
                    t2.nanoseconds);
    }
    if (t1.seconds > t2.seconds) return C_GT;
    if (t1.seconds < t2.seconds) return C_LT;
    if (C_TIME_NANOS(t1.nanoseconds) > C_TIME_NANOS(t2.nanoseconds)) return C_GT;
    if (C_TIME_NANOS(t1.nanoseconds) < C_TIME_NANOS(t2.nanoseconds)) return C_LT;
    return C_EQ;
}

c_time
c_timeAdd(
    c_time t1,
    c_time t2)
{
    c_time tr;

    /* The only valid additions are C_TIME_ABSOLUTE_TIME + C_TIME_RELATIVE_TIME
     * or C_TIME_RELATIVE_TIME + C_TIME_RELATIVE_TIME.
     */
    assert (CHECK_TIME(
               (C_TIME_ISABSOLUTE(t1) && C_TIME_ISRELATIVE(t2)) ||
                (C_TIME_ISRELATIVE(t1) && C_TIME_ISABSOLUTE(t2)) ||
                (C_TIME_ISRELATIVE(t1) && C_TIME_ISRELATIVE(t2)))
           );
    assert ((C_TIME_NANOS(t1.nanoseconds) < 1000000000) ||
            (t1.nanoseconds == C_TIME_INFINITE.nanoseconds));
    assert ((C_TIME_NANOS(t2.nanoseconds) < 1000000000) ||
            (t2.nanoseconds == C_TIME_INFINITE.nanoseconds));

    /* Don't accept adding opposite sign infinites */
    assert(!(c_timeIsInfinite(t1) && c_timeIsMinInfinite(t2)));
    assert(!(c_timeIsMinInfinite(t1) && c_timeIsInfinite(t2)));

    if (t1.nanoseconds == C_TIME_INFINITE.nanoseconds) {
        if ((t1.seconds == C_TIME_INFINITE.seconds) ||
            (t1.seconds == C_TIME_MIN_INFINITE.seconds))
        {
            return t1; /* result stays infinite. */
        } else {
            OS_REPORT(OS_ERROR,
                        "c_timeAdd",0,"Illegal time t1; <%d.%u>",
                        t1.seconds,
                        t1.nanoseconds);
            assert(t1.seconds == C_TIME_INFINITE.seconds);
        }
    }
    if (t2.nanoseconds == C_TIME_INFINITE.nanoseconds) {
        if ((t2.seconds == C_TIME_INFINITE.seconds) ||
            (t2.seconds == C_TIME_INFINITE.seconds))
        {
            return t2; /* result stays infinite. */
        } else {
            OS_REPORT(OS_ERROR,
                        "c_timeAdd",0,"Illegal time t2; <%d.%u>",
                        t2.seconds,
                        t2.nanoseconds);
            assert(t2.seconds == C_TIME_INFINITE.seconds);
        }
    }
    if (t2.seconds > 0) {
        if (C_TIME_INFINITE.seconds - t2.seconds <= t1.seconds) {
            /* is identical to
             * 'C_TIME_INFINITE.seconds <= t1.seconds + t2.seconds'
             * In other words the sum is larger than infinite,
             * so results must be infinite.
             */
            return C_TIME_INFINITE;
        }
    } else {
        if (C_TIME_MIN_INFINITE.seconds - t2.seconds >= t1.seconds) {
            /* is identical to
             * '-(C_TIME_MIN_INFINITE.seconds <= t1.seconds + t2.seconds)'
             * In other words the sum is smaller than min infinite,
             * so results must be min infinite.
             */
            return C_TIME_MIN_INFINITE;
        }
    }

    tr.seconds = t1.seconds + t2.seconds;
    tr.nanoseconds = C_TIME_NANOS(t1.nanoseconds) + C_TIME_NANOS(t2.nanoseconds);
    tr = c_timeNormalize(tr);

    C_TIME_SET_KIND(tr, C_TIME_TIMEKIND_EQUALS(t1, t2) ? C_TIME_GET_KIND(t1) : (C_TIME_ISABSOLUTE(t1) ? C_TIME_GET_KIND(t1) : C_TIME_GET_KIND(t2)));

    return tr;
}

c_time
c_timeSub(
    c_time t1,
    c_time t2)
{
    c_time tr;
    c_long nsecsResult;

    /* All is OK, except mixed REALTIME and MONOTONIC or ELAPSED */
    assert (CHECK_TIME(
                C_TIME_TIMEKIND_EQUALS(t1, t2) ||
                (C_TIME_ISABSOLUTE(t1) && C_TIME_ISRELATIVE(t2)) ||
                (C_TIME_ISRELATIVE(t1) && C_TIME_ISABSOLUTE(t2)))
           );
    assert ((C_TIME_NANOS(t1.nanoseconds) < 1000000000) ||
            (t1.nanoseconds == C_TIME_INFINITE.nanoseconds));
    assert ((C_TIME_NANOS(t2.nanoseconds) < 1000000000) ||
            (t2.nanoseconds == C_TIME_INFINITE.nanoseconds));

    /* Don't accept substracting equal sign infinites */
    assert(!(c_timeIsInfinite(t1) && c_timeIsInfinite(t2)));
    assert(!(c_timeIsMinInfinite(t1) && c_timeIsMinInfinite(t2)));

    if (t1.nanoseconds == C_TIME_INFINITE.nanoseconds) {
        if ((t1.seconds == C_TIME_INFINITE.seconds) ||
            (t1.seconds == C_TIME_MIN_INFINITE.seconds)) {
            return t1; /* result stays infinite. */
        } else {
            OS_REPORT(OS_ERROR,
                        "c_timeSub",0,"Illegal time t1; <%d.%u>",
                        t1.seconds,
                        t1.nanoseconds);
            assert(t1.seconds == C_TIME_INFINITE.seconds);
        }
    } else if (C_TIME_NANOS(t1.nanoseconds) >= 1000000000) {
        OS_REPORT(OS_ERROR,
                    "c_timeSub",0,"Illegal time t1; <%d.%u>",
                    t1.seconds,
                    t1.nanoseconds);
        assert(C_TIME_NANOS(t1.nanoseconds) < 1000000000);
    }
    if (t2.nanoseconds == C_TIME_INFINITE.nanoseconds) {
        if ((t2.seconds == C_TIME_INFINITE.seconds) ||
            (t2.seconds == C_TIME_MIN_INFINITE.seconds)) {
            return t2; /* result stays infinite. */
        } else {
            OS_REPORT(OS_ERROR,
                        "c_timeSub",0,"Illegal time t2; <%d.%u>",
                        t2.seconds,
                        t2.nanoseconds);
            assert(t2.seconds == C_TIME_INFINITE.seconds);
        }
    } else if (C_TIME_NANOS(t2.nanoseconds) >= 1000000000) {
        OS_REPORT(OS_ERROR,
                    "c_timeSub",0,"Illegal time t2; <%d.%u>",
                    t2.seconds,
                    t2.nanoseconds);
        assert(C_TIME_NANOS(t2.nanoseconds) < 1000000000);
    }

    if (t2.seconds > 0) {
        if (C_TIME_MIN_INFINITE.seconds + t2.seconds >= t1.seconds) {
            /* is identical to
             * 'C_TIME_MIN_INFINITE.seconds >= t1.seconds - t2.seconds'
             * In other words the sum is smaller than min infinite,
             * so results must be min infinite.
             */
            return C_TIME_MIN_INFINITE;
        }
    } else {
        if (C_TIME_INFINITE.seconds + t2.seconds <= t1.seconds) {
            /* is identical to
             * '-(C_TIME_INFINITE.seconds >= t1.seconds - t2.seconds)'
             * In other words the sum is larger than infinite,
             * so results must be infinite.
             */
            return C_TIME_INFINITE;
        }
   }

    tr.seconds = t1.seconds - t2.seconds;

    if (tr.seconds == C_TIME_INFINITE.seconds) {
        return C_TIME_INFINITE;
    } else  {
        if (tr.seconds == C_TIME_MIN_INFINITE.seconds) {
            return C_TIME_MIN_INFINITE;
        }
    }

    nsecsResult = (c_long) C_TIME_NANOS(t1.nanoseconds) - (c_long) C_TIME_NANOS(t2.nanoseconds);

    if (nsecsResult < 0) {
        tr.seconds--;
        if (tr.seconds == C_TIME_MIN_INFINITE.seconds) {
            return C_TIME_MIN_INFINITE;
        }
       nsecsResult += 1000000000;
    }
    tr.nanoseconds = (c_ulong) nsecsResult;
    c_timeNormalize(tr);
    C_TIME_SET_KIND(tr, C_TIME_TIMEKIND_EQUALS(t1, t2) ? (C_TIME_ISABSOLUTE(t1) ? C_TIME_RELATIVE : C_TIME_GET_KIND(t1))  : (C_TIME_ISABSOLUTE(t1) ? C_TIME_GET_KIND(t1) : C_TIME_GET_KIND(t2)));

    return tr;
}

c_bool
c_timeValid(
    c_time t1)
{
    c_bool valid;

    if ( ((t1.seconds == C_TIME_INFINITE.seconds) &&
          (t1.nanoseconds == C_TIME_INFINITE.nanoseconds)) ||
         ((t1.seconds == C_TIME_MIN_INFINITE.seconds) &&
          (t1.nanoseconds == C_TIME_MIN_INFINITE.nanoseconds)) )
    {
        valid = TRUE;
    } else {
        if ( (t1.seconds == C_TIME_INFINITE.seconds) ||
             (t1.seconds == C_TIME_MIN_INFINITE.seconds)) {
            valid = FALSE;
        } else {
            if (C_TIME_NANOS(t1.nanoseconds) < 1000000000U) {
                valid = TRUE;
            } else {
                valid = FALSE;
            }
        }
    }

    return valid;
}

c_timeResult
c_timeNanoSleep(
    c_time interval)
{
    os_time t;

    /* This call should only accept relative times. C_TIME_ISRELATIVE(...) does
     * include the INFINITE's as well, so !C_TIME_ISABSOLUTE(...) is used
     * instead. */
    assert (CHECK_TIME(!C_TIME_ISABSOLUTE(interval)));

    t.tv_sec = interval.seconds;
    t.tv_nsec = (os_int32) C_TIME_NANOS(interval.nanoseconds);
    return (c_timeResult) os_nanoSleep(t);

}

/** \brief return floating point representation of c_time reprersentation.
 *
 * because of the limited floating point represenation (64 bits)
 * the value will be limited to a resoltion of about 1 us.
 */
c_double
c_timeToReal(
    c_time t)
{
    c_double tr;

    /* always true expr => gcc warning :
     * assert (t.nanoseconds >= 0);
     */
    assert (C_TIME_NANOS(t.nanoseconds) < 1000000000);

    tr = (c_double)t.seconds +
         (c_double)C_TIME_NANOS(t.nanoseconds) / (c_double)1000000000.0;

    return tr;
}

/** \brief return c_time represenation of floating point reprersentation.
 *
 * because of the limited floating point represenation (64 bits)
 * the value will be limited to a resoltion of about 1 us.
 */
c_time
c_timeFromReal (
    c_double d)
{
    c_time tr;

    if (d >= (c_double)C_TIME_INFINITE.seconds) {
        OS_REPORT(OS_ERROR,"c_timeFromReal",0,"overflow detected: real exceeds max int");
    }
    tr.seconds = (c_long)d;
    tr.nanoseconds = (c_ulong)((d-(c_double)tr.seconds) *
                                  (c_double)1000000000.0);
    C_TIME_SET_KIND(tr, C_TIME_RELATIVE);

    return tr;
}

os_timeW
c_timeToTimeW(
    c_time t)
{
    os_timeW tr = OS_TIMEW_INVALID;

    if (c_timeIsInfinite(t)) {
        tr = OS_TIMEW_INFINITE;
    } else if (c_timeIsMinInfinite(t)) {
        tr = OS_TIMEW_ZERO;
    } else if (t.seconds >= 0) {
        assert(C_TIME_NANOS(t.nanoseconds) < 1000000000U);
        tr.wt = (os_uint64)t.seconds*OS_TIME_SECOND + (os_uint64)t.nanoseconds;
    }
    return tr;
}

os_timeM
c_timeToTimeM(
    c_time t)
{
    os_timeM tr = OS_TIMEM_INVALID;

    if (c_timeIsInfinite(t)) {
        tr = OS_TIMEM_INFINITE;
    } else if (c_timeIsMinInfinite(t)) {
        tr = OS_TIMEM_ZERO;
    } else if (t.seconds >= 0) {
        assert(C_TIME_NANOS(t.nanoseconds) < 1000000000U);
        tr.mt = (os_uint64)t.seconds*OS_TIME_SECOND + (os_uint64)t.nanoseconds;
    }
    return tr;
}

os_timeE
c_timeToTimeE(
    c_time t)
{
    os_timeE tr = OS_TIMEE_INVALID;

    if (c_timeIsInfinite(t)) {
        tr = OS_TIMEE_INFINITE;
    } else if (c_timeIsMinInfinite(t)) {
        tr = OS_TIMEE_ZERO;
    } else if (t.seconds >= 0) {
        assert(C_TIME_NANOS(t.nanoseconds) < 1000000000U);
        tr.et = (os_uint64)t.seconds*OS_TIME_SECOND + (os_uint64)t.nanoseconds;
    }
    return tr;
}

os_duration
c_timeToDuration(
    c_time t)
{
    os_duration dr;

    assert((t.seconds == C_TIME_INFINITE.seconds) == (t.nanoseconds == C_TIME_INFINITE.nanoseconds));

    if (c_timeIsInfinite(t)) {
        dr = OS_DURATION_INFINITE;
    } else if (c_timeIsInvalid(t)) {
        dr = OS_DURATION_INVALID;
    } else {
        assert(C_TIME_NANOS(t.nanoseconds) < 1000000000U);
        dr = (os_duration)t.seconds*OS_DURATION_SECOND + (os_duration)t.nanoseconds;
    }
    return dr;
}

c_time
c_timeFromTimeW(
    os_timeW t)
{
    c_time tr = C_TIME_INFINITE;

    if (OS_TIMEW_ISINVALID(t)) {
        tr.seconds = C_TIME_INVALID.seconds;
        tr.nanoseconds = C_TIME_INVALID.nanoseconds;
    } else if (!OS_TIMEW_ISINFINITE(t)) {
        tr.seconds = (c_long)(t.wt / OS_TIME_SECOND);
        tr.nanoseconds = (c_ulong)(t.wt % OS_TIME_SECOND);
    }

    return tr;
}

c_time
c_timeFromTimeM(
    os_timeM t)
{
    c_time tr = C_TIME_INFINITE;

    if (OS_TIMEM_ISINVALID(t)) {
        tr.seconds = C_TIME_INVALID.seconds;
        tr.nanoseconds = C_TIME_INVALID.nanoseconds;
    } else if (!OS_TIMEM_ISINFINITE(t)) {
        tr.seconds = (c_long)(t.mt / OS_TIME_SECOND);
        tr.nanoseconds = (c_ulong)(t.mt % OS_TIME_SECOND);
    }
    return tr;
}

c_time
c_timeFromTimeE(
    os_timeE t)
{
    c_time tr = C_TIME_INFINITE;

    if (OS_TIMEE_ISINVALID(t)) {
        tr.seconds = C_TIME_INVALID.seconds;
        tr.nanoseconds = C_TIME_INVALID.nanoseconds;
    } else if (!OS_TIMEE_ISINFINITE(t)) {
        tr.seconds = (c_long)(t.et / OS_TIME_SECOND);
        tr.nanoseconds = (c_ulong)(t.et % OS_TIME_SECOND);
    }
    return tr;
}

c_time
c_timeFromDuration(
    os_duration d)
{
    c_time tr = C_TIME_INFINITE;

    if (d == OS_DURATION_INVALID) {
        tr.seconds = C_TIME_INVALID.seconds;
        tr.nanoseconds = C_TIME_INVALID.nanoseconds;
    } else if (d != OS_DURATION_INFINITE) {
        tr.seconds = (c_long)(d/ OS_DURATION_SECOND);
        tr.nanoseconds = (c_ulong)(d% OS_DURATION_SECOND);
    }
    return tr;
}

