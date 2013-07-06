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
d_mergeStateFree(
    d_mergeState mergeState);

int
d_mergeStateCompare(
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
