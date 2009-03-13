
#ifndef V__POLICY_H
#define V__POLICY_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_policy.h"

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

#if defined (__cplusplus)
}
#endif

#endif /* V__POLICY_H */
