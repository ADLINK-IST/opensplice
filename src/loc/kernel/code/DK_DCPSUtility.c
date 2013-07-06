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
/* C includes */
#include <assert.h>

/* kernel includes */
#include "v_dataReader.h"
#include "v_public.h"
#include "v_state.h"
#include "v_topic.h"
#include "v_writer.h"

/* user layer includes */
#include "u_instanceHandle.h"
#include "u_instanceHandle.h"
#include "u_entity.h"
#include "u_query.h"
#include "u_reader.h"
#include "u_time.h"

/* user layer 'private' includes */
#include "u__types.h"

/* DLRL utilities includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLRelation.h"
#include "DMM_DCPSField.h"

/* DLRL kernel includes */
#include "DK_DCPSUtility.h"
#include "DK_ObjectAdmin.h"

/* \brief Utility function which resolves all relevant DCPS (internal) topic info within a <code>DK_TopicInfo</code>
 * object.
 *
 * Each <code>DK_TopicInfo</code> object caches certain DCPS topic objects which can be used for looking up instance
 * handles during the main loop of the DLRL. This utility function will create the required objects and store them with
 * the <code>DK_TopicInfo</code> object which was provided as the 'arg' parameter of this operation. The DCPS DataSample
 * ,DCPS DataSampleOffset and the DCPS message are the objects/values created and stored by this operation.
 *
 * Preconditions:<ul>
 * <li>Must lock the admin mutex of the home to which the topic info object provided within the
 * <code>LookupInstanceHolder</code> struct is locked.</li></ul>
 *
 * \param reader The data reader which reads samples of the topic represented by the <code>DK_TopicInfo</code> object
 * found in the 'arg' parameter.
 * \param arg The <code>DK_TopicInfo</code> object for which the DCPS data needs to be resolved and stored.
 */
static void
DK_DCPSUtility_us_resolveDatabaseTopicPlaceholders(
    v_entity reader,
    c_voidp arg);

/* NOT IN DESIGN */
static void
DK_DCPSUtility_us_createMessage(
    v_entity e,
    c_voidp arg);

typedef struct DK_DCPSUtility_ProxyCreationHolder_s
{
    u_entity proxyEntity;
    u_participant participant;
} DK_DCPSUtility_ProxyCreationHolder;

static void
DK_DCPSUtility_us_createProxyUserEntityAction(
    v_entity e,
    c_voidp arg);

/* \brief This utility function has the signature of a readerCopy function which can be used within DCPS data readers.
 *
 * This function is intended to be used to set the readerCopy function of a data reader to do nothing, usefull when
 * taking an instance from DCPS, when one does not want to copy any data, but just clear the data from the database.
 * In principle this function should only be used as a data reader readerCopy function, but since it does nothing it is
 * less relevant.
 *
 * \param object The sample
 * \param arg user defined args
 */
static v_actionResult
DK_DCPSUtility_us_emptyReaderCopy(
    c_object object,
    c_voidp arg);

/* NOT IN DESIGN */
static void
DK_DCPSUtility_us_verifyHandleResult(
    DLRL_Exception* exception,
    u_result result);

/* NOT IN DESIGN */
typedef struct
DK_DCPSUtilityMessageHolder_s
{
    DLRL_Exception* exception;
    v_message message;
    c_long offset;
} DK_DCPSUtilityMessageHolder;

/* NOT IN DESIGN */
static void
DK_DCPSUtility_us_createTakeDisposedNotNewInstanceQuery(
    v_entity entity,
    c_voidp arg);

/* NOT IN DESIGN */
static void
DK_DCPSUtility_us_takeDisposedNotNewInstanceCallback(
    c_object object,
    c_value arg,
    c_value *result);

v_actionResult
DK_DCPSUtility_us_emptyReaderCopy(
    c_object object,
    c_voidp arg)
{
    v_public kernelPublic = NULL;
    v_actionResult result = V_PROCEED;

    DLRL_INFO(INF_ENTER);

    assert(arg);

    if(object && (*((LOC_boolean*)arg)))
    {
        kernelPublic = v_public(v_readerSampleInstance(object));
        kernelPublic->userDataPublic = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return result;
}

void
DK_DCPSUtility_us_takeInstanceFromDatabase(
    DLRL_Exception* exception,
    u_reader reader,
    u_instanceHandle handle,
    LOC_boolean resetHandle)
{
    u_result result = U_RESULT_OK;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(!u_instanceHandleIsNil(handle));
    assert(reader);

    result = u_readerTakeInstance(reader, handle,
                                  DK_DCPSUtility_us_emptyReaderCopy,
                                  (c_voidp)&resetHandle);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "DCPS take failed.");

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_lookupInstance(
    v_entity reader,
    c_voidp arg)
{
    struct LookupInstanceHolder* holder = (struct LookupInstanceHolder*)arg;
    v_message message = NULL;
    v_dataReaderInstance dataReaderInstance = NULL;

    DLRL_INFO(INF_ENTER);

    assert(reader);
    assert(arg);

    message = DK_TopicInfo_us_getMessage(holder->topicInfo);
    dataReaderInstance = v_dataReaderLookupInstance((v_dataReader)reader,
                                                     message);

    if(dataReaderInstance != NULL)
    {
        holder->relationHandle = u_instanceHandleNew(v_public(dataReaderInstance));
        c_free(dataReaderInstance);
    }
    DLRL_INFO(INF_EXIT);
}

u_entity
DK_DCPSUtility_ts_createProxyUserEntity(
    DLRL_Exception* exception,
    u_entity entity)
{
    DK_DCPSUtility_ProxyCreationHolder holder;
    u_result result;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(entity);

    holder.proxyEntity = NULL;
    holder.participant = u_entityParticipant (entity);
    result = u_entityAction(entity, DK_DCPSUtility_us_createProxyUserEntityAction, &holder);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Failed to create a user layer entity proxy, entityAction error occured.");
    if(!holder.proxyEntity)
    {
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "Failed to create a user layer entity proxy.");
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return holder.proxyEntity;

}

void
DK_DCPSUtility_us_createProxyUserEntityAction(
    v_entity e,
    c_voidp arg)
{
    DK_DCPSUtility_ProxyCreationHolder* holder;
    DLRL_INFO(INF_ENTER);

    assert(e);
    assert(arg);

    holder = (DK_DCPSUtility_ProxyCreationHolder*)arg;
    assert(!holder->proxyEntity);
    holder->proxyEntity = u_entityNew (e, holder->participant, OS_FALSE);

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyValuesIntoDatabaseSample(
    Coll_List* targetFields,
    c_value* sourceValues,
    void* targetSample)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long count = 0;
    c_value aValue;
    c_value aTmpValue;
    DMM_DCPSField* aTargetField = NULL;
    c_field targetDatabaseField = NULL;

    DLRL_INFO(INF_ENTER);

    assert(targetFields);
    assert(targetSample);
    /* sourceValues may be null */

    iterator = Coll_List_getFirstElement(targetFields);
    while(iterator)
{
        aTargetField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        targetDatabaseField = DMM_DCPSField_getDatabaseField(aTargetField);
        aValue = sourceValues[count];
        if(aValue.kind == V_STRING)
        {
            aTmpValue.kind = V_STRING;
            aTmpValue.is.String = c_stringNew(c_getBase(targetDatabaseField), aValue.is.String);
            aValue = aTmpValue;
        }
        c_fieldAssign(targetDatabaseField, targetSample, aValue);
        if(aValue.kind == V_STRING)
        {
            c_free(aValue.is.String);
        }
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_resolveDatabaseTopicPlaceholders(
    v_entity reader,
    c_voidp arg)
{
    v_topic topic;
    v_message message;
    c_object dataSample;
    DK_TopicInfo* topicInfo;

    DLRL_INFO(INF_ENTER);

    assert(reader);
    assert(arg);

    /* TODO ID: 202 */
    topic = v_dataReaderGetTopic(((v_dataReader)reader));
    message = v_topicMessageNew(topic);
    dataSample = (c_object)C_DISPLACE(message,v_topicDataOffset(topic));

    topicInfo = (DK_TopicInfo*)arg;

    DK_TopicInfo_us_setDataSample(topicInfo, dataSample);
    DK_TopicInfo_us_setDataSampleOffset(topicInfo, v_topicDataOffset(topic));
    DK_TopicInfo_us_setMessage(topicInfo, message);

    c_free(topic);

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_resolveDatabaseFields(
    v_entity topic,
    c_voidp arg)
{
    DK_TopicInfoHolder* holder = (DK_TopicInfoHolder*)arg;
    c_type topicType;
    DMM_DCPSTopic* metaTopic = NULL;
    c_field aDatabaseField;
    Coll_List* fields = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DCPSField* aField = NULL;

    DLRL_INFO(INF_ENTER);

    assert(topic);
    assert(arg);

    topicType = v_topicDataType(topic);
    metaTopic = DK_TopicInfo_us_getMetaTopic(holder->topicInfo);
    fields = DMM_DCPSTopic_getFields(metaTopic);
    iterator = Coll_List_getFirstElement(fields);
    while(iterator)
    {
        aField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        aDatabaseField = c_fieldNew(topicType, (c_char*)DMM_DCPSField_getName(aField));
        if(!aDatabaseField)
        {
            DLRL_Exception_THROW(holder->exception, DLRL_OUT_OF_MEMORY,
                "Unable to create DCPS database meta field '%s' within "
                "the DCPS topic type '%s'",
                DLRL_VALID_NAME(DMM_DCPSField_getName(aField)),
                DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(metaTopic)));
        }
        DMM_DCPSField_setDatabaseField(aField, aDatabaseField);
        c_free(aDatabaseField);
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(holder->exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_freeDatabaseFields(
    v_entity entity,
    c_voidp arg)
{
    DMM_DLRLClass* metaClass;
    Coll_Set* topics;
    Coll_Iter* iterator;
    DMM_DCPSTopic* topic;
    Coll_List* fields;
    Coll_Iter* fieldIterator;

    DLRL_INFO(INF_ENTER);

    assert(entity);
    assert(arg);

    metaClass = (DMM_DLRLClass*)arg;

    topics = DMM_DLRLClass_getTopics(metaClass);
    iterator = Coll_Set_getFirstElement(topics);
    while(iterator)
    {
        topic = (DMM_DCPSTopic*)Coll_Iter_getObject(iterator);
        fields = DMM_DCPSTopic_getFields(topic);
        fieldIterator = Coll_List_getFirstElement(fields);
        while(fieldIterator)
        {
            DMM_DCPSField* field;
            c_field dbField;

            field = (DMM_DCPSField*)Coll_Iter_getObject(fieldIterator);
            DMM_DCPSField_setDatabaseField(field, NULL);
            fieldIterator = Coll_Iter_getNext(fieldIterator);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_copyValuesIntoDatabaseAction(
    v_entity entity,
    c_voidp arg)
{
    DK_DCPSUtility_copyValuesHolder* holder;

    DLRL_INFO(INF_ENTER);

    assert(arg);

    holder = (DK_DCPSUtility_copyValuesHolder*)arg;

    DK_DCPSUtility_us_copyValuesIntoDatabaseSample(holder->topicKeys, holder->keysArray, holder->sample);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_freeMessage(
    v_entity entity,
    c_voidp arg)
{
    DLRL_INFO(INF_ENTER);

    assert(arg);

    c_free((v_message)arg);

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyRelationKeysAction(
    v_entity entity,
    c_voidp arg)
{
    DK_DCPSUtility_relationKeysCopyDataHolder* dataHolder;

    DLRL_INFO(INF_ENTER);
    assert(arg);

    dataHolder = (DK_DCPSUtility_relationKeysCopyDataHolder*)arg;
    DK_DCPSUtility_us_copyRelationKeysIntoDatabaseSample(
        dataHolder->exception,
        dataHolder->relation,
        dataHolder->data,
        dataHolder->sampleDatabaseObject);

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyRelationKeysIntoDatabaseSample(
    DLRL_Exception* exception,
    DMM_DLRLRelation* relation,
    DK_ReadData* data,
    c_object targetSample)
{
    Coll_List* ownerKeys;
    Coll_List* targetKeys;
    LOC_long udIndex = -1;
    LOC_unsigned_long ownerKeysSize;
    LOC_unsigned_long targetKeysSize;
    DMM_DCPSField* anOwnerKeyField = NULL;
    DMM_DCPSField* atargetKeyField = NULL;
    c_field databaseFieldtargetKey;
    c_value aValue;
    Coll_Iter* ownerKeysIterator = NULL;
    Coll_Iter* targetKeysIterator = NULL;
    DMM_KeyType type;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(relation);
    assert(data);
    assert(targetSample);

    ownerKeys = DMM_DLRLRelation_getOwnerKeys(relation);
    targetKeys = DMM_DLRLRelation_getTargetKeys(relation);
    ownerKeysSize = Coll_List_getNrOfElements(ownerKeys);
    targetKeysSize = Coll_List_getNrOfElements(targetKeys);
    assert(ownerKeysSize == targetKeysSize);
    ownerKeysIterator = Coll_List_getFirstElement(ownerKeys);
    targetKeysIterator = Coll_List_getFirstElement(targetKeys);
    while(ownerKeysIterator && targetKeysIterator)
    {
        anOwnerKeyField = (DMM_DCPSField*)Coll_Iter_getObject(ownerKeysIterator);
        atargetKeyField = (DMM_DCPSField*)Coll_Iter_getObject(targetKeysIterator);
        udIndex = DMM_DCPSField_getUserDefinedIndex(anOwnerKeyField);
        type = DMM_DCPSField_getFieldType(anOwnerKeyField);
        databaseFieldtargetKey = DMM_DCPSField_getDatabaseField(atargetKeyField);
        /* in a relation a field used is always a foreign key or its a shared (common) key... Anything else is wrong. */
        if(type == DMM_KEYTYPE_FOREIGN_KEY)
        {
            assert(data->foreignKeyValueArray);
            aValue = data->foreignKeyValueArray[udIndex];
        } else
        {
            assert(type == DMM_KEYTYPE_SHARED_KEY);
            assert(data->keyValueArray);
            aValue = data->keyValueArray[udIndex];
        }
        DK_DCPSUtility_us_copyValueIntoDatabaseSample(exception, aValue, targetSample, atargetKeyField);
        DLRL_Exception_PROPAGATE(exception);

        ownerKeysIterator = Coll_Iter_getNext(ownerKeysIterator);
        targetKeysIterator = Coll_Iter_getNext(targetKeysIterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyValueIntoDatabaseSample(
    DLRL_Exception* exception,
    c_value value,
    void* dataSample,
    DMM_DCPSField* field)
{
    c_value aValue = value;
    c_field aDatabaseField;
    c_value aTmpValue;

    DLRL_INFO(INF_ENTER);
    assert(dataSample);
    assert(field);
    assert(exception);

    aDatabaseField = DMM_DCPSField_getDatabaseField(field);

    if(aValue.kind == V_STRING)
    {
        aTmpValue.kind = V_STRING;
        aTmpValue.is.String = c_stringNew(c_getBase(aDatabaseField) ,aValue.is.String);
        DLRL_VERIFY_ALLOC(aTmpValue.is.String, exception, "Unable to allocate memory");
        aValue = aTmpValue;
    }

    c_fieldAssign(aDatabaseField, dataSample, aValue);
    if(aValue.kind == V_STRING)
    {
        c_free(aValue.is.String);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyStringIntoDatabaseSample(
    LOC_string keyValue,
    DMM_DCPSField* metaField,
    void* dataSample)
{
    c_value aValue;
    c_field aDatabaseField;

    DLRL_INFO(INF_ENTER);
    assert(keyValue);
    assert(metaField);
    assert(dataSample);

    aDatabaseField = DMM_DCPSField_getDatabaseField(metaField);
    aValue.kind = V_STRING;
    aValue.is.String = c_stringNew(c_getBase(aDatabaseField), keyValue);
    c_fieldAssign(aDatabaseField, dataSample, aValue);
    c_free(aValue.is.String);

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyIntegerIntoDatabaseSample(
    LOC_long* keyValue,
    DMM_DCPSField* metaField,
    void* dataSample)
{
    c_value aValue;
    c_field aDatabaseField;

    DLRL_INFO(INF_ENTER);
    assert(keyValue);
    assert(metaField);
    assert(dataSample);

    aValue.kind = V_LONG;
    aValue.is.Long = (c_long)*keyValue;

    aDatabaseField = DMM_DCPSField_getDatabaseField(metaField);
    c_fieldAssign(aDatabaseField, dataSample, aValue);
    DLRL_INFO(INF_EXIT);
}

void*
DK_DCPSUtility_us_setUserDataBasedOnHandle(
    DLRL_Exception* exception,
    u_instanceHandle handle,
    void* userData)
{
    u_result result = U_RESULT_OK;
    v_object object;
    void* oldUserData = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(!u_instanceHandleIsNil(handle));
    /* userData may be null */
    result = u_instanceHandleClaim(handle, &object);
    DK_DCPSUtility_us_verifyHandleResult(exception, result);
    DLRL_Exception_PROPAGATE(exception);

    oldUserData = (v_public(object))->userDataPublic;
    (v_public(object))->userDataPublic = userData;
    result = u_instanceHandleRelease(handle);
    DK_DCPSUtility_us_verifyHandleResult(exception, result);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return oldUserData;
}

void
DK_DCPSUtility_us_verifyHandleResult(
    DLRL_Exception* exception,
    u_result result)
{
    DLRL_INFO(INF_ENTER);

    if(result != U_RESULT_OK)
    {
        if(result == U_RESULT_PRECONDITION_NOT_MET)
        {
            DLRL_Exception_THROW(exception,
                                 DLRL_DCPS_ERROR,
                                 "PRECONDITION_NOT_MET: The DCPS has been corrupted."
                                 "Check DCPS error log file for (possibly) "
                                 "more information.");
        } else if(result == U_RESULT_ILL_PARAM)
        {
            DLRL_Exception_THROW(exception,
                                 DLRL_DCPS_ERROR,
                                 "ILL_PARAM: The DCPS has been corrupted."
                                 "Check DCPS error log file for (possibly) "
                                 "more information.");
        } else
        {
            DLRL_Exception_THROW(exception,
                                 DLRL_DCPS_ERROR,
                                 "UNKNOWN_ERROR: The DCPS has been corrupted."
                                 "Check DCPS error log file for (possibly) "
                                 "more information.");
        }
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void*
DK_DCPSUtility_us_getUserDataBasedOnHandle(
    DLRL_Exception* exception,
    u_instanceHandle handle)
{
    u_result result = U_RESULT_OK;
    v_object object = NULL;
    void* userData = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(!u_instanceHandleIsNil(handle));

    result = u_instanceHandleClaim(handle, &object);
    DK_DCPSUtility_us_verifyHandleResult(exception, result);
    DLRL_Exception_PROPAGATE(exception);

    userData = (v_public(object))->userDataPublic;
    result = u_instanceHandleRelease(handle);
    DK_DCPSUtility_us_verifyHandleResult(exception, result);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return userData;
}

/* cache information needed to resolve objects based upon key values only (needed for relation management) */
void
DK_DCPSUtility_us_resolveDCPSDatabaseTopicInfo(
    DK_TopicInfo* topicInfo,
    void* userReader,
    DLRL_Exception* exception)
{
    u_result result = U_RESULT_OK;

    DLRL_INFO(INF_ENTER);

    assert(topicInfo);
    assert(exception);
    assert(userReader);

    result = u_entityWriteAction((u_entity)userReader,
                            DK_DCPSUtility_us_resolveDatabaseTopicPlaceholders,
                            topicInfo);
    DLRL_Exception_PROPAGATE_RESULT(exception,
                                    result,
                                    "An unexpected error occured while trying "
                                    "to link the DLRL meta model with DCPS "
                                    "meta model constructs.");

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN - removedu_dataReader DK_DCPSUtility_ts_getUserLayerReader(DLRL_Exception* exception, u_reader reader){ */


void*
DK_DCPSUtility_us_getIndexFieldValueOfDataSample(
    DLRL_Exception* exception,
    DMM_DCPSField* indexField,
    void* dataSample)
{
    void* retValue = NULL;
    c_field aDatabaseIndexField = NULL;
    c_value sourceValue;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(indexField);
    assert(dataSample);

    aDatabaseIndexField = DMM_DCPSField_getDatabaseField(indexField);
    sourceValue = c_fieldValue(aDatabaseIndexField, dataSample);
    switch (sourceValue.kind) {
    case V_LONG:/* int map */
        DLRL_ALLOC_WITH_SIZE(retValue,
                             sizeof(c_long),
                             exception,
                             "Unable to allocate memory for the indexfield value");
        *((c_long*)retValue) = sourceValue.is.Long;
        break;
    case V_STRING:/* str map */
        DLRL_STRDUP(retValue,
                    (sourceValue.is.String),
                    exception,
                    "Unable to allocate memory for the indexfield value");
        break;
    case V_ULONG:
    case V_BOOLEAN:
    case V_SHORT:
    case V_USHORT:
    case V_FLOAT:
    case V_CHAR:
    case V_OCTET:
    case V_ADDRESS:
    case V_LONGLONG:
    case V_ULONGLONG:
    case V_DOUBLE:
    case V_COUNT:
    case V_WCHAR:
    case V_FIXED:
    case V_WSTRING:
    case V_OBJECT:
    case V_VOIDP:
    case V_UNDEFINED:
        DLRL_Exception_THROW(exception,
                             DLRL_DCPS_ERROR,
                             "Illegal primitive kind for key field (%d)"
                             "Check DCPS error log file for (possibly) "
                             "more information.",
                             sourceValue.kind);
        break;
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return retValue;
}

void
DK_DCPSUtility_us_copyDataFieldsOfDataSampleIntoValueArray(
    DLRL_Exception* exception,
    Coll_List* sourceFields,
    void* dataSample,
    void* values)
{
    DMM_DCPSField* akeyField = NULL;
    c_field aDatabaseKeyField = NULL;
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long count = 0;
    c_value sourceValue;
    c_value* targetValue;
    c_value* valueArray = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(sourceFields);
    assert(((Coll_List_getNrOfElements(sourceFields) > 0) && values) || !values);
    assert(dataSample);

    valueArray = (c_value*)values;
    iterator = Coll_List_getFirstElement(sourceFields);
    while(iterator)
    {
        akeyField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        aDatabaseKeyField = DMM_DCPSField_getDatabaseField(akeyField);
        sourceValue = c_fieldValue(aDatabaseKeyField, dataSample);
        targetValue = &(valueArray[count]);
        if(sourceValue.kind == V_STRING)
        {
            os_free(targetValue->is.String);
           /*  targetValue->is.String = c_stringNew(c_getBase(aDatabaseKeyField) ,sourceValue.is.String); */
           DLRL_STRDUP(targetValue->is.String,
                       (sourceValue.is.String),
                       exception,
                       "Unable to copy string for c_value");
        } else
        {
            *targetValue = sourceValue;
        }
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    DLRL_Exception_EXIT(exception);
    /* no rollback, errors here are not recoverable within this operation as its not the owner of the values parameter */
    DLRL_INFO(INF_EXIT);
}

/* may return null */
c_value*
DK_DCPSUtility_us_cloneValueArray(
    DLRL_Exception* exception,
    void* ownerValues,
    LOC_unsigned_long valuesSize)
{
    c_value* valueArray = NULL;
    LOC_unsigned_long count = 0;
    c_value* ownerValueArray = (c_value*)ownerValues;
    c_value* sourceValue = NULL;
    c_value* targetValue = NULL;
    LOC_unsigned_long tempCount = 0;
    c_value value;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    /* owner values may be null */

    if(valuesSize)
    {
        DLRL_ALLOC_WITH_SIZE(valueArray,
                             (sizeof(struct c_value)*valuesSize),
                             exception,
                             "Unable to allocate array of c_value structs");
        for(count = 0; count < valuesSize; count++)
        {
            sourceValue = &(ownerValueArray[count]);
            targetValue = &(valueArray[count]);

            if(sourceValue->kind == V_STRING)
            {
                targetValue->kind = sourceValue->kind;
                DLRL_STRDUP(targetValue->is.String,
                            (sourceValue->is.String),
                            exception,
                            "Unable to copy string for c_value");
            } else
            {
                *targetValue = *sourceValue;
            }
        }
    }
    DLRL_Exception_EXIT(exception);
    if(valueArray && (exception->exceptionID != DLRL_NO_EXCEPTION))
    {
        while(tempCount < count)
        {
            value = (valueArray[tempCount]);
            if(value.kind == V_STRING)
            {
                os_free(value.is.String);
                value.is.String = NULL;
            }/* else do nothing */
            tempCount++;
        }
        os_free(valueArray);
        valueArray = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return valueArray;
}

c_value*
DK_DCPSUtility_us_cloneKeys(
    DLRL_Exception* exception,
    Coll_List* metaFields,
    void* keyArray,
    void* foreignKeyArray)
{
    c_value* valueArray = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long count  = 0;
    Coll_Iter* iterator = NULL;
    DMM_DCPSField* aKeyField = NULL;
    DMM_KeyType type;
    c_value* aValue = NULL;
    c_value* targetValue = NULL;
    LOC_long udIndex = -1;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(metaFields);
    /* keyArray and foreignKeyArray may be NULL */

    size = Coll_List_getNrOfElements(metaFields);
    if(size > 0)
    {
        DLRL_ALLOC_WITH_SIZE(valueArray,
                             (sizeof(struct c_value)*size),
                             exception,
                             "Unable to allocate array of c_value structs");
        iterator = Coll_List_getFirstElement(metaFields);
        count = 0;
        while(iterator)
        {
            aKeyField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
            targetValue = &(valueArray[count]);
            type = DMM_DCPSField_getFieldType(aKeyField);
            udIndex = DMM_DCPSField_getUserDefinedIndex(aKeyField);
            if(type == DMM_KEYTYPE_FOREIGN_KEY)
            {
                assert(foreignKeyArray);
                aValue = &(((c_value*)foreignKeyArray)[udIndex]);
            } else
            {
                assert(type == DMM_KEYTYPE_SHARED_KEY || type == DMM_KEYTYPE_KEY);
                assert(keyArray);
                aValue = &(((c_value*)keyArray)[udIndex]);
            }
            if(aValue->kind == V_STRING)
            {
                targetValue->kind = aValue->kind;
                /* targetValue->is.String = c_stringNew(c_getBase(DMM_DCPSField_getDatabaseField(aKeyField)) , */
                /*                                                                                    aValue->is.String); */
                DLRL_STRDUP(targetValue->is.String,
                            (aValue->is.String),
                            exception,
                            "Unable to copy string for c_value");
            } else
            {
                *targetValue = *aValue;
            }
            count++;
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return valueArray;
}
/* may return null */
c_value*
DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
    DLRL_Exception* exception,
    Coll_List* sourceFields,
    void* dataSample,
    LOC_unsigned_long additionalCapacity)
{
    DMM_DCPSField* akeyField = NULL;
    c_field aDatabaseKeyField = NULL;
    LOC_unsigned_long count = 0;
    LOC_unsigned_long size = 0;
    c_value* valueArray = NULL;
    Coll_Iter* iterator = NULL;
    c_value sourceValue;
    c_value* targetValue = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(sourceFields);
    assert(dataSample);

    size = Coll_List_getNrOfElements(sourceFields);
    if(size > 0 || additionalCapacity > 0)
    {
        DLRL_ALLOC_WITH_SIZE(valueArray,
                             (sizeof(struct c_value)*size)+additionalCapacity,
                             exception,
                             "Unable to allocate memory");
        iterator = Coll_List_getFirstElement(sourceFields);
        while(iterator)
        {
            akeyField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
            aDatabaseKeyField = DMM_DCPSField_getDatabaseField(akeyField);
            sourceValue = c_fieldValue(aDatabaseKeyField, dataSample);
            targetValue = &(valueArray[count]);

            if(sourceValue.kind == V_STRING)
            {
                targetValue->kind = sourceValue.kind;
                /* targetValue->is.String = c_stringNew(c_getBase(aDatabaseKeyField) ,sourceValue.is.String); */
                DLRL_STRDUP(targetValue->is.String,
                            (sourceValue.is.String),
                            exception,
                            "Unable to copy string for c_value");
            } else
            {
                *targetValue = sourceValue;
            }
    #ifndef NDEBUG
            switch (sourceValue.kind)
            {
            case V_BOOLEAN:
            case V_SHORT:
            case V_USHORT:
            case V_FLOAT:
            case V_CHAR:
            case V_OCTET:
            case V_LONG:
            case V_ULONG:
            case V_LONGLONG:
            case V_ULONGLONG:
            case V_DOUBLE:
            case V_STRING:
                /* do nothing, all ok */
                break;
            case V_COUNT:
            case V_WCHAR:
            case V_FIXED:
            case V_WSTRING:
            case V_OBJECT:
            case V_UNDEFINED:
            default:
                assert(FALSE);
                break;
            }

    #endif
            iterator = Coll_Iter_getNext(iterator);
            count++;
        }
    }
    DLRL_Exception_EXIT(exception);
    /* rollback */
    if(valueArray && (exception->exceptionID != DLRL_NO_EXCEPTION))
    {
        LOC_unsigned_long tempCount = 0;
        while(tempCount < count)
        {
            c_value value = (valueArray[tempCount]);
            if(value.kind == V_STRING)
            {
                os_free(value.is.String);
                value.is.String = NULL;
            }/* else do nothing */
            tempCount++;
        }
        os_free(valueArray);
        valueArray = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return valueArray;
}

void
DK_DCPSUtility_us_fillValidityArray(
    LOC_boolean* validityArray,
    Coll_List* validityFields,
    void* dataSample)
{
    Coll_Iter* iterator = NULL;
    DMM_DCPSField* aField = NULL;
    c_field aDatabaseKeyField = NULL;
    c_value sourceValue;
    LOC_unsigned_long count = 0;
    DLRL_INFO(INF_ENTER);

    assert(validityFields);
    assert((validityArray && (Coll_List_getNrOfElements(validityFields) > 0)) ||
        (!validityArray && (Coll_List_getNrOfElements(validityFields) == 0)) );
    assert(dataSample);

    iterator = Coll_List_getFirstElement(validityFields);
    while(iterator)
    {
        aField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        aDatabaseKeyField = DMM_DCPSField_getDatabaseField(aField);
        sourceValue = c_fieldValue(aDatabaseKeyField, dataSample);

        switch (sourceValue.kind)
        {
        case V_BOOLEAN:
            validityArray[count] = (LOC_boolean)sourceValue.is.Boolean;
            break;
        case V_SHORT:
            validityArray[count] = (LOC_boolean)sourceValue.is.Short;
            break;
        case V_USHORT:
            validityArray[count] = (LOC_boolean)sourceValue.is.UShort;
            break;
        case V_FLOAT:
            validityArray[count] = (LOC_boolean)sourceValue.is.Float;
            break;
        case V_OCTET:
            validityArray[count] = (LOC_boolean)sourceValue.is.Octet;
            break;
        case V_LONG:
            validityArray[count] = (LOC_boolean)sourceValue.is.Long;
            break;
        case V_ULONG:
            validityArray[count] = (LOC_boolean)sourceValue.is.ULong;
            break;
        case V_LONGLONG:
            validityArray[count] = (LOC_boolean)sourceValue.is.LongLong;
            break;
        case V_ULONGLONG:
            validityArray[count] = (LOC_boolean)sourceValue.is.ULongLong;
            break;
        case V_DOUBLE:
            validityArray[count] = (LOC_boolean)sourceValue.is.Double;
            break;
        case V_STRING:
        case V_CHAR:
        case V_COUNT:
        case V_WCHAR:
        case V_FIXED:
        case V_WSTRING:
        case V_OBJECT:
        case V_UNDEFINED:
        default:
            assert(FALSE);
            break;
        }
        count++;
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
}

/* values may be null if array size = 0 */
void
DK_DCPSUtility_us_destroyValueArray(
    void* values,
    LOC_unsigned_long arraySize)
{
    LOC_unsigned_long count = 0;
    c_value value;

    DLRL_INFO(INF_ENTER);

    /* no correlation between values and the size, size may be 0 while the values is not null */

    if(values)
    {

        c_value* valueArray = (c_value*)values;
        for(count = 0; count < arraySize; count++)
        {
            value = (valueArray[count]);
            if(value.kind == V_STRING)
            {
                os_free(value.is.String);
                value.is.String = NULL;
            }/* else do nothing */
        }
        os_free(valueArray);
    }/* else nothing to do */
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_DCPSUtility_us_areValuesEqual(
    c_value* value1,
    c_value* value2)
{
    LOC_boolean areEqual = TRUE;

    DLRL_INFO(INF_ENTER);

    assert(value1);
    assert(value2);

    if(value1->kind != value2->kind)
    {
        areEqual = FALSE;
    } else
    {
        switch (value1->kind)
        {
        case V_BOOLEAN:
            if(value1->is.Boolean != value2->is.Boolean)
            {
                areEqual = FALSE;
            }
            break;
        case V_SHORT:
            if(value1->is.Short != value2->is.Short)
            {
                areEqual = FALSE;
            }
            break;
        case V_USHORT:
            if(value1->is.UShort != value2->is.UShort)
            {
                areEqual = FALSE;
            }
            break;
        case V_FLOAT:
            if(value1->is.Float != value2->is.Float)
            {
                areEqual = FALSE;
            }
            break;
        case V_CHAR:
            if(value1->is.Char != value2->is.Char)
            {
                areEqual = FALSE;
            }
            break;
        case V_OCTET:
            if(value1->is.Octet != value2->is.Octet)
            {
                areEqual = FALSE;
            }
            break;
        case V_LONG:
            if(value1->is.Long != value2->is.Long)
            {
                areEqual = FALSE;
            }
            break;
        case V_ULONG:
            if(value1->is.ULong != value2->is.ULong)
            {
                areEqual = FALSE;
            }
            break;
        case V_LONGLONG:
            if(value1->is.LongLong != value2->is.LongLong)
            {
                areEqual = FALSE;
            }
            break;
        case V_ULONGLONG:
            if(value1->is.ULongLong != value2->is.ULongLong)
            {
                areEqual = FALSE;
            }
            break;
        case V_DOUBLE:
            if(value1->is.Double != value2->is.Double)
            {
                areEqual = FALSE;
            }
            break;
        case V_STRING:
            if((value1->is.String) && (value2->is.String))
            {
                if(0 != strcmp(value1->is.String, value2->is.String))
            {
                    areEqual = FALSE;
                }
            } else if((!(value1->is.String) && (value2->is.String)) ||
                        ((value1->is.String) && !(value2->is.String)))
            {
                areEqual = FALSE;
            }
            break;
        case V_COUNT:
        case V_WCHAR:
        case V_FIXED:
        case V_WSTRING:
        case V_OBJECT:
        case V_UNDEFINED:
        default:
            assert(FALSE);
            break;
        }
    }
    DLRL_INFO(INF_EXIT);
    return areEqual;
}

LOC_long
DK_DCPSUtility_us_getLongValueFromArray(
    void* valueArray,
    LOC_unsigned_long index)
{
    c_value value;
    DLRL_INFO(INF_ENTER);

    assert(valueArray);

    value = ((c_value*)valueArray)[index];
    assert(value.kind == V_LONG);

    DLRL_INFO(INF_EXIT);
    return (LOC_long)value.is.Long;
}

/* compares two value arrays and returns true if they are equal.
   The two value arrays must be of the same size */
LOC_boolean
DK_DCPSUtility_us_areValueArraysEqual(
    void* values1,
    void* values2,
    LOC_unsigned_long size)
{
    LOC_boolean areEqual = TRUE;
    LOC_unsigned_long count = 0;
    c_value* valueArray1 = (c_value*)values1;
    c_value* valueArray2  = (c_value*)values2;
    c_value* value1 = NULL;
    c_value* value2 = NULL;

    DLRL_INFO(INF_ENTER);

    assert(((size > 0) && valueArray1) || !valueArray1);
    assert(((size > 0) && valueArray2) || !valueArray2);

    while((count < size) && areEqual)
    {
        value1 = &(valueArray1[count]);
        value2 = &(valueArray2[count]);
        areEqual = DK_DCPSUtility_us_areValuesEqual(value1, value2);
        count++;
    }
    DLRL_INFO(INF_EXIT);
    return areEqual;
}

/* retrieve the user layer writer for a SAC data reader */
v_message
DK_DCPSUtility_ts_createMessageForDataWriter(
    u_writer writer,
    DLRL_Exception* exception,
    c_long* offset)
{
    u_result result = U_RESULT_OK;
    DK_DCPSUtilityMessageHolder holder;/* on stack defintion to avoid alloc */

    DLRL_INFO(INF_ENTER);

    assert(writer);
    assert(exception);

    /* init the holder */
    holder.exception = exception;
    holder.message = NULL;
    holder.offset = 0;
    /* retrieve the kernel writer for a user layer writer  */

    result = u_entityWriteAction ((u_entity)writer, DK_DCPSUtility_us_createMessage, &holder);
    DLRL_Exception_PROPAGATE_RESULT(exception, result,
                                    "The DCPS DataWriter that was created by "
                                    "the DLRL has already been deleted!");
    DLRL_Exception_PROPAGATE(exception);/* propagate the exception in the holder */

    *offset = holder.offset;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return holder.message;
}

void
DK_DCPSUtility_us_createMessage(
    v_entity e,
    c_voidp arg)
{
    v_writer kwriter = (v_writer)e;
    DK_DCPSUtilityMessageHolder* holder = (DK_DCPSUtilityMessageHolder*)arg;

    DLRL_INFO(INF_ENTER);

    assert(kwriter);
    assert(holder);

    holder->message = v_topicMessageNew(kwriter->topic);
    if(!holder->message)
    {
        DLRL_Exception_THROW(holder->exception, DLRL_OUT_OF_MEMORY,
                             "Not enough memory to complete operation.");
    }
    holder->offset = v_topicDataOffset(kwriter->topic);

    DLRL_Exception_EXIT(holder->exception);
    DLRL_INFO(INF_EXIT);
}

u_instanceHandle
DK_DCPSUtility_ts_getNilHandle()
{
    u_instanceHandle nilHandle;

    DLRL_INFO(INF_ENTER);

    nilHandle = U_INSTANCEHANDLE_NIL;

    DLRL_INFO(INF_EXIT);
    return nilHandle;
}

void
DK_DCPSUtility_us_takeDisposedNotNewInstanceCallback(
    c_object object,
    c_value arg,
    c_value *result)
{
    v_dataReaderInstanceTemplate instance = NULL;
    v_state instanceState;

    DLRL_INFO(INF_ENTER);

    assert(object);
    /* ignore arg, irrelevant */
    assert(result);

    result->kind = V_BOOLEAN;
    result->is.Boolean = FALSE;
    instance = (v_dataReaderInstanceTemplate)(object);
    instanceState = v_dataReaderInstanceState(instance);

    /* only return true for instances which are disposed and not new,
     * because if they are disposed and new it means that a new generation
     * arrived which was disposed directly again. But this is something we
     * need to process and not just take away!
     */
    if (v_stateTest(instanceState,L_DISPOSED) && !v_stateTest(instanceState, L_NEW)
         /*|| v_stateTest(instanceState,L_NOWRITERS) (disabled per 6/21/2007)*/)
    {
        result->is.Boolean = TRUE;
    }
    DLRL_INFO(INF_EXIT);
}


u_reader
DK_DCPSUtility_us_createTakeDisposedNotNewInstanceReader(
    DLRL_Exception* exception,
    u_reader reader)
{
    u_result result = U_RESULT_OK;
    q_expr expression = NULL;
    u_reader queryReader = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(reader);

    result = u_entityAction(u_entity(reader),
                            DK_DCPSUtility_us_createTakeDisposedNotNewInstanceQuery,
                            &expression);
    DLRL_Exception_PROPAGATE_RESULT(exception, result,
                                    "Creation of take instance query expression failed");
    if(!expression)
    {
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Creation of take instance query expression failed "
                             "Check DCPS error log file for (possibly) "
                             "more information.");
    }
    queryReader = (u_reader)u_queryNew(reader,
                                       "DLRL_NOT_ALIVE_QUERY",
                                       expression, NULL);
    if(!queryReader)
    {
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Creation of take instance query failed "
                             "Check DCPS error log file for (possibly) "
                             "more information.");
    }

    DLRL_Exception_EXIT(exeption);
    if(expression)
    {
        q_dispose(expression);
    }
    DLRL_INFO(INF_EXIT);
    return queryReader;
}

void
DK_DCPSUtility_us_createTakeDisposedNotNewInstanceQuery(
    v_entity entity,
    c_voidp arg)
{
    q_expr* expr = NULL;
    c_type type;

    DLRL_INFO(INF_ENTER);

    assert(entity);
    assert(arg);

    expr = (q_expr*)arg ;
    type = c_resolve(c_getBase(c_object(entity)), "c_bool");

    *expr = F1(Q_EXPR_PROGRAM,
               F3(Q_EXPR_CALLBACK,
                  (q_expr)q_newTyp(type),
                  (q_expr)DK_DCPSUtility_us_takeDisposedNotNewInstanceCallback,
                   q_newInt(0)));

    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_registerObjectToWriterInstance(
    DLRL_Exception* exception,
    u_instanceHandle handle,
    DK_ObjectAdmin* object)
{
    Coll_Set* objects = NULL;
    long errorCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(!u_instanceHandleIsNil(handle));
    assert(object);

    objects = (Coll_Set*)DK_DCPSUtility_us_getUserDataBasedOnHandle(exception, handle);
    DLRL_Exception_PROPAGATE(exception);
    if(!objects)
    {
        objects = Coll_Set_new(isObjectAdminCacheAccessLessThen, TRUE);
        if(!objects)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                                 "Out of resources. "
                                 "Unable to allocate a set to contain "
                                 "DLRL objects at the writer side.");
        }
        /* ignore the returned user data (its null anyhows) */
        DK_DCPSUtility_us_setUserDataBasedOnHandle(exception, handle, (void*)objects);
        DLRL_Exception_PROPAGATE(exception);
    } else if(Coll_Set_contains(objects, (void*)object))
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_EXISTING,
                             "Unable to register object '%p' to the "
                             "CacheAccess. another object with the same "
                             "identity already exists within the CacheAccess.",
                             object);
    }
    errorCode = Coll_Set_add(objects, (void*)object);
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                             "Unable to add an unresolved element callback "
                             "container to the list of callback containers "
                             "of an unresolved element");
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_unregisterObjectFromWriterInstance(
    DLRL_Exception* exception,
    DK_ObjectAdmin* object,
    u_writer writer)
{
    Coll_Set* objects = NULL;
    u_instanceHandle handle;
    u_result result;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(object);

    handle = DK_ObjectAdmin_us_getHandle(object);
    assert(!u_instanceHandleIsNil(handle));
    objects = (Coll_Set*)DK_DCPSUtility_us_getUserDataBasedOnHandle(exception, handle);
    assert(objects);
    Coll_Set_remove(objects, object);

    if(Coll_Set_getNrOfElements(objects) == 0)
    {
        /* ignore return value */
        DK_DCPSUtility_us_setUserDataBasedOnHandle(exception, handle, NULL);
        DLRL_Exception_PROPAGATE(exception);
        Coll_Set_delete(objects);
#ifndef NDEBUG
        printf("NDEBUG- optimize usage of u_timeGet()\n");
#endif
        result = u_writerUnregisterInstance(writer, NULL, u_timeGet(), handle);
        DLRL_Exception_PROPAGATE_RESULT(exception, result,"Unregister instance failed");
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_writeMessage(
    v_entity kernelWriter,
    c_voidp arg)
{
    v_writeResult result = V_WRITE_SUCCESS;
    DK_DCPSUtilityWriteMessageArg* messageArg = (DK_DCPSUtilityWriteMessageArg*)arg;

    DLRL_INFO(INF_ENTER);

    assert(kernelWriter);
    assert(messageArg);
#ifndef NDEBUG
    printf("NDEBUG- optimize usage of u_timeGet()\n");
#endif
    result = v_writerWrite((v_writer)kernelWriter,
                           messageArg->message,
                           u_timeGet(),
                           NULL);
    DLRL_Exception_PROPAGATE_WRITE_RESULT(messageArg->exception, result, "");

    DLRL_Exception_EXIT(messageArg->exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_disposeMessage(
    v_entity kernelWriter,
    c_voidp arg)
{
    v_writeResult result = V_WRITE_SUCCESS;
    DK_DCPSUtilityWriteMessageArg* messageArg = (DK_DCPSUtilityWriteMessageArg*)arg;

    DLRL_INFO(INF_ENTER);

    assert(kernelWriter);
    assert(messageArg);
#ifndef NDEBUG
    printf("NDEBUG- optimize usage of u_timeGet()\n");
#endif
    result = v_writerDispose((v_writer)kernelWriter,
                             messageArg->message,
                             u_timeGet(),
                             NULL);
    DLRL_Exception_PROPAGATE_WRITE_RESULT(messageArg->exception, result);

    DLRL_Exception_EXIT(messageArg->exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_DCPSUtility_us_copyFromSource(
    DLRL_Exception* exception,
    Coll_List* sourceKeys,
    c_value* sourceValues,
    Coll_List* targetKeys,
    void* targetDataSample)
{
    Coll_Iter* iterator = NULL;
    Coll_Iter* targetIterator = NULL;
    DMM_DCPSField* aField = NULL;
    DMM_DCPSField* aTargetField = NULL;
    LOC_long fieldIndex = 0;

    DLRL_INFO(INF_ENTER);

    assert(sourceKeys);
    assert(sourceValues);
    assert(targetKeys);
    assert(targetDataSample);
    assert(exception);

    iterator = Coll_List_getFirstElement(sourceKeys);
    targetIterator = Coll_List_getFirstElement(targetKeys);
    while(iterator)
    {
        assert(targetIterator);/* should be valid... */
        aField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        aTargetField = Coll_Iter_getObject(targetIterator);

        fieldIndex = DMM_DCPSField_getUserDefinedIndex(aField);
        DK_DCPSUtility_us_copyValueIntoDatabaseSample(exception, sourceValues[fieldIndex],
                                                      targetDataSample,
                                                      aTargetField);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
        targetIterator = Coll_Iter_getNext(targetIterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
