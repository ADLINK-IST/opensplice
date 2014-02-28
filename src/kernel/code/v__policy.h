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

v_partitionPolicy
v_partitionPolicyAdd(
    v_partitionPolicy _this,
    const c_char *expr,
    c_base base);

v_partitionPolicy
v_partitionPolicyRemove(
    v_partitionPolicy _this,
    const c_char *expr,
    c_base base);

c_iter
v_partitionPolicySplit(
    v_partitionPolicy _this);

v_ownershipResult
v_determineOwnershipByStrength (
    struct v_owner *owner,
    struct v_owner *candidate,
    c_bool claim); /* Workaround for dds1784, see v_policy.c for details. */

#if defined (__cplusplus)
}
#endif

#endif /* V__POLICY_H */
