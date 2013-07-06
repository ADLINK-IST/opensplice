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

#ifndef D_POLICY_H
#define D_POLICY_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define             d_policy(o)                 ((d_policy)o)

/* Create new policy object */
d_policy            d_policyNew                 (const char* namespace,
                                                 c_bool aligner,
                                                 d_alignmentKind alignmentKind,
                                                 c_bool delayedAlignment,
                                                 d_durabilityKind durabilityKind);

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

/* Add merge policy rule to policy */
void                d_policyAddMergeRule        (d_policy policy,
                                                 d_mergePolicy mergeType,
                                                 const char* scope);

/* Get merge policy for role */
d_mergePolicy       d_policyGetMergePolicy      (d_policy policy,
                                                 const char* role);

#if defined (__cplusplus)
}
#endif

#endif
