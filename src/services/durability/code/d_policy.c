
#include "d__policy.h"
#include "d_policy.h"
#include "d_misc.h"

void
d_policyDeinit (
        d_object object)
{
    d_policy policy = d_policy(object);
    d_free (policy->nameSpace);
}

d_policy
d_policyNew (
    const char* namespace,
    c_bool aligner,
    d_alignmentKind alignmentKind,
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

/*typedef struct policyFindData
{
    d_policy lookUp;
    d_policy* policy_out;
}policyFindData;

static void
policyFindWalk (
    void* obj,
    c_iterActionArg userData)
{
    d_policy policy = d_policy(obj);
    policyFindData* walkData = (policyFindData)userData;

    if (!walkData->policy_out) {
        if (<<match-pattern>>(policy->name, lookup->name)) {

        }
    }
}

d_policy
d_policyFind (
    c_iter policies,
    const char* nameSpace,
    c_bool aligner,
    d_alignmentKind alignmentKind,
    d_durabilityKind durabilityKind)
{

}*/
