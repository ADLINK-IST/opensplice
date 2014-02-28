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
/** \file os/darwin/code/os_thread_attr.c
 *  \brief POSIX thread attributes
 *
 * Implements os_threadAttrInit and sets attributes
 * to platform independent values:
 * - scheduling class is OS_SCHED_DEFAULT
 * - thread priority is sched_get_priority_min (SCHED_OTHER)
 */
#include <assert.h>

#ifndef OSPL_ENV_PURIFY
/**
 * Default stack size set to 256KB, when it is too small the OS
 * will automatically increase the stack size
 */
#define OS_STACKSIZE_DEFAULT ((size_t)256*1024)
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
os_result
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    assert (threadAttr != NULL);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = sched_get_priority_min (SCHED_OTHER);
    threadAttr->stackSize = OS_STACKSIZE_DEFAULT;
    return os_resultSuccess;
}
