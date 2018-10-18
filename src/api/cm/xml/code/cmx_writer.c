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
#include "cmx__writer.h"
#include "cmx_writer.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "sd_serializerXML.h"
#include "sd_serializerXMLMetadata.h"
#include "u_publisher.h"
#include "u_topic.h"
#include "u_writer.h"
#include "u_entity.h"
#include "u_observable.h"
#include "v_kernel.h"
#include "v_writer.h"
#include "v_writerQos.h"
#include "v_topic.h"
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
    u_result ur;
    cmx_entity pce, tce;
    c_char* result;
    c_char* context;

    top = NULL;
    wri = NULL;
    ur = U_RESULT_UNDEFINED;
    result = NULL;

    pce = cmx_entityClaim(publisher);
    if(pce != NULL){
        pub = u_publisher(pce->uentity);
        if(pub != NULL){
            tce = cmx_entityClaim(topic);
            if(tce != NULL){
                top = u_topic(tce->uentity);
                if(top != NULL){
                    wri = u_writerNew(pub, name, top, NULL);
                }
                cmx_entityRelease(tce);
            }
        }
        if(wri != NULL){
            ur = U_RESULT_OK;
            if ((qos != NULL) && (strlen(qos) > 0)) {
                ur = u_entitySetXMLQos(u_entity(wri), qos);
                context = "u_entitySetXMLQos";
            }
            if(ur == U_RESULT_OK){
                ur = u_entityEnable(u_entity(wri));
                context = "u_entityEnable";
            }
            if(ur == U_RESULT_OK){
                ur = cmx_entityRegister(u_object(wri), pce->participant, &result);
                context = "cmx_entityRegister";
            }
            if (ur != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                            "cmx_writerNew failed (reason: %s returned %u).",
                            context, ur);
                u_objectFree(u_object(wri));
            }
        }
        cmx_entityRelease(pce);
    }
    return result;
}

c_char*
cmx_writerInit(
    v_writer entity)
{
    assert(C_TYPECHECK(entity, v_writer));
    OS_UNUSED_ARG(entity);

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
    cmx_entity ce;
    struct cmx_writerTypeArg arg;

    arg.result = NULL;
    ce = cmx_entityClaim(writer);

    if(ce != NULL){
        if (u_observableAction(u_observable(ce->uentity), cmx_writerDataTypeAction, &arg) != U_RESULT_OK){
            arg.success = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    }
    return arg.result;
}

void
cmx_writerDataTypeAction(
    v_public entity,
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
    cmx_entity ce;
    struct cmx_writerArg arg;

    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    ce = cmx_entityClaim(writer);

    if(ce != NULL){
        arg.result = data;

        if (u_observableWriteAction(u_observable(ce->uentity), cmx_writerCopy, &arg) != U_RESULT_OK){
            arg.success = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    }
    return arg.success;
}

void
cmx_writerCopy(
    v_public public,
    c_voidp args)
{
    v_entity entity;
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_writerArg *arg;

    entity = v_entity(public);
    arg = (struct cmx_writerArg *)args;

    kw = v_writer(entity);
    message = v_topicMessageNew_s(kw->topic);
    if (message) {
        to = (void *) (message + 1);

        ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
        data = sd_serializerFromString(ser, arg->result);
        if (!sd_serializerDeserializeInto(ser, data, to)) {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Write of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));
            arg->success = CMX_RESULT_FAILED;
        } else {
            arg->success = CMX_RESULT_OK;
            if (v_writerWrite(kw,message,os_timeWGet(),NULL) != V_WRITE_SUCCESS) {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "Write of userdata failed.\nReason: write failed\n");
                arg->success = CMX_RESULT_FAILED;                
            }
        }
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(message);
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Write of userdata failed.\nReason: Out of resources\n");
        arg->success = CMX_RESULT_FAILED;
    }
}

const c_char*
cmx_writerDispose(
    const c_char* writer,
    const c_char* data)
{
    cmx_entity ce;
    struct cmx_writerArg arg;

    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    ce = cmx_entityClaim(writer);

    if(ce != NULL){
        arg.result = data;

        if (u_observableWriteAction(u_observable(ce->uentity), cmx_writerDisposeCopy, &arg) != U_RESULT_OK){
            arg.success = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    }
    return arg.success;
}

void
cmx_writerDisposeCopy(
    v_public public,
    c_voidp args)
{
    v_entity entity;
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_writerArg *arg;

    entity = v_entity(public);
    arg = (struct cmx_writerArg *)args;

    kw = v_writer(entity);
    message = v_topicMessageNew_s(kw->topic);
    if (message) {
        to = (void *) (message + 1);

        ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
        data = sd_serializerFromString(ser, arg->result);
        if (!sd_serializerDeserializeInto(ser, data, to)) {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Write of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));
            arg->success = CMX_RESULT_FAILED;
        } else {
            arg->success = CMX_RESULT_OK;
            v_writerDispose(kw,message,os_timeWGet(),NULL);
        }
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(message);
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Write of userdata failed.\nReason: Out of resources\n");
        arg->success = CMX_RESULT_FAILED;
    }
}

const c_char*
cmx_writerWriteDispose(
    const c_char* writer,
    const c_char* data)
{
    cmx_entity ce;
    struct cmx_writerArg arg;

    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    ce = cmx_entityClaim(writer);

    if(ce != NULL){
        arg.result = data;

        if (u_observableWriteAction(u_observable(ce->uentity), cmx_writerWriteDisposeCopy, &arg) != U_RESULT_OK){
            arg.success = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    }
    return arg.success;
}

void
cmx_writerWriteDisposeCopy(
    v_public public,
    c_voidp args)
{
    v_entity entity;
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_writerArg *arg;

    entity = v_entity(public);
    arg = (struct cmx_writerArg *)args;

    kw = v_writer(entity);
    message = v_topicMessageNew_s(kw->topic);
    if (message) {
        to = (void *) (message + 1);

        ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
        data = sd_serializerFromString(ser, arg->result);
        if (!sd_serializerDeserializeInto(ser, data, to)) {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Write of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));
            arg->success = CMX_RESULT_FAILED;
        } else {
            arg->success = CMX_RESULT_OK;
            v_writerWriteDispose(kw,message,os_timeWGet(),NULL);
        }
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(message);
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Write of userdata failed.\nReason: Out of resources\n");
        arg->success = CMX_RESULT_FAILED;
    }
}

const c_char*
cmx_writerRegister(
    const c_char* writer,
    const c_char* data)
{
    cmx_entity ce;
    struct cmx_writerArg arg;

    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    ce = cmx_entityClaim(writer);

    if(ce != NULL){
        arg.result = data;

        if (u_observableWriteAction(u_observable(ce->uentity), cmx_writerRegisterCopy, &arg) != U_RESULT_OK){
            arg.success = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    }
    return arg.success;
}

void
cmx_writerRegisterCopy(
    v_public public,
    c_voidp args)
{
    v_entity entity;
    v_writer kw;
    v_message message;
    v_writerInstance instance;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_writerArg *arg;

    entity = v_entity(public);
    arg = (struct cmx_writerArg *)args;

    kw = v_writer(entity);
    message = v_topicMessageNew_s(kw->topic);
    if (message) {
        to = (void *) (message + 1);

        ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
        data = sd_serializerFromString(ser, arg->result);
        if (!sd_serializerDeserializeInto(ser, data, to)) {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Register of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));
            arg->success = CMX_RESULT_FAILED;
        } else {
            arg->success = CMX_RESULT_OK;
            v_writerRegister(kw,message,os_timeWGet(),&instance);
            c_free(instance);
        }
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(message);
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Write of userdata failed.\nReason: Out of resources\n");
        arg->success = CMX_RESULT_FAILED;
    }
}

const c_char*
cmx_writerUnregister(
    const c_char* writer,
    const c_char* data)
{
    cmx_entity ce;
    struct cmx_writerArg arg;

    arg.success = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    ce = cmx_entityClaim(writer);

    if(ce != NULL){
        arg.result = data;

        if (u_observableWriteAction(u_observable(ce->uentity), cmx_writerUnregisterCopy, &arg) != U_RESULT_OK){
            arg.success = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    }
    return arg.success;
}

void
cmx_writerUnregisterCopy(
    v_public public,
    c_voidp args)
{
    v_entity entity;
    v_writer kw;
    v_message message;
    void *to;
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_writerArg *arg;

    entity = v_entity(public);
    arg = (struct cmx_writerArg *)args;

    kw = v_writer(entity);
    message = v_topicMessageNew_s(kw->topic);
    if (message) {
        to = (void *) (message + 1);

        ser = sd_serializerXMLNewTyped(v_topicDataType(kw->topic));
        data = sd_serializerFromString(ser, arg->result);
        if (!sd_serializerDeserializeInto(ser, data, to)) {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Unregister of userdata failed.\nReason: %s\nError: %s\n",
                    sd_serializerLastValidationMessage(ser),
                    sd_serializerLastValidationLocation(ser));
            arg->success = CMX_RESULT_FAILED;
        } else {
            arg->success = CMX_RESULT_OK;
            v_writerUnregister(kw,message,os_timeWGet(),NULL);
        }
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(message);
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Write of userdata failed.\nReason: Out of resources\n");
        arg->success = CMX_RESULT_FAILED;
    }
}
