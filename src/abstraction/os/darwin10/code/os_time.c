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

/** \file os/darwin/code/os_time.c
 *  \brief Darwin time management
 *
 * Implements time management for Darwin
 */
#include <os_errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <mach/mach_time.h>

#include <../common/code/os_time.c>
#include <../common/code/os_time_ctime.c>


/*  \brief Get the current time
 */
static os_timeW
os__timeWGet (
    void)
{
    static int timeshift = INT_MAX;
    struct timeval tv;
    os_timeW rt;

    if(timeshift == INT_MAX) {
        const char *p = getenv("OSPL_TIMESHIFT");
        timeshift = (p == NULL) ? 0 : atoi(p);
    }

    (void) gettimeofday (&tv, NULL);

    rt = OS_TIMEW_INIT(tv.tv_sec + timeshift, tv.tv_usec*1000);

    return rt;
}

/** \brief Get high resolution relative time
 *
 */
os_timeM
os_timeMGet (
    void)
{
    static mach_timebase_info_data_t timeInfo;
    os_timeM t;
    uint64_t mt;

    /* The Mach absolute time returned by mach_absolute_time is very similar to
     * the QueryPerformanceCounter on Windows. The update-rate isn't fixed, so
     * that information needs to be resolved to provide a clock with real-time
     * progression.
     *
     * The mach_absolute_time does include time spent during sleep (on Intel
     * CPU's, not on PPC), but not the time spent during suspend.
     *
     * The result is not adjusted based on NTP, so long-term progression by this
     * clock may not match the time progression made by the real-time clock. */
    mt = mach_absolute_time();

    if( timeInfo.denom == 0) {
        (void) mach_timebase_info(&timeInfo);
    }
    t.mt = mt * timeInfo.numer / timeInfo.denom;

    return t;
}

/** \brief Get elapsed time.
 *
 * \return elapsed time since some unspecified fixed past time
 * \return os_timeMGet() otherwise
 */
os_timeE
os_timeEGet (
    void)
{
    /* Elapsed time clock not (yet) supported on this platform. */
    os_timeM mt = os_timeMGet();
    os_timeE t;
    t.et = mt.mt;

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
os_sleep(
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
