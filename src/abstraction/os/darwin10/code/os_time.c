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

/** \file os/darwin/code/os_time.c
 *  \brief Darwin time management
 *
 * Implements time management for Darwin
 * by including the common services and
 * the SVR4 os_timeGet implementation and
 * the POSIX os_nanoSleep implementation
 */

#ifdef SOMETHING_NOT_DEFFED

#include <os_report.h>

#include <time.h>
#include <errno.h>
#include <sys/time.h>

#endif

#include <../common/code/os_time.c>
#include <../posix/code/os_time.c>

/** \brief Translate calendar time into readable string representation
 *
 * ctime_r provides a re-entrant translation function.
 * ctime_r function adds '\n' to the string which must be removed.
 */
char *
os_ctime_r (
    os_time *t,
    char *buf)
{
/*
    if (buf) {
        ctime_r((const time_t *)(&t->tv_sec), buf);
    }
    return buf;
*/
    time_t tt = t->tv_sec;
    if (buf) {
        ctime_r(&tt, buf);
    }
    return buf;
}

/** \brief Get high resolution relative time
 *
 */
os_time
os_hrtimeGet (
    void
    )
{
    os_time t;
    struct timeval tv;

    gettimeofday (&tv, NULL);
    t.tv_sec = tv.tv_sec;
    t.tv_nsec = 1000 * tv.tv_usec;

    return t;
}

/*static os_time (*clockGet)(void) = NULL;*/

#ifdef SOMETHING_NOT_DEFFED

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
    struct timeval t;
    os_time rt;

    if (clockGet) {
        rt = clockGet ();
    } else {
      gettimeofday (&t, NULL);
      rt.tv_sec = t.tv_sec;
      rt.tv_nsec = 1000 * t.tv_usec;
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

#endif

