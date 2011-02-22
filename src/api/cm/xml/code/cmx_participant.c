/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "cmx__participant.h"
#include "cmx_participant.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "cmx_factory.h"
#include "cmx__qos.h"
#include "u_participant.h"
#include "u_entity.h"
#include "v_participant.h"
#include "v_serviceState.h"
#include "v_observable.h"
#include "v_observer.h"
#include "v__serviceManager.h"
#include "v_event.h"
#include "v_participantQos.h"
#include "u_entity.h"
#include "sd_serializerXMLMetadata.h"
#include "os.h"
#include <stdio.h>

c_char*
cmx_participantNew(
    const c_char* uri,
    c_long timeout,
    const c_char* name,
    const c_char* qos)
{
    u_participant p;
    cmx_entityArg arg;
    c_char* result;
    u_result ur;
    v_participantQos pqos;
    cmx_entityKernelArg kernelArg;

    p = u_participantNew(uri, timeout, name, NULL, TRUE);
    result = NULL;

    if(p != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        u_entityAction(u_entity(p), cmx_entityKernelAction, (c_voidp)kernelArg);
        ur = U_RESULT_OK;

        if(qos != NULL){
            pqos = v_participantQos(cmx_qosKernelQosFromKind(qos, K_PARTICIPANT, c_getBase(c_object(kernelArg->kernel))));
            ur = u_entitySetQoS(u_entity(p), (v_qos)pqos);
            c_free(pqos);
            os_free(kernelArg);
        }

        if(ur == U_RESULT_OK){
            cmx_registerEntity(u_entity(p));
            arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
            arg->entity = u_entity(p);
            arg->participant = NULL;
            arg->create = FALSE;
            arg->result = NULL;

            ur = u_entityAction(u_entity(p), cmx_entityNewFromAction, (c_voidp)arg);

            if(ur == U_RESULT_OK){
                result = arg->result;
                os_free(arg);
            }
        } else {
            OS_REPORT_1(OS_ERROR, CM_XML_CONTEXT, 0, "Could not set supplied qos to newly created participant (reason: %u).", ur);
            u_participantFree(p);
        }
    }
    return result;
}

c_char*
cmx_participantInit(
    v_participant entity)
{
    char buf[512];
    v_participant participant;

    participant = v_participant(entity);
    os_sprintf(buf, "<kind>PARTICIPANT</kind>");

    return (c_char*)(os_strdup(buf));
}

c_char*
cmx_participantAllParticipants(
    const c_char* participant)
{
    cmx_walkEntityArg arg;
    u_participant p;
    c_char* result;

    result = NULL;
    p = u_participant(cmx_entityUserEntity(participant));

    if(p != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        arg->length = 0;
        arg->list = NULL;

        arg->entityArg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->entityArg->participant = u_entityParticipant(u_entity(p));
        arg->entityArg->create = TRUE;
        arg->entityArg->result = NULL;

        u_entityAction(u_entity(p), cmx_participantParticipantsAction, (c_voidp)arg);
        result = cmx_convertToXMLList(arg->list, arg->length);

        os_free(arg->entityArg);
        os_free(arg);
    }
    return result;
}

void
cmx_participantParticipantsAction(
    v_entity e,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter participants;
    v_entity participant;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    participants = v_resolveParticipants(v_objectKernel(e), "*");
    participant = v_entity(c_iterTakeFirst(participants));

    while(participant != NULL){
        proceed = cmx_entityNewFromWalk(participant, arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg->result;
            arg->list = c_iterInsert(arg->list, xmlEntity);
            arg->length += strlen(xmlEntity);
        }
        c_free(participant);
        participant = v_entity(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
}
c_char*
cmx_participantAllTopics(
    const c_char* participant)
{
    cmx_walkEntityArg arg;
    u_participant p;
    c_char* result;

    result = NULL;
    p = u_participant(cmx_entityUserEntity(participant));

    if(p != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        arg->length = 0;
        arg->list = NULL;

        arg->entityArg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->entityArg->participant = u_entityParticipant(u_entity(p));
        arg->entityArg->create = TRUE;
        arg->entityArg->result = NULL;

        u_entityAction(u_entity(p), cmx_participantTopicsAction, (c_voidp)arg);
        result = cmx_convertToXMLList(arg->list, arg->length);

        os_free(arg->entityArg);
        os_free(arg);
    }
    return result;
}

void
cmx_participantTopicsAction(
    v_entity e,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter topics;
    v_entity topic;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    topics = v_resolveTopics(v_objectKernel(e), "*");
    topic = v_entity(c_iterTakeFirst(topics));

    while(topic != NULL){
        proceed = cmx_entityNewFromWalk(topic, arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg->result;
            arg->list = c_iterInsert(arg->list, xmlEntity);
            arg->length += strlen(xmlEntity);
        }
        c_free(topic);
        topic = v_entity(c_iterTakeFirst(topics));
    }
    c_iterFree(topics);
}

c_char*
cmx_participantAllDomains(
    const c_char* participant)
{
    cmx_walkEntityArg arg;
    u_participant p;
    c_char* result;

    result = NULL;
    p = u_participant(cmx_entityUserEntity(participant));

    if(p != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        arg->length = 0;
        arg->list = NULL;

        arg->entityArg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->entityArg->participant = u_entityParticipant(u_entity(p));
        arg->entityArg->create = TRUE;
        arg->entityArg->result = NULL;

        u_entityAction(u_entity(p), cmx_participantDomainsAction, (c_voidp)arg);
        result = cmx_convertToXMLList(arg->list, arg->length);

        os_free(arg->entityArg);
        os_free(arg);
    }
    return result;
}

void
cmx_participantDomainsAction(
    v_entity e,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter partitions;
    v_entity partition;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    partitions = v_resolvePartitions(v_objectKernel(e), "*");
    partition = v_entity(c_iterTakeFirst(partitions));

    while(partition != NULL){
        proceed = cmx_entityNewFromWalk(partition, arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg->result;
            arg->list = c_iterInsert(arg->list, xmlEntity);
            arg->length += strlen(xmlEntity);
        }
        c_free(partition);
        partition = v_entity(c_iterTakeFirst(partitions));
    }
    c_iterFree(partitions);
}

const c_char*
cmx_participantRegisterType(
    const c_char* participant,
    const c_char* type)
{
    sd_serializer serializer;
    sd_serializedData meta_data;
    u_entity ue;
    cmx_entityKernelArg kernelArg;
    c_type topicType;
    const c_char* result;
    const c_char* msg;

    ue = cmx_entityUserEntity(participant);

    if(ue != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        u_entityAction(ue, cmx_entityKernelAction, (c_voidp)kernelArg);
        serializer = sd_serializerXMLMetadataNew(c_getBase(c_object(kernelArg->kernel)));
        os_free(kernelArg);

        if(serializer != NULL){
            meta_data = sd_serializerFromString(serializer, type);

            if (meta_data != NULL) {
                topicType = c_type(sd_serializerDeserializeValidated(serializer, meta_data));

                if (topicType == NULL) {
                    if (sd_serializerLastValidationResult(serializer) == SD_VAL_ERROR) {
                        msg = sd_serializerLastValidationMessage(serializer);
                        OS_REPORT_1(OS_ERROR,
                                    CM_XML_CONTEXT, 0,
                                    "Data type could not be registered, "
                                    "because it is not valid: %s",
                                    msg);
                        result = CMX_RESULT_FAILED;
                    } else {
                        result = CMX_RESULT_OK;
                    }
                } else {
                    result = CMX_RESULT_OK;
                }
                sd_serializedDataFree(meta_data);
            } else {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0, "Construction of serialized data failed.");
                result = CMX_RESULT_FAILED;
            }
            sd_serializerFree(serializer);
        } else {
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0, "Serializer could not be initialized");
            result = CMX_RESULT_FAILED;
        }
    } else {
        result = CMX_RESULT_FAILED;
    }
    return result;
}

c_char*
cmx_participantFindTopic(
    const c_char* participant,
    const c_char* topicName)
{
    u_participant up;
    c_char* topics;
    cmx_walkEntityArg arg;

    topics = NULL;
    up = u_participant(cmx_entityUserEntity(participant));

    if(up != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkParticipantArg)));
        arg->length = 0;
        arg->list = NULL;

        arg->entityArg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->entityArg->participant = u_entityParticipant(u_entity(up));
        arg->entityArg->create = TRUE;
        arg->entityArg->result = NULL;
        cmx_walkParticipantArg(arg)->topicName = topicName;

        u_entityAction(u_entity(up), cmx_participantFindTopicAction, (c_voidp)arg);
        topics = cmx_convertToXMLList(arg->list, arg->length);

        os_free(arg->entityArg);
        os_free(arg);
    }
    return topics;
}

void
cmx_participantFindTopicAction(
    v_entity e,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter topics;
    v_entity topic;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    topics = v_resolveTopics(v_objectKernel(e), cmx_walkParticipantArg(arg)->topicName);
    topic = v_entity(c_iterTakeFirst(topics));

    while(topic != NULL){
        proceed = cmx_entityNewFromWalk(topic, arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg->result;
            arg->list = c_iterInsert(arg->list, xmlEntity);
            arg->length += strlen(xmlEntity);
        }
        c_free(topic);
        topic = v_entity(c_iterTakeFirst(topics));
    }
    c_iterFree(topics);
}

static void
cmx_participantInitDetach(
    v_entity entity,
    c_voidp args)
{
    v_kernel k;
    v_serviceManager m;
    v_serviceState splicedState;

    k = v_objectKernel(entity);
    m = v_getServiceManager(k);
    splicedState = v_serviceManagerGetServiceState(m, V_SPLICED_NAME);
    v_observableAddObserver(v_observable(splicedState), v_observer(entity), NULL);
}

static c_ulong
cmx_participantDetach(
    u_dispatcher o,
    c_ulong event,
    c_voidp usrData)
{
    v_serviceStateKind kind;
    u_serviceManager manager;

    if(cmx_isInitialized() == TRUE){
        manager = (u_serviceManager)usrData;

        if(manager != NULL){
            kind = u_serviceManagerGetServiceStateKind(manager, V_SPLICED_NAME);

            if ((kind != STATE_INITIALISING) && (kind != STATE_OPERATIONAL)) {
                cmx_internalDetach();
                u_serviceManagerFree(manager);
                manager = NULL;
            }
        }
    }
    return V_EVENT_SERVICESTATE_CHANGED;
}

const c_char*
cmx_participantAutoDetach(
    const c_char* participant,
    c_bool enable)
{
    c_ulong mask;
    u_participant up;
    const c_char* r;

    r = CMX_RESULT_FAILED;
    up = u_participant(cmx_entityUserEntity(participant));

    if (up != NULL) {
        u_dispatcherGetEventMask(u_dispatcher(up), &mask);

        if (enable == FALSE) {
            mask &= ~V_EVENT_SERVICESTATE_CHANGED;
            u_dispatcherRemoveListener(u_dispatcher(up), cmx_participantDetach);
        } else {
            mask |= V_EVENT_SERVICESTATE_CHANGED;
            u_entityAction(u_entity(up), cmx_participantInitDetach, NULL);
            u_dispatcherInsertListener(u_dispatcher(up), cmx_participantDetach, u_serviceManagerNew(up));
        }
        u_dispatcherSetEventMask(u_dispatcher(up), mask);
        r = CMX_RESULT_OK;
    }
    return r;
}
