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
#include "d__types.h"
#include "d__lock.h"
#include "d__misc.h"

#ifndef D__NAMESPACE_H
#define D__NAMESPACE_H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_nameSpace validity.
 * Because d_nameSpace is a concrete class typechecking is required.
 */
#define             d_nameSpaceIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_NAMESPACE)

/**
 * \brief The d_nameSpace cast macro.
 *
 * This macro casts an object to a d_nameSpace object.
 */
#define d_nameSpace(_this) ((d_nameSpace)(_this))

/**
 * \brief The d_filter cast macro.
 *
 * This macro casts an object to a d_filter object.
 */
#define d_filter(_this) ((d_filter)(_this))

/* used when searching for a match on partition and topic */
C_CLASS(d_nameSpaceSearch);
C_STRUCT(d_nameSpaceSearch) {
    d_partition  partition;
    d_topic      topic;
    c_bool       match;
};

C_STRUCT(d_nameSpace) {
    C_EXTENDS(d_lock);
    d_name           name;
    d_policy         policy;
    d_quality        quality;
    d_networkAddress master;
    d_serviceState   masterState;
    c_bool           masterConfirmed;
    d_table          elements;
    d_mergeState     mergeState;             /* The internal mergeState for this nameSpace */
    d_mergeState     advertisedMergeState;   /* The mergeState that is advertised to fellows */
    d_table          mergedRoleStates;
    c_bool           mustAlignBuiltinTopics; /* TRUE if durability must align builtin topics */
    c_bool           alignable;              /* TRUE if the fellow that owns this nameSpace
                                              * is a candidate to align from. The alignable
                                              * attribute has only meaning for fellow nameSpaces
                                              */
    c_bool           compatibility_check_required; /* Indicator if compatibility check is still required */
    c_ulong          mergeCount;
};

typedef enum d_nameSpaceHelperKind_s {
    D_NS_COUNT, D_NS_COPY
} d_nameSpaceHelperKind;

struct d_nameSpaceHelper{
    d_nameSpaceHelperKind kind;
    os_uint32 count;
    c_char* value;
    d_nameSpace ns;
};

d_nameSpace         d_nameSpaceNew                      (const char * name,
                                                         d_policy policy);

d_nameSpace         d_nameSpaceCopy                     (d_nameSpace nameSpace);

int                 d_nameSpaceCompare                  (d_nameSpace ns1,
                                                         d_nameSpace ns2);

int                 d_nameSpaceCompatibilityCompare     (d_nameSpace ns1,
                                                         d_nameSpace ns2);

int                 d_nameSpaceNameCompare              (d_nameSpace ns1,
                                                         d_nameSpace ns2);

void                d_nameSpaceFree                     (d_nameSpace nameSpace);

int                 d_nameSpaceAddElement               (d_nameSpace nameSpace,
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

c_bool              d_nameSpaceIsAligner                (d_nameSpace nameSpace) ;

c_char*             d_nameSpaceGetPartitions            (d_nameSpace nameSpace);

c_char*             d_nameSpaceGetPartitionTopics       (d_nameSpace nameSpace);

void                d_nameSpaceCopyPartitions           (d_nameSpace to,
                                                         d_nameSpace from);

void                d_nameSpaceSetInitialQuality        (d_nameSpace nameSpace,
                                                         d_quality quality);

d_quality           d_nameSpaceGetInitialQuality        (d_nameSpace nameSpace);

d_nameSpace         d_nameSpaceFromNameSpaces           (d_configuration config,
                                                         d_nameSpaces ns);

d_alignmentKind     d_nameSpaceGetAlignmentKind         (d_nameSpace nameSpace);

d_durabilityKind    d_nameSpaceGetDurabilityKind        (d_nameSpace nameSpace);


void                d_nameSpaceSetMasterState           (d_nameSpace nameSpace,
                                                         d_serviceState serviceState);

d_serviceState      d_nameSpaceGetMasterState           (d_nameSpace nameSpace);

void                d_nameSpaceSetMaster                (d_nameSpace nameSpace,
                                                         d_networkAddress master);

d_networkAddress    d_nameSpaceGetMaster                (d_nameSpace nameSpace);

void                d_nameSpaceMasterConfirmed          (d_nameSpace nameSpace);

void                d_nameSpaceMasterPending            (d_nameSpace nameSpace);

void                d_nameSpaceSetMasterConfirmed       (d_nameSpace nameSpace,
                                                         c_bool masterConfirmed);

c_bool              d_nameSpaceIsMasterConfirmed        (d_nameSpace nameSpace);

c_bool              d_nameSpaceMasterIsMe               (d_nameSpace nameSpace,
                                                         d_admin admin);

c_bool              d_nameSpaceMasterIsAddress          (d_nameSpace nameSpace,
                                                         d_networkAddress addr);

c_bool              d_nameSpaceIsAlignmentNotInitial    (d_nameSpace nameSpace);

c_bool              d_nameSpaceGetDelayedAlignment      (d_nameSpace nameSpace);

c_bool              d_nameSpaceStringMatches            (c_string str,
                                                         c_string pattern);

d_name              d_nameSpaceGetRole                  (d_nameSpace nameSpace);

d_mergePolicy       d_nameSpaceGetMergePolicy           (d_nameSpace nameSpace,
                                                         d_name role);

d_mergeState        d_nameSpaceGetMergeState            (d_nameSpace nameSpace,
                                                         d_name role);

d_mergeState        d_nameSpaceGetAdvertisedMergeState  (d_nameSpace nameSpace);

void                d_nameSpaceSetMergeState            (d_nameSpace nameSpace,
                                                         d_mergeState state);

int                 d_nameSpaceGetMergePolicyFromStates (d_nameSpace ns1,
                                                         d_nameSpace ns2);

void                d_nameSpaceClearMergeState          (d_nameSpace nameSpace,
                                                         d_name role);

void                d_nameSpaceReplaceMergeStates       (d_nameSpace ns1,
                                                         d_nameSpace ns2);

c_iter              d_nameSpaceGetMergedStatesDiff      (d_nameSpace ns1,
                                                         d_nameSpace ns2);

c_bool              d_nameSpaceGetPartitionsAction      (d_element element,
                                                         c_voidp args);

c_bool              d_nameSpaceGetPartitionTopicsAction (d_element element,
                                                         c_voidp args);

void                d_nameSpaceDeinit                   (d_nameSpace nameSpace);

c_bool              d_nameSpaceIsAlignable              (d_nameSpace nameSpace);

void                d_nameSpaceSetAlignable             (d_nameSpace nameSpace,
                                                         c_bool alignable);

c_bool              d_nameSpaceGetEqualityCheck         (d_nameSpace nameSpace);

c_ulong             d_nameSpaceGetMasterPriority        (d_nameSpace nameSpace);

void                d_nameSpaceSyncMergeState           (d_nameSpace nameSpace);

void                d_nameSpaceSetMergeCount            (d_nameSpace nameSpace,
                                                         c_ulong mergeCount);

c_ulong             d_nameSpaceGetMergeCount            (d_nameSpace nameSpace);

#if defined (__cplusplus)
}
#endif

#endif /* D__NAMESPACE_H */
