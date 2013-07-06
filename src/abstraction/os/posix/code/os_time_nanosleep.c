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

/** \file os/posix/code/os_time_nanosleep.c
 *  \brief Time management - os_nonaSleep
 *
 * Implements os_nanoSleep for POSIX
 */

#include <time.h>
#include <errno.h>

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
    return rv;
}
