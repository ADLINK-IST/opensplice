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
#include "d__types.h"

#ifndef D_NAMESPACE_H
#define D_NAMESPACE_H

#if defined (__cplusplus)
extern "C" {
#endif

#define             d_element(o)                        ((d_element)o)
#define             d_nameSpace(o)                      ((d_nameSpace)o)

d_nameSpace         d_nameSpaceNew                      (d_configuration config,
                                                         const char * name);

d_nameSpace         d_nameSpaceNew_w_policy             (d_configuration config,
                                                         const char * name,
                                                         c_bool aligner,
                                                         d_alignmentKind alignmentKind,
                                                         c_bool delayedAlignment,
                                                         d_durabilityKind durabilityKind);

d_nameSpace         d_nameSpaceCopy                     (d_nameSpace nameSpace);

int                 d_nameSpaceCompare                  (d_nameSpace ns1,
                                                         d_nameSpace ns2);

int                 d_nameSpaceCompatibilityCompare     (d_nameSpace ns1,
                                                         d_nameSpace ns2);

int					d_nameSpaceNameCompare				(d_nameSpace ns1,
														 d_nameSpace ns2);

void                d_nameSpaceFree                     (d_nameSpace nameSpace);

void                d_nameSpaceAddElement               (d_nameSpace nameSpace,
                                                         const char * name,
                                                         const char * partition,
                                                         const char * topic);

char *              d_nameSpaceGetName                  (d_nameSpace nameSpace);

d_policy            d_nameSpaceGetPolicy                (d_nameSpace nameSpace);


c_bool              d_nameSpaceIsEmpty                  (d_nameSpace nameSpace);

c_bool              d_nameSpaceIsIn                     (d_nameSpace nameSpace,
                                                         d_partition partition,
                                                         d_topic topic);

void                d_nameSpaceElementWalk              (d_nameSpace nameSpace,
                                                         c_bool ( * action ) (
                                                            d_element element,
                                                            c_voidp userData),
                                                         c_voidp args);

d_name              d_elementGetExpression              (d_element element);

c_bool              d_nameSpaceIsAligner                (d_nameSpace nameSpace) ;

c_char*             d_nameSpaceGetPartitions            (d_nameSpace nameSpace);

c_char*             d_nameSpaceGetPartitionTopics       (d_nameSpace nameSpace);

void                d_nameSpaceCopyPartitions           (d_nameSpace to,
                                                         d_nameSpace from);

void                d_nameSpaceSetInitialQuality        (d_nameSpace nameSpace,
                                                         d_quality quality);

d_quality           d_nameSpaceGetInitialQuality        (d_nameSpace nameSpace);

d_nameSpace         d_nameSpaceFromNameSpaces           (d_nameSpaces ns);

d_alignmentKind     d_nameSpaceGetAlignmentKind         (d_nameSpace nameSpace);

d_durabilityKind    d_nameSpaceGetDurabilityKind        (d_nameSpace nameSpace);


void				d_nameSpaceSetMasterState			(d_nameSpace nameSpace,
														 d_serviceState serviceState);

d_serviceState		d_nameSpaceGetMasterState			(d_nameSpace nameSpace);

void                d_nameSpaceSetMaster                (d_nameSpace nameSpace,
                                                         d_networkAddress master);

d_networkAddress    d_nameSpaceGetMaster                (d_nameSpace nameSpace);

void				d_nameSpaceMasterConfirmed			(d_nameSpace nameSpace);

void				d_nameSpaceMasterPending			(d_nameSpace nameSpace);

c_bool				d_nameSpaceIsMasterConfirmed		(d_nameSpace nameSpace);

c_bool              d_nameSpaceMasterIsMe               (d_nameSpace nameSpace,
                                                         d_admin admin);

c_bool              d_nameSpaceIsAlignmentNotInitial    (d_nameSpace nameSpace);

c_bool              d_nameSpaceGetDelayedAlignment      (d_nameSpace nameSpace);

c_bool
d_nameSpaceStringMatches(
    c_string str,
    c_string pattern);

d_name
d_nameSpaceGetRole(
    d_nameSpace nameSpace);

d_mergePolicy
d_nameSpaceGetMergePolicy(
    d_nameSpace nameSpace,
    d_name role);

d_mergeState
d_nameSpaceGetMergeState(
    d_nameSpace nameSpace,
    d_name role);

void
d_nameSpaceSetMergeState(
    d_nameSpace nameSpace,
    d_mergeState state);

int
d_nameSpaceGetMergePolicyFromStates (
    d_nameSpace ns1,
    d_nameSpace ns2);

void
d_nameSpaceClearMergeState(
    d_nameSpace nameSpace,
    d_name role);

void
d_nameSpaceReplaceMergeStates(
    d_nameSpace ns1,
    d_nameSpace ns2);

c_iter
d_nameSpaceGetMergedStatesDiff(
    d_nameSpace ns1,
    d_nameSpace ns2);


#if defined (__cplusplus)
}
#endif

#endif /* D_NAMESPACE_H */
