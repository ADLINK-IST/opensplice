/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */


/** \file os/vxworks5.5/code/os_time.c
 *  \brief vxWorks time management
 *
 * Implements time management for vxWorks
 * by including the common services and
 * the SVR4 os_timeGet implementation and
 * the POSIX os_nanoSleep implementation
 */

#include "../common/code/os_time.c"
#include "../posix/code/os_time.c"


/** \brief Translate calendar time into readable string representation
 *
 * ctimeW_r provides a re-entrant translation function.
 * ctimeW_r function adds '\n' to the string which must be removed.
 */
os_size_t
os_ctimeW_r (
    os_timeW *t,
    char *buf,
    os_size_t bufsz)
{
    time_t tt;
    assert(bufsz >= OS_CTIME_R_BUFSIZE);
    assert(t);

    if (!buf) return 0;
    tt = (time_t)OS_TIMEW_GET_SECONDS(*t);
#if defined (VXWORKS_55) || defined (VXWORKS_64) || defined (VXWORKS_65) || defined (VXWORKS_66) || defined (VXWORKS_67) || defined (VXWORKS_68)
/* This code is also compiled for VxWorks 6.x km targets. The OS_REV is
 * overruled in $(OSPL_OUTER_HOME)/setup/vxworks.km-default.mak. */
    size_t length = bufsz;
    ctime_r (&tt, buf, &length);
#else
    OS_UNUSED_ARG(bufsz);
    ctime_r (&tt, buf);
#endif
    assert(strchr (buf, '\n') == &buf[24]);
    buf[24] = '\0';
    return 24;
}
