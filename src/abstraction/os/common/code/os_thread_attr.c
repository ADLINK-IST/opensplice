/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

/** \file os/common/code/os_thread_attr.c
 *  \brief Common thread attributes
 *
 * Implements os_threadAttrInit and sets attributes
 * to platform independent values:
 * - scheduling class is OS_SCHED_DEFAULT
 * - thread priority is 0
 */

#include <assert.h>

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
    threadAttr->schedPriority = 0;
}
