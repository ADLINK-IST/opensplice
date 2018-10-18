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
#include "d__mergeAction.h"
#include "d__mergeState.h"
#include "d__nameSpace.h"
#include "d__fellow.h"
#include "d__sampleChainListener.h"
#include "d__table.h"
#include "d__types.h"
#include "d__conflict.h"
#include "os_heap.h"
#include "os_stdlib.h"

d_mergeAction
d_mergeActionNew(
    d_conflict conflict,   /* conflict that lead to the creation of this mergeAction */
    d_nameSpace nameSpace,
    c_iter fellows,
    d_mergeState newState)
{
    d_mergeAction action;
    c_ulong i;
    d_fellow fellow;

    assert(d_conflictIsValid(conflict));
    assert(d_nameSpaceIsValid(nameSpace));
    assert(fellows);
    assert(c_iterLength(fellows));

    /* Allocate mergeAction object */
    action = d_mergeAction(os_malloc(C_SIZEOF(d_mergeAction)));
    /* Call super-init */
    d_lockInit(d_lock(action), D_MERGE_ACTION,
               (d_objectDeinitFunc)d_mergeActionDeinit);
    /* Initialize mergeAction */
    action->conflict = d_conflict(d_objectKeep(d_object(conflict)));
    action->nameSpace = d_nameSpace(d_objectKeep(d_object(nameSpace)));
    action->fellows = d_tableNew(d_fellowCompare, d_fellowFree);
    /* Initialize the table with the fellows */
    for (i=0;i<c_iterLength(fellows); i++) {
        fellow = d_fellow(d_objectKeep(c_iterObject(fellows, i)));
        assert(d_fellowIsValid(fellow));
        (void)d_tableInsert(action->fellows, fellow);
    }
    if (newState) {
        /* New state value when merge action is completed */
        action->newState = d_mergeStateNew(newState->role, newState->value);
    } else {
        /* Merge state will be determined */
        action->newState = NULL;
    }
    action->chains = d_tableNew(d_chainCompare, d_chainFree);
    action->stateChanged = FALSE;
    return action;
}


void
d_mergeActionFree(
    d_mergeAction action)
{
    assert(d_mergeActionIsValid(action));

    d_objectFree(d_object(action));
}


void
d_mergeActionDeinit(
    d_mergeAction action)
{
    assert(d_mergeActionIsValid(action));

    if (action->conflict) {
        d_conflictFree(action->conflict);
        action->conflict = NULL;
    }
    if (action->nameSpace) {
        d_nameSpaceFree(action->nameSpace);
        action->nameSpace = NULL;
    }
    if (action->fellows) {
        d_tableFree(action->fellows);
        action->fellows = NULL;
    }
    if (action->newState) {
        d_mergeStateFree(action->newState);
        action->newState = NULL;
    }
    if (action->chains) {
        d_tableFree(action->chains);
        action->chains = NULL;
    }
    if (action->newState) {
        d_mergeStateFree (action->newState);
        action->newState = NULL;
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(action));
}


c_bool
d_mergeActionAddChain(
    d_mergeAction action,
    d_chain chain)
{
    d_chain existing;
    c_bool success;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if (action && chain) {
        d_lockLock(d_lock(action));
        existing = d_tableInsert(action->chains, d_objectKeep(d_object(chain)));
        d_lockUnlock(d_lock(action));
        if (existing) {
            /* The chain is a duplicate chain, which means that there
             * already exists a chain for a request of the same group.
             * In that case undo the keep. */
            d_chainFree(chain);
            success = FALSE;
        } else {
            success = TRUE;
        }
    } else {
        success = FALSE;
    }
    return success;
}

c_bool
d_mergeActionRemoveChain(
    d_mergeAction action,
    d_chain chain)
{
    d_chain found;
    c_bool success;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action && chain){
        d_lockLock(d_lock(action));
        found = d_tableRemove(action->chains, chain);
        d_lockUnlock(d_lock(action));

        if(found){
            d_chainFree (found);
            success = TRUE;
        } else {
            success = FALSE;
        }
    } else {
        success = FALSE;
    }
    return success;

}

d_chain
d_mergeActionGetChain(
    d_mergeAction action,
    d_chain dummy)
{
    d_chain chain;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action && dummy){
        chain = d_tableFind(action->chains, dummy);
    } else {
        chain = NULL;
    }
    return chain;
}

c_ulong
d_mergeActionGetChainCount(
    d_mergeAction action)
{
    c_ulong result;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action){
        result = d_tableSize(action->chains);
    } else {
        result = 0;
    }
    return result;
}

void
d_mergeActionChainWalk(
    d_mergeAction mergeAction,
    c_bool ( * action ) (d_chain chain, c_voidp data),
    c_voidp userData)
{
    assert(d_objectIsValid(d_object(mergeAction), D_MERGE_ACTION) == TRUE);

    if(action){
        d_tableWalk(mergeAction->chains, action, userData);
    }
    return;
}


d_nameSpace
d_mergeActionGetNameSpace(
    d_mergeAction action)
{
    d_nameSpace nameSpace;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if (action) {
        nameSpace = action->nameSpace;
    } else {
        nameSpace = NULL;
    }
    return nameSpace;
}

d_mergeState
d_mergeActionGetNewState(
    d_mergeAction action)
{
    d_mergeState newState;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action){
        newState = action->newState;
    } else {
        newState = NULL;
    }
    return newState;
}


/**
 * \brief Compare two mergeActions
 *
 * Two mergeActions are considered equal if their nameSpaces and
 * their resulting mergeState are identical.
 */
int
d_mergeActionCompare(
    d_mergeAction a1,
    d_mergeAction a2)
{
    int result;

    assert(d_mergeActionIsValid(a1));
    assert(d_mergeActionIsValid(a2));

    if (a1 == a2) {
        result = 0;
    } else {
        /* Compare the namespace */
        result = d_nameSpaceCompare(a1->nameSpace, a2->nameSpace);
        if (result == 0) {
            /* Compare the resulting merge state */
            result = d_mergeStateCompare(a1->newState, a2->newState);
            /* Currently the table of fellows in mergeAction is
             * not part of the compare! This may have to change
             * in future. */
        }
    }
    return result;
}

void
d_mergeActionSetStateChanged(
    d_mergeAction action,
    c_bool stateChanged)
{
    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if (action) {
        /* Because this current is a one-way set FALSE -> TRUE added assert to
         * validate this is true */
        assert(stateChanged || !action->stateChanged);

        action->stateChanged = stateChanged;
    }
}

c_bool
d_mergeActionGetStateChanged(
    d_mergeAction action)
{
    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    return action ? action->stateChanged : FALSE;
}

void
d_mergeActionUpdateMergeCount(
    d_mergeAction action)
{
    d_fellow fellow;
    d_tableIter iter;
    d_nameSpace fellowNameSpace;
    c_ulong mergeCount;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION));

    for (fellow = d_tableIterFirst(action->fellows, &iter);
         fellow != NULL;
         fellow = d_tableIterNext(&iter)) {
        fellowNameSpace = d_fellowGetNameSpace(fellow, action->nameSpace);
        mergeCount = d_nameSpaceGetMergeCount(fellowNameSpace);
        d_nameSpaceSetMergeCount(fellowNameSpace, ++mergeCount);
    }
}
