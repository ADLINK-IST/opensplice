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

/** \file os/posix/code/os_time.c
 *  \brief Time management
 *
 * Implements posix based time management functions
 */

#include "os_report.h"

#ifdef __APPLE__
#include <sys/time.h>
#endif
#include <time.h>
#include <errno.h>

static os_time (*clockGet)(void) = NULL;

/** \brief Get the current time
 *
 * \b os_timeGet gets the current time by calling 
 * \b clock_gettime with clock ID \b CLOCK_REALTIME
 * and converting the result in \b struct
 * \b timespec format into \b os_time format.
 */
os_time
os_timeGet (
    void)
{
#ifdef __APPLE__
    struct timeval tv;
#else
    struct timespec t;
#endif
    int result;
    os_time rt;

    if (clockGet) {
        rt = clockGet ();
    } else {
#ifdef __APPLE__
        result = gettimeofday (&tv, NULL);
#else
        result = clock_gettime (CLOCK_REALTIME, &t);
#endif
        if (result == 0) {
#ifdef __APPLE__
	    rt.tv_sec = tv.tv_sec;
	    rt.tv_nsec = tv.tv_usec*1000;
#else
	    rt.tv_sec = t.tv_sec;
	    rt.tv_nsec = t.tv_nsec;
#endif
        } else {
	    OS_REPORT_1 (OS_WARNING, "os_timeGet", 1, "clock_gettime failed with error %d", errno);
	    rt.tv_sec = 0;
	    rt.tv_nsec = 0;
        } 
    } 
    return rt;
}

/** \brief Set the user clock
 *
 * \b os_timeSetUserClock sets the current time source
 * get function.
 */
void
os_timeSetUserClock (
    os_time (*userClock)(void)
    )
{
    clockGet = userClock;
}

/** \brief Suspend the execution of the calling thread for the specified time
 *
 * \b os_nanoSleep suspends the calling thread for the required
 * time by calling \b nanosleep. First it converts the \b delay in
 * \b os_time definition into a time in \b struct \b timeval definition.
 * In case the \b nanosleep is interrupted, the call is re-entered with
 * the remaining time.
 */
os_result
os_nanoSleep (
    os_time delay)
{
    struct timespec t;
    struct timespec r;
    int result;
    os_result rv;

    assert (delay.tv_nsec >= 0);
    assert (delay.tv_nsec < 1000000000);
    if( delay.tv_sec >= 0 ) {
        t.tv_sec = delay.tv_sec;
        t.tv_nsec = delay.tv_nsec;
        result = nanosleep (&t, &r);
        while (result && errno == EINTR) {
            t = r;
            result = nanosleep (&t, &r);
        }
        if (result == 0) {
            rv = os_resultSuccess;
        } else {
            rv = os_resultFail;
        }
    } else {
        /* Negative time-interval gives an illegal param error in most posix implementations.
         * However, VxWorks casts it to an unsigned int, and waits for years.
         */
        rv = os_resultFail;
    }
    return rv;
}
