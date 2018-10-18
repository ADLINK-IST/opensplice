/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/** \file os/solaris/code/os_thread_attr.c
 *  \brief Solaris thread attributes
 *
 * Implements os_threadAttrInit and sets attributes:
 * - scheduling class is OS_SCHED_DEFAULT
 * - thread priority is the current priority
 */

#if OS_SOLARIS_VER == 8
#include "../posix/code/os_thread_attr.c"
#else
#include <assert.h>

/** \brief Initialize thread attributes
 *
 * - Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 *   (take the platforms default scheduling class, Time-sharing for
 *   non realtime platforms, Real-time for realtime platforms)
 * - Set \b procAttr->schedPriority to whatever we already have
 */
void
os_threadAttrInit (
    os_threadAttr *threadAttr)
{
    struct sched_param param;

    assert (threadAttr != NULL);
    sched_getparam (0, &param);
    threadAttr->schedClass = OS_SCHED_DEFAULT;
    threadAttr->schedPriority = param.sched_priority;
    threadAttr->stackSize = 0;
}
#endif
