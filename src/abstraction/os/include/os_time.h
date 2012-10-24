/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief User clock handle
 */
typedef void *os_userClockHandle;

/** \brief Time structure definition
 */
typedef struct os_time {
    /** Seconds since 1-jan-1970 00:00 */
    os_timeSec tv_sec;
    /** Count of nanoseconds within the second */
    os_int32 tv_nsec;
    /** os_time can be used for a duration type with the following
	semantics for negative durations: tv_sec specifies the
	sign of the duration, tv_nsec is always possitive and added
	to the real value (thus real value is tv_sec+tv_nsec/10^9,
	for example { -1, 500000000 } is -0.5 seconds) */ 
} os_time;

/** \brief Get the current time
 *
 * Possible Results:
 * - returns "the current time"
 */
OS_API os_time
os_timeGet(void);

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
    os_time t2);

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
    os_time t2);

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
    double multiply);

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
    os_time t);

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
    os_time t2);

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
    os_time t);

/** \brief Convert a floating point time representation into time
 *
 * Possible Results:
 * - returns t in os_time representation
 */
OS_API os_time
os_realToTime(
    os_timeReal t);

/** \brief Suspend the execution of the calling thread for the specified time
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     the thread is suspended for the specified time
 * - returns os_resultFail if
 *     the thread is not suspended for the specified time because of a failure
 */
OS_API os_result
os_nanoSleep(
    os_time delay);

/** \brief Get high resolution time
 *
 * Possible Results:
 * - returns "a high resolution time (not necessarily real time)"
 */
OS_API os_time
os_hrtimeGet(void);

/** \brief Translate calendar time into readable string representation
 *
 * Possible Results:
 * - returns buf if buf != NULL
 * - returns NULL if buf == NULL
 */
OS_API char *
os_ctime_r(
    os_time *t,
    char *buf);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_TIME_H */
