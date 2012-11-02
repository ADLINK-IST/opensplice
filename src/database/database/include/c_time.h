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
#ifndef C_TIME_H
#define C_TIME_H

#include "c_base.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DB
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


typedef struct c_time {
    c_long seconds;
    c_ulong nanoseconds;
} c_time;

static const c_time C_TIME_ZERO         = {0,0};
static const c_time C_TIME_INFINITE     = {0x7fffffff,0x7fffffffU};
static const c_time C_TIME_MIN_INFINITE = {-0x7fffffff,0x7fffffffU};
static const c_time C_TIME_INVALID      = {-1, 0xffffffffU};

OS_API c_time      c_timeNormalize(c_time t);
OS_API c_equality  c_timeCompare (c_time t1, c_time t2);
OS_API c_time      c_timeAdd     (c_time t1, c_time t2);
OS_API c_time      c_timeSub     (c_time t1, c_time t2);
OS_API c_bool      c_timeValid   (c_time t1);
OS_API c_timeResult c_timeNanoSleep(c_time interval);
OS_API c_double    c_timeToReal  (c_time t1);
OS_API c_time      c_timeFromReal(c_double d);

#define c_timeIsZero(t) \
        ((t.seconds == 0) && \
         (t.nanoseconds == 0) ? TRUE : FALSE)

#define c_timeIsInfinite(t) \
        ((t.seconds == 0x7fffffff) && \
         (t.nanoseconds == 0x7fffffffU) ? TRUE : FALSE)

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
