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
#include "d_topicInfo.h"
#include "d_storeMMF.h"
#include "u_entity.h"
#include "sd_serializerXMLMetadata.h"
#include "sd_serializerXMLTypeinfo.h"
#include "sd_serializerXML.h"
#include "v_entity.h"
#include "v_topic.h"
#include "c_stringSupport.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_abstract.h"
#include "c_clone.h"


#define USERDATA_FIELD_NAME "userData"
static c_type
messageTypeNew(
    c_base base,
    const c_char *typeName)
{
    c_metaObject o;
    c_type baseType,dataType,type, foundType;
    c_char *name;
    c_long length, sres;

    if (base == NULL) {
        return NULL;
    }
    dataType = c_resolve(base,typeName);
    if (dataType == NULL) {
        return NULL;
    }
    baseType = c_resolve(base, "kernelModule::v_message");
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


#define MESSAGE_FORMAT "v_message<%s>"
#define MESSAGE_NAME "v_message<>"
    length = sizeof(MESSAGE_NAME) + strlen(typeName);
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
#define KEY_NAME   "<Key>"
#define KEY_FORMAT "%s<Key>"

    if (name != NULL) {
        length = sizeof(KEY_NAME) + strlen(name);
        typeName = os_malloc(length);
        sres = snprintf(typeName,length,KEY_FORMAT,name);
        assert(sres == (length-1));
    } else {
        assert(FALSE);
        length = 100;
        typeName = os_malloc(length);
        os_sprintf(typeName,PA_ADDRFMT KEY_NAME,(c_address)o);
    }
#undef KEY_NAME
#undef KEY_FORMAT
    foundType = c_type(c_metaBind(c_metaObject(base),typeName,o));

    c_free(o);
    os_free(typeName);

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

static c_type
createTopicKeyType (
    c_type  messageType,
    const c_char *keyExpr)
{
    c_string typeName;
    c_char *name;
    c_long length;
    c_type keyType;
    c_array keyList;
    c_bool proceed;

    keyList = NULL;
    proceed = createMessageKeyList(messageType, keyExpr, &keyList);

    if (proceed) {
        typeName = c_metaName(c_metaObject(messageType));
        length = strlen(typeName) + strlen(keyExpr) + 3;
        name = os_alloca(length);
        snprintf(name,length,"%s<%s>",typeName,keyExpr);
        keyType = createKeyType(name,keyList);
        c_free(typeName);
        os_freea(name);
    } else {
        keyType = NULL;
    }
    c_free(keyList);

    return keyType;
}


static c_type
createInstanceType(
    c_type messageType,
    d_topicInfo topicInfo)
{
    c_type instanceType, baseType, foundType;
    c_metaObject o;
    c_char *name;
    c_long length,sres;
    c_base base;

    base = c_getBase(messageType);
    baseType = c_resolve(base,"durabilityModule2::d_instanceTemplate");
    assert(baseType != NULL);

    instanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(instanceType)->extends = c_class(baseType);

    foundType = createTopicKeyType(messageType, topicInfo->keyExpr);

    if ( foundType != NULL) {
        o = c_metaDeclare(c_metaObject(instanceType),"key",M_ATTRIBUTE);
        c_property(o)->type = foundType;
        c_free(o);
    }
    c_metaObject(instanceType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(instanceType));

#define INSTANCE_NAME "d_instance<d_sample<>>"
#define INSTANCE_FORMAT "d_instance<d_sample<%s>>"

    length = sizeof(INSTANCE_NAME) + strlen(topicInfo->name);
    name = os_malloc(length);
    sres = snprintf(name,length,INSTANCE_FORMAT,topicInfo->name);
    assert(sres == (length-1));
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT

    foundType = c_type(c_metaBind(c_metaObject(base),
                                  name,
                                  c_metaObject(instanceType)));
    os_free(name);
    c_free(instanceType);

    return foundType;
}


static c_char *
createInstanceKeyExpr (
    c_base base,
    v_topic topic)
{
    c_char fieldName[16];
    c_char *keyExpr;
    c_long i,nrOfKeys,totalSize;
    c_array keyList;

    assert(C_TYPECHECK(topic,v_topic));
    keyList = v_topicMessageKeyList(topic);
    nrOfKeys = c_arraySize(keyList);
    if (nrOfKeys>0) {
        totalSize = nrOfKeys * strlen("key.field0,");
        if (nrOfKeys > 9) {
            totalSize += (nrOfKeys-9);
            if (nrOfKeys > 99) {
                totalSize += (nrOfKeys-99);
            }
        }
        keyExpr = c_stringMalloc(base, totalSize);
        keyExpr[0] = 0;
        for (i=0;i<nrOfKeys;i++) {
            os_sprintf(fieldName,"key.field%d",i);
            os_strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { os_strcat(keyExpr,","); }
        }
    } else {
        keyExpr = NULL;
    }
    return keyExpr;
}

static c_type
createSampleType(
    c_type messageType,
    const c_char* topicName)
{
    c_base base;
    c_type sampleType, baseType, found, foundType = NULL;
    c_metaObject o;
    c_char *name;
    c_long length,sres;

    assert(messageType);
    assert(topicName);

#define SAMPLE_NAME   "d_sample<>"
#define SAMPLE_FORMAT "d_sample<%s>"
    /* Create a name and bind type to name */
    /* The sizeof contains \0 */
    length = sizeof(SAMPLE_NAME) + strlen(topicName);
    name = os_malloc(length);
    if(name){
        sres = snprintf(name, length, SAMPLE_FORMAT, topicName);
        assert(sres == (length-1));
#undef SAMPLE_NAME
#undef SAMPLE_FORMAT

        base = c_getBase(messageType);
        baseType = c_resolve(base, "durabilityModule2::d_sample");
        assert(baseType != NULL);

        sampleType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
        c_class(sampleType)->extends = c_class(baseType);

        o = c_metaDefine(c_metaObject(sampleType),M_ATTRIBUTE);
        c_property(o)->type = c_keep(messageType);
        found = c_type(c_metaBind(c_metaObject(sampleType),"message",o));
        c_free(o);
        c_free(found);

        c_metaObject(sampleType)->definedIn = c_keep(base);
        c_metaFinalize(c_metaObject(sampleType));

        foundType = c_type(c_metaBind(c_metaObject(base),
                                      name,
                                      c_metaObject(sampleType)));

        c_free(sampleType);
        os_free(name);
    }

    return foundType;
}

#if 0
static c_type
cloneType(
    c_base src,
    c_base dst,
    c_type object)
{
    sd_serializer serializer, deserializer;
    sd_serializedData data;
    c_char* xmlData;
    c_type result;

    result = NULL;
    serializer = sd_serializerXMLMetadataNew(src);
    deserializer = sd_serializerXMLMetadataNew(dst);

    if(serializer && deserializer){
        data = sd_serializerSerialize(serializer, object);

        if(data){
            xmlData = sd_serializerToString(serializer, data);
            sd_serializedDataFree(data);

            if(xmlData){
                data = sd_serializerFromString(deserializer, xmlData);

                if(data){
                    result = c_type(sd_serializerDeserializeValidated(
                            deserializer, data));
                    sd_serializedDataFree(data);
                }
                os_free(xmlData);
            }
        }
        sd_serializerFree(serializer);
        sd_serializerFree(deserializer);
    }
    return result;
}
#elif 0
static c_type
cloneType(
    c_base src,
    c_base dst,
    c_type object)
{
    sd_serializer serializer, deserializer;
    sd_serializedData data;
    c_char* xmlData;
    c_type result;

    result = NULL;
    serializer = sd_serializerXMLTypeinfoNew(src, FALSE);
    deserializer = sd_serializerXMLTypeinfoNew(dst, FALSE);

    if(serializer && deserializer){
        data = sd_serializerSerialize(serializer, object);

        if(data){
            xmlData = sd_serializerToString(serializer, data);
            sd_serializedDataFree(data);

            if(xmlData){
                data = sd_serializerFromString(deserializer, xmlData);

                if(data){
                    result = c_type(sd_serializerDeserializeValidated(
                            deserializer, data));
                    sd_serializedDataFree(data);
                }
                os_free(xmlData);
            }
        }
        sd_serializerFree(serializer);
        sd_serializerFree(deserializer);
    }
    return result;
}
#else
static c_type
cloneType(
    c_base src,
    c_base dst,
    c_type object)
{
    c_clone c;
    c_type type;

    OS_UNUSED_ARG(src);

    c = c_cloneNew(dst);

    type = c_cloneCloneObject(c, object);

    c_cloneFree(c);

    return type;
}
#endif

static c_object
cloneObject(
    c_base src,
    c_base dst,
    c_object object)
{
    sd_serializer serializer, deserializer;
    sd_serializedData data;
    c_char* xmlData;
    c_type result;

    result = NULL;
    serializer = sd_serializerXMLNew(src);
    deserializer = sd_serializerXMLNew(dst);

    if(serializer && deserializer){
        data = sd_serializerSerialize(serializer, object);

        if(data){
            xmlData = sd_serializerToString(serializer, data);
            sd_serializedDataFree(data);

            if(xmlData){
                data = sd_serializerFromString(deserializer, xmlData);

                if(data){
                    result = c_type(sd_serializerDeserializeValidated(deserializer, data));
                    sd_serializedDataFree(data);
                }
                os_free(xmlData);
            }
        }
        sd_serializerFree(serializer);
        sd_serializerFree(deserializer);
    }
    return result;
}

d_topicInfo
d_topicInfoNew(
    d_storeMMFKernel kernel,
    const v_topic vtopic)
{
    d_topicInfo topic;
    c_base base, ddsBase;
    c_type type, srcDataType;
    c_string name;

    if(kernel && vtopic){
        base = c_getBase(kernel);
        ddsBase = c_getBase(vtopic);
        type = c_resolve(base,"durabilityModule2::d_topicInfo");

        if(type){
            topic = c_new(type);
            c_free(type);

            if(topic){
                topic->name = c_stringNew(base, v_entity(vtopic)->name);

                srcDataType = v_topicDataType(vtopic);

                name = c_metaScopedName(c_metaObject(srcDataType));
                topic->typeName = c_stringNew(base, name);
                assert(topic->typeName);
                os_free(name);

                topic->dataType = cloneType(ddsBase, base, srcDataType);
                assert(topic->dataType);

                topic->keyExpr = c_stringNew(base, vtopic->keyExpr);
                assert(topic->keyExpr);

                topic->messageType = messageTypeNew(base, topic->typeName);
                assert(topic->messageType);

                topic->keyType = createTopicKeyType(topic->messageType,
                        topic->keyExpr);
                assert(topic->keyType);

                topic->instanceKeyExpr = createInstanceKeyExpr(base, vtopic);

                topic->sampleType = createSampleType(
                        topic->messageType, topic->name);
                assert(topic->sampleType);

                topic->instanceType = createInstanceType(
                        topic->messageType, topic);
                assert(topic->instanceType);

                topic->qos = cloneObject(ddsBase, base, vtopic->qos);
                assert(topic->qos);
            }
        } else {
            OS_REPORT(OS_ERROR,
                    "d_topicInfoNew",0,
                    "Failed to allocate d_topicInfo.");
            topic = NULL;
        }
    } else {
        OS_REPORT(OS_ERROR,
                "d_topicInfoNew",0,
                "Illegal constructor parameter.");
        topic = NULL;
    }
    return topic;
}

c_string
d_topicInfoGetName(
    d_topicInfo topicInfo)
{
    c_string name;

    if(topicInfo){
        name = c_keep(topicInfo->name);
    } else {
        name = NULL;
    }
    return name;
}

c_type
d_topicInfoGetMessageType(
    d_topicInfo topicInfo)
{
    c_type result;

    if(topicInfo){
        result = c_keep(topicInfo->messageType);
    } else {
        result = NULL;
    }
    return result;
}

c_type
d_topicInfoGetInstanceType(
    d_topicInfo topicInfo)
{
    c_type result;

    if(topicInfo){
        result = c_keep(topicInfo->instanceType);
    } else {
        result = NULL;
    }
    return result;
}

c_string
d_topicInfoGetInstanceKeyExpr(
    d_topicInfo topicInfo)
{
    c_string result;

    if(topicInfo){
        result = c_keep(topicInfo->instanceKeyExpr);
    } else {
        result = NULL;
    }
    return result;
}

d_storeResult
d_topicInfoInject(
    d_topicInfo _this,
    d_store store,
    u_participant participant)
{
    d_storeResult result;
    c_type type;
    struct baseFind f;
    u_topic utopic;

    u_entityAction(u_entity(participant), d_storeGetBase, &f);

    type = cloneType(c_getBase(_this), f.base, _this->dataType);

    if(type){
        utopic = u_topicNew(participant, _this->name, _this->typeName,
                _this->keyExpr, _this->qos);

        if(utopic) {
           d_storeReport(store, D_LEVEL_FINE, "Topic %s created.\n", _this->name);
           (void)u_topicFree(utopic);
           result = D_STORE_RESULT_OK;
        } else {
           result = D_STORE_RESULT_METADATA_MISMATCH;
           d_storeReport(store, D_LEVEL_SEVERE,
               "Topic '%s' with typeName '%s' and keyList '%s' could NOT be created.\n",
               _this->name, _this->typeName, _this->keyExpr);
           OS_REPORT_3(OS_ERROR, "d_topicInfoInject", (os_int32)result,
               "Topic '%s' with typeName '%s' and keyList '%s' could NOT be created.\n",
               _this->name, _this->typeName, _this->keyExpr);

        }
    } else {
        result = D_STORE_RESULT_METADATA_MISMATCH;
        OS_REPORT_1(OS_ERROR,
                  "d_topicInfoInject",(os_int32)result,
                  "Failed to register type '%s'.", _this->typeName);

    }
    return result;
}
