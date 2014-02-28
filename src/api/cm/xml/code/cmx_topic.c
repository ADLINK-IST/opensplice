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
#include "cmx_topic.h"
#include "cmx__topic.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "cmx__qos.h"
#include "sd_serializer.h"
#include "sd_serializerXMLMetadata.h"
#include "u_entity.h"
#include "u_topic.h"
#include "u_participant.h"
#include "v_topic.h"
#include "v_kernel.h"
#include "v_topicQos.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

struct cmx_topicQos{
    const c_char* topicName;
    v_topicQos qos;
    v_kernel kernel;
};

c_char*
cmx_topicNew(
    const c_char* participant,
    const c_char* name,
    const c_char* typeName,
    const c_char* keyList,
    const c_char* qos)
{
    u_participant par;
    u_topic top;
    c_char* result;
    u_result ur;
    v_topicQos tQos;
    struct cmx_topicQos tq;

    result = NULL;    
    par = u_participant(cmx_entityUserEntity(participant));
    
    if(par != NULL){
        tq.qos = NULL;
        tq.topicName = name;
        ur = u_entityAction(u_entity(par), cmx_topicQosAction, &tq);
        
        if(ur == U_RESULT_OK){
            if((tq.qos == NULL) && (qos != NULL)){
                tQos = v_topicQos(cmx_qosKernelQosFromKind(qos, K_TOPIC, c_getBase(c_object(tq.kernel))));
                
                if(tQos == NULL){
                    tQos = v_topicQosNew(tq.kernel, NULL);
                }
                top = u_topicNew(par, name, typeName, keyList, tQos);
                c_free(tQos);
            } else {
                tQos = tq.qos;
                top = u_topicNew(par, name, typeName, keyList, tQos);
            }
            
            if(top != NULL){
                C_STRUCT(cmx_entityArg) arg;

                cmx_registerEntity(u_entity(top));
                arg.entity = u_entity(top);
                arg.create = FALSE;
                arg.participant = NULL;
                arg.result = NULL;
                ur = u_entityAction(u_entity(top), cmx_entityNewFromAction, &arg);
                
                if(ur == U_RESULT_OK){
                    result = arg.result; /* Transfer string content to return result. */
                }
            }
        }
    }
    return result;
}

c_char*
cmx_topicInit(
    v_topic entity)
{

    os_uint32 metaLength = 60; /* length of the base string */
    c_char* buf;
    c_char* metaName;
    c_char* keyExpr;
    
    metaName = c_metaScopedName((c_metaObject)(v_topicDataType(entity)));
    assert(metaName);
    keyExpr = v_topicMessageKeyExpr(entity);
    

    if(keyExpr){
        buf = (char *)os_malloc(strlen(metaName)+strlen(keyExpr)+metaLength+1);
        os_sprintf(buf, 
            "<keyList>%s</keyList><typename>%s</typename><kind>TOPIC</kind>",
            keyExpr,
            metaName);
        os_free(keyExpr);
    } else {
        buf = (char *)os_malloc(strlen(metaName)+metaLength+1);
        os_sprintf(buf, 
            "<keyList></keyList><typename>%s</typename><kind>TOPIC</kind>", 
            metaName);
    }
    os_free(metaName);
    
    return buf;
}

struct cmx_topicArg {
    c_char* result;
};

c_char*
cmx_topicDataType(
    const c_char* topic)
{
    u_entity entity;
    c_bool result;
    c_char* type;
    struct cmx_topicArg arg;
    
    type = NULL;
    entity = cmx_entityUserEntity(topic);
    arg.result = NULL;
    
    if(entity != NULL){
        result = u_entityAction(entity, cmx_topicDataTypeAction, &arg);
    }
    return arg.result;
}

void
cmx_topicDataTypeAction(
    v_entity entity,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    c_type type;
    struct cmx_topicArg *arg;
    arg = (struct cmx_topicArg *)args;
    
    if(v_object(entity)->kind == K_TOPIC){
        type = v_topicDataType(entity);
        ser = sd_serializerXMLMetadataNew(c_getBase(type));
        data = sd_serializerSerialize(ser, type);
        arg->result = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
    }
}

void
cmx_topicQosAction(
    v_entity entity,
    c_voidp args)
{
    struct cmx_topicQos* tq;
    c_iter topics;
    v_topic topic;
    
    tq = (struct cmx_topicQos*)args;
    topics = v_resolveTopics(v_objectKernel(entity), tq->topicName);
    
    if(topics != NULL){
        if(c_iterLength(topics) > 0){
            topic = v_topic(c_iterTakeFirst(topics));
            tq->qos = v_topicQosRef(topic);
            
            while(topic != NULL){
                c_free(topic);
                topic = v_topic(c_iterTakeFirst(topics));
            }
        }
        c_iterFree(topics);
    }
    tq->kernel = v_objectKernel(entity);
}
