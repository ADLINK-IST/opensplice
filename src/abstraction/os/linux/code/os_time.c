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

/** \file os/linux/code/os_time.c
 *  \brief Linux time management
 *
 * Implements time management for Linux
 * by including the common services and
 * the SVR4 os_timeGet implementation and
 * the POSIX os_nanoSleep implementation
 */

#include "../common/code/os_time.c"
#include "../posix/code/os_time.c"

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
    struct timespec tv;

    clock_gettime (CLOCK_REALTIME, &tv);
    t.tv_sec = tv.tv_sec;
    t.tv_nsec = tv.tv_nsec;

    return t;
}
