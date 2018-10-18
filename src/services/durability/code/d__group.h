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

#ifndef D__GROUP_H
#define D__GROUP_H

#include "d__types.h"
#include "d__admin.h"
#include "d__misc.h"
#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define D_BITSPERBYTE            (8)
#define D_BITS(type)             (D_BITSPERBYTE * (os_int32)sizeof (type))
#if defined(__STDC__)
#define D_HIBITL                 (1UL << (D_BITS(os_int32) - 1))
#else
#define D_HIBITL                 (1L  << (D_BITS(os_int32) - 1))
#endif

/**
 * Macro that checks the d_group validity.
 * Because d_group is a concrete class typechecking is required.
 */
#define             d_groupIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_GROUP)

/**
 * \brief The d_group cast macro.
 *
 * This macro casts an object to a d_group object.
 */
#define d_group(_this) ((d_group)(_this))

C_STRUCT(d_group) {
    C_EXTENDS(d_object);
    d_topic topic;
    d_partition partition;
    d_durabilityKind kind;
    d_completeness completeness;
    d_quality quality;
    v_group vgroup;
    c_ulong storeCount;
    c_bool private;
    c_ulong creationRetryCount;   /* number of tries to create a local group, used in the d_groupCreationQueueRun() */
    c_bool storeMessagesLoaded;
};

d_group             d_groupNew              (const c_char* partition,
                                             const c_char* topic,
                                             d_durabilityKind kind,
                                             d_completeness completeness,
                                             d_quality quality);

void                d_groupDeinit           (d_group group);

void                d_groupFree             (d_group group);

int                 d_groupCompare          (d_group group1,
                                             d_group group2);

d_completeness      d_groupGetCompleteness  (d_group group);

d_partition         d_groupGetPartition     (d_group group);

d_topic             d_groupGetTopic         (d_group group);

d_durabilityKind    d_groupGetKind          (d_group group);

d_quality           d_groupGetQuality       (d_group group);

void                d_groupUpdate           (d_group group,
                                             d_completeness completeness,
                                             d_quality quality,
                                             d_admin admin);

void                d_groupSetKernelGroup   (d_group group,
                                             v_group vgroup);

void                d_groupSetKernelGroupCompleteness(
                                            _In_ d_group group,
                                            _In_ c_bool complete);

void                d_groupSetKernelGroupNoInterest(
                                            _In_ d_group group);

v_group             d_groupGetKernelGroup   (d_group group);

c_bool              d_groupIsBuiltinGroup   (d_group group);

void                d_groupSetComplete      (d_group group,
                                             d_admin admin);

void                d_groupSetNoInterest    (d_group group,
                                             d_admin admin);

void                d_groupSetUnaligned     (d_group group,
                                             d_admin admin);

void                d_groupSetIncomplete    (d_group group,
                                             d_admin admin);

void                d_groupSetPrivate       (d_group group,
                                             c_bool isPrivate);

c_bool              d_groupIsPrivate        (d_group group);

void                d_groupPublishStateUpdate (d_group group,
                                               d_admin admin);

void                d_groupSetStoreMessagesLoaded(d_group group,
                                             c_bool isLoaded);

c_bool              d_groupIsStoreMessagesLoaded(d_group group);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUP_H */
