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
#include "cmx__subscriber.h"
#include "cmx_subscriber.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "cmx__qos.h"
#include "u_subscriber.h"
#include "u_participant.h"
#include "u_entity.h"
#include "v_subscriber.h"
#include "v_subscriberQos.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_subscriberNew(
    const c_char* participant,
    const c_char* name,
    const c_char* qos)
{
    u_participant par;
    u_subscriber sub;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;
    v_subscriberQos sqos;
    cmx_entityKernelArg kernelArg;

    result = NULL;    
    par = u_participant(cmx_entityUserEntity(participant));
    
    if(par != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        u_entityAction(u_entity(par), cmx_entityKernelAction, (c_voidp)kernelArg);
        
        if(qos != NULL){
            sqos = v_subscriberQos(cmx_qosKernelQosFromKind(qos, K_SUBSCRIBER, c_getBase(c_object(kernelArg->kernel))));
            
            if(sqos == NULL){
                sqos = v_subscriberQosNew(kernelArg->kernel, NULL);
            }
        } else {
            sqos = v_subscriberQosNew(kernelArg->kernel, NULL);
        } 
        sub = u_subscriberNew(par, name, sqos, TRUE);
        c_free(sqos);
        os_free(kernelArg);
        
        if(sub != NULL){
            cmx_registerEntity(u_entity(sub));
            arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
            arg->entity = u_entity(sub);
            arg->create = FALSE;
            arg->participant = NULL;
            arg->result = NULL;
            ur = u_entityAction(u_entity(sub), cmx_entityNewFromAction, (c_voidp)(arg));
            
            if(ur == U_RESULT_OK){
                result = arg->result;
                os_free(arg);
            }
        }
    }
    return result;
}


const c_char*
cmx_subscriberSubscribe(
    const c_char* subscriber,
    const c_char* domainExpr)
{
    u_subscriber sub;
    u_result ur;
    const c_char* result;
    
    sub = u_subscriber(cmx_entityUserEntity(subscriber));
    
    if(sub != NULL){
        ur = u_subscriberSubscribe(sub, domainExpr);
        
        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else {
            result = CMX_RESULT_FAILED;
        }
    } else {
        result = CMX_RESULT_FAILED;
    }
    return result;
}

c_char*
cmx_subscriberInit(
    v_subscriber entity)
{
    assert(C_TYPECHECK(entity, v_subscriber));
    
    return (c_char*)(os_strdup("<kind>SUBSCRIBER</kind>"));
}
