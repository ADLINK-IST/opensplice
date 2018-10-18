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
#include "v__topicImpl.h"
#include "v__topicQos.h"
#include "v_projection.h"
#include "v__kernel.h"
#include "v__entity.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_public.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v__builtin.h"
#include "v__status.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "ut_crc.h"
#include "v_event.h"
#include "v_policy.h"
#include "v__partition.h"
#include "v_configuration.h"
#include "v__policy.h"
#include "os_heap.h"
#include "v_topic.h"
#include "v__participant.h"

#include "sd_serializerXMLTypeinfo.h"

#include "c_stringSupport.h"
#include "vortex_os.h"
#include "os_report.h"
#include <ctype.h>

#define USERDATA_FIELD_NAME "userData"

static v_accessMode
v_partitionDetermineTopicAccessMode(
    const c_char* topicName,
    v_kernel kernel);

static c_type
messageTypeExtNew(
    v_kernel kernel,
    c_type dataType)
{
    c_base base;
    c_metaObject o;
    c_type type;

    static const char baseTypeName[] = "kernelModule::v_messageExt";

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    assert (base);

    type = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(type)->extends = c_class(c_resolve(base, baseTypeName));
    assert(c_class(type)->extends);
    o = c_metaDeclare(c_metaObject(type), USERDATA_FIELD_NAME, M_ATTRIBUTE);
    c_property(o)->type = dataType;
    c_free(o);
    c_metaObject(type)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(type));

    return type;
}

static c_type
messageTypeNew(
    v_kernel kernel,
    c_type dataType)
{
    c_base base;
    c_metaObject o;
    c_type baseType, type, foundType;
    c_char *typeName;
    c_char *name;
    os_size_t length;
    int sres;

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    assert (base);

    /* TODO: wrong place to do kernel locking, so v_kernelType should deal with it. */
    c_lockWrite(&kernel->lock);
    baseType = v_kernelType(kernel,K_MESSAGE);
    c_lockUnlock(&kernel->lock);
    assert(baseType != NULL);

    type = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(type)->extends = c_keep(c_class(baseType));
    o = c_metaDeclare(c_metaObject(type), USERDATA_FIELD_NAME, M_ATTRIBUTE);
    c_property(o)->type = dataType;
    c_free(o);
    c_metaObject(type)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(type));
    /* Create a name and bind type to name */

#define MESSAGE_FORMAT "v_message<%s>"
#define MESSAGE_NAME "v_message<>"
    typeName = c_metaScopedName(c_metaObject(dataType));
    length = sizeof(MESSAGE_NAME) + strlen(typeName); /* sizeof contains \0 */
    name = os_malloc(length);
    sres = snprintf(name,length,MESSAGE_FORMAT,typeName);
    assert(sres >= 0 && (os_size_t) sres == (length-1));
    OS_UNUSED_ARG(sres);
    os_free(typeName);
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
    c_ulong i,length;
    int sres;

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
        size_t sz = strlen(PREFIX) + sizeof(PATH_SEPARATOR) + strlen(name);
        newName = (char *)os_malloc(sz);
        sres = snprintf(newName,sz,"%s"PATH_SEPARATOR"%s",PREFIX, name);
        assert(sres >= 0 && (os_size_t) sres == (sz-1));
        OS_UNUSED_ARG(sres);
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
            OS_REPORT(OS_ERROR,
                        "create message key list failed", V_RESULT_INTERNAL_ERROR,
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
    c_ulong i, nkeys;
    c_array members;
    c_metaObject o;
    c_field field;

    if (keyList == NULL) {
        return NULL;
    }
    base = c_getBase(keyList);
    nkeys = c_arraySize(keyList);
    if (nkeys == 0) {
        return NULL;
    }
    o = c_metaDefine(c_metaObject(base),M_STRUCTURE);
    members = c_arrayNew(c_member_t(base),nkeys);

    for (i=0;i<nkeys;i++) {
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
        os_size_t length = sizeof(KEY_NAME) + strlen(name);
        int sres;
        typeName = os_malloc(length);
        sres = snprintf(typeName,length,KEY_FORMAT,name);
        assert(sres >= 0 && (os_size_t) sres == (length-1));
        OS_UNUSED_ARG(sres);
#undef KEY_NAME
#undef KEY_FORMAT
    } else {
        assert(FALSE); /* Not supposed to happen anymore */
        typeName = os_malloc(100);
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

static os_boolean
keysConsistent(
    v_topicImpl topic,
    const char *keyExpr)
{
    c_iter keyExprNames, topicKeyNames;
    c_char *name, *found;
    os_boolean consistent;

    keyExprNames = c_splitString(keyExpr,", \t");
    topicKeyNames = c_splitString(topic->keyExpr,", \t");
    consistent = (c_iterLength(keyExprNames) == c_iterLength(topicKeyNames));
    name = c_iterTakeFirst(keyExprNames);
    while ((name != NULL) && consistent) {
        found = c_iterTakeAction(topicKeyNames,compare_names,name);
        if (!found) {
            consistent = OS_FALSE;
        }
        os_free(name);
        os_free(found);
        name = c_iterTakeFirst(keyExprNames);
    }
    name = c_iterTakeFirst(keyExprNames);
    while (name != NULL) {
        os_free(name);
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
v_topicImplKeyTypeCreate (
    v_topicImpl _this,
    const c_char *keyExpr,
    c_array *keyListRef)
{
    c_type keyType;
    c_array keyList;

    assert(_this);
    assert(C_TYPECHECK(_this,v_topicImpl));

    keyList = NULL;
    if (createMessageKeyList(_this->messageType, keyExpr, &keyList) == FALSE) {
        keyType = NULL;
    } else {
        c_char *typeName = c_metaName(c_metaObject(_this->messageType));
        os_size_t length = strlen(typeName) + strlen(keyExpr) + 3;
        c_char *name = os_alloca(length);
        snprintf(name,length,"%s<%s>",typeName,keyExpr);
        keyType = createKeyType(name,keyList);
        c_free(typeName);
        os_freea(name);
    }
    if (keyListRef) {
        *keyListRef = c_keep(keyList);
    }
    c_free(keyList);
    return keyType;
}

static c_bool
isValidName(const c_char *name)
{
    c_bool valid = FALSE;
    /* DDS Spec:
     *  |  TOPICNAME - A topic name is an identifier for a topic, and is defined as any series of characters
     *  |     ‘a’, ..., ‘z’,
     *  |     ‘A’, ..., ‘Z’,
     *  |     ‘0’, ..., ‘9’,
     *  |     ‘-’ but may not start with a digit.
     * It is considered that ‘-’ is an error in the spec and should say ‘_’. So, that's what we'll check for.
     */
    assert(name);
    if ((name[0] != '\0') && (!isdigit((unsigned char)name[0]))) {
        while (isalnum((unsigned char)*name) || (*name == '_') || (*name == '/')) {
            name++;
        }
        if (*name == '\0') {
            valid = TRUE;
        }
    }

   return valid;
}

static c_bool
isTopicConsistent(
    const v_topicImpl found,
    const c_char *name,
    const c_type type,
    const v_topicQos qos,
    const c_char *keyExpr)
{
    c_bool consistent = TRUE;

    if (c_compareString(v_topicName(found), name) != C_EQ) {
        OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Precondition not met: Create Topic \"%s\" failed: name <%s> differs existing name <%s>.",
                  name, name, v_topicName(found));
        return FALSE;
    }
    if (found->dataType != type) {
        c_char *typeName1 = c_metaScopedName(c_metaObject(found->dataType));
        c_char *typeName2 = c_metaScopedName(c_metaObject(type));
        OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Precondition not met: Create Topic \"%s\" failed: type <%s> differs existing type <%s>.",
                  name, typeName2, typeName1);
        os_free(typeName1);
        os_free(typeName2);
        return FALSE;
    }

    if (found->qos == NULL) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic \"%s\" failed: QoS value undefined", name);
        return FALSE;
    }

    if (!v_durabilityPolicyIEqual(found->qos->durability, qos->durability)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Durability'.", name);
        consistent = FALSE;
    }
    if (!v_durabilityServicePolicyIEqual(found->qos->durabilityService, qos->durabilityService)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'DurabilityService'.", name);
        consistent = FALSE;
    }
    if (!v_deadlinePolicyIEqual(found->qos->deadline, qos->deadline)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Deadline'.", name);
        consistent = FALSE;
    }
    if (!v_latencyPolicyIEqual(found->qos->latency, qos->latency)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Latency'.", name);
        consistent = FALSE;
    }
    if (!v_livelinessPolicyIEqual(found->qos->liveliness, qos->liveliness)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Liveliness'.", name);
        consistent = FALSE;
    }
    if (!v_reliabilityPolicyIEqual(found->qos->reliability, qos->reliability)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Reliability'.", name);
        consistent = FALSE;
    }
    if (!v_orderbyPolicyIEqual(found->qos->orderby, qos->orderby)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'OrderBy'.", name);
        consistent = FALSE;
    }
    if (!v_resourcePolicyIEqual(found->qos->resource, qos->resource)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Resource'.", name);
        consistent = FALSE;
    }
    if (!v_transportPolicyIEqual(found->qos->transport, qos->transport)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Transport'.", name);
        consistent = FALSE;
    }
    if (!v_lifespanPolicyIEqual(found->qos->lifespan, qos->lifespan)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Lifespan'.", name);
        consistent = FALSE;
    }
    if (!v_ownershipPolicyIEqual(found->qos->ownership, qos->ownership)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'Ownership'.", name);
        consistent = FALSE;
    }
    if (!v_historyPolicyIEqual(found->qos->history, qos->history)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: Unmatching QoS Policy: 'History'.", name);
        consistent = FALSE;
    }
    if (!keysConsistent(found, keyExpr)) {
        OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
                  "Create Topic <%s> failed: key \"%s\" doesn't match already existing Topic key \"%s\".",
                  name, keyExpr?keyExpr:"NULL", found->keyExpr?found->keyExpr:"NULL");
        consistent = FALSE;
    }
    return consistent;
}

v_topicImpl
v_topicImplNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyExpr,
    v_topicQos qos,
    c_bool announce)
{
    v_result result;
    v_topicImpl topic,found;
    c_type type;
    c_type msgType;
    c_array keyList;
    v_topicQos newQos;
    c_char *str;
    v_message builtinMsg = NULL;

    assert(C_TYPECHECK(kernel,v_kernel));
    if (name == NULL) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_ILL_PARAM,
                  "Topic '?' is not created. No name specified (NULL).");
        return NULL;
    }

    if (!isValidName(name)) {
        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_ILL_PARAM,
                  "Topic '%s' is not created. Name has invalid syntax.", name);
        return NULL;
    }

    if (v_topicQosCheck(qos) != V_RESULT_OK) {
        OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_ILL_PARAM,
                  "Topic '%s' is not created: Topic QoS is invalid.", name);
        return NULL;
    }
    type = c_resolve(c_getBase(kernel),typeName);
    if (type == NULL) {
        OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_ILL_PARAM,
                  "Topic '%s' is not created: Type '%s' is not found.", name, typeName);
        return NULL;
    }
    newQos = v_topicQosNew(kernel,qos);
    if (newQos == NULL) {
        OS_REPORT(OS_FATAL, "v_topicNew", V_RESULT_INTERNAL_ERROR,
                  "Topic '%s' is not created: failed to copy topic QoS.", name);
        c_free(type);
        return NULL;
    }

    topic = NULL;
    found = v_topicImpl(v_lookupTopic(kernel, name));
    if (found) {
        /* Check found topic with new topic */
        /* compare topic, if inconsistent, reject creation! */

        if(isTopicConsistent(found, name, type, newQos, keyExpr)) {
            topic = c_keep(found);
        }

        c_free(found);
        /* newQos is not used because the existing topic is returned */
        v_topicQosFree(newQos);
    } else {
        msgType = messageTypeNew(kernel,type);
        if (msgType != NULL) {
            if (createMessageKeyList(msgType, keyExpr, &keyList) == FALSE) {
                topic = NULL; /* illegal key expression */
                v_topicQosFree(newQos);
                OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_ILL_PARAM,
                            "Failed to create Topic '%s': invalid key expression '%s'.",
                            name, keyExpr);
            } else {
                topic = v_topicImpl(v_objectNew(kernel,K_TOPIC));
                v_entityInit(v_entity(topic), name);
                topic->messageExtType = messageTypeExtNew(kernel,type);
                topic->messageType = msgType;
                topic->messageKeyList = keyList;
                topic->keyType = createKeyType(name,keyList);
                topic->dataType = c_keep(type);
                topic->qos = newQos;
                topic->keyExpr = c_stringNew(c_getBase(kernel), keyExpr);
                topic->accessMode = v_partitionDetermineTopicAccessMode(name, kernel);

                /* determine CRC codes */
                str = c_metaScopedName(c_metaObject(type));
                topic->crcOfName = ut_crcCalculate(name, strlen(name));
                topic->crcOfTypeName = ut_crcCalculate(str, strlen(str));
                os_free(str);
                assert(found == NULL);
                result = v_entityEnable(v_entity(topic));
                switch(result)
                {
                    case V_RESULT_OK:
                        if(announce)
                        {
                            builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,v_topic(topic));
                        }
                        break;
                    case V_RESULT_PRECONDITION_NOT_MET:
                        /* Request is superfluous, so release previous topic and lookup existing precursor. */
                        c_free(topic);
                        topic = v_topicImpl(v_lookupTopic(kernel, name));
                        break;
                    case V_RESULT_INCONSISTENT_QOS:
                        c_free(topic);
                        topic = NULL;
                        OS_REPORT(OS_WARNING, "v_topicNew", V_RESULT_INCONSISTENT_QOS,
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
                        OS_REPORT(OS_ERROR, "v_topicNew", result,
                                "Failed to create Topic '%s': an unexpected error occurred.",
                                name);
                }
            }
            c_free(msgType);
        } else {
            topic = NULL;
            v_topicQosFree(newQos);
            OS_REPORT(OS_ERROR, "v_topicNew", V_RESULT_ILL_PARAM,
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
v__topicImplEnable (
    _Inout_ v_topicImpl topic)
{
    v_kernel kernel;
    v_topicImpl found;
    v_result result;

    assert(topic);
    assert(C_TYPECHECK(topic,v_topicImpl));

    kernel = v_objectKernel(topic);
    found = v_topicImpl(v__addTopic(kernel,v_topic(topic)));
    /* If the topic was already added to the kernel,
     * then the calling method should know so it can perform
     * a roll-back.
     */
    if(found != topic) {
        if (isTopicConsistent(found, v_topicName(topic), topic->dataType, topic->qos, topic->keyExpr)) {
            /* Compatible and already enabled topic found, so this request is superfluous.  */
            result = V_RESULT_PRECONDITION_NOT_MET;
        } else {
            result = V_RESULT_INCONSISTENT_QOS;
        }
    } else {
        result = V_RESULT_OK;
    }

    return result;
}

void
v_topicImplAnnounce(
    v_topicImpl topic)
{
    v_message builtinMsg;
    v_kernel kernel;

    assert(topic);
    assert(C_TYPECHECK(topic,v_topiImpl));
    /* publish V_TOPICINFO_TOPIC. */

    kernel = v_objectKernel(v_object(topic));
    /* TODO: wrong place to do kernel locking, so v_builtinCreateTopicInfo should deal with it. */
    c_lockWrite(&kernel->lock);
    builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,v_topic(topic));
    c_lockUnlock(&kernel->lock);
    v_writeBuiltinTopic(kernel, V_TOPICINFO_ID, builtinMsg);
    c_free(builtinMsg);


}

void
v_topicImplFree(
    v_topicImpl _this)
{
    assert(C_TYPECHECK(_this,v_topicImpl));

    v_entityFree(v_entity(_this));
}

void
v_topicImplDeinit(
    v_topicImpl _this)
{
    assert(C_TYPECHECK(_this,v_topicImpl));

    v_entityDeinit(v_entity(_this));
}

v_message
v_topicImplMessageNew(
    v_topicImpl topic)
{
    v_message message;

    assert(C_TYPECHECK(topic,v_topicImpl));

    message = (v_message)c_new(topic->messageType);
    if (message) {
        message->allocTime = os_timeEGet();
        message->qos = NULL;
        V_MESSAGE_INIT(message);
    } else {
        OS_REPORT(OS_FATAL,
                  "v_topicMessageNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate message.");
        assert(FALSE);
    }
    return message;
}

v_message
v_topicImplMessageNew_s(
    v_topicImpl topic)
{
    v_message message;

    assert(C_TYPECHECK(topic,v_topicImpl));

    message = (v_message)c_new_s(topic->messageType);
    if (message) {
        message->allocTime = os_timeEGet();
        message->qos = NULL;
        V_MESSAGE_INIT(message);
    } else {
        OS_REPORT(OS_FATAL,
                  "v_topicMessageNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate message.");
    }
    return message;
}

c_char *
v_topicImplMessageKeyExpr(
    v_topicImpl topic)
{
    c_string fieldName;
    c_char *keyExpr;
    c_ulong i,nrOfKeys;
    c_size totalSize;
    c_array keyList;

    assert(C_TYPECHECK(topic,v_topicImpl));

    keyList = topic->messageKeyList;
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
v_topicImplLookupWriters(
    v_topicImpl topic)
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
        entities = v_participantGetEntityList(participant);

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
v_topicImplLookupReaders(
    v_topicImpl topic)
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
        entities = v_participantGetEntityList(participant);

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
v_topicImplGetQos(
    v_topicImpl _this)
{
    v_topicQos qos;

    assert(_this);
    assert(C_TYPECHECK(_this,v_topicImpl));

    OSPL_LOCK(_this);
    qos = c_keep(_this->qos);
    OSPL_UNLOCK(_this);

    return qos;
}

v_result
v_topicImplSetQos(
    v_topicImpl _this,
    v_topicQos tmpl)
{
    v_result result;
    v_topicQos qos;
    v_qosChangeMask cm;
    v_message builtinMsg = NULL;
    v_kernel kernel;

    assert(C_TYPECHECK(_this,v_topicImpl));
    /* Do not use C_TYPECHECK on qos parameter,
     * since it might be allocated on heap!
     */
    result = v_topicQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        OSPL_LOCK(_this);
        kernel = v_objectKernel(_this);
        qos = v_topicQosNew(kernel, tmpl);
        if (!qos) {
            OSPL_UNLOCK(_this);
            return V_RESULT_OUT_OF_MEMORY;
        }
        result = v_topicQosCompare(_this->qos, qos, v__entityEnabled_nl(v_entity(_this)), &cm);
        if ((result == V_RESULT_OK) && (cm != 0)) {
            c_free(_this->qos);
            _this->qos = c_keep(qos);
            builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,v_topic(_this));
        }
        OSPL_UNLOCK(_this);
        if (builtinMsg != NULL) {
            v_writeBuiltinTopic(kernel, V_TOPICINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
        c_free(qos);
    }
    return result;
}

void
v_topicImplNotify(
    v_topicImpl topic,
    v_event event,
    c_voidp userData)
{
    OS_UNUSED_ARG(topic);
    OS_UNUSED_ARG(event);
    OS_UNUSED_ARG(userData);
    assert(C_TYPECHECK(topic,v_topicImpl));
}

void
v_topicImplNotifyInconsistentTopic(
    v_topicImpl topic)
{
    C_STRUCT(v_event) e;

    assert(C_TYPECHECK(topic,v_topicImpl));

    v_statusNotifyInconsistentTopic(v_entity(topic)->status);

    e.kind = V_EVENT_INCONSISTENT_TOPIC;
    e.source = v_observable(topic);
    e.data = NULL;

    e.handled = v_entityNotifyListener(v_entity(topic), &e);
    OSPL_THROW_EVENT(topic, &e);
}

void
v_topicImplNotifyAllDataDisposed(
    v_topicImpl topic)
{
    C_STRUCT(v_event) e;

    assert(C_TYPECHECK(topic,v_topicImpl));

    v_statusNotifyAllDataDisposed(v_entity(topic)->status);
    e.kind = V_EVENT_ALL_DATA_DISPOSED;
    e.source = v_observable(topic);
    e.data = NULL;
    e.handled = v_entityNotifyListener(v_entity(topic), &e);
    OSPL_THROW_EVENT(topic, &e);
}

v_result
v_topicImplGetInconsistentTopicStatus(
    v_topicImpl _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(C_TYPECHECK(_this,v_topicImpl));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_topicStatus(status)->inconsistentTopic, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_INCONSISTENT_TOPIC);
        }
        v_topicStatus(status)->inconsistentTopic.totalChanged = 0;
        OSPL_UNLOCK(_this);
    }

    return result;
}

v_result
v_topicImplGetAllDataDisposedStatus(
    v_topicImpl _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(C_TYPECHECK(_this,v_topicImpl));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        OSPL_LOCK(_this);
        status = v_entity(_this)->status;
        result = action(&v_topicStatus(status)->allDataDisposed, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_ALL_DATA_DISPOSED);
        }
        v_topicStatus(status)->allDataDisposed.totalChanged = 0;
        OSPL_UNLOCK(_this);
    }

    return result;
}

void
v_topicImplMessageCopyKeyValues(
    v_topicImpl topic,
    v_message dst,
    v_message src)
{
    c_array keyFields;
    c_ulong nKeys;
    c_field field;
    c_ulong i;

    assert(C_TYPECHECK(topic, v_topicImpl));
    assert(C_TYPECHECK(dst, v_message));
    assert(C_TYPECHECK(src, v_message));

    keyFields = topic->messageKeyList;
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
v_topicImplDisposeAllData(
    v_topicImpl _this)
{
    v_message msg;
    v_participant participant;
    v_kernel kernel;
    v_result res = V_RESULT_OUT_OF_MEMORY;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_topicImpl));

    kernel = v_objectKernel(_this);
    participant = kernel->builtin->participant;
    msg = v_participantCreateCandMCommand( participant );
    if ( msg != NULL )
    {
        res = v_participantCandMCommandSetDisposeAllData( participant,
                                                          msg,
                                                          v_entity(_this)->name,
                                                          "*" );
        if ( res == V_RESULT_OK )
        {
           res = v_participantWriteCandMCommand( participant, msg );
        }

        if ( res == V_RESULT_OK )
        {
            res = v_kernelDisposeAllData(kernel, "*", v_entity(_this)->name, msg->writeTime);
        }

        if ( res != V_RESULT_OK )
        {
            OS_REPORT(OS_WARNING, "topic", V_RESULT_INTERNAL_ERROR,
                      "Dispose All Data failed due to internal error.");
        }

        c_free(msg);
    }
    return (res);
}

os_char *
v_topicImplMetaDescriptor (
    v_topicImpl _this)
{
    sd_serializer serializer;
    sd_serializedData serData;
    c_base base;
    c_metaObject obj = NULL;
    c_type type = NULL;
    os_char *descriptor = NULL;

    if (_this) {
        assert(C_TYPECHECK(_this,v_topicImpl));

        obj = c_metaResolve(c_metaObject(_this->messageType),USERDATA_FIELD_NAME);
        if (obj != NULL) {
            type = c_property(obj)->type;
            base = c_getBase(type);
            if (base) {
                serializer = sd_serializerXMLTypeinfoNew(base, FALSE);
                if ( serializer ) {
                    serData = sd_serializerSerialize(serializer, (c_object)type);
                    if ( serData ) {
                        descriptor = sd_serializerToString(serializer, serData);
                        sd_serializedDataFree(serData);
                    }
                    sd_serializerFree(serializer);
                }
            }
            c_free(obj);
        }
    }
    return descriptor;
}

v_result
v_topicImplFillTopicInfo (
    struct v_topicInfo *info,
    v_topicImpl topic)
{
    c_base base = c_getBase (c_object (topic));
    v_topicQos topicQos = topic->qos;
    v_result result;
    info->key.systemId = topic->crcOfName;
    info->key.localId = topic->crcOfTypeName;
    info->key.serial = 0;
    v_policyConvToExt_topic_name (&info->name, v_topicName (topic));
    if ((result = v_policyConvToExt_type_name (base, &info->type_name, topic->dataType)) != V_RESULT_OK) {
        return result;
    }
    if ((result = v_topicQosFillTopicInfo (info, topicQos)) != V_RESULT_OK) {
        c_free (info->type_name);
        info->type_name = NULL;
        return result;
    }
    if ((result = v_policyConvToExt_topic_meta_data (base, &info->meta_data, &info->key_list, topic->dataType, topic->keyExpr)) != V_RESULT_OK) {
        c_free (info->type_name);
        info->type_name = NULL;
        return result;
    }
    return V_RESULT_OK;
}

v_topicImpl
v_topicImplNewFromTopicInfo (
    v_kernel kernel,
    const struct v_topicInfo *info,
    c_bool announce)
{
    v_topicQos qos;
    v_topicImpl newTopic = NULL;
    sd_serializer serializer;
    sd_serializedData meta_data;
    c_type topicType = NULL;
    c_char *msg;
    c_char *loc;

    assert(C_TYPECHECK(kernel,v_kernel));

    serializer = sd_serializerXMLTypeinfoNew(c_getBase(c_object(kernel)), FALSE /* do not escape " characters */);
    if (serializer != NULL) {
        meta_data = sd_serializerFromString(serializer, info->meta_data);
        if (meta_data != NULL) {
            topicType = c_type(sd_serializerDeserialize(serializer, meta_data));
            if (topicType == NULL) {
                msg = sd_serializerLastValidationMessage(serializer);
                loc = sd_serializerLastValidationLocation(serializer);
                if (loc == NULL) {
                    OS_REPORT(OS_ERROR, "v_topicImplNewFromTopicInfo", 0,
                              "Deserialization of type failed: "
                              "%s at <unknown>", msg);
                } else {
                    OS_REPORT(OS_ERROR, "v_topicImplNewFromTopicInfo", 0,
                              "Deserialization of type failed: "
                              "%s at %s", msg, loc);
                }
            }
            sd_serializedDataFree(meta_data);
        } else {
            OS_REPORT(OS_ERROR, "v_topicNewFromTopicInfo", 0,
                      "Failed to create serializedData object");
        }
        sd_serializerFree(serializer);
    } else {
        OS_REPORT(OS_ERROR, "v_topicNewFromTopicInfo", 0,
                  "Failed to create serializerXMLTypeinfoNew");
    }

    if (topicType != NULL) {
        qos = v_topicQosFromTopicInfo(c_getBase (kernel), info);
        newTopic = v_topicImplNew(kernel, info->name, info->type_name, info->key_list, qos, announce);
        c_free(qos);
    }
    return newTopic;
}
