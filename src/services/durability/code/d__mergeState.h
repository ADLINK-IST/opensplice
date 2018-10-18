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
#ifndef D__MERGESTATE_H_
#define D__MERGESTATE_H_

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_mergeState(s) ((d_mergeState)(s))

d_mergeState
d_mergeStateNew(
    const d_name role,
    c_ulong value);

void
d_mergeStateInit(
    d_mergeState mergeState,
    const d_name role,
    c_ulong value);

void
d_mergeStateFree(
    d_mergeState mergeState);

void
d_mergeStateDeinit(
    d_mergeState mergeState);

int
d_mergeStateCompare(
    d_mergeState state1,
    d_mergeState state2);

int
d_mergeStateRoleValueCompare(
    d_mergeState state1,
    d_mergeState state2);

void
d_mergeStateSetRole(
    d_mergeState mergeState,
    d_name role);

void
d_mergeStateSetValue(
    d_mergeState mergeState,
    c_ulong value);


#if defined (__cplusplus)
}
#endif

#endif /* D__MERGESTATE_H_ */
