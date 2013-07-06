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
#ifndef DLRL_KERNEL_DCPS_UTILITY_H
#define DLRL_KERNEL_DCPS_UTILITY_H

/* kernel includes */
#include "v_dataReader.h"
#include "v_dataReaderInstance.h"
#include "v_entity.h"

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL MetaModel includes */
#include "DMM_DCPSField.h"

/* DLRL kernel includes */
#include "DK_TopicInfo.h"


#if defined (__cplusplus)
extern "C" {
#endif

/* \brief This struct is used as a holder for information when performing a look-up in DCPS of a specific instance
 * handle .
 */
struct LookupInstanceHolder
{
    /* Contains all relevant topic information needed to perform the look-up */
    DK_TopicInfo* topicInfo;
    /* Should be inited to <code>NULL</code>, if succesfull this will contain the instance handle found. If not it will
     * remain <code>NULL</code>.
     */
    u_instanceHandle relationHandle;
};

/* NOT IN DESIGN */
typedef struct DK_TopicInfoHolder_s
{
    DLRL_Exception* exception;
    DK_TopicInfo* topicInfo;
}DK_TopicInfoHolder;

typedef struct DK_DCPSUtility_copyValuesHolder_s
{
    void* keysArray;
    Coll_List* topicKeys;
    void* sample;
} DK_DCPSUtility_copyValuesHolder;

typedef struct DK_DCPSUtility_relationKeysCopyDataHolder_s
{
    DLRL_Exception* exception;
    DMM_DLRLRelation* relation;
    DK_ReadData* data;
    c_object sampleDatabaseObject;
} DK_DCPSUtility_relationKeysCopyDataHolder;

/* NOT IN DESIGN */
typedef struct DK_DCPSUtilityWriteMessageArg_s
{
    v_message message;
    DLRL_Exception* exception;
} DK_DCPSUtilityWriteMessageArg;

/* \brief Utility function to look up a specific instance within the provided DCPS data reader.
 *
 * <p>This function is intended to be used through a DCPS 'u_entityAction(...)' call. It should not be called directly.
 * The 'arg' parameter is expected to be a 'struct LookupInstanceHolder*' and correctly filled. Which means the
 * topicInfo attribute must be set to the <code>DK_TopicInfo</code> object corresponding to the DCPS data reader
 * as provided in the 'reader' arg. The 'relationHandle' attribute of the struct should be <code>NULL</code>, as thats
 * the only way to determine if the lookup actually found an instance handle. If this attribute is not <code>NULL</code>
 * then one doesnt know if anything was found. Furthermore the lockReader boolean must be correctly set to indicate if
 * the reader should be locked by the operation (<code>TRUE</code>) or not (<code>FALSE</code>).
 * The 'relationHandle' attribute will remain <code>NULL</code> if no handle could be found (due to it not being present
 * or due to an error).</p>
 * <p>The <code>DK_TopicInfo</code> object contained within the <code>LookupInstanceHolder</code> struct is used to get
 * the actual lookup information. Each topic info object maintains DCPS (internal) topic objects related to an instance
 * handle. Before this operation is called one must first get the DCPS data base sample maintained by the topic info
 * object and copy the correct keys into the sample. If this is not performed correctly then this operation will give
 * undefined behavior.</p>
 *
 * Preconditions:<ul>
 * <li>Must lock the admin mutex of the home to which the topic info object provided within the
 * <code>LookupInstanceHolder</code> struct is locked.</li>
 * <li>Must ensure that the DCPS data reader is locked if the 'lockReader' attribute of the
 * <code>LookupInstanceHolder</code> struct indicates that the operation doesnt need to claim the lock on the reader
 * itself. Be aware that a reader is always locked from within a data reader readerCopy function context!</li></lu>
 *
 * \param reader A locked or unlocked DCPS dataReader object on DCPS kernel level.
 * \param arg A correctly filled <code>LookupInstanceHolder</code> struct.
 */
void
DK_DCPSUtility_us_lookupInstance(
    v_entity reader,
    c_voidp arg);

/* \brief Copies values from the sourceSample into the targetSample based upon a MetaModel DLRLRelation object.
 *
 * Any data already contained within the target sample will be automatically cleared and overridden with the new
 * data.
 *
 * Preconditions:<ul>
 * <li>The <code>DK_ObjectHomeAdmin</code> object representing the owner part of the meta model and the The
 * <code>DK_ObjectHomeAdmin</code> object representing the target part of the meta model must both be locked to garantee
 * threadsafe access to the MetaModel info as well as the topic info and samples.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relation The MetaModel relation describing the sourceSample layout and the targetSample layout.
 * \param data The data structure containing the values that need to be copied. This data structure basically represents
 * the (partial) 'owner' object of the relation.
 * \param targetSample The sample into which the key values need to be copied. This sample basically represents the
 * (partial) 'target' object of the relation.
 */
void
DK_DCPSUtility_us_copyRelationKeysIntoDatabaseSample(
    DLRL_Exception* exception,
    DMM_DLRLRelation* relation,
    DK_ReadData* data,
    c_object targetSample);

/* \brief Utility function to store a kernel entity as userData within a DCPS instance handle
 *
 * This function assumes no user data is already set for the instance handle in question, if it is then an exception
 * will be raised and the 'old' user data entity will be 'unreference counted'. IE the release function of the entity
 * will be called. This operation will not duplicate the user data provided, so that should be done by the calling
 * operation.
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param handle The handle for which to set the userData.
 * \param userData The entity (reference count increased for this operationb by 1) to be set as userData, may be
 * <code>NULL</code>
 */
void
DK_DCPSUtility_us_setHandleUserData(
    DLRL_Exception* exception,
    u_instanceHandle handle,
    DK_Entity* userData);

/* \brief Returns the user layer DCPS DataReader based upon a SAC layer DCPS DataReader.
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param reader The DataReader for which the user layer DCPS reader needs to be retrieved
 *
 * \return <code>NULL</code> if and only if an exception occured. Otherwise returns the user layer DCPS DataReader
 * based upon a SAC layer DCPS DataReader.
 */
/* NOT IN DESIGN - removedu_dataReader DK_DCPSUtility_ts_getUserLayerReader(DLRL_Exception* exception, eader reader); */

/* \brief Utility function which resolves all relevant DCPS (internal) topic info within a <code>DK_TopicInfo</code>
 * object.
 *
 * Each <code>DK_TopicInfo</code> object caches certain DCPS topic objects which can be used for looking up instance
 * handles during the main loop of the DLRL. This utility function will create the required objects and store them with
 * the <code>DK_TopicInfo</code> object which was provided as the 'arg' parameter of this operation. The DCPS DataSample,
 * DCPS DataSampleOffset and the DCPS message are the objects/values created and stored by this operation.
 *
 * Preconditions:<ul>
 * <li>Must lock the admin mutex of the home to which the topic info object provided within the
 * <code>LookupInstanceHolder</code> struct is locked.</li></ul>
 *
 * \param topicInfo The <code>DK_TopicInfo</code> object for which the DCPS topic info needs to be resolved.
 * \param reader The data reader which reads samples of the topic represented by the <code>DK_TopicInfo</code> object
 * found in the 'arg' parameter.
 * \param arg The <code>DK_TopicInfo</code> object for which the DCPS data needs to be resolved and stored.
 */
void
DK_DCPSUtility_us_resolveDCPSDatabaseTopicInfo(
    DK_TopicInfo* topicInfo,
    void* userReader,
    DLRL_Exception* exception);

/* NOT IN DESIGN void DK_DCPSUtility_ts_setReaderCopy(DLRL_Exception* exception, u_reader reader */
/* caller must free return value and possibly any string contained within the structure., if the source fields list has a */
/* length of 0 then this operation will return a null pointer */
c_value*
DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
    DLRL_Exception* exception,
    Coll_List* sourceFields,
    void* dataSample,
    LOC_unsigned_long additionalCapacity);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_fillValidityArray(
    LOC_boolean* validityArray,
    Coll_List* validityFields,
    void* dataSample);

void
DK_DCPSUtility_us_copyDataFieldsOfDataSampleIntoValueArray(
    DLRL_Exception* exception,
    Coll_List* sourceFields,
    void* dataSample,
    void* values);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_takeInstanceFromDatabase(
    DLRL_Exception* exception,
    u_reader reader,
    u_instanceHandle handle,
    LOC_boolean resetHandle);

/* caller must free return value */
void*
DK_DCPSUtility_us_getIndexFieldValueOfDataSample(
    DLRL_Exception* exception,
    DMM_DCPSField* indexField,
    void* dataSample);

void
DK_DCPSUtility_us_destroyValueArray(
    void* values,
    LOC_unsigned_long arraySize);

/* value array may be null if the arraysize == 0, will return true in this case as they are the same :) */
LOC_boolean
DK_DCPSUtility_us_areValueArraysEqual(
    void* values1,
    void* values2,
    LOC_unsigned_long size);

/* return null if the valuesSize == 0 */
c_value*
DK_DCPSUtility_us_cloneValueArray(
    DLRL_Exception* exception,
    void* ownerValues,
    LOC_unsigned_long valuesSize);

/* NOT IN DESIGN -- removed and added params */
c_value*
DK_DCPSUtility_us_cloneKeys(
    DLRL_Exception* exception,
    Coll_List* metaFields,
    void* keyArray,
    void* foreignKeyArray);

void
DK_DCPSUtility_us_copyValuesIntoDatabaseSample(
    Coll_List* targetFields,
    c_value* sourceValues,
    void* targetSample);

/* NOT IN DESIGN */
v_message
DK_DCPSUtility_ts_createMessageForDataWriter(
    u_writer writer,
    DLRL_Exception* exception,
    c_long* offset);

/* NOT IN DESIGN */
void*
DK_DCPSUtility_us_setUserDataBasedOnHandle(
    DLRL_Exception* exception,
    u_instanceHandle handle,
    void* userData);

/* NOT IN DESIGN */
void*
DK_DCPSUtility_us_getUserDataBasedOnHandle(
    DLRL_Exception* exception,
    u_instanceHandle handle);

/* NOT IN DESIGN - removed */
/* void DK_DCPSUtility_us_testSampleData(DK_TopicInfo* _this, void* sampleData); */

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_resolveDatabaseFields(
    v_entity topic,
    c_voidp arg);

void
DK_DCPSUtility_us_freeDatabaseFields(
    v_entity entity,
    c_voidp arg);

void
DK_DCPSUtility_us_freeMessage(
    v_entity entity,
    c_voidp arg);

void
DK_DCPSUtility_us_copyRelationKeysAction(
    v_entity entity,
    c_voidp arg);

void
DK_DCPSUtility_copyValuesIntoDatabaseAction(
    v_entity entity,
    c_voidp arg);

/* NOT IN DESIGN */
u_reader
DK_DCPSUtility_us_createTakeDisposedNotNewInstanceReader(
    DLRL_Exception* exception,
    u_reader reader);

/* NOT IN DESIGN -- access level changed */
LOC_boolean
DK_DCPSUtility_us_areValuesEqual(
    c_value* value1,
    c_value* value2);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_registerObjectToWriterInstance(
    DLRL_Exception* exception,
    u_instanceHandle handle,
    DK_ObjectAdmin* object);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_unregisterObjectFromWriterInstance(
    DLRL_Exception* exception,
    DK_ObjectAdmin* object,
    u_writer writer);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_copyFromSource(
    DLRL_Exception* exception,
    Coll_List* sourceKeys,
    c_value* sourceValues,
    Coll_List* targetKeys,
    void* targetDataSample);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_writeMessage(
    v_entity kernelWriter,
    c_voidp arg);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_disposeMessage(
    v_entity kernelWriter,
    c_voidp arg);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_copyIntegerIntoDatabaseSample(
    LOC_long* keyValue,
    DMM_DCPSField* metaField,
    void* dataSample);

/* NOT IN DESIGN */
void
DK_DCPSUtility_us_copyStringIntoDatabaseSample(
    LOC_string keyValue,
    DMM_DCPSField* metaField,
    void* dataSample);

/* NOT IN DESIGN */
LOC_long
DK_DCPSUtility_us_getLongValueFromArray(
    void* valueArray,
    LOC_unsigned_long index);

void
DK_DCPSUtility_us_copyValueIntoDatabaseSample(
    DLRL_Exception* exception,
    c_value value,
    void* dataSample,
    DMM_DCPSField* field);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_DCPS_UTILITY_H */
