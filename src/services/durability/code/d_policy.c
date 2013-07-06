
#include "d__policy.h"
#include "d_policy.h"
#include "d_misc.h"

void
d_policyDeinit (
        d_object object)
{
    d_policy policy = d_policy(object);
    d_policyMergeRule* mergeRule;

    if (policy->mergePolicyRules) {

        mergeRule = (d_policyMergeRule*)c_iterTakeFirst (policy->mergePolicyRules);
        while (mergeRule) {
            os_free (mergeRule->scope);
            d_free (mergeRule);
            mergeRule = c_iterTakeFirst (policy->mergePolicyRules);
        }
        c_iterFree (policy->mergePolicyRules);
    }

    d_free (policy->nameSpace);
}

d_policy
d_policyNew (
    const char* namespace,
    c_bool aligner,
    d_alignmentKind alignmentKind,
    c_bool delayedAlignment,
    d_durabilityKind durabilityKind)
{
    d_policy policy;

    policy = d_policy(d_malloc((os_uint32)C_SIZEOF(d_policy), "Policy"));
    if (policy != NULL){

        d_objectInit(d_object(policy), D_POLICY, d_policyDeinit);

        if(namespace != NULL){
            policy->nameSpace = os_strdup(namespace);
        } else {
            policy->nameSpace = os_strdup("*");
        }

        policy->aligner = aligner;
        policy->alignmentKind = alignmentKind;
        policy->durabilityKind = durabilityKind;
        policy->delayedAlignment = delayedAlignment;
        policy->mergePolicyRules = NULL;
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
        d_objectFree(d_object(policy), D_POLICY);
    }
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
    const char* role;
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
    const char* role)
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
