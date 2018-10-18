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

#ifndef V__SPLICED_H
#define V__SPLICED_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_spliced.h"

_Check_return_
_Ret_notnull_
v_spliced
v_splicedNew (
    _In_ v_kernel kernel);

void
v_splicedInit (
    _Inout_ v_spliced _this);

void
v_splicedDeinit (
    v_spliced _this);

void
v_splicedHeartbeat (
    v_spliced _this);

void
v_splicedCheckHeartbeats (
    v_spliced _this);

/* This operation visits all discovered subscription info messages.
 * The given action function is called upon each subscription message.
 * the action signature : os_boolean (*action) (const v_message subscription, void *arg)
 * This message shall be renamed to v_kernelWalkSubscriptions when the admin moves to the kernel.
 */
v_result
v_splicedWalkSubscriptions(
    v_spliced spliced,
    v_subscription_action action,
    c_voidp arg);

/* This operation visits all discovered publications info messages.
 * The given action function is called upon each publication message.
 * the action signature : os_boolean (*action) (const v_message publication, void *arg)
 * This message shall be renamed to v_kernelWalkPublications when the admin moves to the kernel.
 */
v_result
v_splicedWalkPublications(
    v_spliced spliced,
    v_publication_action action,
    c_voidp arg);

#if defined (__cplusplus)
}
#endif

#endif /* V__SPLICED_H */
