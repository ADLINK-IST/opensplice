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

#ifndef D__POLICY_H
#define D__POLICY_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_policy validity.
 * Because d_policy is a concrete class typechecking is required.
 */
#define             d_policyIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_POLICY)

/**
 * \brief The d_policy cast macro.
 *
 * This macro casts an object to a d_policy object.
 */
#define d_policy(_this) ((d_policy)(_this))


typedef struct
d_policyMergeRule {
    d_mergePolicy mergeType;
    c_char* scope;
} d_policyMergeRule;

C_STRUCT(d_policy) {
    C_EXTENDS(d_object);
    d_name           nameSpace;
    c_bool           aligner;
    d_alignmentKind  alignmentKind;
    c_bool           delayedAlignment;
    d_durabilityKind durabilityKind;
    c_iter           mergePolicyRules;
    c_bool           equalityCheck;    /* Indicates if a hash over the set of samples
                                        * needs to be calculated to determine equality.
                                        * Used for align-on-change.
                                        */
    c_ulong          masterPriority;   /* Indicates the willingness of the federation
                                        * to become master for the given nameSpace.
                                        * Needs to be used in combination with 'aligner'
                                        * is TRUE and where 0 indicates never become
                                        * master but do align my master.
                                        */
};


/* Create new policy object */
d_policy            d_policyNew                 (const char* namespace,
                                                 c_bool aligner,
                                                 d_alignmentKind alignmentKind,
                                                 c_bool delayedAlignment,
                                                 d_durabilityKind durabilityKind,
                                                 c_bool equalityCheck,
                                                 c_ulong masterPriority);

void                d_policyDeinit              (d_policy policy);

/* Delete policies */
void                d_policyFree                (d_policy policy);

/* Find policy for namespace ==> Move to configuration */
d_policy            d_policyGet                 (d_configuration config,
                                                 const char* namespace);

/* Get aligner policy */
c_bool              d_policyGetAligner          (d_policy policy);

/* Get policy for alignment kind */
d_alignmentKind     d_policyGetAlignmentKind    (d_policy policy);

/* Get policy for delayed alignment */
c_bool              d_policyGetDelayedAlignment (d_policy policy);

/* Get policy for durabiltiy kind */
d_durabilityKind    d_policyGetDurabilityKind   (d_policy policy);

/* Get namespace pattern from policy */
c_string            d_policyGetNameSpace        (d_policy policy);

/* Get policy for equalityCheck */
c_bool              d_policyGetEqualityCheck    (d_policy policy);

/* Get policy for masterPriority */
c_ulong             d_policyGetMasterPriority   (d_policy policy);

/* Add merge policy rule to policy */
void                d_policyAddMergeRule        (d_policy policy,
                                                 d_mergePolicy mergeType,
                                                 const char* scope);

/* Get merge policy for role */
d_mergePolicy       d_policyGetMergePolicy      (d_policy policy,
                                                 char* role);

#if defined (__cplusplus)
}
#endif

#endif /* D__POLICY_H */
