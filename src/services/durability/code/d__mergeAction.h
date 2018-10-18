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
#include "c_iterator.h"

#ifndef D__MERGEACTION_H
#define D__MERGEACTION_H

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_mergeAction validity.
 * Because d_mergeAction is a concrete class typechecking is required.
 */
#define             d_mergeActionIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_MERGE_ACTION)

/**
 * \brief The d_mergeAction cast macro.
 *
 * This macro casts an object to a d_mergeAction object.
 */
#define d_mergeAction(_this) ((d_mergeAction)(_this))

C_STRUCT(d_mergeAction) {
    C_EXTENDS(d_lock);
    d_conflict conflict;      /* conflict that led to the creation of this mergeAction */
    d_nameSpace nameSpace;    /* Local nameSpace that needs merging. */
    d_table fellows;          /* Fellows to merge with. */
    d_mergeState newState;    /* New state for merge (after action is handled) */
    d_table chains;           /* Requests that have not been fulfilled yet. */
    c_bool stateChanged;      /* Has one of the chains updated the state */
};


d_mergeAction                   d_mergeActionNew                 (d_conflict conflict,
                                                                  d_nameSpace nameSpace,
                                                                  c_iter fellows,
                                                                  d_mergeState fellowState);

void                            d_mergeActionFree                (d_mergeAction action);

void                            d_mergeActionDeinit              (d_mergeAction action);

c_bool                          d_mergeActionAddChain            (d_mergeAction action,
                                                                  d_chain chain);

c_bool                          d_mergeActionRemoveChain         (d_mergeAction action,
                                                                  d_chain dummy);

d_chain                         d_mergeActionGetChain            (d_mergeAction action,
                                                                  d_chain dummy);

c_ulong                         d_mergeActionGetChainCount       (d_mergeAction action);

void                            d_mergeActionChainWalk           (d_mergeAction mergeAction,
                                                                  c_bool ( * action ) (
                                                                     d_chain chain,
                                                                     c_voidp data),
                                                                  c_voidp userData);

int                             d_mergeActionCompare             (d_mergeAction a1,
                                                                  d_mergeAction a2);

d_nameSpace                     d_mergeActionGetNameSpace        (d_mergeAction action);

d_mergeState                    d_mergeActionGetNewState         (d_mergeAction action);

void                            d_mergeActionSetStateChanged     (d_mergeAction action,
                                                                  c_bool stateChanged);

c_bool                          d_mergeActionGetStateChanged     (d_mergeAction action);

void                            d_mergeActionUpdateMergeCount    (d_mergeAction action);

#if defined (__cplusplus)
}
#endif

#endif /* D__MERGEACTION_H */
