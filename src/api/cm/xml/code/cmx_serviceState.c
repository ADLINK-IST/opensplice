/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "cmx__serviceState.h"
#include "cmx__entity.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_serviceStateInit(
    v_serviceState state)
{
    char buf[512];

    assert(state);

    if(state->name == NULL){
        os_sprintf(buf, "<kind>SERVICESTATE</kind><statename>NULL</statename><state>%s</state>",
        cmx_serviceStateKindToString(state->stateKind));
    } else {
        os_sprintf(buf, "<kind>SERVICESTATE</kind><statename>%s</statename><state>%s</state>",
        state->name,
        cmx_serviceStateKindToString(state->stateKind));
    }
    return (c_char*)(os_strdup(buf));
}

const c_char*
cmx_serviceStateKindToString(
    v_serviceStateKind stateKind)
{
    const c_char* r;

    r = NULL;

    switch(stateKind){
        case STATE_NONE:                        r = "NONE";         break;
        case STATE_INITIALISING:                r = "INITIALISING"; break;
        case STATE_OPERATIONAL:                 r = "OPERATIONAL";  break;
        case STATE_INCOMPATIBLE_CONFIGURATION:  r = "INCOMPATIBLE_CONFIGURATION"; break;
        case STATE_TERMINATING:                 r = "TERMINATING";  break;
        case STATE_TERMINATED:                  r = "TERMINATED";   break;
        case STATE_DIED:                        r = "DIED";         break;
        default:                                assert(FALSE);      break;
    }
    return r;
}

v_serviceStateKind
cmx_serviceStateKindFromString(
    const c_char* stateKind)
{
    v_serviceStateKind result;

    if(stateKind == NULL){
        result = STATE_NONE;
    } else if(strcmp(stateKind, "INITIALISING") == 0){
        result = STATE_INITIALISING;
    } else if(strcmp(stateKind, "OPERATIONAL") == 0){
        result = STATE_OPERATIONAL;
    } else if(strcmp(stateKind, "INCOMPATIBLE_CONFIGURATION") == 0){
        result = STATE_INCOMPATIBLE_CONFIGURATION;
    } else if(strcmp(stateKind, "TERMINATING") == 0){
        result = STATE_TERMINATING;
    } else if(strcmp(stateKind, "TERMINATED") == 0){
        result = STATE_TERMINATED;
    } else if(strcmp(stateKind, "DIED") == 0){
        result = STATE_DIED;
    } else {
       assert(FALSE);
       result = STATE_NONE;
    }
    return result;
}
