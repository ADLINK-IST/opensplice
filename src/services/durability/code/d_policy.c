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

#include "d__policy.h"
#include "d__misc.h"
#include "d__thread.h"
#include "d__durability.h"
#include "d__configuration.h"

void
d_policyDeinit (
        d_policy policy)
{
    d_policyMergeRule* mergeRule;

    assert(d_policyIsValid(policy));

    if (policy->mergePolicyRules) {
        mergeRule = (d_policyMergeRule*)c_iterTakeFirst (policy->mergePolicyRules);
        while (mergeRule) {
            os_free (mergeRule->scope);
            d_free (mergeRule);
            mergeRule = c_iterTakeFirst (policy->mergePolicyRules);
        }
        c_iterFree (policy->mergePolicyRules);
    }
    if (policy->nameSpace) {
       os_free(policy->nameSpace);
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(policy));
}

d_policy
d_policyNew (
    const char* namespace,
    c_bool aligner,
    d_alignmentKind alignmentKind,
    c_bool delayedAlignment,
    d_durabilityKind durabilityKind,
    c_bool equalityCheck,
    c_ulong masterPriority)
{
    d_policy policy;

    /* Allocate policy object */
    policy = d_policy(d_malloc(C_SIZEOF(d_policy), "Policy"));
    if (policy != NULL){
        /* Call super-init */
        d_objectInit(d_object(policy), D_POLICY,
                     (d_objectDeinitFunc)d_policyDeinit);
        /* Initialize policy object */
        if (namespace != NULL) {
            policy->nameSpace = os_strdup(namespace);
        } else {
            policy->nameSpace = os_strdup("*");
        }
        policy->aligner = aligner;
        policy->alignmentKind = alignmentKind;
        policy->durabilityKind = durabilityKind;
        policy->delayedAlignment = delayedAlignment;
        policy->mergePolicyRules = NULL;
        policy->equalityCheck = equalityCheck;
        policy->masterPriority = masterPriority;
    }
    return policy;
}

c_bool
d_policyGetAligner (
        d_policy policy)
{
    c_bool aligner;

    aligner = FALSE;

    if (d_objectIsValid(d_object(policy), D_POLICY)){
        aligner = policy->aligner;
    }

    return aligner;
}

c_bool
d_policyGetEqualityCheck (
    d_policy policy)
{
    assert(d_policyIsValid(policy));

    return policy->equalityCheck;
}

c_ulong
d_policyGetMasterPriority (
    d_policy policy)
{
    assert(d_policyIsValid(policy));

    return policy->masterPriority;
}

d_alignmentKind
d_policyGetAlignmentKind (
        d_policy policy)
{
    d_alignmentKind alignmentKind;

    alignmentKind = D_ALIGNEE_INITIAL;

    if (d_objectIsValid(d_object(policy), D_POLICY)){
        alignmentKind = policy->alignmentKind;
    }

    return alignmentKind;
}

d_durabilityKind
d_policyGetDurabilityKind (
        d_policy policy)
{
    d_durabilityKind durabilityKind;

    durabilityKind = D_DURABILITY_ALL;

    if (d_objectIsValid(d_object(policy), D_POLICY)){
        durabilityKind = policy->durabilityKind;
    }

    return durabilityKind;
}

/* Get namespace pattern from policy */
c_string
d_policyGetNameSpace (
    d_policy policy)
{
    c_string nameSpace = "";

    if (d_objectIsValid(d_object(policy), D_POLICY)){
        nameSpace = policy->nameSpace;
    }

    return nameSpace;
}

void
d_policyFree(
    d_policy policy )
{
    if (policy) {
        assert(d_policyIsValid(policy));
    }

    d_objectFree(d_object(policy));
}

void
d_policyAddMergeRule(
    d_policy policy,
    d_mergePolicy mergeType,
    const char* scope)
{
    d_policyMergeRule* rule;

    rule = os_malloc (sizeof(d_policyMergeRule));
    assert (rule);

    rule->mergeType = mergeType;
    rule->scope = os_strdup(scope);

    if (!policy->mergePolicyRules){
        policy->mergePolicyRules = c_iterNew (rule);
    }else {
        c_iterAppend (policy->mergePolicyRules, rule);
    }
}

struct matchMergePolicyHelper
{
    char* role;
    d_mergePolicy mergeType;
    c_bool found;
};

static void
matchMergePolicyWalk (
    void* o,
    c_iterActionArg arg)
{
    d_policyMergeRule* rule;
    struct matchMergePolicyHelper* helper;

    rule = (d_policyMergeRule*)o;
    helper = (struct matchMergePolicyHelper*)arg;

    if (!helper->found) {
        if (d_patternMatch (helper->role, rule->scope)) {
            helper->mergeType = rule->mergeType;
            helper->found = TRUE;
        }
    }
}

/* Get merge rule from policy */
/* TODO: scope matching should use same algorithm as networking! */
d_mergePolicy
d_policyGetMergePolicy(
    d_policy policy,
    char* role)
{
    struct matchMergePolicyHelper helper;
    d_mergePolicy result;

    helper.role = role;
    helper.found = FALSE;

    /* Find rule for role */
    c_iterWalk (policy->mergePolicyRules, matchMergePolicyWalk, &helper);

    /* If no rule is found, do not merge */
    if (helper.found) {
        result = helper.mergeType;
    }else {
        result = D_MERGE_IGNORE;
    }

    return result;
}

/* Get policy for delayed alignment */
c_bool
d_policyGetDelayedAlignment (
    d_policy policy)
{
    c_bool delayedAlignment;

    delayedAlignment = FALSE;

    if (d_objectIsValid(d_object(policy), D_POLICY)){
        delayedAlignment = policy->delayedAlignment;
    }

    return delayedAlignment;
}
