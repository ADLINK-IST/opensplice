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

#ifndef D__POLICY_H
#define D__POLICY_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

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
};

#if defined (__cplusplus)
}
#endif

#endif
