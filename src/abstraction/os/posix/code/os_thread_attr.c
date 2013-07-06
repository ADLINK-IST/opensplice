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

/** \file os/posix/code/os_thread_attr.c
 *  \brief POSIX thread attributes
 *
 * Implements os_threadAttrInit and sets attributes
 * to platform independent values:
 * - scheduling class is OS_SCHED_DEFAULT
 * - thread priority is the middle of the available range
 */

#include <assert.h>

/** \brief Initialize thread attributes
 *
 * - Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 *   (take the platforms default scheduling class, Time-sharing for
 *   non realtime platforms, Real-time for realtime platforms)
 * - Set \b procAttr->schedPriority to \b 0
 */
os_result
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    assert (threadAttr != NULL);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = (sched_get_priority_min (SCHED_OTHER) + sched_get_priority_max (SCHED_OTHER)) / 2;
    threadAttr->stackSize = 0;
    return os_resultSuccess;
}
