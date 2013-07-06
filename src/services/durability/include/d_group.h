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

#ifndef D_GROUP_H
#define D_GROUP_H

#include "d__types.h"
#include "v_group.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_group(g)    ((d_group)(g))

#define D_BITSPERBYTE            (8)
#define D_BITS(type)             (D_BITSPERBYTE * (os_int32)sizeof (type))
#if defined(__STDC__)
#define D_HIBITL                 (1UL << (D_BITS(os_int32) - 1))
#else
#define D_HIBITL                 (1L  << (D_BITS(os_int32) - 1))
#endif
#define D_MAXLONG                ((os_int32)(~D_HIBITL))
#define D_MAX_QUALITY            (D_MAXLONG)    /**< the maximum value of a d_quality item */

d_group             d_groupNew              (const c_char* partition,
                                             const c_char* topic,
                                             d_durabilityKind kind,
                                             d_completeness completeness,
                                             d_quality quality);

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
                                             d_quality quality);

void                d_groupSetKernelGroup   (d_group group,
                                             v_group vgroup);

v_group             d_groupGetKernelGroup   (d_group group);

c_bool              d_groupIsBuiltinGroup   (d_group group);

void                d_groupSetComplete      (d_group group);

void                d_groupSetUnaligned     (d_group group);

void                d_groupSetIncomplete    (d_group group);

void                d_groupSetPrivate       (d_group group,
                                             c_bool isPrivate);

c_bool              d_groupIsPrivate        (d_group group);

#if defined (__cplusplus)
}
#endif

#endif /* D__GROUP_H */
