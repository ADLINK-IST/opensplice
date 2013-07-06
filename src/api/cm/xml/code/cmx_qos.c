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

#include "cmx__qos.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "os_report.h"
#include "sd_serializerXML.h"
#include "c_base.h"

void
cmx_qosNew(
    v_entity e,
    c_char **xmlQos)
{
    sd_serializer ser;
    sd_serializedData data;
    v_qos qos;

    assert(xmlQos != NULL);

    *xmlQos = NULL;

    qos = v_entityGetQos(e);
    if (qos != NULL) {
        ser = sd_serializerXMLNewTyped(c_getType(qos));
        data = sd_serializerSerialize(ser, qos);
        *xmlQos = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
        c_free(qos);
    }
}

v_qos
cmx_qosKernelQos(
    const c_char* entity,
    const c_char* qos)
{
    v_qos vqos;
    u_entity ue;
    struct cmx_qosArg args;
    u_result ur;

    vqos = NULL;

    if( (qos != NULL) && (entity != NULL) ){
        ue = cmx_entityUserEntity(entity);

        if(ue != NULL){
            ur = u_entityAction(ue, cmx_qosAction, &args);

            if(ur == U_RESULT_OK) {
                vqos = cmx_qosKernelQosFromKind(qos, args.kind, args.base);
            }
        }
    }
    return vqos;
}

v_qos
cmx_qosKernelQosFromKind(
    const c_char* qos,
    v_kind entityKind,
    c_base base)
{
    c_type qosType;
    sd_serializer ser;
    sd_serializedData data;
    sd_validationResult valResult;
    v_qos vqos;

    vqos = NULL;
    qosType = NULL;

    if(qos == NULL){}
    else if(strcmp(qos, "") == 0){}
    else{
        switch(entityKind){
            case K_NETWORKING:
            /*fallthrough intentionally.*/
            case K_DURABILITY:
            /*fallthrough intentionally.*/
            case K_CMSOAP:
            /*fallthrough intentionally.*/
            case K_RNR:
            /*fallthrough intentionally.*/
            case K_SPLICED:
            /*fallthrough intentionally.*/
            case K_SERVICE:
            /*fallthrough intentionally.*/
            case K_PARTICIPANT:
                qosType = c_resolve(base, "kernelModule::v_participantQos");
            break;
            case K_PUBLISHER:
                qosType = c_resolve(base, "kernelModule::v_publisherQos");
            break;
            case K_SUBSCRIBER:
                qosType = c_resolve(base, "kernelModule::v_subscriberQos");
            break;
            case K_WRITER:
                qosType = c_resolve(base, "kernelModule::v_writerQos");
            break;
            case K_DATAREADER:
                qosType = c_resolve(base, "kernelModule::v_readerQos");
            break;
            case K_TOPIC:
                qosType = c_resolve(base, "kernelModule::v_topicQos");
            break;
            case K_DOMAIN:
                qosType = c_resolve(base, "kernelModule::v_partitionQos");
            break;
            default:
            break;
        }

        if(qosType != NULL){
            ser = sd_serializerXMLNewTyped(qosType);
            data = sd_serializerFromString(ser, qos);
            vqos = (v_qos)(sd_serializerDeserializeValidated(ser, data));
            valResult = sd_serializerLastValidationResult(ser);

            if(valResult != SD_VAL_SUCCESS){
                OS_REPORT_2(OS_ERROR, CM_XML_CONTEXT, 0,
                            "Creation of qos failed.\nReason: %s\nError: %s\n",
                            sd_serializerLastValidationMessage(ser),
                            sd_serializerLastValidationLocation(ser));
            }
            sd_serializedDataFree(data);
            sd_serializerFree(ser);
            c_free(qosType);
        }
    }
    return vqos;
}

void
cmx_qosAction(
    v_entity entity,
    c_voidp args)
{
    struct cmx_qosArg* arg;

    arg = (struct cmx_qosArg*)args;
    arg->kind = v_object(entity)->kind;
    arg->base = c_getBase(c_object(entity));
}
