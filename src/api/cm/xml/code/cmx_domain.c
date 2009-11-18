/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "cmx__factory.h"
#include "cmx__domain.h"
#include "cmx_domain.h"
#include "cmx__entity.h"
#include "u_domain.h"
#include "v_domain.h"
#include "u_participant.h"
#include "u_entity.h"
#include "os_heap.h"
#include <stdio.h>
#include <os_stdlib.h>

c_char*
cmx_domainNew(
    const c_char* participant,
    const c_char* name)
{
    u_participant par;
    u_domain dom;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;

    result = NULL;    
    par = u_participant(cmx_entityUserEntity(participant));
    
    if(par != NULL){
        dom = u_domainNew(par, name, NULL);
    
        if(dom != NULL){
            cmx_registerEntity(u_entity(dom));
            arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
            arg->entity = u_entity(dom);
            arg->create = FALSE;
            arg->participant = NULL;
            arg->result = NULL;
            ur = u_entityAction(u_entity(dom), cmx_entityNewFromAction, (c_voidp)(arg));
            
            if(ur == U_RESULT_OK){
                result = arg->result;
                os_free(arg);
            }
        }
    }
    return result;
}

c_char*
cmx_domainInit(
    v_domain entity)
{
    char buf[512];
    v_domain domain;
    
    domain = v_domain(entity);
    sprintf(buf, "<kind>DOMAIN</kind>");
    
    return (c_char*)(os_strdup(buf));
}
