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

/** \file os/solaris/code/os_process_attr.c
 *  \brief POSIX process attributes
 *
 * Implements os_procAttrInit and sets attributes:
 * - scheduling class is OS_SCHED_DEFAULT
 * - process priority is whatever we already have
 * - locking policy is OS_UNLOCKED
 * - user credentials is default
 */

#if OS_SOLARIS_VER == 8
#include "../posix/code/os_process_attr.c"
#else
#include <assert.h>
#include <sched.h>

/** \brief Initialize process attributes
 *
 * Set \b procAttr->schedClass to \b OS_SCHED_DEFAULT
 * (take the platforms default scheduling class, Time-sharing for
 * non realtime platforms, Real-time for realtime platforms)
 * Set \b procAttr->schedPriority to \b the current priority
 * Set \b procAttr->lockPolicy to \b OS_LOCK_DEFAULT
 * (no locking on non realtime platforms, locking on
 * realtime platforms)
 * Set \b procAttr->userCred.uid to 0
 * (don't change the uid of the process)
 * Set \b procAttr->userCred.gid to 0
 * (don't change the gid of the process)
 */
void
os_procAttrInit (
    os_procAttr *procAttr)
{
    struct sched_param param;

    assert (procAttr != NULL);
    sched_getparam (0, &param);
    procAttr->schedClass = OS_SCHED_DEFAULT;
    procAttr->schedPriority = param.sched_priority;
    procAttr->lockPolicy = OS_LOCK_DEFAULT;
    procAttr->userCred.uid = 0;
    procAttr->userCred.gid = 0;
    procAttr->activeRedirect = 0;
}
#endif
