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

#ifndef D_NEWGROUP_H
#define D_NEWGROUP_H

#include "d__types.h"
#include "d__misc.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_newGroup(n) ((d_newGroup)(n))

_Ret_notnull_
d_newGroup  d_newGroupNew               (_In_ d_admin admin,
                                         _In_opt_z_ const c_char* partition,
                                         _In_opt_z_ const c_char* topic,
                                         _In_ d_durabilityKind kind,
                                         _In_ d_completeness completeness,
                                         _In_ d_quality quality);

void        d_newGroupSetAlignerCount   (d_newGroup newGroup,
                                         c_ulong count);

void        d_newGroupFree              (d_newGroup group);

int         d_newGroupCompare           (d_newGroup g1,
                                         d_newGroup g2);

#if defined (__cplusplus)
}
#endif

#endif /* D_NEWGROUP_H */
