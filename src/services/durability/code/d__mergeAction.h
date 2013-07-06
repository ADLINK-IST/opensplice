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
#include "d_lock.h"
#include "c_iterator.h"

#ifndef D__MERGEACTION_H_
#define D__MERGEACTION_H_

#if defined (__cplusplus)
extern "C" {
#endif


C_STRUCT(d_mergeAction) {
    C_EXTENDS(d_lock);
    d_nameSpace nameSpace;    /* Local nameSpace that needs merging. */
    d_fellow fellow;          /* Fellow to merge with. */
    d_mergeState newState;    /* New state for merge (after action is handled) */
    d_table chains;           /* Requests that have not been fulfilled yet. */
};

#define d_mergeAction(m) ((d_mergeAction)(m))

d_mergeAction
d_mergeActionNew(
    d_nameSpace nameSpace,
    d_fellow fellow,
    d_mergeState fellowState);

void
d_mergeActionFree(
    d_mergeAction action);

void
d_mergeActionDeinit(
    d_object object);

c_bool
d_mergeActionAddChain(
    d_mergeAction action,
    d_chain chain);

c_bool
d_mergeActionRemoveChain(
    d_mergeAction action,
    d_chain dummy);

d_chain
d_mergeActionGetChain(
    d_mergeAction action,
    d_chain dummy);

c_ulong
d_mergeActionGetChainCount(
    d_mergeAction action);

void
d_mergeActionChainWalk(
    d_mergeAction mergeAction,
    c_bool ( * action ) (d_chain chain, c_voidp data),
    c_voidp userData);

int
d_mergeActionCompare(
    d_mergeAction a1,
    d_mergeAction a2);


d_fellow
d_mergeActionGetFellow(
    d_mergeAction action);

d_nameSpace
d_mergeActionGetNameSpace(
    d_mergeAction action);

d_mergeState
d_mergeActionGetNewState(
    d_mergeAction action);

#if defined (__cplusplus)
}
#endif

#endif /* D__MERGEACTION_H_ */
