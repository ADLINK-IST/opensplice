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

/** \file os/posix/code/os_process_attr.c
 *  \brief POSIX process attributes
 *
 * Implements os_procAttrInit and sets attributes
 * to platform independent values:
 * - scheduling class is OS_SCHED_DEFAULT
 * - process priority is the middle of the available range
 * - locking policy is OS_UNLOCKED
 * - user credentials is default
 */

#include <assert.h>
#include <sched.h>

/** \brief Initialize process attributes
 *
 * Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 * (take the platforms default scheduling class, Time-sharing for
 * non realtime platforms, Real-time for realtime platforms)
 * Set \b procAttr->schedPriority to \b 0
 * Set \b procAttr->lockPolicy to \b OS_LOCK_DEFAULT
 * (no locking on non realtime platforms, locking on
 * realtime platforms)
 * Set \b procAttr->userCred.uid to 0
 * (don't change the uid of the process)
 * Set \b procAttr->userCred.gid to 0
 * (don't change the gid of the process)
 */
os_result
os_procAttrInit (
    os_procAttr *procAttr)
{
    assert (procAttr != NULL);
    procAttr->schedClass = OS_SCHED_DEFAULT;
    procAttr->schedPriority = (sched_get_priority_min (SCHED_OTHER) + sched_get_priority_max (SCHED_OTHER)) / 2;
    procAttr->lockPolicy = OS_LOCK_DEFAULT;
    procAttr->userCred.uid = 0;
    procAttr->userCred.gid = 0;
    procAttr->activeRedirect = 0;
    
    return os_resultSuccess;
}
