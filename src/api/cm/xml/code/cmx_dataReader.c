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
#include "cmx__factory.h"
#include "cmx__dataReader.h"
#include "cmx_dataReader.h"
#include "cmx__entity.h"
#include "cmx__qos.h"
#include "u_entity.h"
#include "u_subscriber.h"
#include "u_dataReader.h"
#include "v_dataReader.h"
#include "v_readerQos.h"
#include "os_heap.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_dataReaderNew(
    const c_char* subscriber,
    const c_char* name,
    const c_char* view,
    const c_char* qos)
{
    u_subscriber sub;
    u_dataReader rea;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;
    cmx_entityKernelArg kernelArg;
    v_readerQos rqos;
    q_expr qexpr;

    result = NULL;
    sub = u_subscriber(cmx_entityUserEntity(subscriber));

    if(sub != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        u_entityAction(u_entity(sub),
                       cmx_entityKernelAction,
                       (c_voidp)kernelArg);

        if(qos != NULL){
            rqos = v_readerQos(cmx_qosKernelQosFromKind(qos, K_DATAREADER, c_getBase(c_object(kernelArg->kernel))));

            if(rqos == NULL){
                rqos = v_readerQosNew(kernelArg->kernel, NULL);
            }
        } else {
            rqos = v_readerQosNew(kernelArg->kernel, NULL);
        }
        if(view != NULL){
            qexpr = q_parse(view);

            if(qexpr != NULL){
                rea = u_dataReaderNew(sub, name,  qexpr, NULL, rqos, TRUE);
                q_dispose(qexpr);
            } else {
                rea = NULL;
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "cmx_dataReaderNew: invalid view expression.");
            }
        } else {
            rea = u_dataReaderNew(sub, name,  NULL, NULL, rqos, TRUE);
        }
        c_free(rqos);
        os_free(kernelArg);

        if(rea != NULL){
            cmx_registerEntity(u_entity(rea));
            arg = cmx_entityArg(os_malloc((os_uint32)(C_SIZEOF(cmx_entityArg))));
            arg->entity = u_entity(rea);
            arg->create = FALSE;
            arg->participant = NULL;
            arg->result = NULL;
            ur = u_entityAction(u_entity(rea),
                                cmx_entityNewFromAction,
                                (c_voidp)(arg));

            if(ur == U_RESULT_OK){
                result = arg->result;
                os_free(arg);
            }
        }
    }
    return result;
}

c_char*
cmx_dataReaderInit(
    v_dataReader entity)
{
    assert(C_TYPECHECK(entity, v_dataReader));

    return (c_char*)(os_strdup("<kind>DATAREADER</kind>"));
}

const c_char*
cmx_dataReaderWaitForHistoricalData(
    const c_char* dataReader,
    const c_time timeout)
{
    u_result ur;
    const c_char* result;
    u_dataReader entity;

    entity = u_dataReader(cmx_entityUserEntity(dataReader));

    if(entity != NULL){
        ur = u_dataReaderWaitForHistoricalData(entity, timeout);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else if(ur == U_RESULT_TIMEOUT){
            result = CMX_RESULT_TIMEOUT;
        } else {
            result = CMX_RESULT_FAILED;
        }
    } else {
        result = CMX_RESULT_ENTITY_NOT_AVAILABLE;
    }
    return result;
}
