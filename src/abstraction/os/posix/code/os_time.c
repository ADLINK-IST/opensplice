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

/** \file os/posix/code/os_time.c
 *  \brief Time management
 *
 * Implements posix based time management functions
 */

#include <time.h>
#include "os_errno.h"

/*  \brief Get the current time
 */
static os_timeW
os__timeWGet(void)
{
    struct timespec t;
    os_timeW rt;

    /* POSIX specifies that all implementations of clock_gettime(...)
     * support the system-wide real-time clock, which is identified
     * by CLOCK_REALTIME, so the result of the call below can safely
     * be ignored. */
    (void) clock_gettime (CLOCK_REALTIME, &t);

    rt = OS_TIMEW_INIT(t.tv_sec, t.tv_nsec);

    return rt;
}

/** \brief Get high resolution, monotonic time.
 *
 * Sufficiently recent versions of GNU libc and the Linux kernel support
 * monotonic clocks. In case monotonic clocks are NOT supported the
 * real-time clock is used as fallback. This ensures that the products
 * still works as long as no time jumps occur.
 *
 * \return high resolution, monotonic time if monotonic clocks are supported.
 * \return real-time, otherwise
 */
os_timeM
os_timeMGet(void)
{
    os_timeM t;
    struct timespec tv;

#ifdef _POSIX_MONOTONIC_CLOCK
    /* The CLOCK_MONOTONIC is the best (and most accurate) monotonic clock for
     * obtaining time with near real-time progression, since it may be slewed
     * based on NTP adjustments on some Linux providing more stable time
     * progression (assuming that the Caesium clocks are better at timekeeping
     * than the quartz crystal in an average PC). */
    (void) clock_gettime (CLOCK_MONOTONIC, &tv);
#else

    /* Unfortunately no monotonic clock is supported for this POSIX platform.
     * Fall-back to the CLOCK_REALTIME instead. Unfortunately this means that
     * time jumps are not supported. */
    (void) clock_gettime (CLOCK_REALTIME, &tv);
#endif

    t = OS_TIMEM_INIT(tv.tv_sec, tv.tv_nsec);

    return t;
}

/** \brief Get elapsed time.
 *
 * This is implemented by means of the CLOCK_BOOTTIME clock which is available
 * since Linux 2.6.39, but is Linux specific. If it is not available, the
 * monotonic clock will be used as a fall-back.
 *
 * \return elapsed time since some unspecified fixed past time
 * \return os_timeMGet() otherwise
 */
os_timeE
os_timeEGet(void)
{
    os_timeE t;
#ifdef CLOCK_BOOTTIME
    struct timespec tv;

    /* The CLOCK_BOOTTIME includes time spent during suspend, but is
     * Linux specific. */
    if ( 0 != clock_gettime (CLOCK_BOOTTIME, &tv)) { 
        /* Clock_boottime not supported, try monotonic */ 
        (void) clock_gettime (CLOCK_MONOTONIC, &tv); 
    }
    t = OS_TIMEE_INIT(tv.tv_sec, tv.tv_nsec);
#else
    os_timeM m = os_timeMGet();
    t.et = m.mt;
#endif /* CLOCK_MONOTONIC_BOOTTIME */

    return t;
}

/** \brief Suspend the execution of the calling thread for the specified time
 *
 * \b os_nanoSleep suspends the calling thread for the required
 * time by calling \b nanosleep. First it converts the \b delay in
 * \b os_duration definition into a time in \b struct \b timeval definition.
 * In case the \b nanosleep is interrupted, the call is re-entered with
 * the remaining time.
 */
os_result
ospl_os_sleep(
    os_duration delay)
{
    struct timespec t;
    struct timespec r;
    int result;
    os_result rv;

    if (OS_DURATION_ISPOSITIVE(delay)) {
        /* Time should be normalized */
        t.tv_sec = (time_t) OS_DURATION_GET_SECONDS(delay);
        t.tv_nsec = OS_DURATION_GET_NANOSECONDS(delay);
        result = nanosleep (&t, &r);
        while (result && os_getErrno() == EINTR) {
            t = r;
            result = nanosleep (&t, &r);
        }
        if (result == 0) {
            rv = os_resultSuccess;
        } else {
            rv = os_resultFail;
        }
    } else {
        rv = os_resultFail;
    }
    return rv;
}

