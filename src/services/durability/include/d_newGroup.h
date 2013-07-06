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

#ifndef D_NEWGROUP_H
#define D_NEWGROUP_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_newGroup(n) ((d_newGroup)(n))

d_newGroup  d_newGroupNew               (d_admin admin,
                                         const c_char* partition,
                                         const c_char* topic,
                                         d_durabilityKind kind,
                                         d_completeness completeness,
                                         d_quality quality);

void        d_newGroupSetAlignerCount   (d_newGroup newGroup,
                                         c_ulong count);

void        d_newGroupFree              (d_newGroup group);

int         d_newGroupCompare           (d_newGroup g1,
                                         d_newGroup g2);

#if defined (__cplusplus)
}
#endif

#endif /* D_NEWGROUP_H */
