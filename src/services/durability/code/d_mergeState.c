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
    d_mergeStateInit(mergeState, role, value);

    return mergeState;
}

void
d_mergeStateInit(
    d_mergeState mergeState,
    const d_name role,
    c_ulong value)
{
    if (mergeState) {
        if (role) {
            mergeState->role = os_strdup(role);
        } else {
            mergeState->role = NULL;
        }
        mergeState->value = value;
    }
}


void
d_mergeStateFree(
    d_mergeState mergeState)
{
    /* Note that mergeState can be NULL.
     * Therefore, a NULL check is needed before freeing the mergeState.
     */
    if (mergeState) {
        d_mergeStateDeinit(mergeState);
        os_free(mergeState);
    }
    return;
}

void
d_mergeStateDeinit(
    d_mergeState mergeState)
{
    if (mergeState) {
       if(mergeState->role){
           os_free(mergeState->role);
           mergeState->role = NULL;
       }
    }
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

/* Compares mergeStates only on the role, primarily used in tableCompare */
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

/* Compares mergeStates based on role AND value */
int
d_mergeStateRoleValueCompare(
    d_mergeState state1,
    d_mergeState state2)
{
    int result;

    result = d_mergeStateCompare(state1, state2);
    if (result == 0) {
        if ((state1 != NULL) && (state2 != NULL)) {
            if (state1->value < state2->value) {
                result = -1;
            } else if (state1->value > state2->value) {
                result = 1;
            } else {
                result = 0;
            }
        } else {
            /* The only possibility is both state1 == NULL && state2 == NULL */
            assert(state1 == NULL);
            assert(state2 == NULL);
            result = 0;
        }
    }
    return result;
}
