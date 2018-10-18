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
/** \file os/linux/code/os_thread_attr.c
 *  \brief POSIX thread attributes
 *
 * Implements os_threadAttrInit and sets attributes
 * to platform independent values:
 * - scheduling class is OS_SCHED_DEFAULT
 * - thread priority is the middle of the available range
 */
#include <assert.h>
#include <limits.h>

#ifndef OSPL_ENV_PURIFY
/**
 * Default stack size set to 64KB, when it is too small the OS
 * will automatically increase the stack size
 */
#define OS_STACKSIZE_DEFAULT ((size_t)64*1024)
#else
#define OS_STACKSIZE_DEFAULT ((size_t)10*1024*1024)
#endif

/** \brief Initialize thread attributes
 *
 * - Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 *   (take the platforms default scheduling class, Time-sharing for
 *   non realtime platforms, Real-time for realtime platforms)
 * - Set \b procAttr->schedPriority to \b 0
 */
void
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    assert (threadAttr != NULL);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = (sched_get_priority_min (SCHED_OTHER) + sched_get_priority_max (SCHED_OTHER)) / 2;

    if (OS_STACKSIZE_DEFAULT < PTHREAD_STACK_MIN)
    {
       threadAttr->stackSize = PTHREAD_STACK_MIN;
    }
    else
    {
       threadAttr->stackSize = OS_STACKSIZE_DEFAULT;
    }
}
