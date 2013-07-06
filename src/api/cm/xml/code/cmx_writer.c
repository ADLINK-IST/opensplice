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
#include "cmx__writer.h"
#include "cmx_writer.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "cmx__qos.h"
#include "sd_serializerXML.h"
#include "sd_serializerXMLMetadata.h"
#include "u_publisher.h"
#include "u_topic.h"
#include "u_writer.h"
#include "u_entity.h"
#include "v_kernel.h"
#include "v_writer.h"
#include "v_writerQos.h"
#include "v_topic.h"
#include "v_time.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_writerNew(
    const c_char* publisher,
    const c_char* name,
    const c_char* topic,
    const c_char* qos)
{
    u_publisher pub;
    u_topic top;
    u_writer wri;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;
    cmx_entityKernelArg kernelArg;
    v_writerQos wQos;
    
    result = NULL;
    pub = u_publisher(cmx_entityUserEntity(publisher));
    
    if(pub != NULL){
        top = u_topic(cmx_entityUserEntity(topic));
        
        if(top != NULL){
            kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
            u_entityAction(u_entity(pub), cmx_entityKernelAction, (c_voidp)kernelArg);
            
            if(qos != NULL){
                wQos = v_writerQos(cmx_qosKernelQosFromKind(qos, K_WRITER, c_getBase(c_object(kernelArg->kernel))));
                
                if(wQos == NULL){
                    wQos = v_writerQosNew(kernelArg->kernel, NULL);
                    wQos->reliability.kind = V_RELIABILITY_RELIABLE;
                }
            } else {
                wQos = v_writerQosNew(kernelArg->kernel, NULL);
                wQos->reliability.kind = V_RELIABILITY_RELIABLE;
            }
            wri = u_writerNew(pub, name, top, NULL, wQos, TRUE);
            os_free(kernelArg);
            c_free(wQos);
            
            if(wri != NULL){
                cmx_registerEntity(u_entity(wri));
                arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
                arg->entity = u_entity(wri);
                arg->create = FALSE;
                arg->participant = NULL;
                arg->result = NULL;
                ur = u_entityAction(u_entity(wri), cmx_entityNewFromAction, (c_voidp)(arg));
                
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
cmx_writerInit(
    v_writer entity)
{
    assert(C_TYPECHECK(entity, v_writer));

    return (c_char*)(os_strdup("<kind>WRITER</kind>"));
}

struct cmx_writerTypeArg {
    c_char* result;
    const c_char* success;
};

c_char*
cmx_writerDataType(
    const c_char* writer)
{
    u_entity entity;
    struct cmx_writerTypeArg arg;
    
    entity = cmx_entityUserEntity(writer);
    arg.result = NULL;
    
    if(entity != NULL){
        u_entityAction(entity, cmx_writerDataTypeAction, &arg);
    }
    return arg.result;
}

void
cmx_writerDataTypeAction(
    v_entity entity,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    c_type type;
    struct cmx_writerTypeArg *arg;
    arg = (struct cmx_writerTypeArg *)args;
    
    type = NULL;
    
    switch(v_object(entity)->kind){
    case K_WRITER:
        type = v_topicDataType(v_writer(entity)->topic);      
    break;
    default:
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0, "Trying to resolve dataType of writer that is not a writer.\n");
        assert(FALSE);
    break;
    }
    
    if(type != NULL){
        ser = sd_serializerXMLMetadataNew(c_getBase(type));
        data = sd_serializerSerialize(ser, type);
        arg->result = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
    }
}

struct cmx_writerArg {
    const c_char* result;
    const c_char* success;
};

const c_char*
cmx_writerWrite(
    const c_char* writer, 
    const c_char* data)
{
    u_entity entity;
    struct cmx_writerArg arg;
    
    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    entity = cmx_entityUserEntity(writer);

    if(entity != NULL){
        arg.result = data;

        u_entityWriteAction(entity, cmx_writerCopy, &arg);
    }
    return arg.success;
}

void
cmx_writerCopy(
    v_entity entity,
    c_voidp args)
{
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    sd_validationResult valResult;
    struct cmx_writerArg *arg;
    
    arg = (struct cmx_writerArg *)args;
    
    kw = v_writer(entity);
    message = v_topicMessageNew(kw->topic);
    to = C_DISPLACE(message,v_topicDataOffset(kw->topic));
    
    ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
    data = sd_serializerFromString(ser, arg->result);
    sd_serializerDeserializeIntoValidated(ser, data, to);
    valResult = sd_serializerLastValidationResult(ser);
        
    if(valResult != SD_VAL_SUCCESS){
        OS_REPORT_2(OS_ERROR, CM_XML_CONTEXT, 0, 
                    "Write of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));           
        arg->success = CMX_RESULT_FAILED;
    } else {
        arg->success = CMX_RESULT_OK;
    }
    sd_serializedDataFree(data);
    sd_serializerFree(ser);

    /* Note that the last param of v_writerWrite is NULL,
       performance can be improved if the instance is provided. */    
    v_writerWrite(kw,message,v_timeGet(),NULL);
    c_free(message);
}

const c_char*
cmx_writerDispose(
    const c_char* writer, 
    const c_char* data)
{
    u_entity entity;
    struct cmx_writerArg arg;
    
    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    entity = cmx_entityUserEntity(writer);

    if(entity != NULL){
        arg.result = data;

        u_entityWriteAction(entity, cmx_writerDisposeCopy, &arg);
    }
    return arg.success;
}

void
cmx_writerDisposeCopy(
    v_entity entity,
    c_voidp args)
{
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    sd_validationResult valResult;
    struct cmx_writerArg *arg;
    
    arg = (struct cmx_writerArg *)args;
    
    kw = v_writer(entity);
    message = v_topicMessageNew(kw->topic);
    to = C_DISPLACE(message,v_topicDataOffset(kw->topic));
    
    ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
    data = sd_serializerFromString(ser, arg->result);
    sd_serializerDeserializeIntoValidated(ser, data, to);
    valResult = sd_serializerLastValidationResult(ser);
        
    if(valResult != SD_VAL_SUCCESS){
        OS_REPORT_2(OS_ERROR, CM_XML_CONTEXT, 0, 
                    "Write of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));           
        arg->success = CMX_RESULT_FAILED;
    } else {
        arg->success = CMX_RESULT_OK;
    }
    sd_serializedDataFree(data);
    sd_serializerFree(ser);
    
    /* Note that the last param of v_writerDispose is NULL,
       performance can be improved if the instance is provided. */    
    v_writerDispose(kw,message,v_timeGet(),NULL);
    c_free(message);
}

const c_char*
cmx_writerWriteDispose(
    const c_char* writer, 
    const c_char* data)
{
    u_entity entity;
    struct cmx_writerArg arg;
    
    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    entity = cmx_entityUserEntity(writer);

    if(entity != NULL){
        arg.result = data;
        u_entityWriteAction(entity, cmx_writerWriteDisposeCopy, &arg);
    }
    return arg.success;
}

void
cmx_writerWriteDisposeCopy(
    v_entity entity,
    c_voidp args)
{
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    sd_validationResult valResult;
    struct cmx_writerArg *arg;
    
    arg = (struct cmx_writerArg *)args;
    
    kw = v_writer(entity);
    message = v_topicMessageNew(kw->topic);
    to = C_DISPLACE(message,v_topicDataOffset(kw->topic));
    
    ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
    data = sd_serializerFromString(ser, arg->result);
    sd_serializerDeserializeIntoValidated(ser, data, to);
    valResult = sd_serializerLastValidationResult(ser);
        
    if(valResult != SD_VAL_SUCCESS){
        OS_REPORT_2(OS_ERROR, CM_XML_CONTEXT, 0, 
                    "Write of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));           
        arg->success = CMX_RESULT_FAILED;
    } else {
        arg->success = CMX_RESULT_OK;
    }
    sd_serializedDataFree(data);
    sd_serializerFree(ser);
    
    /* Note that the last param of v_writerWriteDispose is NULL,
       performance can be improved if the instance is provided. */    
    v_writerWriteDispose(kw,message,v_timeGet(),NULL);
    c_free(message);
}

const c_char*
cmx_writerRegister(
    const c_char* writer, 
    const c_char* data)
{
    u_entity entity;
    struct cmx_writerArg arg;
    
    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    entity = cmx_entityUserEntity(writer);

    if(entity != NULL){
        arg.result = data;

        u_entityWriteAction(entity, cmx_writerRegisterCopy, &arg);
    }
    return arg.success;
}

void
cmx_writerRegisterCopy(
    v_entity entity,
    c_voidp args)
{
    v_writer kw;
    v_message message;
    v_writerInstance instance;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    sd_validationResult valResult;
    struct cmx_writerArg *arg;
    
    arg = (struct cmx_writerArg *)args;
    
    kw = v_writer(entity);
    message = v_topicMessageNew(kw->topic);
    to = C_DISPLACE(message,v_topicDataOffset(kw->topic));
    
    ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
    data = sd_serializerFromString(ser, arg->result);
    sd_serializerDeserializeIntoValidated(ser, data, to);
    valResult = sd_serializerLastValidationResult(ser);
        
    if(valResult != SD_VAL_SUCCESS){
        OS_REPORT_2(OS_ERROR, CM_XML_CONTEXT, 0, 
                    "Register of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));           
        arg->success = CMX_RESULT_FAILED;
    } else {
        arg->success = CMX_RESULT_OK;
    }
    sd_serializedDataFree(data);
    sd_serializerFree(ser);
    
    v_writerRegister(kw,message,v_timeGet(),&instance);
    c_free(message);
    c_free(instance);
}

const c_char*
cmx_writerUnregister(
    const c_char* writer, 
    const c_char* data)
{
    u_entity entity;
    struct cmx_writerArg arg;
    
    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    entity = cmx_entityUserEntity(writer);

    if(entity != NULL){
        arg.result = data;

        u_entityWriteAction(entity, cmx_writerUnregisterCopy, &arg);
    }
    return arg.success;
}

void
cmx_writerUnregisterCopy(
    v_entity entity,
    c_voidp args)
{
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    sd_validationResult valResult;
    struct cmx_writerArg *arg;
    
    arg = (struct cmx_writerArg *)args;
    
    kw = v_writer(entity);
    message = v_topicMessageNew(kw->topic);
    to = C_DISPLACE(message,v_topicDataOffset(kw->topic));
    
    ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
    data = sd_serializerFromString(ser, arg->result);
    sd_serializerDeserializeIntoValidated(ser, data, to);
    valResult = sd_serializerLastValidationResult(ser);
        
    if(valResult != SD_VAL_SUCCESS){
        OS_REPORT_2(OS_ERROR, CM_XML_CONTEXT, 0, 
                    "Unregister of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));           
        arg->success = CMX_RESULT_FAILED;
    } else {
        arg->success = CMX_RESULT_OK;
    }
    sd_serializedDataFree(data);
    sd_serializerFree(ser);
    
    /* Note that the last param of v_writerWriteDispose is NULL,
       performance can be improved if the instance is provided. */    
    v_writerUnregister(kw,message,v_timeGet(),NULL);
    c_free(message);
}
