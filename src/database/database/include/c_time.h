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
#ifndef C_TIME_H
#define C_TIME_H

#include "c_base.h"
#include "os_if.h"
#include "os_time.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef os_result c_timeResult;

#define TIME_RESULT_SUCCESS     (os_resultSuccess)
#define TIME_RESULT_UNAVAILABLE (os_resultUnavailable)
#define TIME_RESULT_TIMEOUT     (os_resultTimeout)
#define TIME_RESULT_BUSY        (os_resultBusy)
#define TIME_RESULT_INVALID     (os_resultInvalid)
#define TIME_RESULT_FAIL        (os_resultFail)


/* By defining C_TIME_CHECK, proper use of the different time-domains is enabled
 * by using the most significant bits of the nanoseconds. This is however not
 * incorporated throughout the entire product. For example the serializers aren't
 * aware of this, so the wire-compatibility between release- and dev-builds would
 * be broken if this were to be enabled by default in dev-builds. There are also
 * some conversion (e.g., from real to c_time) which don't cover all cases
 * correctly (the result is assumed to be relative, while it is perfectly valid
 * to convert an absolute time to a real and back). */
#ifdef C_TIME_CHECK
/* The two most significant bits of the nanoseconds part of the time encode the
 * time-domain as follows:
 *
 *   C_TIME_RELATIVE  == 00
 *   C_TIME_MONOTONIC == 01
 *   C_TIME_ELAPSED   == 10
 *   C_TIME_REALTIME  == 11
 *
 * The C_TIME(_MIN)_INFINITE values also have bit 30 set (C_TIME_MONOTONIC). This
 * isn't a problem, since both infinites are part of the C_TIME_MONOTONIC domain
 * as well. */

#   define CHECK_TIME(expr) expr
#   define C_TIME_MONOTONIC (1U << 30)
#   define C_TIME_ELAPSED   (1U << 31)
#   define C_TIME_REALTIME  (C_TIME_MONOTONIC | C_TIME_ELAPSED)
#   define C_TIME_RELATIVE  (0U)
#   define C_TIME_NANOS(nanos) ((nanos) & (C_TIME_MONOTONIC - 1))
#   define C_TIME_FLAGS(nanos) ((nanos) & (C_TIME_MONOTONIC | C_TIME_REALTIME))
#   define C_TIME_GET_KIND(t) C_TIME_FLAGS((t).nanoseconds)
#   define C_TIME_SET_KIND(t, k) ((t).nanoseconds = (C_TIME_NANOS((t).nanoseconds) | (k)))
#   define C_TIME_ISINFINITE(t) (c_timeIsInfinite(t) || c_timeIsMinInfinite(t))
#   define C_TIME_ISABSOLUTE(t) (C_TIME_ISINFINITE(t) || (C_TIME_GET_KIND(t) & (C_TIME_REALTIME | C_TIME_MONOTONIC | C_TIME_ELAPSED))) /* C_TIME_REALTIME ==  C_TIME_MONOTONIC | C_TIME_ELAPSED, but the intention is clear this (verbose) way. */
#   define C_TIME_ISRELATIVE(t) (C_TIME_ISINFINITE(t) || (C_TIME_GET_KIND(t) == 0))
#   define C_TIME_TIMEKIND_EQUALS(t1, t2) (C_TIME_GET_KIND(t1) == C_TIME_GET_KIND(t2))
#else
#   define CHECK_TIME(expr) (TRUE)
#   define C_TIME_MONOTONIC (TRUE)
#   define C_TIME_ELAPSED (TRUE)
#   define C_TIME_REALTIME (TRUE)
#   define C_TIME_RELATIVE (TRUE)
#   define C_TIME_NANOS(nanos) (nanos)
#   define C_TIME_GET_KIND(t) (TRUE)
#   define C_TIME_SET_KIND(t, k) ((void)(t))
#   define C_TIME_ISINFINITE(t) (TRUE)
#   define C_TIME_ISABSOLUTE(t) (TRUE)
#   define C_TIME_ISRELATIVE(t) (TRUE)
#   define C_TIME_TIMEKIND_EQUALS(t1, t2) (TRUE)
#endif

typedef struct c_time {
    c_long seconds;
    c_ulong nanoseconds;
} c_time;

OS_API extern const c_time C_TIME_ZERO;
OS_API extern const c_time C_TIME_INFINITE;
OS_API extern const c_time C_TIME_MIN_INFINITE;
OS_API extern const c_time C_TIME_INVALID;

OS_API c_time      c_timeNormalize(c_time t);
OS_API c_equality  c_timeCompare (c_time t1, c_time t2);
OS_API c_time      c_timeAdd     (c_time t1, c_time t2);
OS_API c_time      c_timeSub     (c_time t1, c_time t2);
OS_API c_bool      c_timeValid   (c_time t1);
OS_API c_timeResult c_timeNanoSleep(c_time interval);
OS_API c_double    c_timeToReal  (c_time t1);
OS_API c_time      c_timeFromReal(c_double d);

#define c_timeIsZero(t) \
    (((t).seconds == 0) && ((t).nanoseconds == 0))

#define c_timeIsInfinite(t) \
    (((t).seconds == C_TIME_INFINITE.seconds) && ((t).nanoseconds == C_TIME_INFINITE.nanoseconds))

#define c_timeIsMinInfinite(t) \
    (((t).seconds == C_TIME_MIN_INFINITE.seconds) && ((t).nanoseconds == C_TIME_MIN_INFINITE.nanoseconds))

#define c_timeIsInvalid(t) \
    (((t).seconds == C_TIME_INVALID.seconds) && ((t).nanoseconds == C_TIME_INVALID.nanoseconds))

OS_API os_timeW
c_timeToTimeW(
    c_time time);

OS_API os_timeM
c_timeToTimeM(
    c_time time);

OS_API os_timeE
c_timeToTimeE(
    c_time time);

OS_API os_duration
c_timeToDuration(
    c_time time);


OS_API c_time
c_timeFromTimeW(
    os_timeW time);

OS_API c_time
c_timeFromTimeM(
    os_timeM time);

OS_API c_time
c_timeFromTimeE(
    os_timeE time);

OS_API c_time
c_timeFromDuration(
    os_duration duration);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
