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
#include "cmx__participant.h"
#include "cmx_participant.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "cmx_factory.h"
#include "u_serviceManager.h"
#include "u_participant.h"
#include "u_entity.h"
#include "u_observable.h"
#include "v_participant.h"
#include "v_serviceState.h"
#include "v_observable.h"
#include "v_observer.h"
#include "v__serviceManager.h"
#include "v_event.h"
#include "v_participantQos.h"
#include "sd_serializerXMLMetadata.h"
#include "vortex_os.h"
#include <stdio.h>

c_char*
cmx_participantNew(
    const c_char* uri,
    const c_char* domainId,
    c_long timeout,
    const c_char* name,
    const c_char* qos)
{
    u_participant p;
    u_result ur;
    u_domainId_t did;
    int pos;
    c_char* result;
    const c_char* context;

    if (*domainId == '\0') {
        did = U_DOMAIN_ID_ANY;
    } else if (sscanf (domainId,"%d%n", &did, &pos) != 1 || domainId[pos] != '\0') {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                   "cmx_participantNew failed (reason: illegal argument: domainId \"%s\").",
                   domainId);
        return NULL;
    }

    p = u_participantNew(uri, did, timeout > 0 ? (unsigned)timeout : 0, name, NULL, TRUE);
    if(p == NULL){
        /* Error reported by u_participantNew() */
        goto err_u_participantNew;
    }

    if(qos && *qos){
        if((ur = u_entitySetXMLQos(u_entity(p), qos)) != U_RESULT_OK) {
            context = "u_entitySetXMLQos";
            goto err_entity;
        }
    }

    if((ur = u_entityEnable(u_entity(p))) != U_RESULT_OK) {
        context = "u_entityEnable";
        goto err_entity;
    }


    if((ur = cmx_entityRegister(u_object(p), NULL, &result)) != U_RESULT_OK) {
        context = "cmx_entityRegister";
        goto err_entity;
    }
    return result;

/* Error handling */
err_entity:
    OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
            "cmx_participantNew failed (reason: %s returned %u).",
            context, ur);
    u_objectFree(u_object(p));
err_u_participantNew:
    return NULL;
}

c_char*
cmx_participantInit(
    v_participant entity)
{
    assert(C_TYPECHECK(entity, v_participant));
    OS_UNUSED_ARG(entity);

    return (c_char*)(os_strdup("<kind>PARTICIPANT</kind>"));
}

c_char*
cmx_participantAllParticipants(
    const c_char* participant)
{
    cmx_walkEntityArg arg;
    u_result ur;
    c_char* result;
    cmx_entity ce;

    result = NULL;
    ce = cmx_entityClaim(participant);

    if(ce != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        if (arg != NULL) {
            arg->length = 0;
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;

            ur = u_observableAction(u_observable(ce->uentity),
                                    cmx_participantParticipantsAction,
                                    (c_voidp)arg);
            if (ur == U_RESULT_OK) {
                result = cmx_convertToXMLList(arg->list, arg->length);
            }
            os_free(arg);
        }
        cmx_entityRelease(ce);
    }
    return result;
}

void
cmx_participantParticipantsAction(
    v_public p,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter participants;
    v_entity participant;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    participants = v_resolveParticipants(v_objectKernel(p), "*");
    participant = v_entity(c_iterTakeFirst(participants));

    while(participant != NULL){
        proceed = cmx_entityNewFromWalk(v_public(participant), &arg->entityArg);
        if(proceed == TRUE){
            xmlEntity = arg->entityArg.result;
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
    u_result ur;
    cmx_walkEntityArg arg;
    c_char* result;
    cmx_entity ce;

    result = NULL;
    ce = cmx_entityClaim(participant);

    if (ce != NULL) {
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        if (arg != NULL){
            arg->length = 0;
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;

            ur = u_observableAction(u_observable(ce->uentity),
                                    cmx_participantTopicsAction,
                                    (c_voidp)arg);
            if (ur == U_RESULT_OK) {
                result = cmx_convertToXMLList(arg->list, arg->length);
            }
            os_free(arg);
        }
        cmx_entityRelease(ce);
    }
    return result;
}

void
cmx_participantTopicsAction(
    v_public p,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter topics;
    v_entity topic;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    topics = v_resolveTopics(v_objectKernel(p), "*");
    topic = v_entity(c_iterTakeFirst(topics));

    while(topic != NULL){
        proceed = cmx_entityNewFromWalk(v_public(topic), &arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg.result;
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
    u_result ur;
    cmx_walkEntityArg arg;
    c_char* result;
    cmx_entity ce;

    result = NULL;
    ce = cmx_entityClaim(participant);

    if(ce != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        if (arg != NULL) {
            arg->length = 0;
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;

            ur = u_observableAction(u_observable(ce->uentity),
                                    cmx_participantDomainsAction,
                                    (c_voidp)arg);
            if (ur == U_RESULT_OK) {
                result = cmx_convertToXMLList(arg->list, arg->length);
            }
            os_free(arg);
        }
        cmx_entityRelease(ce);
    }
    return result;
}

void
cmx_participantDomainsAction(
    v_public p,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter partitions;
    v_entity partition;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    partitions = v_resolvePartitions(v_objectKernel(p), "*");
    partition = v_entity(c_iterTakeFirst(partitions));

    while(partition != NULL){
        proceed = cmx_entityNewFromWalk(v_public(partition), &arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg.result;
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
    cmx_entityKernelArg kernelArg;
    c_type topicType;
    cmx_entity ce;
    const c_char* result;
    const c_char* msg;

    ce = cmx_entityClaim(participant);
    if(ce != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        if (u_observableAction(u_observable(ce->uentity),
                               cmx_entityKernelAction,
                               (c_voidp)kernelArg) == U_RESULT_OK)
        {
            serializer = sd_serializerXMLMetadataNew(c_getBase(c_object(kernelArg->kernel)));

            if(serializer != NULL){
                meta_data = sd_serializerFromString(serializer, type);

                if (meta_data != NULL) {
                    topicType = c_type(sd_serializerDeserialize(serializer, meta_data));

                    if (topicType == NULL) {
                        msg = sd_serializerLastValidationMessage(serializer);
                        OS_REPORT(OS_ERROR,
                                  CM_XML_CONTEXT, 0,
                                  "Data type could not be registered, "
                                  "because it is not valid: %s",
                                  msg);
                        result = CMX_RESULT_FAILED;
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
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0, "Kernel object could not be retrieved");
            result = CMX_RESULT_FAILED;
        }
        os_free(kernelArg);
        cmx_entityRelease(ce);
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
    u_result ur;
    c_char* topics;
    cmx_walkEntityArg arg;
    cmx_entity ce;

    topics = NULL;

    ce = cmx_entityClaim(participant);

    if(ce != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkParticipantArg)));
        if (arg != NULL){
            arg->length = 0;
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;
            cmx_walkParticipantArg(arg)->topicName = topicName;

            ur = u_observableAction(u_observable(ce->uentity),
                                    cmx_participantFindTopicAction,
                                    (c_voidp)arg);
            if (ur == U_RESULT_OK) {
                topics = cmx_convertToXMLList(arg->list, arg->length);
            }
            os_free(arg);
        }
        cmx_entityRelease(ce);
    }
    return topics;
}

void
cmx_participantFindTopicAction(
    v_public p,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_iter topics;
    v_entity topic;
    c_bool proceed;
    c_char* xmlEntity;

    arg = cmx_walkEntityArg(args);
    topics = v_resolveTopics(v_objectKernel(p), cmx_walkParticipantArg(arg)->topicName);
    topic = v_entity(c_iterTakeFirst(topics));

    while(topic != NULL){
        proceed = cmx_entityNewFromWalk(v_public(topic), &arg->entityArg);

        if(proceed == TRUE){
            xmlEntity = arg->entityArg.result;
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
    v_public entity,
    c_voidp args)
{
    v_kernel k;
    v_serviceManager m;
    v_serviceState splicedState;

    OS_UNUSED_ARG(args);

    k = v_objectKernel(entity);
    m = v_getServiceManager(k);
    splicedState = v_serviceManagerGetServiceState(m, V_SPLICED_NAME);
    v_observableAddObserver(v_observable(splicedState), v_observer(entity), NULL);
}

static c_ulong
cmx_participantDetach(
    u_observable o,
    c_ulong event,
    c_voidp usrData)
{
    v_serviceStateKind kind;
    u_serviceManager manager;

    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(event);
    OS_UNUSED_ARG(usrData);

    if ((event & V_EVENT_SERVICESTATE_CHANGED) == V_EVENT_SERVICESTATE_CHANGED) {
        if(cmx_isInitialized() == TRUE){
            manager = (u_serviceManager)usrData;

            if(manager != NULL){
                kind = u_serviceManagerGetServiceStateKind(manager, V_SPLICED_NAME);

                if ((kind != STATE_INITIALISING) && (kind != STATE_OPERATIONAL)) {
                    cmx_internalDetach();
                    u_objectFree(manager);
                    manager = NULL;
                }
            }
        }
    }
    return event;
}

const c_char*
cmx_participantAutoDetach(
    const c_char* participant,
    c_bool enable)
{
    c_ulong mask;
    u_participant up;
    u_result result;
    cmx_entity ce;

    ce = cmx_entityClaim(participant);

    if (ce == NULL) {
        goto errorGetEntity;
    }
    up = u_participant(ce->uentity);
    result = u_observableGetListenerMask(u_observable(up), &mask);

    if (result != U_RESULT_OK) {
        goto errorGetMask;
    }
    if (enable == FALSE) {
        mask &= ~V_EVENT_SERVICESTATE_CHANGED;
        result = u_observableRemoveListener(u_observable(up), cmx_participantDetach);
        if (result != U_RESULT_OK) {
            goto errorRemoveListener;
        }
    } else {
        mask |= V_EVENT_SERVICESTATE_CHANGED;
        result = u_observableAction(u_observable(up), cmx_participantInitDetach, NULL);
        if (result != U_RESULT_OK) {
            goto errorEntityAction;
        }
        result = u_observableAddListener(u_observable(up),
                                         cmx_participantDetach,
                                         u_serviceManagerNew(up));
        if (result != U_RESULT_OK) {
            goto errorInsertListener;
        }
    }
    result = u_observableSetListenerMask(u_observable(up), mask);
    if ( result != U_RESULT_OK) {
        goto errorSetListenerMark;
    }
    
    cmx_entityRelease(ce);

    return CMX_RESULT_OK;
    
errorSetListenerMark:
errorInsertListener:
errorEntityAction:
errorRemoveListener:
errorGetMask:
errorGetEntity:
    return CMX_RESULT_FAILED;
}

c_char*
cmx_participantDomainId(
    const c_char* participant)
{
    cmx_entity ce;
    u_participant up;
    c_char* result;
    u_domainId_t did;
    int written;

    ce = cmx_entityClaim(participant);

    if (ce == NULL) {
        did = U_DOMAIN_ID_INVALID;
    } else {
        up = u_participant(ce->uentity);
        did = u_participantGetDomainId(up);
        cmx_entityRelease(ce);
    }
    /* worst-case DOMAIN_ID_ANY: 2147483647  */
    result = os_malloc(sizeof(char) * 10 + 1);
    written = os_sprintf(result, "%d", did);
    assert(written > 0 && written <= 11);

    return result;
}
