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

/** \file os/solaris10/code/os_thread_attr.c
 *  \brief Solaris thread attributes
 *
 * Implements os_threadAttrInit and sets attributes:
 * - scheduling class is OS_SCHED_DEFAULT
 * - thread priority is the current priority
 */

#include <assert.h>

/** \brief Initialize thread attributes
 *
 * - Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 *   (take the platforms default scheduling class, Time-sharing for
 *   non realtime platforms, Real-time for realtime platforms)
 * - Set \b procAttr->schedPriority to whatever we already have
 */
os_result
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    struct sched_param param;

    assert (threadAttr != NULL);
    sched_getparam (0, &param);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = param.sched_priority;
    threadAttr->stackSize = 0;
    return os_resultSuccess;
}
