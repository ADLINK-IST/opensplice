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

#ifdef _EXTENT_
#include "c_extent.h"
#endif
#include "sd_serializerXMLTypeinfo.h"

#include "c_stringSupport.h"
#include "os.h"
#include "os_report.h"

#define USERDATA_FIELD_NAME "userData"

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
        return NULL;
    }
    dataType = c_resolve(base,typeName);
    if (dataType == NULL) {
        return NULL;
    }
    baseType = v_kernelType(kernel,K_MESSAGE);
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
    c_type fieldType;
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

    fieldType = c_field_t(c_getBase(messageType));
    keyList = c_arrayNew(fieldType,length);
    c_free(fieldType);
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
            sprintf(keyName,"field%d",i);
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
        sprintf(typeName,PA_ADDRFMT"<Key>",(c_address)o);
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

static c_bool
topicConsistent(
    v_topic topic,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyExpr,
    v_topicQos qos)
{
    c_bool consistent = FALSE;
    c_char *rTypeName;

    if (c_compareString(v_topicName(topic), name) == C_EQ) {
        /* only check when equal names */
        rTypeName = c_metaScopedName(c_metaObject(v_topicDataType(topic)));
        if (c_compareString(rTypeName, typeName) == C_EQ) {
            if (v_topicQosEqual(topic->qos, qos, OS_API_INFO)) {
                consistent = keysConsistent(topic, keyExpr);
            }
        } else {
            OS_REPORT_2(OS_API_INFO, "topicConsistent", 21,
                        "Topics typenames %s and %s are different",
                        rTypeName, typeName);
        }
        os_free(rTypeName);
    } else {
        OS_REPORT_2(OS_API_INFO, "topicConsistent", 21,
                    "Topics names %s and %s are different",
                    v_topicName(topic), name);
    }

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

v_topic
v__topicNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyExpr,
    v_topicQos qos,
    c_bool announce)
{
    v_topic topic,found;
    c_type type;
    c_array keyList;
    c_property field;
    v_topicQos newQos;
    c_char *str;
    v_message builtinMsg = NULL;

    assert(C_TYPECHECK(kernel,v_kernel));

    if (name == NULL) {
        return NULL;
    }

    newQos = v_topicQosNew(kernel,qos);
    if (newQos == NULL) {
        OS_REPORT(OS_ERROR, "v_topicNew", 0,
            "Topic not created: inconsistent qos");
        return NULL;
    }

    c_lockWrite(&kernel->lock);
    found = v__lookupTopic(kernel, name);
    if (found) {
        /* Check found topic with new topic */
        /* compare topic, if inconsistent, reject creation! */
        if (topicConsistent(found, name, typeName, keyExpr, newQos) == FALSE) {
            c_free(found);
            topic = NULL; /* rejected topic */
        } else {
            topic = found; /* accepted topic, transfer refCount */
        }
        /* newQos is not used because the existing topic is returned */
        v_topicQosFree(newQos);
    } else {
        type = messageTypeNew(kernel,typeName);
        if (type != NULL) {
            if (createMessageKeyList(type, keyExpr, &keyList) == FALSE) {
                topic = NULL; /* illegal key expression */
                v_topicQosFree(newQos);
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

                /* determine CRC codes */
                str = c_metaScopedName(c_metaObject(field->type));
                topic->crcOfName = v_crcCalculate(kernel->crc, name, strlen(name));
                topic->crcOfTypeName = v_crcCalculate(kernel->crc, str, strlen(str));
                os_free(str);
#ifdef _EXTENT_
#define _MAXTYPESIZE_ (3072)
                {
                    c_long count;
                    if (type->size < _MAXTYPESIZE_) {
                        count = 64;
                    } else {
                        count = 0;
                    }

                    topic->messageExtent = c_extentSyncNew(type,count,TRUE);
                }
#endif
                v_topicEnable(topic);

                if(announce){
                    builtinMsg = v_builtinCreateTopicInfo(kernel->builtin,topic);
                }
            }
            c_free(type);
        } else {
            topic = NULL;
            v_topicQosFree(newQos);
        }
    }
    c_lockUnlock(&kernel->lock);

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
        assert(found == topic);
        result = V_RESULT_OK;
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

#ifdef _EXTENT_
    message = (v_message)c_extentCreate(topic->messageExtent);
#else
    message = (v_message)c_new(v_topicMessageType(topic));
#endif
#ifndef _NAT_
    message->allocTime = v_timeGet();
#endif
    message->qos = NULL;
    V_MESSAGE_INIT(message);
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
            strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { strcat(keyExpr,","); }
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
        entities = c_select(participant->entities, 0);
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
        entities = c_select(participant->entities, 0);
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
    /* Do not use C_TYPECHECK on qos parameter, since it might be allocated on heap! */

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
