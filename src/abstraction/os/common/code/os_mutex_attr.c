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

/** \file os/common/code/os_mutex_attr.c
 *  \brief Common mutual exclusion semaphore attributes
 *
 * Implements os_mutexAttrInit and sets attributes
 * to platform independent values:
 * - scope is OS_SCOPE_SHARED
 */

/** \brief Initialize mutex attribute
 *
 * Set \b mutexAttr->scopeAttr to \b OS_SCOPE_PRIVATE
 * Set \b mutexAttr->errorCheckingAttr to \b OS_ERRORCHECKING_DISABLED
 */
_Post_satisfies_(mutexAttr->scopeAttr == OS_SCOPE_PRIVATE)
_Post_satisfies_(mutexAttr->errorCheckingAttr == OS_ERRORCHECKING_DISABLED)
void
os_mutexAttrInit (
    _Out_ os_mutexAttr *mutexAttr)
{
    mutexAttr->scopeAttr = OS_SCOPE_PRIVATE;
    /* By setting errorCheckingAttr to OS_ERRORCHECKING_ENABLED or
     * OS_ERRORCHECKING_DISABLED, error-checking can easily be enabled/disabled
     * for all mutexes that don't explicitly set the option themselves. */
    mutexAttr->errorCheckingAttr = OS_ERRORCHECKING_DISABLED;
}
