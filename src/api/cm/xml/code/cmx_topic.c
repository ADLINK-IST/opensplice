/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "cmx_topic.h"
#include "cmx__topic.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "sd_serializer.h"
#include "sd_serializerXML.h"
#include "sd_serializerXMLMetadata.h"
#include "u_observable.h"
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
    const c_char* qosXml;
    v_topicQos qos;
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
    u_result ur;
    struct cmx_topicQos tq;
    cmx_entity ce;
    c_char* result;
    c_char* context;

    top = NULL;
    ur = U_RESULT_UNDEFINED;
    result = NULL;
    tq.qos = NULL;
    tq.qosXml = qos;
    tq.topicName = name;

    result = NULL;
    ce = cmx_entityClaim(participant);
    if(ce != NULL){
        par = u_participant(ce->uentity);
        /*
         * The Topic is created rather differently than all the other entities.
         * There are two reasons for that:
         *      - A Topic is always created enabled.
         *      - A Topic can already be available within the system.
         *
         * The first reason doesn't allow us to do the normal entity creation
         * sequence, which is f.i.:
         *      - u_subscriberNew(...);
         *      - u_entitySetXMLQos(sub, qos);
         *      - u_entityEnable(sub);
         * Because the QoS is set after the creation, we couldn't set QoSses with
         * immutable policies other than the default for Topic.
         *
         * The second reason makes sure that we have to check at the kernel to
         * see if the given Topic is already available and extract its QoS from
         * the kernel to be able to create the Topic without a QoS clash.
         *
         * So, before we can create a Topic, we have to either translate or
         * extract the QoS and then pass the found QoS into the creation.
         *
         * Translation has the higher priority compared to the extraction.
         *
         * If there is nothing to translate or extract, then the Topic will be
         * created with a NULL QoS, which means the default.
         *
         * Function cmx_topicQosAction() takes care of the actual translation
         * or extraction. Result will be in tq.qos.
         */
        ur = u_observableAction(u_observable(par), cmx_topicQosAction, &tq);
        if (ur == U_RESULT_OK) {
            top = u_topicNew(par, name, typeName, keyList, tq.qos);
        }
        if (top) {
            ur = u_entityEnable(u_entity(top));
            context = "u_entityEnable";
            if (ur == U_RESULT_OK) {
                ur = cmx_entityRegister(u_object(top), ce, &result);
                context = "cmx_entityRegister";
            }
            if (ur != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                            "cmx_topicNew failed (reason: %s returned %u).",
                            context, ur);
                u_objectFree(u_object(top));
            }
        }
        cmx_entityRelease(ce);
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
    cmx_entity ce;
    struct cmx_topicArg arg;

    arg.result = NULL;

    ce = cmx_entityClaim(topic);

    if(ce != NULL){
        (void)u_observableAction(u_observable(ce->uentity), cmx_topicDataTypeAction, &arg);
        cmx_entityRelease(ce);
    }
    return arg.result;
}

void
cmx_topicDataTypeAction(
    v_public entity,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    c_type type;
    struct cmx_topicArg *arg;
    arg = (struct cmx_topicArg *)args;

    if((v_object(entity)->kind == K_TOPIC) || (v_object(entity)->kind == K_TOPIC_ADAPTER)){
        type = v_topicDataType(v_topic(entity));
        ser = sd_serializerXMLMetadataNew(c_getBase(type));
        data = sd_serializerSerialize(ser, type);
        arg->result = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
    }
}

static void
cmx_topicQosActionTranslate(
    v_kernel kernel,
    struct cmx_topicQos* tq)
{
    c_base base;
    c_type qosType;
    sd_serializer ser;
    sd_serializedData data;

    if(strcmp(tq->qosXml, "") != 0) {
        base = c_getBase(c_object(kernel));
        qosType = c_resolve(base, "kernelModuleI::v_topicQos");
        assert(qosType);
        ser = sd_serializerXMLNewTyped(qosType);
        data = sd_serializerFromString(ser, tq->qosXml);
        tq->qos = (v_topicQos)(sd_serializerDeserialize(ser, data));
        if(tq->qos == NULL){
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                        "Creation of Topic qos failed.\nReason: %s\nError: %s\n",
                        sd_serializerLastValidationMessage(ser),
                        sd_serializerLastValidationLocation(ser));
        }
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(qosType);
    }
}

static void
cmx_topicQosActionExtract(
    v_kernel kernel,
    struct cmx_topicQos* tq)
{
    c_iter topics;
    v_topic topic;

    topics = v_resolveTopics(kernel, tq->topicName);
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
}

void
cmx_topicQosAction(
    v_public entity,
    c_voidp args)
{
    struct cmx_topicQos* tq;
    v_kernel kernel;

    tq = (struct cmx_topicQos*)args;
    kernel = v_objectKernel(entity);

    assert(tq);
    assert(kernel);

    if ((tq->qosXml != NULL) && (strlen(tq->qosXml) > 0)) {
        /* We were provided with an XML QoS: translate it. */
        cmx_topicQosActionTranslate(kernel, tq);
    } else {
        /* We were not provided with an XML QoS: try to extract a QoS
         * from the kernel if this topic already exists. */
        cmx_topicQosActionExtract(kernel, tq);
    }
}

