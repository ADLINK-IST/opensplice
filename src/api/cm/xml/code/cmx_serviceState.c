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
#include "cmx__serviceState.h"
#include "cmx__entity.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_serviceStateInit(
    v_serviceState entity)
{
    char buf[512];
    
    if(v_entityName(entity) == NULL){
        os_sprintf(buf, "<kind>SERVICESTATE</kind><statename>NULL</statename><state>%s</state>", 
        cmx_serviceStateKindToString(entity->stateKind));
    } else {
       os_sprintf(buf, "<kind>SERVICESTATE</kind><statename>%s</statename><state>%s</state>", 
        v_entityName(entity),
        cmx_serviceStateKindToString(entity->stateKind));
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
