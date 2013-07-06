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
#include "cmx__query.h"
#include "cmx_query.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "u_entity.h"
#include "u_reader.h"
#include "u_query.h"
#include "v_dataReaderQuery.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_queryNew(
    const c_char* reader,
    const c_char* name,
    const c_char* expression)
{
    u_reader rea;
    u_query que;
    q_expr qexpr;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;

    result = NULL;    
    rea = u_reader(cmx_entityUserEntity(reader));
    
    if(rea != NULL){
        qexpr = q_parse(expression);
        
        if(qexpr != NULL){
            que = u_queryNew(rea, name, qexpr, NULL);
            q_dispose(qexpr);
        
            if(que != NULL){
                cmx_registerEntity(u_entity(que));
                arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
                arg->entity = u_entity(que);
                arg->create = FALSE;
                arg->participant = NULL;
                arg->result = NULL;
                ur = u_entityAction(u_entity(que),
                                    cmx_entityNewFromAction,
                                    (c_voidp)(arg));
                
                if(ur == U_RESULT_OK){
                    result = arg->result;
                    os_free(arg);
                }
            }
        }
    }
    return result;
}

c_char*
cmx_queryInit(
    v_query entity)
{
    v_dataReaderQuery query;
    char buf[512];

    query = v_dataReaderQuery(entity);

    if(query->expression){
        if(query->params){
            os_sprintf(buf, 
                "<kind>QUERY</kind>"
                "<expression><![CDATA[%s]]></expression>"
                "<params><![CDATA[%s]]></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>", 
                query->expression, query->params, 
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        } else {
            os_sprintf(buf, 
                "<kind>QUERY</kind>"
                "<expression><![CDATA[%s]]></expression>"
                "<params></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>", 
                query->expression, 
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        }
    } else {
        if(query->params){
            os_sprintf(buf, 
                "<kind>QUERY</kind>"
                "<expression></expression>"
                "<params><![CDATA[%s]]></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>", 
                query->params,  
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        } else {
            os_sprintf(buf, 
                "<kind>QUERY</kind>"
                "<expression></expression>"
                "<params></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>", 
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        }
    }
    return (c_char*)(os_strdup(buf));
}
