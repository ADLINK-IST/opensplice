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
#include "d__mergeAction.h"
#include "d__mergeState.h"
#include "d_nameSpace.h"
#include "d_sampleChainListener.h"
#include "d_fellow.h"
#include "d_table.h"
#include "os_heap.h"
#include "os_stdlib.h"

d_mergeAction
d_mergeActionNew(
    d_nameSpace nameSpace,
    d_fellow fellow,
    d_mergeState newState)
{
    d_mergeAction action;

    assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE) == TRUE);
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    assert(newState);
    assert(newState->role);

    action = d_mergeAction(os_malloc(C_SIZEOF(d_mergeAction)));

    if(action){
        d_lockInit(d_lock(action), D_MERGE_ACTION, d_mergeActionDeinit);
        action->nameSpace = d_nameSpace(d_objectKeep(d_object(nameSpace)));
        action->fellow = d_fellow(d_objectKeep(d_object(fellow)));
        action->newState = (d_mergeState)(os_malloc(C_SIZEOF(d_mergeState)));
        action->newState->value = newState->value;
        action->newState->role = os_strdup(newState->role);
        action->chains = d_tableNew(d_chainCompare, d_chainFree);
    }
    return action;
}

void
d_mergeActionFree(
    d_mergeAction action)
{
    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action){
        d_lockFree(d_lock(action), D_MERGE_ACTION);
    }
}

void
d_mergeActionDeinit(
    d_object object)
{
    d_mergeAction action;

    assert(d_objectIsValid(object, D_MERGE_ACTION) == TRUE);

    if(object){
        action = d_mergeAction(object);

        if(action->nameSpace){
            d_nameSpaceFree(action->nameSpace);
            action->nameSpace = NULL;
        }
        if(action->fellow){
            d_fellowFree(action->fellow);
            action->fellow = NULL;
        }
        if(action->newState){
            if(action->newState->role){
                os_free(action->newState->role);
                action->newState->role = NULL;
            }
            os_free(action->newState);
            action->newState = NULL;
        }
        if(action->chains){
            d_tableFree(action->chains);
            action->chains = NULL;
        }
        if(action->newState){
            d_mergeStateFree (action->newState);
            action->newState = NULL;
        }
    }
    return;
}

c_bool
d_mergeActionAddChain(
    d_mergeAction action,
    d_chain chain)
{
    d_chain existing;
    c_bool success;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action && chain){
        d_lockLock(d_lock(action));
        d_objectKeep (d_object(chain));
        existing = d_tableInsert(action->chains, chain);
        d_lockUnlock(d_lock(action));

        if(existing){
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

d_fellow
d_mergeActionGetFellow(
    d_mergeAction action)
{
    d_fellow fellow;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action){
        fellow = action->fellow;
    } else {
        fellow = NULL;
    }
    return fellow;
}

d_nameSpace
d_mergeActionGetNameSpace(
    d_mergeAction action)
{
    d_nameSpace nameSpace;

    assert(d_objectIsValid(d_object(action), D_MERGE_ACTION) == TRUE);

    if(action){
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

int
d_mergeActionCompare(
    d_mergeAction a1,
    d_mergeAction a2)
{
    int result;

    if(a1 && a2){
        result = d_fellowCompare(a1->fellow, a2->fellow);

        if(result == 0){
            result = d_mergeStateCompare(a1->newState, a2->newState);

            if(result == 0){
                result = d_nameSpaceCompare(a1->nameSpace, a2->nameSpace);
            }
        }
    } else if(a1){
        result = 1;
    } else if(a2){
        result = -1;
    } else {
        result = 0;
    }
    return result;
}
