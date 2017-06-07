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
/****************************************************************
 * Interface definition for time management of SPLICE-DDS       *
 ****************************************************************/

#ifndef OS_TIME_H
#define OS_TIME_H

/** \file os_time.h
 *  \brief Time management - get time, delay and calculate with time
 */

#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

/** \brief Specification of maximum time values (platform dependent)
 *
 * Note these definitions are depricated.
 *
 */
#define OS_TIME_INFINITE_SEC OS_MAX_INTEGER(os_timeSec)
#define OS_TIME_INFINITE_NSEC OS_MAX_INTEGER(os_int32)

#define OS_TIME_ZERO_SEC     0
#define OS_TIME_ZERO_NSEC    0

#define os_timeIsInfinite(t) \
    (((t).tv_sec == OS_TIME_INFINITE_SEC) && ((t).tv_nsec == OS_TIME_INFINITE_NSEC))

/** \brief Specification of common time and duration values
 */
#define OS_DURATION_MICROSECOND ((os_int64) 1000)
#define OS_DURATION_MILLISECOND ((os_int64) 1000000)
#define OS_DURATION_SECOND ((os_int64) 1000000000)
#define OS_DURATION_INFINITE ((os_int64) 0x7fffffffffffffffll)
#define OS_DURATION_MIN_INFINITE (-OS_DURATION_INFINITE) /* 0x8000000000000001ll */
#define OS_DURATION_INVALID ((os_int64) 0x8000000000000000ll)
#define OS_DURATION_ZERO ((os_int64) 0x0)

#define OS_DURATION_ISZERO(d) (d == OS_DURATION_ZERO)
#define OS_DURATION_ISINFINITE(d) (d == OS_DURATION_INFINITE)
#define OS_DURATION_ISMIN_INFINITE(d) (d == OS_DURATION_MIN_INFINITE)
#define OS_DURATION_ISINVALID(d) (d == OS_DURATION_INVALID)
#define OS_DURATION_ISPOSITIVE(d) (d <= OS_DURATION_INFINITE && d >= 0)
#define OS_DURATION_PRINT(d) ((d)/OS_DURATION_SECOND),(os_uint32)((d)%OS_DURATION_SECOND)

#define OS_DURATION_GET_SECONDS(d) (d/OS_DURATION_SECOND)
#define OS_DURATION_GET_NANOSECONDS(d) ((os_int32)(d%OS_DURATION_SECOND))

#define OS_DURATION_SEC(d) ((d)%OS_DURATION_SECOND)
#define OS_DURATION_NSEC(d) ((d) < 0 ? -((d)/OS_DURATION_SECOND) : ((d)/OS_DURATION_SECOND))
#define OS_DURATION_INIT(s,ns) (s*OS_DURATION_SECOND+ns)

#define OS_TIME_MICROSECOND ((os_uint64) 1000)
#define OS_TIME_MILLISECOND ((os_uint64) 1000000)
#define OS_TIME_SECOND ((os_uint64) 1000000000)

#define OS_TIME_ZERO ((os_uint64) 0x0)
#define OS_TIME_INFINITE ((os_uint64) 0x7fffffffffffffffull)
#define OS_TIME_INVALID ((os_uint64) 0xffffffffffffffffull)

/* (OS_TIME_INFINITE - (OS_TIME_SECOND - 1) - 1) / OS_TIME_SECOND */
#define OS_TIME_MAX_VALID_SECONDS (9223372035ll)

/** \brief Specification of common time and duration test and setters
 */

#define OS_TIMEW_MICROSECONDS(d) os_timeWAdd(OS_TIMEW_ZERO,(d)*OS_DURATION_MICROSECOND)
#define OS_TIMEM_MICROSECONDS(d) os_timeMAdd(OS_TIMEM_ZERO,(d)*OS_DURATION_MICROSECOND)
#define OS_TIMEE_MICROSECONDS(d) os_timeEAdd(OS_TIMEE_ZERO,(d)*OS_DURATION_MICROSECOND)

#define OS_TIMEW_MILLISECONDS(d) os_timeWAdd(OS_TIMEW_ZERO,(d)*OS_DURATION_MILLISECOND)
#define OS_TIMEM_MILLISECONDS(d) os_timeMAdd(OS_TIMEM_ZERO,(d)*OS_DURATION_MILLISECOND)
#define OS_TIMEE_MILLISECONDS(d) os_timeEAdd(OS_TIMEE_ZERO,(d)*OS_DURATION_MILLISECOND)

#define OS_TIMEW_SECONDS(d) os_timeWAdd(OS_TIMEW_ZERO,(d)*OS_DURATION_SECOND)
#define OS_TIMEM_SECONDS(d) os_timeMAdd(OS_TIMEM_ZERO,(d)*OS_DURATION_SECOND)
#define OS_TIMEE_SECONDS(d) os_timeEAdd(OS_TIMEE_ZERO,(d)*OS_DURATION_SECOND)

#define OS_TIME_ISINFINITE(t) ((t)+1 > OS_TIME_INFINITE) /* +1 => INVALID not considered future */
#define OS_TIMEW_ISINFINITE(t) OS_TIME_ISINFINITE((t).wt)
#define OS_TIMEM_ISINFINITE(t) OS_TIME_ISINFINITE((t).mt)
#define OS_TIMEE_ISINFINITE(t) OS_TIME_ISINFINITE((t).et)

#define OS_TIME_ISFINITE(t) ((t) < OS_TIME_INFINITE)
#define OS_TIMEW_ISFINITE(t) OS_TIME_ISFINITE((t).wt)
#define OS_TIMEM_ISFINITE(t) OS_TIME_ISFINITE((t).mt)
#define OS_TIMEE_ISFINITE(t) OS_TIME_ISFINITE((t).et)

#define OS_TIME_ISINVALID(t) ((t) == OS_TIME_INVALID)
#define OS_TIMEW_ISINVALID(t) OS_TIME_ISINVALID((t).wt)
#define OS_TIMEM_ISINVALID(t) OS_TIME_ISINVALID((t).mt)
#define OS_TIMEE_ISINVALID(t) OS_TIME_ISINVALID((t).et)

#define OS_TIME_ISZERO(t) ((t) == OS_TIME_ZERO)
#define OS_TIMEW_ISZERO(t) OS_TIME_ISZERO((t).wt)
#define OS_TIMEM_ISZERO(t) OS_TIME_ISZERO((t).mt)
#define OS_TIMEE_ISZERO(t) OS_TIME_ISZERO((t).et)

#define OS_TIME_ISNORMALIZED(t) ((t) <= OS_TIME_INFINITE)
#define OS_TIMEW_ISNORMALIZED(t) OS_TIME_ISNORMALIZED((t).wt)
#define OS_TIMEM_ISNORMALIZED(t) OS_TIME_ISNORMALIZED((t).mt)
#define OS_TIMEE_ISNORMALIZED(t) OS_TIME_ISNORMALIZED((t).et)

#define OS_TIME_NORMALIZE(t)  OS_TIME_ISINFINITE(t) ? OS_TIME_INFINITE : (t)
#define OS_TIMEW_NORMALIZE(t) OS_TIMEW_ISINFINITE(t) ? OS_TIMEW_INFINITE : (t)
#define OS_TIMEM_NORMALIZE(t) OS_TIMEM_ISINFINITE(t) ? OS_TIMEM_INFINITE : (t)
#define OS_TIMEE_NORMALIZE(t) OS_TIMEE_ISINFINITE(t) ? OS_TIMEE_INFINITE : (t)

#define OS_TIME_PRINT(t)  ((t)/OS_TIME_SECOND), (os_uint32)((t)%OS_TIME_SECOND)
#define OS_TIMEW_PRINT(t) OS_TIME_PRINT((t).wt)
#define OS_TIMEM_PRINT(t) OS_TIME_PRINT((t).mt)
#define OS_TIMEE_PRINT(t) OS_TIME_PRINT((t).et)

#define OS_TIMEW_INIT(s,ns) os_timeWInit((os_uint64)(s)*OS_TIME_SECOND + (os_uint64)(ns))
#define OS_TIMEM_INIT(s,ns) os_timeMInit((os_uint64)(s)*OS_TIME_SECOND + (os_uint64)(ns))
#define OS_TIMEE_INIT(s,ns) os_timeEInit((os_uint64)(s)*OS_TIME_SECOND + (os_uint64)(ns))

#define OS_TIME_GET_SECONDS(t)  ((t)/OS_TIME_SECOND)
#define OS_TIMEW_GET_SECONDS(t) OS_TIME_GET_SECONDS((t).wt)
#define OS_TIMEM_GET_SECONDS(t) OS_TIME_GET_SECONDS((t).mt)
#define OS_TIMEE_GET_SECONDS(t) OS_TIME_GET_SECONDS((t).et)

#define OS_TIME_GET_NANOSECONDS(t)  ((os_uint32)((t)%OS_TIME_SECOND))
#define OS_TIMEW_GET_NANOSECONDS(t) OS_TIME_GET_NANOSECONDS((t).wt)
#define OS_TIMEM_GET_NANOSECONDS(t) OS_TIME_GET_NANOSECONDS((t).mt)
#define OS_TIMEE_GET_NANOSECONDS(t) OS_TIME_GET_NANOSECONDS((t).et)

#define OS_TIMEW_GET_VALUE(t) ((t).wt)
#define OS_TIMEM_GET_VALUE(t) ((t).mt)
#define OS_TIMEE_GET_VALUE(t) ((t).et)

#define OS_TIMEW_SET_VALUE(t,v) ((t).wt = (os_uint64)v)
#define OS_TIMEM_SET_VALUE(t,v) ((t).mt = (os_uint64)v)
#define OS_TIMEE_SET_VALUE(t,v) ((t).et = (os_uint64)v)

/** \brief Time structure definitions
 */
typedef struct os_time {
    /** Seconds since the Unix epoch; 1-jan-1970 00:00:00 (UTC) */
    os_timeSec tv_sec;
    /** Number of nanoseconds since the Unix epoch, modulo 10^9. */
    os_int32 tv_nsec;
    /** os_time can be used for a duration type with the following
    semantics for negative durations: tv_sec specifies the
    sign of the duration, tv_nsec is always positive and added
    to the real value (thus real value is tv_sec+tv_nsec/10^9,
    for example { -1, 500000000 } is -0.5 seconds) */
} os_time; /* Depricated */

typedef struct os_timeW{    /* wall clock time */
    os_uint64 wt;
} os_timeW;

typedef struct os_timeM{    /* monotonic time */
    os_uint64 mt;
} os_timeM;

typedef struct os_timeE{      /* elapsed time */
    os_uint64 et;
} os_timeE;

typedef os_int64 os_duration; /* duration in nanoseconds. */

static const os_timeW OS_TIMEW_ZERO = {OS_TIME_ZERO};
static const os_timeM OS_TIMEM_ZERO = {OS_TIME_ZERO};
static const os_timeE OS_TIMEE_ZERO = {OS_TIME_ZERO};

static const os_timeW OS_TIMEW_INFINITE = {OS_TIME_INFINITE};
static const os_timeM OS_TIMEM_INFINITE = {OS_TIME_INFINITE};
static const os_timeE OS_TIMEE_INFINITE = {OS_TIME_INFINITE};

static const os_timeW OS_TIMEW_INVALID = {OS_TIME_INVALID};
static const os_timeM OS_TIMEM_INVALID = {OS_TIME_INVALID};
static const os_timeE OS_TIMEE_INVALID = {OS_TIME_INVALID};

/** \brief Basic duration operations Add, Subtract and Compare
 */

OS_API os_duration
os_durationAdd(
    os_duration d1,
    os_duration d2);

OS_API os_duration
os_durationSub(
    os_duration d1,
    os_duration d2);

OS_API os_duration
os_durationAbs(
    os_duration d);

OS_API os_duration
os_durationMul(
    os_duration d,
    double multiply);

OS_API os_compare
os_durationCompare(
    os_duration d1,
    os_duration d2);


/** \brief Get the current time.
 *
 *  The returned time since the Unix Epoch in nanosecond resolution.
 *  (Thursday, 1 January 1970, 00:00:00 (UTC)).
 *
 * Possible Results:
 * - returns "the current time"
 */
OS_API os_time
os_timeGet(void); /* Depricated */

OS_API os_timeW
os_timeWGet(void);

/** \brief Get high resolution, monotonic time.
 *
 * The monotonic clock is a clock with near real-time progression and can be
 * used when a high-resolution time is needed without the need for it to be
 * related to the wall-clock. The resolution of the clock is typically the
 * highest available on the platform.
 *
 * The clock is not guaranteed to be strictly monotonic, but on most common
 * platforms it will be (based on performance-counters or HPET's).
 *
 * If no monotonic clock is available the real time clock is used.
 *
 * \return a high-resolution time with no guaranteed relation to wall-time
 *         when available
 * \return wall-time, otherwise
 */
OS_API os_time
os_timeGetMonotonic(void); /* Depricated */

OS_API os_timeM
os_timeMGet(void);

/** \brief Get high resolution, elapsed (and thus monotonic) time since some
 * fixed unspecified past time.
 *
 * The elapsed time clock is a clock with near real-time progression and can be
 * used when a high-resolution suspend-aware monotonic clock is needed, without
 * having to deal with the complications of discontinuities if for example the
 * time is changed. The fixed point from which the elapsed time is returned is
 * not guaranteed to be fixed over reboots of the system.
 *
 * If no elapsed time clock is available, the monotonic time clock is used as a
 * fall-back.
 *
 * \return elapsed time since some unspecified fixed past time
 * \return the monotonic time otherwise
 */
OS_API os_time
os_timeGetElapsed(void); /* Depricated */

OS_API os_timeE
os_timeEGet(void);

/** \brief Initializes the timeW to a time value
 *
 * Returns:
 * - set wall-time
 */
OS_API os_timeW
os_timeWInit(
    os_uint64 value);

/** \brief Initializes the timeW to a time value
 *
 * Returns:
 * - set monotonic-time
 */
OS_API os_timeM
os_timeMInit(
    os_uint64 value);

/** \brief Initializes the timeW to a time value
 *
 * Returns:
 * - set elapsed-time
 */
OS_API os_timeE
os_timeEInit(
    os_uint64 value);

/** \brief Add time t1 to time t2
 *
 * Possible Results:
 * - returns t1 + t2 when
 *     the result fits within the time structure
 * - returns an unspecified value when
 *     the result does not fit within the time structure
 */
OS_API os_time
os_timeAdd(
    os_time t1,
    os_time t2); /* Depricated */

/** \brief Adds a duration to a time value
 *
 * Returns:
 * - returns t+d
 * - returns OS_TIME_INFINITE if (d == OS_DURATION_INFINITE)
 * - returns OS_TIME_INFINITE if (t+d > OS_TIME_INFINITE)
 */
OS_API os_timeW
os_timeWAdd(
    os_timeW t,
    os_duration d);

OS_API os_timeM
os_timeMAdd(
    os_timeM t,
    os_duration d);

OS_API os_timeE
os_timeEAdd(
    os_timeE t,
    os_duration d);

/** \brief Subtract time t2 from time t1
 *
 * Possible Results:
 * - returns t1 - t2 when
 *     the result fits within the time structure
 * - returns an unspecified value when
 *     the result does not fit within the time structure
 */
OS_API os_time
os_timeSub(
    os_time t1,
    os_time t2); /* Depricated */

/** \brief Subtract time t2 from time t1
 *
 * Results:
 * - returns the duration (t1 - t2)
 * - returns OS_TIME_INFINITE if (t1 == OS_TIME_INFINITE)
 * - returns OS_TIME_INVALID if (t2 == OS_TIME_INFINITE)
 */
OS_API os_timeW
os_timeWSub(
    os_timeW t,
    os_duration d);

OS_API os_timeM
os_timeMSub(
    os_timeM t,
    os_duration d);

OS_API os_timeE
os_timeESub(
    os_timeE t,
    os_duration d);

/** \brief Determine the duration difference between two times
 *
 * Results:
 * - returns the duration (t1 - t2)
 * - returns OS_DURATION_INFINITE if (t1 == OS_TIME_INFINITE)
 * - returns OS_DURATION_INVALID if (t2 == OS_TIME_INFINITE)
 */
OS_API os_duration
os_timeWDiff(
    os_timeW t1,
    os_timeW t2);

OS_API os_duration
os_timeMDiff(
    os_timeM t1,
    os_timeM t2);

OS_API os_duration
os_timeEDiff(
    os_timeE t1,
    os_timeE t2);

/** \brief Multiply time t with a real value
 *
 * Possible Results:
 * - returns t * multiply when
 *     the result fits within the time structure
 * - returns an unspecified value when
 *     the result does not fit within the time structure
 */
OS_API os_time
os_timeMulReal(
    os_time t1,
    double multiply); /* Depricated */

/** \brief Determine the absolute value of time t
 *
 * Possible Results:
 * - returns |t| when
 *     the result fits within the time structure
 * - returns an unspecified value when
 *     the result does not fit within the time structure
 */
OS_API os_time
os_timeAbs(
    os_time t); /* Depricated */

/** \brief Compare time t1 with time t2
 *
 * Possible Results:
 * - returns OS_LESS when
 *     value t1 < value t2
 * - returns OS_MORE when
 *     value t1 > value t2
 * - returns OS_EQUAL when
 *     value t1 = value t2
 */
OS_API os_compare
os_timeCompare(
    os_time t1,
    os_time t2); /* Depricated */

OS_API os_compare
os_timeWCompare(
    os_timeW t1,
    os_timeW t2);

OS_API os_compare
os_timeMCompare(
    os_timeM t1,
    os_timeM t2);

OS_API os_compare
os_timeECompare(
    os_timeE t1,
    os_timeE t2);

/** \brief conversion routines to convert old time to new time representations.
 */
OS_API os_timeW
os_timeToTimeW(
    os_time t);

OS_API os_timeM
os_timeToTimeM(
    os_time t);

OS_API os_timeE
os_timeToTimeE(
    os_time t);

OS_API os_duration
os_timeToDuration(
    os_time t);

/** \brief conversion routines to convert new time representations to old time.
 */
OS_API os_time
os_timeWToTime(
    os_timeW t);

OS_API os_time
os_timeMToTime(
    os_timeM t);

OS_API os_time
os_timeEToTime(
    os_timeE t);

OS_API os_time
os_durationToTime(
    os_duration d);

/** \brief Convert time t into a floating point representation of t
 *
 * Postcondition:
 * - Due to the limited accuracy, the least significant part
 *   of the floating point time will be about 1 us.
 *
 * Possible Results:
 * - returns floating point representation of t
 */
OS_API os_timeReal
os_timeToReal(
    os_time t); /* Depricated */

OS_API os_timeReal
os_durationToReal(
    os_duration d);

/** \brief Convert a floating point time representation into time
 *
 * Possible Results:
 * - returns t in os_time representation
 */
OS_API os_time
os_realToTime(
    os_timeReal t); /* Depricated */

OS_API os_duration
os_realToDuration(
    os_timeReal t);

/** \brief Suspend the execution of the calling thread for the specified time
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     the thread is suspended for the specified time
 * - returns os_resultFail if
 *     the thread is not suspended for the specified time because of a failure,
 *     for example when a negative delay is supplied or when the ns-part is not
 *     normalized.
 */
OS_API os_result
os_nanoSleep(
    os_time delay); /* Depricated */

OS_API os_result
os_sleep(
    os_duration delay);

/** \brief Translate calendar time into a readable string representation
 *
 * It converts the calendar time t into a '\0'-terminated string of the form:
 *
 *        "Mon Feb 03 14:28:56 2014"
 *
 * The time-zone information may not be included on all platforms and may be a
 * non-abbreviated string too. In order to obtain the time-zone, more room (at
 * least 4 bytes more for an abbreviated, and unknown more for spelled out) needs
 * to be provided in buf than the minimum of OS_CTIME_R_BUFSIZE. On Windows (if
 * enough room is provided) it may for example expand to:
 *
 *        "Wed Oct 01 15:59:53 W. Europe Daylight Time 2014"
 *
 * And on Linux to:
 *
 *        "Wed Oct 01 15:59:53 CEST 2014"
 *
 * \param t the time to be translated
 * \param buf a buffer to which the string must be written, with room for holding
 *            at least OS_CTIME_R_BUFSIZE (26) characters.
 *
 * Possible Results:
 * If buf != NULL, buf contains a readable string representation of time t. The
 * string is '\0'-terminated (and doesn't include a '\n' as the last character).
 * \return The number of bytes written (not including the terminating \0) to buf
 * and 0 if buf was NULL.
 */
OS_API os_size_t
os_ctimeW_r(
    os_timeW *t,
    char *buf,
    os_size_t bufsz);

/** Minimum capacity of buffer supplied to os_ctime_r
 *
 * Apparently, the French national representation of the abbreviated weekday
 * name and month name is 4 characters, so therefore added 2 to the expected
 * size of 26.
 */
#define OS_CTIME_R_BUFSIZE (28)

/** \brief Change the clock to use a user-defined function instead of the system clock
 *
 */
OS_API void
os_timeSetUserClock (
    os_time (*userClockFunc)(void));

OS_API void
os_timeSetUserClock64(
    os_fptr userClock64Func);

/** \brief data structure to support detection of power events.
 *
 * Note power event support is Depricated.
 *
 */
typedef struct os_timePowerEvents_s {
    os_uint32   suspendCount;           /**< The number of detected suspends. */
    os_timeW    suspendLastDetected;    /**< The time of the last detected suspend (real time). */
    pa_uint32_t resumeCount;            /**< The number of detected resumes. */
    os_timeW    resumeLastDetected;     /**< The time of the last detected resume (real time). */
} os_timePowerEvents;

/** \brief Query (and optionally synchronize) on the number of detected power events.
 *
 * This call can be used to retrieve the number of power events (suspend/resume) that were
 * detected. It is possible to block on changes by specifying a maxBlockingTime.
 *
 * The lastDetected timestamps are retrieved with os_getTime() and are the times on which the
 * event was detected (which may not be the exact time at which the events actually occurred).
 * The reported counts are monotonically increased on detection of power events.
 *
 * There is no guarantee that suspends (either hibernate or sleep) are detected. In general not
 * all events may be detectable. Only the last resume event is guaranteed to be detected.
 *
 * The initial state (when no events are detected yet) is all counts and times zeroed.
 *
 * \param [in,out] events       Pointer to a struct in which the result of the call is returned.
 *                              If maxBlockingTime == 0, events is an out-parameter, otherwise it
 *                              is in/out. The call will block until the actual state is different
 *                              from the state pointed to by events.
 * \param [in] maxBlockingTime  The maximum time the call may block for the state to change from
 *                              the state specified in events. If 0, the call will not block and
 *                              return immediately the current state.
 * \retval os_resultSuccess     When the call succeeded and the struct pointed to by events contains
 *                              the new status.
 * \retval os_resultTimeout     Iff maxBlockingTime != 0 and maxBlockingTime elapsed before the state
 *                              changed.
 *
 * \pre     The parameter events is not NULL and points to a location sufficiently large to hold an
 *          os_powerEvents struct.
 * \pre     The parameter maxBlockingTime is a valid time representation.
 * \post    The struct pointed to by events contains the current values.
 */
OS_API os_result
os_timeGetPowerEvents(
    os_timePowerEvents *events,
    os_duration maxBlockingTime);

#define OS_DURATION_TO_STRING_BUFSIZE (22)
#define OS_TIME_TO_STRING_BUFSIZE (22)
#define OS_TIMEW_TO_STRING_BUFSIZE OS_TIME_TO_STRING_BUFSIZE
#define OS_TIMEE_TO_STRING_BUFSIZE OS_TIME_TO_STRING_BUFSIZE
#define OS_TIMEM_TO_STRING_BUFSIZE OS_TIME_TO_STRING_BUFSIZE

OS_API char *
os_durationImage (
    os_duration d,
    char *buf,
    os_size_t bufsz);

OS_API char *
os_timeWImage (
    os_timeW t,
    char *buf,
    os_size_t bufsz);

OS_API char *
os_timeMImage (
    os_timeM t,
    char *buf,
    os_size_t bufsz);

OS_API char *
os_timeEImage (
    os_timeE t,
    char *buf,
    os_size_t bufsz);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_TIME_H */
