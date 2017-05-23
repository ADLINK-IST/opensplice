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

#ifndef V__POLICY_H
#define V__POLICY_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_policy.h"

typedef enum {
    V_OWNERSHIP_ALREADY_OWNER, /* already owner, strength might have changed */
    V_OWNERSHIP_OWNER, /* became owner */
    V_OWNERSHIP_NOT_OWNER, /* did not became owner */
    V_OWNERSHIP_OWNER_RESET, /* reset because of NIL owner (dispose all) */
    V_OWNERSHIP_SHARED_QOS,
    V_OWNERSHIP_INCOMPATIBLE_QOS
} v_ownershipResult;

v_partitionPolicyI
v_partitionPolicyAdd(
    v_partitionPolicyI _this,
    const c_char *expr,
    c_base base);

v_partitionPolicyI
v_partitionPolicyRemove(
    v_partitionPolicyI _this,
    const c_char *expr,
    c_base base);

c_iter
v_partitionPolicySplit(
    v_partitionPolicyI _this);

v_ownershipResult
v_determineOwnershipByStrength (
    struct v_owner *owner,
    const struct v_owner *candidate,
    c_bool claim); /* Workaround for dds1784, see v_policy.c for details. */

#if defined (__cplusplus)
}
#endif

#endif /* V__POLICY_H */
