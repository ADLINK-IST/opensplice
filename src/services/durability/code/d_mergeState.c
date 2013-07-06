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
#include "d__mergeState.h"
#include "os_heap.h"
#include "os_stdlib.h"

d_mergeState
d_mergeStateNew(
    const d_name role,
    c_ulong value)
{
    d_mergeState mergeState;

    mergeState = d_mergeState(os_malloc(C_SIZEOF(d_mergeState)));

    if(role){
        mergeState->role = os_strdup(role);
    } else {
        mergeState->role = NULL;
    }
    mergeState->value = value;

    return mergeState;
}


void
d_mergeStateFree(
    d_mergeState mergeState)
{
    if(mergeState){
        if(mergeState->role){
            os_free(mergeState->role);
            mergeState->role = NULL;
        }
        os_free(mergeState);
    }
    return;
}

void
d_mergeStateSetRole(
    d_mergeState mergeState,
    d_name role)
{
    if(mergeState){
        if(mergeState->role){
            os_free(mergeState->role);
            mergeState->role = NULL;
        }
        if(role){
            mergeState->role = os_strdup(role);
        }
    }
    return;
}

void
d_mergeStateSetValue(
    d_mergeState mergeState,
    c_ulong value)
{
    if(mergeState){
        mergeState->value = value;
    }
    return;
}

int
d_mergeStateCompare(
    d_mergeState state1,
    d_mergeState state2)
{
    int result;

    if(state1 && state2){
        if(state1->role && state2->role){
            result = strcmp(state1->role, state2->role);
        } else if(state1->role){
            result = 1;
        } else if(state2->role){
            result = -1;
        } else {
            result = 0;
        }
    } else if(state1){
        result = 1;
    } else if(state2){
        result = -1;
    } else {
        result = 0;
    }
    return result;
}
