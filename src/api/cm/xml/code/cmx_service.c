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
#include "cmx__service.h"
#include "cmx__entity.h"
#include "cmx__serviceState.h"
#include "u_entity.h"
#include "os_heap.h"
#include "v_service.h"
#include "v_entity.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_serviceInit(
    v_service entity)
{
    assert(C_TYPECHECK(entity, v_service));

    return (c_char*)(os_strdup("<kind>SERVICE</kind>"));
}

c_char*
cmx_serviceGetState(
    const c_char* service)
{
    u_entity uservice;
    u_result actionSuccess;
    cmx_entityArg arg;
    c_char* result;

    result = NULL;
    uservice = cmx_entityUserEntity(service);

    if(uservice != NULL){
        arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->participant = u_entityParticipant(uservice);
        arg->create = FALSE;
        arg->result = NULL;
        arg->entity = NULL;

        actionSuccess = u_entityAction(uservice, cmx_serviceAction, arg);

        if(actionSuccess == U_RESULT_OK){
            result = arg->result;
            os_free(arg);
        }
    }
    return result;
}

void
cmx_serviceAction(
    v_entity service,
    c_voidp args)
{
    assert( (v_object(service)->kind == K_SERVICE) ||
            (v_object(service)->kind == K_SPLICED) ||
            (v_object(service)->kind == K_NETWORKING) ||
            (v_object(service)->kind == K_DURABILITY) ||
            (v_object(service)->kind == K_CMSOAP) ||
            (v_object(service)->kind == K_RNR) );

    cmx_entityNewFromAction(v_entity(v_service(service)->state), args);
}

const c_char*
cmx_serviceSetState(
    const c_char* service,
    const c_char* state)
{
    const c_char* result;
    u_entity uservice;
    uservice = cmx_entityUserEntity(service);
    result = CMX_RESULT_FAILED;

    if(uservice != NULL){
        /*@todo TODO: Implement state change.*/
        result = CMX_RESULT_OK;
    }
    return result;
}
