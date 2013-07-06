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
#include "cmx__publisher.h"
#include "cmx_publisher.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "cmx__qos.h"
#include "u_participant.h"
#include "u_publisher.h"
#include "u_entity.h"
#include "v_publisher.h"
#include "v_publisherQos.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_publisherNew(
    const c_char* participant,
    const c_char* name,
    const c_char* qos)
{
    u_participant par;
    u_publisher pub;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;
    v_publisherQos pqos;
    cmx_entityKernelArg kernelArg;

    result = NULL;    
    par = u_participant(cmx_entityUserEntity(participant));
    
    if(par != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        u_entityAction(u_entity(par), cmx_entityKernelAction, (c_voidp)kernelArg);
        
        if(qos != NULL){
            pqos = v_publisherQos(cmx_qosKernelQosFromKind(qos, K_PUBLISHER, c_getBase(c_object(kernelArg->kernel))));
            
            if(pqos == NULL){
                pqos = v_publisherQosNew(kernelArg->kernel, NULL);
            }
        } else {
            pqos = v_publisherQosNew(kernelArg->kernel, NULL);
        } 
        pub = u_publisherNew(par, name, pqos, TRUE);
        os_free(kernelArg);
        c_free(pqos);
        
        if(pub != NULL){
            cmx_registerEntity(u_entity(pub));
            arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
            arg->entity = u_entity(pub);
            arg->create = FALSE;
            arg->participant = NULL;
            arg->result = NULL;
            ur = u_entityAction(u_entity(pub), cmx_entityNewFromAction, (c_voidp)(arg));
            
            if(ur == U_RESULT_OK){
                result = arg->result;
                os_free(arg);
            }
        }
    }
    return result;
}

c_char*
cmx_publisherInit(
    v_publisher entity)
{
    assert(C_TYPECHECK(entity, v_publisher));

    return (c_char*)(os_strdup("<kind>PUBLISHER</kind>"));
}

const c_char*
cmx_publisherPublish(
    const c_char* publisher,
    const c_char* domainExpr)
{
    u_publisher pub;
    u_result ur;
    const c_char* result;
    
    pub = u_publisher(cmx_entityUserEntity(publisher));
    
    if(pub != NULL){
        ur = u_publisherPublish(pub, domainExpr);
        
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
