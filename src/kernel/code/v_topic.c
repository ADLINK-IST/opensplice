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
#include "v__topic.h"
#include "v__topicQos.h"
#include "v_projection.h"
#include "v__kernel.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_public.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v__builtin.h"
#include "v__status.h"
#include "v_message.h"
#include "v__messageQos.h"
#include "v__crc.h"
#include "v_event.h"
#include "v_policy.h"
#include "v_time.h"
#include "v__partition.h"
#include "v_configuration.h"
#include "v__policy.h"
#include "os_heap.h"
#include "v_topic.h"
#include "v__participant.h"

#include "sd_serializerXMLTypeinfo.h"

#include "c_stringSupport.h"
#include "os.h"
#include "os_report.h"

#define USERDATA_FIELD_NAME "userData"

static v_accessMode
v_partitionDetermineTopicAccessMode(
    const c_char* topicName,
    v_kernel kernel);

static c_type
messageTypeNew(
    v_kernel kernel,
    const c_char *typeName)
{
    c_base base;
    c_metaObject o;
    c_type baseType,dataType,type, foundType;
    c_char *name;
    c_long length, sres;

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    if (base == NULL) {
        OS_REPORT_1(OS_ERROR,
                    "v_topic::messageTypeNew", 21,
                    "Could not create type 'v_message<%s>', "
                    "invalid kernel reference.",
                    typeName);
        return NULL;
    }
    dataType = c_resolve(base,typeName);
    if (dataType == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "v_topic::messageTypeNew", 21,
                    "Could not create type 'v_message<%s>', unknown type '%s'.",
                    typeName, typeName);
        return NULL;
    }
    c_lockWrite(&kernel->lock);
    baseType = v_kernelType(kernel,K_MESSAGE);
    c_lockUnlock(&kernel->lock);
    assert(baseType != NULL);

    type = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(type)->extends = c_keep(c_class(baseType));
    o = c_metaDeclare(c_metaObject(type),
                      USERDATA_FIELD_NAME,
                      M_ATTRIBUTE);
    c_property(o)->type = dataType;
    c_free(o);
    c_metaObject(type)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(type));
    /* Create a name and bind type to name */

#define MESSAGE_FORMAT "v_message<%s>"
#define MESSAGE_NAME "v_message<>"
    length = sizeof(MESSAGE_NAME) + strlen(typeName); /* sizeof contains \0 */
    name = os_malloc(length);
    sres = snprintf(name,length,MESSAGE_FORMAT,typeName);
    assert(sres == (length-1));
#undef MESSAGE_FORMAT
#undef MESSAGE_NAME
    foundType = c_type(c_metaBind(c_metaObject(base),name,c_metaObject(type)));
    os_free(name);
    c_free(type);

    return foundType;
}

static c_bool
createMessageKeyList(
    c_type messageType,
    const c_char *topicKeyExpr,
    c_array *keyListRef)
{
    c_array keyList;
    c_field field;
    c_iter splitNames, newNames;
    c_char *name, *newName;
    c_long i,length,sres;

    assert(keyListRef != NULL);

    *keyListRef = NULL;
    if (topicKeyExpr == NULL) {
        return TRUE;
    }
    newNames = NULL;
    splitNames = c_splitString(topicKeyExpr,", \t");
    while ((name = c_iterTakeFirst(splitNames)) != NULL) {
#define PATH_SEPARATOR "."
#define PREFIX "userData"
        /* sizeof(PATH_SEPARATOR) includes \0 */
        length = strlen(PREFIX) + sizeof(PATH_SEPARATOR) + strlen(name);
        newName = (char *)os_malloc(length);
        sres = snprintf(newName,length,"%s"PATH_SEPARATOR"%s",PREFIX, name);
        assert(sres == (length-1));
#undef PREFIX
#undef PATH_SEPARATOR

        os_free(name);
        newNames = c_iterAppend(newNames,newName);
    }
    c_iterFree(splitNames);
    length = c_iterLength(newNames);
    if (length == 0) {
        return TRUE;
    }

    keyList = c_arrayNew(c_field_t(c_getBase(messageType)),length);
    i=0;
    while ((name = c_iterTakeFirst(newNames)) != NULL) {
        field = c_fieldNew(messageType,name);
        if (field == NULL) {
            OS_REPORT_1(OS_API_INFO,
                        "create message key list failed", 21,
                        "specified key field name %s not found",
                        name);
            os_free(name);
            c_iterFree(newNames);
            c_free(keyList);
            return FALSE;
        }
        keyList[i++] = field;
        os_free(name);
    }
    c_iterFree(newNames);
    *keyListRef = keyList;
    return TRUE;
}

#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),#t))

static c_type
createKeyType(
    const c_char *name,
    c_array keyList)
{
    c_base base;
    c_type foundType;
    c_char *typeName;
    c_char keyName[16];
    c_long i, length, sres;
    c_array members;
    c_metaObject o;
    c_field field;

    if (keyList == NULL) {
        return NULL;
    }
    base = c_getBase(keyList);
    length = c_arraySize(keyList);
    if (length == 0) {
        return NULL;
    }
    o = c_metaDefine(c_metaObject(base),M_STRUCTURE);
    members = c_arrayNew(c_member_t(base),length);

    for (i=0;i<length;i++) {
        field = keyList[i];
        assert(field != NULL);
        members[i] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
        os_sprintf(keyName,"field%d",i);
        c_specifier(members[i])->name = c_stringNew(base,keyName);
        c_specifier(members[i])->type = c_keep(c_fieldType(field));
    }
    c_structure(o)->members = members;
    c_metaObject(o)->definedIn = c_metaObject(base);
    c_metaFinalize(o);
    if (name != NULL) {
#define KEY_NAME   "<Key>"
#define KEY_FORMAT "%s<Key>"
        /* The sizeof contains \0 */
        length = sizeof(KEY_NAME) + strlen(name);
        typeName = os_malloc(length);
        sres = snprintf(typeName,length,KEY_FORMAT,name);
        assert(sres == (length-1));
#undef KEY_NAME
#undef KEY_FORMAT
    } else {
        assert(FALSE); /* Not supposed to happen anymore */
        length = 100;
        typeName = os_malloc(length);
        os_sprintf(typeName,PA_ADDRFMT"<Key>",(c_address)o);
    }
    foundType = c_type(c_metaBind(c_metaObject(base),typeName,o));

    c_free(o);
    os_free(typeName);

    return foundType;

}

#undef ResolveType

#define KEY_INDEX (sizeof(USERDATA_FIELD_NAME)) /* Note: the zero terminator
                                                   represents the '.' */

static c_bool
compare_names(
    c_voidp n1,
    c_voidp n2)
{
    return (strcmp((c_char *)n1,(c_char *)n2) == 0);
}

static c_bool
keysConsistent(
    v_topic topic,
    const char *keyExpr)
{
    c_iter keyExprNames, topicKeyNames;
    c_char *name, *found;
    c_bool consistent;

    keyExprNames = c_splitString(keyExpr,", \t");
    topicKeyNames = c_splitString(v_topicKeyExpr(topic),", \t");
    consistent = (c_iterLength(keyExprNames) == c_iterLength(topicKeyNames));
    name = c_iterTakeFirst(keyExprNames);
    while (name != NULL) {
        found = c_iterTakeAction(topicKeyNames,compare_names,name);
        if (!found) {
            consistent = FALSE;
        }
        os_free(name);
        os_free(found);
        name = c_iterTakeFirst(keyExprNames);
    }
    c_iterFree(keyExprNames);
    name = c_iterTakeFirst(topicKeyNames);
    while (name != NULL) {
        os_free(name);
        name = c_iterTakeFirst(topicKeyNames);
    }
    c_iterFree(topicKeyNames);

    return consistent;
}

c_type
v_topicKeyTypeCreate (
    v_topic _this,
    const c_char *keyExpr,
    c_array *keyListRef)
{
    c_string typeName;
    c_char *name;
    c_long length;
    c_type keyType;
    c_array keyList;

    assert(_this);
    assert(C_TYPECHECK(_this,v_topic));

    if (_this) {
        keyList = NULL;
        if (createMessageKeyList(_this->messageType,
                                 keyExpr,
                                 &keyList) == FALSE) {
            keyType = NULL;
        } else {
            typeName = c_metaName(c_metaObject(_this->messageType));
            length = strlen(typeName) + strlen(keyExpr) + 3;
            name = os_alloca(length);
            snprintf(name,length,"%s<%s>",typeName,keyExpr);
            keyType = createKeyType(name,keyList);
            c_free(typeName);
            os_freea(name);
        }
    } else {
        keyType = NULL;
    }
    if (keyListRef) {
        *keyListRef = c_keep(keyList);
    }
    c_free(keyList);
    return keyType;
}

v_topic
v_topicNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyExpr,
    v_topicQos qos)
{
    return v__topicNew(kernel, name, typeName, keyExpr, qos, TRUE);
}

static c_bool
isTopicConsistent(
    const v_topic found,
    const c_char *name,
    const c_char *typeName,
    const v_topicQos qos,
    const c_char *keyExpr)
{
    c_bool consistent = TRUE;

    if (c_compareString(v_topicName(found), name) == C_EQ) {
        c_char *rTypeName = c_metaScopedName(c_metaObject(v_topicDataType(found)));
        if (c_compareString(rTypeName, typeName) == C_EQ) {
            if (found->qos) {
                c_bool valid = v_durabilityPolicyEqual(found->qos->durability, qos->durability);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Durability'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_durabilityServicePolicyEqual(found->qos->durabilityService, qos->durabilityService);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'DurabilityService'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_deadlinePolicyEqual(found->qos->deadline, qos->deadline);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Deadline'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_latencyPolicyEqual(found->qos->latency, qos->latency);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Latency'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_livelinessPolicyEqual(found->qos->liveliness, qos->liveliness);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Liveliness'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_reliabilityPolicyEqual(found->qos->reliability, qos->reliability);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Reliability'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_orderbyPolicyEqual(found->qos->orderby, qos->orderby);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'OrderBy'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_resourcePolicyEqual(found->qos->resource, qos->resource);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Resource'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_transportPolicyEqual(found->qos->transport, qos->transport);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Transport'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_lifespanPolicyEqual(found->qos->lifespan, qos->lifespan);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Lifespan'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_ownershipPolicyEqual(found->qos->ownership, qos->ownership);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'Ownership'.",
                                name);
                    consistent = FALSE;
                }
                valid = v_historyPolicyEqual(found->qos->history, qos->history);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                "Create Topic <%s> failed: Unmatching QoS Policy: 'History'.",
                                name);
                    consistent = FALSE;
                }
                valid = keysConsistent(found, keyExpr);
                if (!valid)
                {
                    OS_REPORT_1(OS_WARNING, "v_topicNew", 21,
                                "Create Topic \"%s\" failed: Key differs exiting definition",
                                name);
                    consistent = FALSE;
                }
            } else {
                OS_REPORT_1(OS_WARNING, "v_topicNew", 21,
                            "Create Topic \"%s\" failed: QoS value undefined",
                            name);
                consistent = FALSE;
            }
        } else {
            OS_REPORT_3(OS_WARNING, "v_topicNew", 21,
                        "Create Topic \"%s\" failed: typename <%s> differs exiting definition <%s>.",
                        name, rTypeName, typeName);
            consistent = FALSE;
        }
        os_free(rTypeName);
    } else {
        OS_REPORT_3(OS_WARNING, "v_topicNew", 21,
                    "Create Topic \"%s\" failed: name <%s> differs existing name <%s>.",
                    name, name, v_topicName(found));
        consistent = FALSE;
    }

    return consistent;
}

v_topic
v__topicNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyExpr,
    v_topicQos qos,
    c_bool announce)
{
    v_result result;
    v_topic topic,found;
    c_type type;
    c_array keyList;
    c_property field;
    v_topicQos newQos;
    c_char *str;
    v_message builtinMsg = NULL;

    assert(C_TYPECHECK(kernel,v_kernel));

    if (name == NULL) {
        OS_REPORT(OS_WARNING, "v_topicNew", 0,
                  "Topic '?' is not created. No name specified (NULL).");
        return NULL;
    }

    newQos = v_topicQosNew(kernel,qos);
    if (newQos == NULL) {
        OS_REPORT_1(OS_ERROR, "v_topicNew", 0,
                    "Topic '%s' is not created: inconsistent qos",
                    name);
        return NULL;
    }

    topic = NULL;
    found = v_lookupTopic(kernel, name);
    if (found) {
        /* Check found topic with new topic */
        /* compare topic, if inconsistent, reject creation! */

        if(isTopicConsistent(found,
                           name,
                           typeName,
                           newQos,
                           keyExpr))
        {
            topic = c_keep(found);
        }

        c_free(found);
        /* newQos is not used because the existing topic is returned */
        v_topicQosFree(newQos);
    } else {
        type = messageTypeNew(kernel,typeName);
        if (type != NULL) {
            if (createMessageKeyList(type, keyExpr, &keyList) == FALSE) {
                topic = NULL; /* illegal key expression */
                v_topicQosFree(newQos);
                OS_REPORT_2(OS_ERROR, "v_topicNew", 0,
                            "Failed to create Topic '%s': invalid key expression '%s'.",
                            name, keyExpr);
            } else {
                field = c_property(c_metaResolve(c_metaObject(type),
                                                 USERDATA_FIELD_NAME));
                assert(field != NULL);

                topic = v_topic(v_objectNew(kernel,K_TOPIC));
                v_observerInit(v_observer(topic),name,NULL,TRUE);
                topic->messageType = type;
                topic->messageKeyList = keyList;
                topic->keyType = createKeyType(name,keyList);
                topic->dataField = field;
                topic->qos = newQos;
                topic->keyExpr = c_stringNew(c_getBase(kernel), keyExpr);
                topic->accessMode = v_partitionDetermineTopicAccessMode(name, kernel);

                /* determine CRC codes */
                str = c_metaScopedName(c_metaObject(field->type));
                topic->crcOfName = v_crcCalculate(kernel->crc, name, strlen(name));
                topic->crcOfTypeName = v_crcCalculate(kernel->crc, str, strlen(str));
                os_free(str);
                assert(found == NULL);
                result = v_topicEnable(topic);
                switch(result)
                {
                    case V_RESULT_OK:
                        if(announce)
                        {
                            builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,topic);
                        }
                        break;
                    case V_RESULT_PRECONDITION_NOT_MET:
                        /* Request is superfluous, so release previous topic and lookup existing precursor. */
                        c_free(topic);
                        topic = v_lookupTopic(kernel, name);
                        break;
                    case V_RESULT_INCONSISTENT_QOS:
                        c_free(topic);
                        topic = NULL;
                        OS_REPORT_1(OS_WARNING, "v_topicNew", 0,
                                    "Failed to create Topic '%s': a topic with non-matching QoS already exists.",
                                    name);
                        break;
                    default:
                        /* this should never happen,
                         * V_RESULT_ILL_PARAM should also not be returned,
                         * as ill_param is only returned when topic == NULL
                         */
                        c_free(topic);
                        topic = NULL;
                        OS_REPORT_1(OS_ERROR, "v_topicNew", 0,
                                "Failed to create Topic '%s': an unexpected error occurred.",
                                name);
                }
            }
            c_free(type);
        } else {
            topic = NULL;
            v_topicQosFree(newQos);
            OS_REPORT_2(OS_ERROR, "v_topicNew", 0,
                        "Failed to create Topic '%s': could not resolve type '%s'.",
                        name, typeName);
        }
    }
    if(builtinMsg){
        v_writeBuiltinTopic(kernel, V_TOPICINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }

    return topic;
}

/* !!!This function is not THREAD-SAFE!!!
 * To add the topic to the set of topics the kernel->lock must be locked.
 * However this is currently implicitly done in the new operator.
 */
v_result
v_topicEnable (
    v_topic topic)
{
    v_kernel kernel;
    v_topic found;
    v_result result;

    assert(topic);
    assert(C_TYPECHECK(topic,v_topic));

    if (topic) {
        kernel = v_objectKernel(topic);
        found = v__addTopic(kernel,topic);
        /*
         * If the topic was already added to the kernel,
         * then the calling method should know so it can perform
         * a roll-back.
         */
        if(found != topic)
        {
            c_char *typeName = c_metaScopedName(c_metaObject(v_topicDataType(topic)));

            if(typeName)
            {
                if(isTopicConsistent(found,
                                   v_topicName(topic),
                                   typeName,
                                   topic->qos,
                                   v_topicKeyExpr(topic)))
                {
                    /* Compatible and already enabled topic found,
                     * so this request is superfluous.
                     */
                    result = V_RESULT_PRECONDITION_NOT_MET;
                }
                else
                {
                    result = V_RESULT_INCONSISTENT_QOS;
                }

                os_free(typeName);
            }
            else
            {
                result = V_RESULT_INTERNAL_ERROR;
            }
        }
        else
        {
            result = V_RESULT_OK;
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

void
v_topicAnnounce(
    v_topic topic)
{
    v_message builtinMsg;
    v_kernel kernel;

    assert(topic);
    assert(C_TYPECHECK(topic,v_topic));
    /* publish V_TOPICINFO_TOPIC. */

    kernel = v_objectKernel(v_object(topic));
    c_lockWrite(&kernel->lock);
    builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,topic);
    c_lockUnlock(&kernel->lock);
    v_writeBuiltinTopic(kernel, V_TOPICINFO_ID, builtinMsg);
    c_free(builtinMsg);


}

void
v_topicFree(
    v_topic topic)
{
    assert(C_TYPECHECK(topic,v_topic));

    v_observerFree(v_observer(topic));
}

void
v_topicDeinit(
    v_topic topic)
{
    assert(C_TYPECHECK(topic,v_topic));

    v_observerDeinit(v_observer(topic));
}

v_message
v_topicMessageNew(
    v_topic topic)
{
    v_message message;

    assert(C_TYPECHECK(topic,v_topic));

    message = (v_message)c_new(topic->messageType);
    if (message) {
#ifndef _NAT_
        message->allocTime = v_timeGet();
#endif
        message->qos = NULL;
        V_MESSAGE_INIT(message);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_topicMessageNew",0,
                  "Failed to allocate message.");
        assert(FALSE);
    }
    return message;
}

c_object
c_messageUserData(
    v_topic topic,
    v_message o)
{
    assert(C_TYPECHECK(topic,v_topic));
    assert(C_TYPECHECK(o,v_message));

    return (c_object)(C_DISPLACE(o,c_property(topic->dataField)->offset));
}

c_char *
v_topicMessageKeyExpr(
    v_topic topic)
{
    c_string fieldName;
    c_char *keyExpr;
    c_long i,nrOfKeys,totalSize;
    c_array keyList;

    assert(C_TYPECHECK(topic,v_topic));

    keyList = v_topicMessageKeyList(topic);
    nrOfKeys = c_arraySize(keyList);

    if (nrOfKeys>0) {
        totalSize = 0;
        for (i=0;i<nrOfKeys;i++) {
            fieldName = c_fieldName(keyList[i]);
            totalSize += (strlen(fieldName)+1);
        }
        keyExpr = (char *)os_malloc(totalSize+1);
        keyExpr[0] = 0;
        for (i=0;i<nrOfKeys;i++) {
            fieldName = c_fieldName(keyList[i]);
            os_strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { os_strcat(keyExpr,","); }
        }
    } else {
        keyExpr = NULL;
    }
    return keyExpr;
}

c_iter
v_topicLookupWriters(
    v_topic topic)
{
    c_iter participants;
    c_iter entities;
    c_iter result;
    c_iter writers;
    v_entity entity;
    v_entity writer;
    v_participant participant;

    result = NULL;
    participants = v_resolveParticipants(v_objectKernel(topic), "*");
    participant = v_participant(c_iterTakeFirst(participants));

    while(participant != NULL){
        c_lockRead(&participant->lock);
        entities = ospl_c_select(participant->entities, 0);
        c_lockUnlock(&participant->lock);
        entity = v_entity(c_iterTakeFirst(entities));

        while(entity != NULL){
            if(v_objectKind(entity) == K_PUBLISHER){
                writers = v_publisherLookupWriters(v_publisher(entity),
                                                   v_topicName(topic));
                writer = v_entity(c_iterTakeFirst(writers));

                while(writer != NULL){
                    result = c_iterInsert(result, writer);
                    writer = v_entity(c_iterTakeFirst(writers));
                }
                c_iterFree(writers);
            }
            c_free(entity);
            entity = v_entity(c_iterTakeFirst(entities));
        }
        c_iterFree(entities);
        c_free(participant);
        participant = v_participant(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
    return result;
}

c_iter
v_topicLookupReaders(
    v_topic topic)
{
    v_kernel kernel;
    c_iter participants;
    c_iter entities;
    c_iter result;
    c_iter readers;
    v_entity entity;
    v_entity reader;
    v_participant participant;

    result = NULL;
    kernel = v_objectKernel(topic);
    participants = v_resolveParticipants(kernel, "*");
    participant = v_participant(c_iterTakeFirst(participants));

    while(participant != NULL){
        c_lockRead(&participant->lock);
        entities = ospl_c_select(participant->entities, 0);
        c_lockUnlock(&participant->lock);
        entity = v_entity(c_iterTakeFirst(entities));

        while(entity != NULL){
            if(v_objectKind(entity) == K_SUBSCRIBER){
                readers = v_subscriberLookupReadersByTopic(v_subscriber(entity),
                                                           v_topicName(topic));
                reader = v_entity(c_iterTakeFirst(readers));

                while(reader != NULL){
                    result = c_iterInsert(result, reader);
                    reader = v_entity(c_iterTakeFirst(readers));
                }
                c_iterFree(readers);
            }
            c_free(entity);
            entity = v_entity(c_iterTakeFirst(entities));
        }
        c_iterFree(entities);
        c_free(participant);
        participant = v_participant(c_iterTakeFirst(participants));
    }
    c_iterFree(participants);
    return result;
}

v_topicQos
v_topicGetQos(
    v_topic topic)
{
    assert(C_TYPECHECK(topic,v_topic));

    return c_keep(topic->qos);
}

v_result
v_topicSetQos(
    v_topic topic,
    v_topicQos qos)
{
    v_result result;
    v_qosChangeMask cm;
    v_message builtinMsg;
    v_kernel kernel;

    assert(C_TYPECHECK(topic,v_topic));
    /* Do not use C_TYPECHECK on qos parameter,
     * since it might be allocated on heap!
     */
    kernel = v_objectKernel(topic);
    c_lockWrite(&kernel->lock);
    result = v_topicQosSet(topic->qos, qos, v_entity(topic)->enabled, &cm);
    if ((result == V_RESULT_OK) && (cm != 0)) {
        builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,topic);
        v_writeBuiltinTopic(kernel, V_TOPICINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
    c_lockUnlock(&kernel->lock);

    return result;
}

void
v_topicNotify(
    v_topic topic,
    v_event event,
    c_voidp userData)
{
    OS_UNUSED_ARG(topic);
    OS_UNUSED_ARG(event);
    OS_UNUSED_ARG(userData);
    assert(C_TYPECHECK(topic,v_topic));
}

void
v_topicNotifyInconsistentTopic(
    v_topic topic)
{
    C_STRUCT(v_event) e;
    c_bool changed;

    assert(C_TYPECHECK(topic,v_topic));

    changed = v_statusNotifyInconsistentTopic(v_entity(topic)->status);
    if (changed) {
        e.kind = V_EVENT_INCONSISTENT_TOPIC;
        e.source = v_publicHandle(v_public(topic));
        e.userData = NULL;
        v_observerNotify(v_observer(topic), &e, NULL);
        v_observableNotify(v_observable(topic), &e);
    }
}

void
v_topicNotifyAllDataDisposed(
    v_topic topic)
{
    C_STRUCT(v_event) e;
    c_bool changed;

    assert(C_TYPECHECK(topic,v_topic));

    changed = v_statusNotifyAllDataDisposed(v_entity(topic)->status);
    if (changed) {
        e.kind = V_EVENT_ALL_DATA_DISPOSED;
        e.source = v_publicHandle(v_public(topic));
        e.userData = NULL;
        v_observerNotify(v_observer(topic), &e, NULL);
        v_observableNotify(v_observable(topic), &e);
    }
}

v_result
v_topicGetInconsistentTopicStatus(
    v_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(C_TYPECHECK(_this,v_topic));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_topicStatus(status)->inconsistentTopic, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_INCONSISTENT_TOPIC);
        }
        v_topicStatus(status)->inconsistentTopic.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
    }

    return result;
}

v_result
v_topicGetAllDataDisposedStatus(
    v_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(C_TYPECHECK(_this,v_topic));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_topicStatus(status)->allDataDisposed, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_ALL_DATA_DISPOSED);
        }
        v_topicStatus(status)->allDataDisposed.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
    }

    return result;
}

void
v_topicMessageCopyKeyValues(
    v_topic topic,
    v_message dst,
    v_message src)
{
    c_array keyFields;
    c_long nKeys;
    c_field field;
    c_long i;

    assert(C_TYPECHECK(topic, v_topic));
    assert(C_TYPECHECK(dst, v_message));
    assert(C_TYPECHECK(src, v_message));

    keyFields = v_topicMessageKeyList(topic);
    nKeys = c_arraySize(keyFields);
    for (i = 0; i < nKeys; i++) {
        field = (c_field)keyFields[i];
        c_fieldCopy(field, (c_object)src, field, (c_object)dst);
    }
}

/* ES, dds1576 added */
v_accessMode
v_partitionDetermineTopicAccessMode(
    const c_char* topicName,
    v_kernel kernel)
{
    v_configuration config;
    v_cfElement root;
    v_cfElement element;
    c_iter iter;
    v_accessMode retVal = V_ACCESS_MODE_UNDEFINED;
    c_value expression;
    c_value accessMode;

    config = v_getConfiguration(kernel);
    if(config)
    {
        root = v_configurationGetRoot(config);
        /* Iterate over all TopicAccess elements */
        iter = v_cfElementXPath(root, "Domain/TopicAccess");
        while(c_iterLength(iter) > 0)
        {
            element = v_cfElement(c_iterTakeFirst(iter));
            /* Get the topic expression value, it should be a string */
            expression = v_cfElementAttributeValue(element, "topic_expression");
            if(expression.kind == V_STRING)
            {
                if(v_partitionStringMatchesExpression(topicName, expression.is.String))
                {
                    /* The topic matches the expression.*/
                    accessMode = v_cfElementAttributeValue(element, "access_mode");
                    if(accessMode.kind == V_STRING)
                    {
                        /* A correct solution space can be realized between multiple
                         * expressions having an AND relationship by specifying the
                         * following rules R&W=RW, R&N=N, W&N=N, RW&N=N.
                         */
                        switch(retVal)
                        {
                            case V_ACCESS_MODE_UNDEFINED: /* start state */
                                if(strcmp(accessMode.is.String, "none") == 0)
                                {
                                    retVal = V_ACCESS_MODE_NONE;
                                } else if(strcmp(accessMode.is.String, "write") == 0)
                                {
                                    retVal = V_ACCESS_MODE_WRITE;
                                } else if(strcmp(accessMode.is.String, "read") == 0)
                                {
                                    retVal = V_ACCESS_MODE_READ;
                                } else if(strcmp(accessMode.is.String, "readwrite") == 0)
                                {
                                    retVal = V_ACCESS_MODE_READ_WRITE;
                                }
                                break;
                            case V_ACCESS_MODE_WRITE:
                                if(strcmp(accessMode.is.String, "read") == 0 ||
                                   strcmp(accessMode.is.String, "readwrite") == 0)
                                {
                                    retVal = V_ACCESS_MODE_READ_WRITE;
                                } else if(strcmp(accessMode.is.String, "none") == 0)
                                {
                                    retVal = V_ACCESS_MODE_NONE;
                                }
                                break;
                            case V_ACCESS_MODE_READ:
                                if(strcmp(accessMode.is.String, "write") == 0 ||
                                   strcmp(accessMode.is.String, "readwrite") == 0)
                                {
                                    retVal = V_ACCESS_MODE_READ_WRITE;
                                } else if(strcmp(accessMode.is.String, "none") == 0)
                                {
                                    retVal = V_ACCESS_MODE_NONE;
                                }
                                break;
                            case V_ACCESS_MODE_READ_WRITE:
                                if(strcmp(accessMode.is.String, "none") == 0)
                                {
                                    retVal = V_ACCESS_MODE_NONE;
                                }
                                break;
                            default: /* case V_ACCESS_MODE_NONE > none always remains none */
                                break;
                        }
                    }
                }
            }
        }
        if(iter)
        {
            c_iterFree(iter);
        }
        if(root)
        {
            c_free(root);
        }
    }
    if(retVal == V_ACCESS_MODE_UNDEFINED)
    {
        /* No specific rights defined, fall back to default */
        retVal = V_ACCESS_MODE_READ_WRITE;
    }
    return retVal;
}

v_result
v_topicDisposeAllData(
    v_topic topic)
{
    v_message msg;
    v_participant participant;
    v_kernel kernel;
    v_result res = V_RESULT_OUT_OF_MEMORY;

    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(topic);
    participant = kernel->builtin->participant;
    msg = v_participantCreateCandMCommand( participant );
    if ( msg != NULL )
    {
        res = v_participantCandMCommandSetDisposeAllData( participant,
                                                          msg,
                                                          v_entity(topic)->name,
                                                          "*" );
        if ( res == V_RESULT_OK )
        {
           res = v_participantWriteCandMCommand( participant, msg );
        }

        c_free(msg);
    }
    return (res);
}

c_type
v_topicGetUserType (
    v_topic topic)
{
    c_type type = NULL;
    c_metaObject obj = NULL;
    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));
    obj = c_metaResolve(c_metaObject(v_topicMessageType(topic)),USERDATA_FIELD_NAME);
    type = c_property(obj)->type;
    c_free(obj);
    return type;
}
