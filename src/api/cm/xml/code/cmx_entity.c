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
#include "cmx_entity.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "cmx__participant.h"
#include "cmx__service.h"
#include "cmx__serviceState.h"
#include "cmx__publisher.h"
#include "cmx__subscriber.h"
#include "cmx__writer.h"
#include "cmx__query.h"
#include "cmx__domain.h"
#include "cmx__reader.h"
#include "cmx__topic.h"
#include "cmx__waitset.h"
#include "cmx__factory.h"
#include "sd_serializer.h"
#include "sd_serializerXML.h"
#include "u_observable.h"
#include "u_entity.h"
#include "u__entity.h"
#include "u_participant.h"
#include "u__participant.h"
#include "u_service.h"
#include "u_dataReader.h"
#include "u_networkReader.h"
#include "u_topic.h"
#include "u_groupQueue.h"
#include "u_partition.h"
#include "u_query.h"
#include "u_waitsetEntry.h"
#include "u_publisher.h"
#include "u_subscriber.h"
#include "v_kernel.h"
#include "v_entity.h"
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_groupQueue.h"
#include "v_networking.h"
#include "v_networkQueue.h"
#include "v_durability.h"
#include "v_cmsoap.h"
#include "v_statistics.h"
#include "v_kernelStatistics.h"
#include "v_writerStatistics.h"
#include "v_dataReaderStatistics.h"
#include "v_queryStatistics.h"
#include "v_networkReader.h"
#include "v_networkReaderStatistics.h"
#include "v_networkingStatistics.h"
#include "v_durabilityStatistics.h"
#include "v_groupQueueStatistics.h"
#include "v_cmsoapStatistics.h"
#include "c_typebase.h"
#include "v_public.h"
#include "v_qos.h"
#include "vortex_os.h"
#include "os_stdlib.h"
#include "sd_serializer.h"
#include "os_abstract.h"
#include "os_stdlib.h"
#include <stdio.h>

/* Number of escape-characters. This determines the size of the arrays used for
 * the replacements. */
#define CMX_XML_NUM_ESCAPE_CHARS (5)
/* Array containing the characters that need to be escaped. The character will
 * be replaced by the string in CMX_XML_REPLACE_CHARS[] with the same index, so
 * indices of this array must match the indices of CMX_XML_REPLACE_CHARS[]. */
static c_char CMX_XML_ESCAPE_CHARS[CMX_XML_NUM_ESCAPE_CHARS] = {'\'', '"', '&', '>', '<'};
/* Array containing the replacement strings of the characters that need to be
 * escaped, as specified in CMX_XML_ESCAPE_CHARS[]. The escaped character will
 * be replaced by the string in CMX_XML_REPLACE_CHARS[] with the same index, so
 * indices of this array must match the indices of CMX_XML_ESCAPE_CHARS[]. */
static c_char* CMX_XML_REPLACE_CHARS[CMX_XML_NUM_ESCAPE_CHARS] = {"&apos;", "&quot;", "&amp;", "&gt;", "&lt;"};
/* Array containing the strlen's of the replace-sequences. Indices must match
 * with CMX_XML_REPLACE_CHARS[].*/
static c_ulong CMX_XML_REPLACE_CHARS_LEN[CMX_XML_NUM_ESCAPE_CHARS] = {6, 6, 5, 4, 4};

/* Please note that the understanding documentation is written for HTML display.
 *
 * The strange part reads:
 * '''->'&apos;',
 * '"'->'&quot;',
 * '&'->'&amp;',
 * '>'->'&gt;' and
 * '<'->'&lt;'.
 */
/**
 * Escapes a string so it can be stored in an XML-container.
 *
 * It will replace all occurrences of characters in CMX_XML_ESCAPE_CHARS[] with
 * the strings in the same index in CMX_XML_REPLACE_CHARS[]. It will thus replace
 * &apos;&apos;&apos;-&gt;&apos;&amp;apos;&apos;,
 * &apos;&apos;&apos;-&gt;&apos;&amp;quot;&apos;,
 * &apos;&amp;&apos;-&gt;&apos;&amp;amp;&apos;,
 * &apos;&gt;&apos;-&gt;&apos;&amp;gt;&apos; and
 * &apos;&lt;&apos;-&gt;&apos;&amp;lt;&apos;.
 * @param string The string to be escaped.
 * @return An escaped copy of string. Needs to be freed with os_free().
 */
static c_char*
getXMLEscapedString(
    const c_char* string)
{
    c_ulong strLen = 0;
    c_ulong extraStrLen = 0;
    c_ulong i, j, inserts;
    c_char* result = NULL;
    c_bool match, shouldReplace = FALSE;

    if(string){
        /* Calculate length of resulting string */
        strLen = 0;

        while(string[strLen] != '\0'){
            match = FALSE;

            for(j = 0; !match && j < CMX_XML_NUM_ESCAPE_CHARS; j++){
                if(string[strLen] == CMX_XML_ESCAPE_CHARS[j]){
                    /* Count extra length. The original character is counted
                     * by strLen, so count with original character excluded. */
                    extraStrLen += CMX_XML_REPLACE_CHARS_LEN[j] - 1;
                    match = TRUE;   /* And we can stop looking */
                }
            }
            strLen++;
            shouldReplace |= match;
        }

        if(shouldReplace){
            /* Replacements needed, so allocate memory for result-string */
            result = (c_char*)os_malloc(strLen + extraStrLen + 1);

            if(result){
                inserts = 0;

                /* Now insert the escapes */
                for(i = 0; i < strLen; i++){
                    match = FALSE;

                    for(j = 0; !match && j < CMX_XML_NUM_ESCAPE_CHARS; j++){
                        if(string[i] == CMX_XML_ESCAPE_CHARS[j]){
                            os_strncpy(&result[i + inserts], CMX_XML_REPLACE_CHARS[j], CMX_XML_REPLACE_CHARS_LEN[j]);
                            /* Count extra length. The original character is counted
                             * by strLen, so count with original character excluded. */
                            inserts += CMX_XML_REPLACE_CHARS_LEN[j] - 1;
                            match = TRUE;   /* We can stop looking */
                        }
                    }

                    if(!match){
                        /* No replace happened, so include original character */
                        result[i + inserts] = string[i];
                    }
                }
                /* NUL-terminate result */
                result[strLen + extraStrLen] = '\0';
            } else {
                /* Memory-claim denied, return NULL */
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                        "Heap-memory claim of size %d denied, cannot generate escaped XML string.",
                        strLen + extraStrLen + 1);
            }
        } else {
            /* No replacements needed, so just duplicate the string */
            result = os_strdup(string);
        }
    }
    return result;
}

/**
 * Resolves the user entities that match the supplied XML entities. This is done
 * by casting the contents of the pointer tags in the XML entity to user
 * entities.
 *
 * @param xmlEntities The XML representation of the user entities to resolve.
 * @return The user entities that match the supplied XML entities.
 */
static c_iter
cmx_entityCmxEntities(
    const c_char* xmlEntities)
{
    size_t length = 0;
    c_ulong i = 0;
    c_ulong nrOfEntities = 0;
    c_char * substring;
    c_char * xmlEntity = NULL;
    cmx_entity cmEntity;
    c_iter cmEntities = NULL;

    c_iter xmlEntityList = c_iterNew(NULL);
    if (!xmlEntityList) goto err_iterNewEntityList;

    cmEntities = c_iterNew(NULL);
    if (!cmEntities) goto err_iterNewEntities;

    xmlEntities+=12;                                            /*<entityList>*/
    substring = strstr(xmlEntities, "</entity>");
    while(substring){
        length = (size_t) (substring - xmlEntities + 9);                   /*</entity>*/
        xmlEntity = os_malloc((length+1) * sizeof(c_char));
        os_strncpy(xmlEntity, xmlEntities, length);
        xmlEntity[length] = '\0';
        c_iterAppend(xmlEntityList, xmlEntity);
        xmlEntities = xmlEntities + length;
        substring = strstr(xmlEntities, "</entity>");
    }
    nrOfEntities = c_iterLength(xmlEntityList);
    for(i = 0 ; i<nrOfEntities; i++){
        xmlEntity = (c_char*)c_iterTakeFirst(xmlEntityList);
        cmEntity = cmx_entityClaim(xmlEntity);

        if(cmEntity){
            c_iterAppend(cmEntities, cmEntity);
        }
        os_free(xmlEntity);
    }

err_iterNewEntities:
    c_iterFree(xmlEntityList);
err_iterNewEntityList:
    return cmEntities;
}

_Ret_z_
const c_char *
cmx__uresult (
        _In_ u_result ures)
{
    switch(ures) {
        case U_RESULT_OK:
            return CMX_RESULT_OK;
        case U_RESULT_ILL_PARAM:
            return CMX_RESULT_ILL_PARAM;
        case U_RESULT_IMMUTABLE_POLICY:
            return CMX_RESULT_IMMUTABLE_POLICY;
        case U_RESULT_INCONSISTENT_QOS:
            return CMX_RESULT_INCONSISTENT_QOS;
        case U_RESULT_TIMEOUT:
            return CMX_RESULT_TIMEOUT;
        case U_RESULT_PRECONDITION_NOT_MET:
            return CMX_RESULT_PRECONDITION_NOT_MET;
        default:
            return CMX_RESULT_FAILED;
    }
}

void
cmx_entityNewFromAction(
    v_public entity,
    c_voidp args)
{
    (void)cmx_entityNewFromWalk(entity, args);
}

static c_bool
cmx_entityNewEntityFromWalk(
    v_entity entity,
    cmx_entityArg arg)
{
    cmx_entity participant;
    c_char* special;
    c_bool  result = FALSE;

    special = cmx_entityGetTypeXml(v_public(entity));
    if (special != NULL) {
        u_object proxy;
        if (arg->create == TRUE) {
            if (arg->entity->participant) {
                participant = arg->entity->participant;
            } else {
                participant = arg->entity;
            }
            assert(u_objectKind(participant->uentity) == U_PARTICIPANT);
            proxy = u_object(u_observableCreateProxy(v_public(entity),
                                                     u_participant(participant->uentity)));
            if (proxy != NULL) {
                cmx_registerObject(proxy,participant);
            }
        } else {
            proxy = arg->entity->uentity;
        }
        /* Get the entity XML representation. */
        arg->result = (c_voidp)cmx_entityXml(entity->name,
                                             (c_address)proxy,
                                             &(v_public(entity)->handle),
                                             v_entityEnabled(entity),
                                             special);
        os_free(special);
        result = TRUE;
    }
    return result;
}

static c_bool
cmx_entityNewWaitSetFromWalk(
    v_waitset waitset,
    cmx_entityArg arg)
{
    cmx_entity participant;
    c_char* special;
    c_bool  result = FALSE;

    special = cmx_entityGetTypeXml(v_public(waitset));
    if(special != NULL){
        u_object proxy = NULL;
        if(arg->create == TRUE){
            if (arg->entity->participant) {
                participant = arg->entity->participant;
            } else {
                participant = arg->entity;
            }
            proxy = u_object(u_observableCreateProxy(v_public(waitset),
                                                      u_participant(participant->uentity)));
            if (proxy != NULL) {
                cmx_registerObject(u_object(proxy), participant);
            }
        }
        /* Get the waitset XML representation. */
        arg->result = (c_voidp)cmx_entityXml(NULL,
                                             (c_address)proxy,
                                             &(v_public(waitset)->handle),
                                             TRUE,
                                             special);
        os_free(special);
        result = TRUE;
    }
    return result;
}

c_bool
cmx_entityNewFromWalk(
    v_public object,
    c_voidp args)
{
    cmx_entityArg arg = cmx_entityArg(args);
    c_bool result = FALSE;

    assert(object);
    assert(arg);

    if (c_instanceOf(object, "v_entity")) {
        /* Is this really an entity? */
        result = cmx_entityNewEntityFromWalk((v_entity)object, arg);
    } else if (c_instanceOf(object, "v_waitset")) {
        /* Or is this entity actually a waitset? */
        result = cmx_entityNewWaitSetFromWalk((v_waitset)object, arg);
    } else if (c_instanceOf(object, "v_listener")) {
        /* Ignore listeners, these are (for now) internal objects */
    } else {
        /* Should never come here. */
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Unknown object kind: '%d'\n",
                    v_object(object)->kind);
        assert(FALSE);
    }

    return result;
}

c_char*
cmx_entityGetTypeXml(
    v_public object)
{
    c_char* result;

    result = NULL;

    switch(v_object(object)->kind){
    case K_PARTICIPANT:
        result = cmx_participantInit((v_participant)object);
    break;
    case K_NETWORKING:
        /*fallthrough on purpose.*/
    case K_DURABILITY:
    case K_NWBRIDGE:
    case K_CMSOAP:
    case K_SPLICED:
    case K_RNR:
    case K_DBMSCONNECT:
    case K_SERVICE:
        result = cmx_serviceInit((v_service)object);
    break;
    case K_PUBLISHER:
        result = cmx_publisherInit((v_publisher)object);
    break;
    case K_SERVICESTATE:
        result = cmx_serviceStateInit((v_serviceState)object);
    break;
    case K_SUBSCRIBER:
        result = cmx_subscriberInit((v_subscriber)object);
    break;
    case K_WRITER:
        result = cmx_writerInit((v_writer)object);
    break;
    case K_QUERY:
        /*fallthrough on purpose.*/
    case K_DATAREADERQUERY:
        result = cmx_queryInit((v_query)object);
    break;
    case K_DOMAIN:
        result = cmx_domainInit((v_partition)object);
    break;
    case K_NETWORKREADER: /*fallthrough on purpose.*/
    case K_DATAREADER: /*fallthrough on purpose.*/
    case K_DATAVIEW: /*fallthrough on purpose.*/
    case K_DELIVERYSERVICE: /*fallthrough on purpose.*/
    case K_GROUPQUEUE:
        result = cmx_readerInit((v_reader)object);
    break;
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        result = cmx_topicInit((v_topic)object);
    break;
    case K_WAITSET:
        result = cmx_waitsetInit((v_waitset)object);
    break;
    case K_LISTENER:
    break;
    default:
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                    "Unknown entity kind: '%d'\n",
                    v_object(object)->kind);
        assert(FALSE);
    break;
    }

    return result;
}

c_char*
cmx_entityInit(
    v_entity entity,
    u_entity proxy,
    c_bool init)
{
    c_char* result;
    u_result ur;

    ur = U_RESULT_OK;
    result = NULL;

    if((proxy == NULL) && (init == TRUE)){
        ur = U_RESULT_ILL_PARAM;
    }
    if(ur == U_RESULT_OK){
        switch(v_object(entity)->kind){
        case K_PARTICIPANT:
            result = cmx_participantInit((v_participant)entity);
        break;
        case K_NETWORKING:
            /*fallthrough on purpose.*/
        case K_DURABILITY:
        case K_NWBRIDGE:
        case K_CMSOAP:
        case K_SPLICED:
        case K_RNR:
        case K_DBMSCONNECT:
        case K_SERVICE:
            result = cmx_serviceInit((v_service)entity);
        break;
        case K_PUBLISHER:
            result = cmx_publisherInit((v_publisher)entity);
        break;
        case K_SERVICESTATE:
            result = cmx_serviceStateInit((v_serviceState)entity);
        break;
        case K_SUBSCRIBER:
            result = cmx_subscriberInit((v_subscriber)entity);
        break;
        case K_WRITER:
            result = cmx_writerInit((v_writer)entity);
        break;
        case K_QUERY:
            /*fallthrough on purpose.*/
        case K_DATAREADERQUERY:
            result = cmx_queryInit((v_query)entity);
        break;
        case K_DOMAIN:
            result = cmx_domainInit((v_partition)entity);
        break;
        case K_NETWORKREADER:
            /*fallthrough on purpose.*/
        case K_DATAREADER:
            /*fallthrough on purpose.*/
        case K_DELIVERYSERVICE:
            /*fallthrough on purpose.*/
        case K_GROUPQUEUE:
            result = cmx_readerInit((v_reader)entity);
        break;
        case K_TOPIC:
        case K_TOPIC_ADAPTER:
            result = cmx_topicInit((v_topic)entity);
        break;
        case K_WAITSET:
            result = cmx_waitsetInit((v_waitset)entity);
        break;
        default:
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                        "Unknown entity kind: '%d'\n",
                        v_object(entity)->kind);
            assert(0);
        break;
        }
    } else {
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "cmx_entityInit ur != U_RESULT_OK.");
        cmx_deregisterObject(u_object(proxy));
        assert(0);
    }

    return result;
}

void
cmx_entityFree(
    c_char* entity)
{
    cmx_entity e;
    if(entity != NULL){
        e = cmx_entityClaim(entity);
        if(e != NULL){
            cmx_deregisterObject(u_object(e->uentity));
            cmx_factoryReleaseEntity(e);
        }
        os_free(entity);
    }
}

void
cmx_entityKindAction(
    v_public object,
    c_voidp args)
{
    cmx_entityKindArg arg;

    arg = cmx_entityKindArg(args);
    arg->kind = v_object(object)->kind;
}

c_bool
cmx_entityWalkAction(
    v_entity e,
    c_voidp args)
{
  cmx_walkEntityArg arg;
  c_char* xmlEntity;
  c_bool add;
  c_bool proceed;
  v_object object;

  /*
   * This is the callback of a call to u_entityWalkEntities().
   * In contradication of the function name, it will return a waitset
   * when it was called with a participant. This is because the waitset
   * was a entity in the past.
   *
   * So, an entity pointer is given, even while it can be a waitset. This
   * still works, but off course should change in the future.
   *
   * For now, just cast the entity to an object to indicate that we're not
   * always dealing with an entity here.
   */
  object = (v_object)e;

  arg = cmx_walkEntityArg(args);
  add = FALSE;

  if(object != NULL){
      switch(arg->filter){
        /*User filter, select all entities of the supplied+inherited kinds */
        case K_ENTITY:/*Always add the entity.*/
            if(object->kind != K_DELIVERYSERVICE){
                add = TRUE;
            }
        break;
        case K_QUERY:
            switch(object->kind){
            case K_QUERY:
            case K_DATAREADERQUERY:
                                add = TRUE; break;
            default:            break;
            }
        break;
        case K_TOPIC:
            switch(object->kind){
            case K_TOPIC:       add = TRUE;  break;
            default:            break;
            }
        break;
        case K_TOPIC_ADAPTER:
            switch(object->kind){
            case K_TOPIC_ADAPTER:add = TRUE;  break;
            default:            break;
            }
        break;
        case K_PUBLISHER:
            switch(object->kind){
            case K_PUBLISHER:   add = TRUE;  break;
            default:            break;
            }
        break;
        case K_SUBSCRIBER:
            switch(object->kind){
            case K_SUBSCRIBER:  add = TRUE;  break;
            default:            break;
            }
        break;
        case K_DOMAIN:
            switch(object->kind){
            case K_DOMAIN:      add = TRUE;  break;
            default:            break;
            }
        break;
        case K_READER:
            switch(object->kind){
            case K_READER:
            case K_DATAREADER:
            case K_NETWORKREADER:
            case K_GROUPQUEUE:
            case K_QUERY:
            case K_DATAREADERQUERY:
                                add = TRUE;  break;
            default:            break;
            }
        break;
        case K_DATAREADER:
            switch(object->kind){
            case K_DATAREADER:  add = TRUE;  break;
            default:            break;
            }
        break;
        case K_GROUPQUEUE:
            switch(object->kind){
            case K_GROUPQUEUE:  add = TRUE;  break;
            default:            break;
            }
        break;
        case K_NETWORKREADER:
            switch(object->kind){
            case K_NETWORKREADER:   add = TRUE;  break;
            default:                break;
            }
        break;
        case K_WRITER:
            switch(object->kind){
            case K_WRITER:      add = TRUE;  break;
            default:            break;
            }
        break;
        case K_PARTICIPANT:
            switch(object->kind){
            case K_PARTICIPANT:
            case K_SERVICE:
            case K_SPLICED:
            case K_NETWORKING:
            case K_DURABILITY:
            case K_NWBRIDGE:
            case K_CMSOAP:
            case K_RNR:
            case K_DBMSCONNECT:
                                add = TRUE;  break;
            default:            break;
            }
        break;
        case K_SERVICE:
            switch(object->kind){
            case K_SERVICE:
            case K_SPLICED:
            case K_NETWORKING:
            case K_DURABILITY:
            case K_NWBRIDGE:
            case K_CMSOAP:
            case K_RNR:
            case K_DBMSCONNECT:
                                add = TRUE;  break;
            default:            break;
            }
        break;
        case K_SERVICESTATE:
            switch(object->kind){
            case K_SERVICESTATE:add = TRUE;  break;
            default:            break;
            }
        break;
        case K_WAITSET:
            switch(object->kind){
            case K_WAITSET: add = TRUE;  break;
            default:            break;
            }
        break;
        default:
            OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                        "Unknown Entity found in cmx_entityWalkAction: %d\n",
                        object->kind);
        break;
        }
    }
    if(add == TRUE){
        proceed = cmx_entityNewFromWalk(v_public(object), &arg->entityArg);
        if(proceed == TRUE){
            xmlEntity = arg->entityArg.result;

            if(xmlEntity == NULL){
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                            "Entity found: %d\n",
                            object->kind);
                assert(FALSE);
            } else {
                arg->list = c_iterInsert(arg->list, xmlEntity);
                arg->length += strlen(xmlEntity);
            }
        }
    }
    return TRUE;
}

c_char *
cmx_entityOwnedEntities(
    const c_char* entity,
    const c_char* filter)
{
    cmx_entity ce;
    c_char* result;
    u_result walkSuccess;
    cmx_walkEntityArg arg;

    result = NULL;
    ce = cmx_entityClaim(entity);

    if(ce != NULL){
        if (u_objectKind(u_object(ce->uentity)) == U_WAITSETENTRY) {
            /* Waitset: return an empty list.
             * Waitsets aren't and weren't supported by u_entityWalkEntities() anyway. */
            result = cmx_convertToXMLList(NULL, 0);
        } else {
            arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
            arg->length = 0;
            arg->filter = cmx_resolveKind(filter);
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;
            walkSuccess = u_entityWalkEntities(u_entity(ce->uentity),
                                               cmx_entityWalkAction,
                                               (c_voidp)(arg));

            if (walkSuccess == U_RESULT_OK) {
                result = cmx_convertToXMLList(arg->list, arg->length);
            }
            os_free(arg);
        }
        cmx_entityRelease(ce);
    }
    return result;
}

struct cmx_entityHandleArg {
    c_ulong index;
    c_ulong serial;
};

void
getHandleAction(
    v_public object,
    c_voidp args)
{
    struct cmx_entityHandleArg *arg;
    arg = (struct cmx_entityHandleArg *)args;

    arg->index =  object->handle.index;
    arg->serial =  object->handle.serial;
}

c_ulong
cmx_entityPathFinder(const c_char* entity,
        c_ulong childIndex, c_ulong childSerial, c_iter* resultPath){
    cmx_entity ce;
    cmx_entity tempChild;
    c_char* xmlTempChild;
    v_kind kind;
    u_result walkSuccess;
    cmx_walkEntityArg arg;
    struct cmx_entityHandleArg handle;
    c_ulong resultLength=0;

    ce = cmx_entityClaim(entity);
    if(ce != NULL){
        kind = K_ENTITY; /*FORCED*/

        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));

        if(arg){
            arg->length = 0;
            arg->filter = kind;
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;

            walkSuccess = u_entityWalkEntities(u_entity(ce->uentity),
                                               cmx_entityWalkAction,
                                               (c_voidp)(arg));

            if (walkSuccess == U_RESULT_OK) {
                /*First level of children*/
                if(arg->list != NULL && arg->length > 0){
                    xmlTempChild = (c_char*)c_iterTakeFirst(arg->list);
                    while(xmlTempChild){
                        tempChild = cmx_entityClaim(xmlTempChild);
                        if(tempChild != NULL){
                            (void)u_observableAction(u_observable(ce->uentity),
                                                     getHandleAction,
                                                     (c_voidp)&handle);
                            if((handle.index == childIndex) && (handle.serial == childSerial)){
                                c_iterInsert(*resultPath, xmlTempChild);
                                resultLength+=strlen(xmlTempChild);
                                xmlTempChild = NULL;
                                break;
                            }else{
                                resultLength+=cmx_entityPathFinder(xmlTempChild, childIndex, childSerial, resultPath);
                                if(c_iterLength(*resultPath)==0){
                                    cmx_entityFree(xmlTempChild);
                                    xmlTempChild = (c_char*)c_iterTakeFirst(arg->list);
                                }else{
                                    c_iterInsert(*resultPath, xmlTempChild);
                                    resultLength+=strlen(xmlTempChild);
                                    xmlTempChild = NULL;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if(c_iterLength(arg->list)>0){
                xmlTempChild = (c_char*)c_iterTakeFirst(arg->list);
                while(xmlTempChild){
                    cmx_entityFree(xmlTempChild);
                    xmlTempChild = (c_char*)c_iterTakeFirst(arg->list);
                }
            }
            c_iterFree(arg->list);
            os_free(arg);
        }
    }
    return resultLength;
}

c_char *
cmx_entityGetEntityTree(
    const c_char* entity,
    const c_char* childIndex,
    const c_char* childSerial)
{
    c_char* result;
    c_ulong resultLength = 0;
    c_iter resultPath = c_iterNew(NULL);
    c_ulong _childIndex = 0;
    c_ulong _childSerial = 0;
    if(sscanf(childIndex,"%u", &_childIndex) && sscanf(childSerial,"%u", &_childSerial)){
        resultLength = cmx_entityPathFinder(entity, _childIndex, _childSerial, &resultPath);
    }
    result = cmx_convertToXMLList(resultPath, resultLength);

    return result;
}

c_char *
cmx_entityDependantEntities(
    const c_char* entity,
    const c_char* filter)
{
    cmx_entity ce;
    c_char* result;
    u_result walkSuccess;
    cmx_walkEntityArg arg;

    result = NULL;

    ce = cmx_entityClaim(entity);

    if(ce != NULL){
        switch (u_objectKind(u_object(ce->uentity))) {
        case U_WAITSET:
            /* Waitset: return an empty list.
             * Waitsets aren't and weren't supported by u_entityWalkEntities() anyway. */
            result = cmx_convertToXMLList(NULL, 0);
        break;
        default:
            arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
            arg->length = 0;
            arg->filter = cmx_resolveKind(filter);
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;

            walkSuccess = u_entityWalkDependantEntities(u_entity(ce->uentity),
                                                        cmx_entityWalkAction,
                                                        (c_voidp)(arg));

            if (walkSuccess == U_RESULT_OK) {
                result = cmx_convertToXMLList(arg->list, arg->length);
            }
            os_free(arg);
        break;
        }
        cmx_entityRelease(ce);
    }
    return result;
}

struct cmx_statusArg {
    c_char* result;
};

c_char*
cmx_entityStatus(
    const c_char* entity)
{
    cmx_entity ce;
    struct cmx_statusArg arg;

    arg.result = NULL;

    ce = cmx_entityClaim(entity);
    if(ce != NULL){
        if (u_objectKind(u_object(ce->uentity)) != U_WAITSET) {
            (void)u_observableAction(u_observable(ce->uentity), cmx_entityStatusAction, &arg);
        }
        cmx_entityRelease(ce);
    }
    return arg.result;
}

void
cmx_entityStatusAction(
    v_public entity,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_statusArg *arg;
    arg = (struct cmx_statusArg *)args;

    /* Only entities will enter this function (no waitsets). */

    ser = sd_serializerXMLNewTyped(c_getType(c_object(v_entity(entity)->status)));
    data = sd_serializerSerialize(ser, c_object(v_entity(entity)->status));
    arg->result = sd_serializerToString(ser, data);
    sd_serializedDataFree(data);
    sd_serializerFree(ser);
}

struct cmx_statisticsArg {
    c_char* result;
};

c_char*
cmx_entityStatistics(
    const c_char* entity)
{
    cmx_entity ce;
    struct cmx_statisticsArg arg;

    arg.result = NULL;

    ce = cmx_entityClaim(entity);
    if(ce != NULL){
        if (u_objectKind(u_object(ce->uentity)) != U_WAITSET) {
            (void)u_observableAction(u_observable(ce->uentity), cmx_entityStatisticsAction, &arg);
        }
        cmx_entityRelease(ce);
    }
    return arg.result;
}

c_char*
cmx_entitiesStatistics(
    const c_char* entities)
{
    c_iter cmEntityList;
    c_iter cmStatisticsList = c_iterNew(NULL);
    cmx_entity cmEntity;
    struct cmx_statisticsArg temp, arg;
    size_t statisticsSize = 0;
    c_char* openTag = "<statistics>";
    c_char* closeTag = "</statistics>";
    c_char* emptyStat = "<object></object>";
    c_char* cmStatistics = NULL;
    u_result result;

    cmEntityList = cmx_entityCmxEntities(entities);
    arg.result = NULL;
    temp.result = NULL;

    if(cmEntityList != NULL && c_iterLength(cmEntityList) > 0){
        cmEntity = (cmx_entity) c_iterTakeFirst(cmEntityList);
        while(cmEntity){
            if (u_objectKind(u_object(cmEntity->uentity)) != U_WAITSET) {
                result = u_observableAction(u_observable(cmEntity->uentity),
                                            cmx_entityStatisticsAction,
                                            &temp);
                if(temp.result != NULL && result == U_RESULT_OK){
                    statisticsSize += strlen(temp.result);
                    c_iterAppend(cmStatisticsList, temp.result);
                    temp.result = NULL;
                } else {
                    statisticsSize += strlen(emptyStat);
                    c_iterAppend(cmStatisticsList, os_strdup(emptyStat));
                }
            }
            cmx_entityRelease(cmEntity);
            cmEntity = (cmx_entity) c_iterTakeFirst(cmEntityList);
        }
    }
    c_iterFree(cmEntityList);
    arg.result = os_malloc((statisticsSize+strlen(openTag)+strlen(closeTag)+1)*sizeof(c_char));
    *arg.result = '\0';
    os_strcat(arg.result, openTag);
    if(c_iterLength(cmStatisticsList) > 0){
        cmStatistics = (c_char*)c_iterTakeFirst(cmStatisticsList);
        while(cmStatistics){
            os_strcat(arg.result, cmStatistics);
            os_free(cmStatistics);
            cmStatistics = (c_char*)c_iterTakeFirst(cmStatisticsList);
        }
    }
    os_strcat(arg.result, closeTag);
    c_iterFree(cmStatisticsList);
    return arg.result;
}

void
cmx_entityStatisticsAction(
    v_public object,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_statisticsArg *arg;
    v_statistics statistics;

    arg = (struct cmx_statisticsArg *)args;

    if (object == NULL) {
        /* Somebody actually tries to get statistics of a non-entity.
         * There are none. */
        arg->result = NULL;

    } else {

        switch(v_objectKind(object)) {
        case K_WRITER:
            statistics = v_statistics(v_writer(object)->statistics);
        break;
        case K_DATAREADER:
            statistics = v_statistics(v_dataReader(object)->statistics);
        break;
        case K_QUERY:
        case K_DATAREADERQUERY:
        case K_DATAVIEWQUERY:
            statistics = v_statistics(v_query(object)->statistics);
        break;
        case K_NETWORKREADER:
            statistics = v_statistics(v_networkReader(object)->statistics);
        break;
        case K_NETWORKING:
            statistics = v_statistics(v_networking(object)->statistics);
        break;
        case K_KERNEL:
            statistics = v_statistics(v_kernel(object)->statistics);
        break;
        case K_DURABILITY:
            statistics = v_statistics(v_durability(object)->statistics);
        break;
        case K_CMSOAP:
            statistics = v_statistics(v_cmsoap(object)->statistics);
        break;
        default:
            /* Remaining entities don't have specific statistics yet. */
            statistics = NULL;
            arg->result = NULL;
        break;
        }

        if(statistics != NULL){
            ser = sd_serializerXMLNewTyped(c_getType((c_object)statistics));
            data = sd_serializerSerialize(ser, statistics);
            arg->result = sd_serializerToString(ser, data);
            sd_serializedDataFree(data);
            sd_serializerFree(ser);
        }
    }
}

struct cmx_resetStatisticsArg {
    const c_char* fieldName;
    const c_char* result;
};

const c_char*
cmx_entityResetStatistics(
    const c_char* entity,
    const c_char* fieldName)
{
    cmx_entity ce;
    struct cmx_resetStatisticsArg arg;

    arg.result = CMX_RESULT_ENTITY_NOT_AVAILABLE;

    ce = cmx_entityClaim(entity);

    if(ce != NULL){
        arg.fieldName = fieldName;
        if (u_objectKind(u_object(ce->uentity)) != U_WAITSET) {
            (void)u_observableAction(u_observable(ce->uentity),
                                     cmx_entityStatisticsResetAction,
                                     &arg);
        }
        cmx_entityRelease(ce);
    }

    return arg.result;
}

void
cmx_entityStatisticsResetAction(
    v_public object,
    c_voidp args)
{
    struct cmx_resetStatisticsArg *arg;
    c_bool result = TRUE;
    arg = (struct cmx_resetStatisticsArg *)args;

    switch (v_objectKind(object)) {
    case K_KERNEL:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_kernel(object)->statistics), arg->fieldName);
        } else {
            v_kernelStatisticsInit(v_kernel(object)->statistics);
        }
    break;
    case K_WRITER:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_writer(object)->statistics), arg->fieldName);
        } else {
            v_writerStatisticsInit(v_writer(object)->statistics);
        }
    break;
    case K_DATAREADER:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_dataReader(object)->statistics), arg->fieldName);
        } else {
            v_dataReaderStatisticsInit(v_dataReader(object)->statistics);
        }
    break;
    case K_DATAVIEWQUERY:
    case K_QUERY:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_query(object)->statistics), arg->fieldName);
        } else {
            v_queryStatisticsInit(v_query(object)->statistics);
        }
    break;
    case K_NETWORKREADER:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_networkReader(object)->statistics), arg->fieldName);
        } else {
            v_networkReaderStatisticsInit(v_networkReader(object)->statistics);
        }
    break;
    case K_DURABILITY:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_durability(object)->statistics), arg->fieldName);
        } else {
            v_durabilityStatisticsInit(v_durability(object)->statistics);
        }
    break;
    case K_NWBRIDGE:
    break;
    case K_CMSOAP:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_cmsoap(object)->statistics), arg->fieldName);
        } else {
            v_cmsoapStatisticsInit(v_cmsoap(object)->statistics);
        }
    break;
    case K_NETWORKING:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_networking(object)->statistics), arg->fieldName);
        } else {
            v_networkingStatisticsInit(v_networking(object)->statistics);
        }
    break;
    case K_RNR:
    break;
    case K_DBMSCONNECT:
    break;
    case K_GROUPQUEUE:
        if (arg->fieldName) {
            result = v_statisticsResetField(v_statistics(v_groupQueue(object)->statistics), arg->fieldName);
        } else {
            v_groupQueueStatisticsInit(v_groupQueue(object)->statistics);
        }
    break;
    default:
        result = FALSE;
    break;
    }
    if(result == TRUE){
        arg->result = CMX_RESULT_OK;
    } else {
        arg->result = CMX_RESULT_FAILED;
    }
}

void
cmx_entityStatisticsFieldResetAction(
    v_public object,
    c_voidp args)
{
    struct cmx_resetStatisticsArg *arg;
    c_bool result = FALSE;
    v_statistics statistics = NULL;

    arg = (struct cmx_resetStatisticsArg *)args;

    switch (v_objectKind(object)) {
    case K_KERNEL:
        statistics = v_statistics(v_kernel(object)->statistics);
    break;
    case K_WRITER:
        statistics = v_statistics(v_writer(object)->statistics);
    break;
    case K_DATAREADER:
        statistics = v_statistics(v_dataReader(object)->statistics);
    break;
    case K_DATAVIEWQUERY:
    case K_QUERY:
        statistics = v_statistics(v_query(object)->statistics);
    break;
    case K_NETWORKREADER:
        statistics = v_statistics(v_networkReader(object)->statistics);
    break;
    case K_DURABILITY:
        statistics = v_statistics(v_durability(object)->statistics);
    break;
    case K_NWBRIDGE:
    break;
    case K_CMSOAP:
        statistics = v_statistics(v_cmsoap(object)->statistics);
    break;
    case K_NETWORKING:
        statistics = v_statistics(v_networking(object)->statistics);
    break;
    case K_RNR:
    break;
    case K_DBMSCONNECT:
    break;
    case K_GROUPQUEUE:
    break;
    default:
        statistics = NULL;
    break;
    }

    if(statistics != NULL){
        result = v_statisticsResetField(statistics, arg->fieldName);
    }

    if(result == TRUE){
        arg->result = CMX_RESULT_OK;
    } else {
        arg->result = CMX_RESULT_FAILED;
    }
}

c_char*
cmx_entityQoS(
    const c_char* entity)
{
    c_char* result;
    cmx_entity ce;

    result = NULL;
    ce = cmx_entityClaim(entity);

    if (ce != NULL) {
        if (u_objectKind(u_object(ce->uentity)) != U_WAITSET) {
            (void)u_entityGetXMLQos(u_entity(ce->uentity), &result);
        }
        cmx_entityRelease(ce);
    }

    return result;
}

const c_char*
cmx_entitySetQoS(
    const c_char* entity,
    const c_char* qos)
{
    cmx_entity ce;
    u_result ur;
    const c_char* result;

    result = CMX_RESULT_FAILED;
    if (qos != NULL && (strlen(qos) > 0)) {
        ce = cmx_entityClaim(entity);
        if(ce != NULL){
            if (u_objectKind(u_object(ce->uentity)) != U_WAITSET) {
                ur = u_entitySetXMLQos(u_entity(ce->uentity), qos);

                if(ur == U_RESULT_OK){
                    result = CMX_RESULT_OK;
                } else if(ur == U_RESULT_ILL_PARAM){
                    result = CMX_RESULT_ILL_PARAM;
                } else if(ur == U_RESULT_INCONSISTENT_QOS){
                    result = CMX_RESULT_INCONSISTENT_QOS;
                } else if(ur == U_RESULT_IMMUTABLE_POLICY){
                    result = CMX_RESULT_IMMUTABLE_POLICY;
                } else {
                    result = CMX_RESULT_FAILED;
                }
            } else {
                result = CMX_RESULT_ILL_PARAM;
            }
            cmx_entityRelease(ce);
        }
    } else {
        result = CMX_RESULT_ILL_PARAM;
    }
    return result;
}

const c_char*
cmx_entityEnable(
    const c_char* entity)
{
    cmx_entity ce;
    const c_char* result;
    u_result ures;

    result = CMX_RESULT_ENTITY_NOT_AVAILABLE;
    ce = cmx_entityClaim(entity);

    if(ce != NULL){
        if (u_objectKind(ce->uentity) != U_WAITSET) {
            ures = u_entityEnable(u_entity(ce->uentity));
            if(ures == U_RESULT_OK) {
                /* TODO: update XML-entity? */
            }
            result = cmx__uresult(ures);
        } else {
            result = CMX_RESULT_ILL_PARAM;
        }
        cmx_entityRelease(ce);
    }

    return result;
}

c_char*
cmx_entityXml(
        const c_string name,
        const c_address proxy,
        const v_handle *handle,
        const c_bool enabled,
        const c_string special)
{
    c_char* xml_enabled;
    c_char* xml_name;
    c_ulong xml_hdl_index  = 0;
    c_ulong xml_hdl_serial = 0;
    char buf[1024];

    assert(special);

    if(enabled){
        xml_enabled = "TRUE";
    } else {
        xml_enabled = "FALSE";
    }

    if(name){
        xml_name = getXMLEscapedString(name);
    } else {
        xml_name = os_strdup("NULL");
    }

    if (handle) {
        xml_hdl_index  = handle->index;
        xml_hdl_serial = handle->serial;
    }

    os_sprintf(buf, "<entity><pointer>"PA_ADDRFMT"</pointer><handle_index>%u</handle_index><handle_serial>%u</handle_serial><name>%s</name><enabled>%s</enabled>%s</entity>",
                proxy,
                xml_hdl_index,
                xml_hdl_serial,
                xml_name,
                xml_enabled,
                special);

    os_free(xml_name);

    return os_strdup(buf);
}

cmx_entity
cmx_entityClaim(
    const c_char* xmlEntity)
{
    c_char* copy;
    c_char* temp;
    c_char* savePtr;
    u_object e;
    int assigned;
    cmx_entity entity;

    entity = NULL;

    if(xmlEntity != NULL){
        if(cmx_isInitialized() == OS_TRUE){
            copy = (c_char*)(os_malloc(strlen(xmlEntity) + 1));

            if(copy != NULL){
                (void)os_strcpy(copy, xmlEntity);

                temp = os_strtok_r((c_char*)copy, "</>", &savePtr);/*<entity>*/

                if(temp != NULL){
                    temp = os_strtok_r(NULL, "</>", &savePtr);/*<pointer>*/

                    if(temp != NULL){
                        temp = os_strtok_r(NULL, "</>", &savePtr);/*...the pointer*/

                        if(temp != NULL){
                            assigned = sscanf(temp, PA_ADDRFMT, (c_address *)(&e));

                            if (assigned != 1) {
                                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                                        "Failed to retrieve entity address from xml string '%s' and address 0x%s",
                                        xmlEntity, temp);
                            } else {
                                entity = cmx_factoryClaimEntity(e);

                                if(entity == NULL){
                                    OS_REPORT(OS_WARNING, CM_XML_CONTEXT, 0,
                                         "Entity "PA_ADDRFMT" (0x%s) from string '%s' has already been freed.\n",
                                         (c_address)e, temp, xmlEntity);
                                }
                            }
                        }
                    }
                }
            }
            os_free(copy);
        } else {
            cmx_detach();
        }
    }
    return entity;
}

void
cmx_entityKernelAction(
    v_public object,
    c_voidp args)
{
    cmx_entityKernelArg arg;

    arg = cmx_entityKernelArg(args);

    if(object != NULL){
        arg->kernel = v_objectKernel(object);
    }
}

u_result
cmx_entityRegister (
    u_object object,
    cmx_entity participant,
    c_char **xml)
{
    u_result ur = U_RESULT_ILL_PARAM;
    if (object && xml) {
        C_STRUCT(cmx_entityArg) arg;
        arg.create = FALSE;
        arg.result = NULL;
        arg.entity = cmx_registerObject(object, participant);
        ur = u_observableAction(u_observable(object),
                                cmx_entityNewFromAction,
                                (c_voidp)&arg);
        if(ur == U_RESULT_OK){
            *xml = arg.result;
        } else {
            cmx_deregisterObject(object);
        }
    }
    return ur;
}

