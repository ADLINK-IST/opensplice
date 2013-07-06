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
#include "os_stdlib.h"

/* OS abstraction layer includes */
#include "os_heap.h"

/* kernel includes */
#include "u_instanceHandle.h"
#include "v_public.h"
#include "v_readerSample.h"

/* user layer includes */
#include "u_dataReader.h"
#include "u_entity.h"
#include "u_query.h"

/* DLRL utilities includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL MetaModel includes */
#include "DMM_DCPSField.h"
#include "DMM_DLRLRelation.h"

/* DLRL kernel includes */
#include "DK_CollectionReader.h"
#include "DK_ObjectReader.h"
#include "DK_DCPSUtility.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_MapAdmin.h"
#include "DK_SetAdmin.h"
#include "DK_Types.h"
#include "DLRL_Kernel_private.h"

struct DK_CollectionReadInfo_s;
typedef struct DK_CollectionReadInfo_s DK_CollectionReadInfo;

struct DK_CollectionReadData_s;
typedef struct DK_CollectionReadData_s DK_CollectionReadData;

static void
DK_CollectionReader_us_destroy(
    DK_Entity * _this);

static void
DK_CollectionReader_us_tryUnregisterUnresolvedCollection(
    DK_CollectionReader* _this,
    void* userData,
    DK_CollectionReadData* data,
    DK_Collection* collection,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex);

static void
DK_CollectionReader_us_removeElementFromCollection(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data,
    DK_Collection* collection,
    LOC_boolean resetHandle);

static void
DK_CollectionReader_us_cleanUnresolvedElement(
    DK_CollectionReader* _this,
    void* userData,
    DK_CollectionReadData* data,
    DK_Collection* collection);

static void
DK_CollectionReader_us_processNewElementGeneration(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long targetKeysSize,
    LOC_unsigned_long collectionIndex);

static void
DK_CollectionReader_us_processNewDeletedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex,
    DK_ObjectAdmin* owner);


static void
DK_CollectionReader_us_processDeletedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex,
    LOC_boolean takeInstance);

static void
DK_CollectionReader_us_processModifiedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long targetKeysSize);

static u_instanceHandle
DK_CollectionReader_us_lookupHandle(
    DLRL_Exception* exception,
    DK_TopicInfo* topicInfo,
    DK_ObjectReader* reader);

static DK_ObjectAdmin*
DK_CollectionReader_us_lookupObject(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* keysArray,
    DMM_DCPSTopic* topic,
    DMM_DLRLClass* theClass);

static DK_ObjectAdmin*
DK_CollectionReader_us_getCollectionOwner(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data);

static DK_ObjectAdmin*
DK_CollectionReader_us_getCollectionTarget(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data);

static void
DK_CollectionReader_us_addElementToCollection(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data,
    DK_Collection* collection);

static void
DK_CollectionReader_us_processElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long targetKeysSize,
    LOC_unsigned_long collectionIndex);

static void
DK_CollectionReader_us_processNewElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long targetKeysSize,
    LOC_unsigned_long collectionIndex,
    DK_ObjectAdmin* owner);

static DK_Collection*
DK_CollectionReader_us_getCollection(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* owner,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex);

static void
DK_CollectionReader_us_registerUnresolvedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data);

/* \brief The DLRL specific reader copy operation for reading updates to
 * collections.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the
 * object in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code>
 * objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the object in question belong.</li></ul>
 *
 * \param samples The samples as provided by DCPS
 * \param readerInfo The reader info for each sample. DLRL userdata is included
 * here.
 */
static v_actionResult
DK_CollectionReader_us_readerCopy(
    c_object object,
    c_voidp arg);

#define ENTITY_NAME "DLRL Kernel CollectionReader"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

struct DK_CollectionReadInfo_s
{
    DMM_Basis type;
    DLRL_Exception* exception;
    Coll_List dataSamples;
    Coll_List* keyFields;
    Coll_List* targetFields;
    DMM_DCPSField* indexField;
    c_long offset;
};

struct DK_CollectionReadData_s
{
    DK_ObjectHolder* holder;
    DK_ReadAction action;
    u_instanceHandle handle;
    c_value* ownerKeysArray;
    c_value* targetKeysArray;
    void* indexField;
    LOC_long noWritersCount;
    LOC_long disposedCount;
};

DK_CollectionReader*
DK_CollectionReader_new(
    DLRL_Exception* exception,
    u_reader reader,
    u_reader queryReader,
    DLRL_LS_object ls_reader,
    DK_TopicInfo* topicInfo,
    DMM_DLRLMultiRelation* relation)
{
    DK_CollectionReader* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(reader);
    assert(topicInfo);
    assert(relation);
    assert(queryReader);
    /* ls_reader may be null */

    DLRL_ALLOC(_this, DK_CollectionReader, exception, "%s", allocError);

    _this->alive = TRUE;
    _this->reader = reader;
    _this->queryReader = queryReader;
    _this->ls_reader = ls_reader;
    _this->metaRelation = relation;
    _this->topicInfo = (DK_TopicInfo*)DK_Entity_ts_duplicate(
        (DK_Entity*)topicInfo);

    DK_Entity_us_init(
        &(_this->entity),
        DK_CLASS_COLLECTION_READER,
        DK_CollectionReader_us_destroy);
    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        /* set user layer reader to null to prevent double free, as caller
         * of this operation will still assume ownership if this function has
         * failed!
         */
        _this->reader = NULL;
        DK_CollectionReader_us_delete(_this, NULL);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }

    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_CollectionReader_us_destroy(
    DK_Entity * _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_CollectionReader*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_delete(
    DK_CollectionReader* _this,
    void* userData)
{
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    u_result result = U_RESULT_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    if(_this->alive)
    {
        DLRL_Exception_init(&exception);
        if(_this->queryReader)
        {
            result = u_queryFree(u_query(_this->queryReader));

            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception,
                   result,
                   "Unable to free the query reader");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "query datareader\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
            _this->queryReader = NULL;
        }
        if(_this->reader)
        {
            /* no duplicate done */
            cache = DK_ObjectHomeAdmin_us_getCache(
                DK_TopicInfo_us_getOwner(_this->topicInfo));
            dcpsUtilityBridge.deleteDataReader(
                &exception,
                userData,
                cache,
                _this->reader,
                _this->ls_reader);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(
                    REPORT_ERROR,
                    "Exception %s occured when attempting to delete the DCPS "
                        "datareader\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                    exception.exceptionMessage);
                /* reset the exception, maybe it's used again later in this
                 * deletion function. We dont propagate the exception here
                 * anyway, so it can do no harm as we already logged the
                 * exception directly above.
                 */
                DLRL_Exception_init(&exception);
            }
            result = u_entityFree(u_entity(_this->reader));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception,
                   result,
                   "Unable to free the user layer reader!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer datareader\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
            _this->reader = NULL;
            /* ls_reader and userReader pointer become invalid once the
             * datareader has been deleted.
             */
            _this->ls_reader = NULL;
        }
        if(_this->topicInfo)
        {
            DK_Entity_ts_release((DK_Entity*)_this->topicInfo);
            _this->topicInfo = NULL;
        }
        if(_this->metaRelation)
        {
            /* not the owner so dun have to free */
            _this->metaRelation = NULL;
        }
        _this->alive = FALSE;
    }

    DLRL_INFO(INF_EXIT);
}

/* no duplicate done! */
DK_TopicInfo*
DK_CollectionReader_us_getTopicInfo(
    DK_CollectionReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicInfo;
}

DLRL_LS_object
DK_CollectionReader_us_getLSReader(
    DK_CollectionReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_reader;
}

void
DK_CollectionReader_us_enable(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    dcpsUtilityBridge.enableEntity(exception, userData, _this->ls_reader);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

u_reader
DK_CollectionReader_us_getReader(
    DK_CollectionReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->reader;
}


/* assumes the ObjectHome of the owner object is LOCKED as well as the accessed
 * target object home a lock on the cache admin must be present (updates)
 */
void
DK_CollectionReader_us_processDCPSUpdates(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long collectionIndex)
{
/* TODO ID: 178 - turned off    tatusKindMask mask; */
    DMM_DCPSTopic* relationTopic = NULL;
    LOC_unsigned_long count = 0;
    LOC_unsigned_long targetKeysSize = 0;
    LOC_unsigned_long keysSize = 0;
    LOC_unsigned_long size = 0;
    DK_CollectionReadData* data = NULL;
    u_result result = U_RESULT_OK;
    /* on stack definition, saving out an alloc */
    DK_CollectionReadInfo collReadInfo;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);

    /* TODO ID: 178 - turned off
     *   mask = ataReader_get_status_changes(_this->reader);
     */
    /* TODO ID: 178 - turned off
     * if((mask & ATA_AVAILABLE_STATUS) == ATA_AVAILABLE_STATUS){
     */

    relationTopic = DMM_DLRLMultiRelation_getRelationTopic(_this->metaRelation);

    collReadInfo.exception = exception;
    collReadInfo.offset = _this->topicInfo->topicDataSampleOffset;
    collReadInfo.type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
    Coll_List_init(&collReadInfo.dataSamples);
    collReadInfo.keyFields = DMM_DLRLMultiRelation_getRelationTopicOwnerFields(
        _this->metaRelation);
    collReadInfo.targetFields = DMM_DLRLMultiRelation_getRelationTopicTargetFields(
        _this->metaRelation);
    collReadInfo.indexField = DMM_DLRLMultiRelation_getIndexField(
        _this->metaRelation);

    targetKeysSize = Coll_List_getNrOfElements(collReadInfo.targetFields);
    keysSize = Coll_List_getNrOfElements(collReadInfo.keyFields);

    result =  u_readerRead(
        _this->reader,
        DK_CollectionReader_us_readerCopy,
        (c_voidp)&collReadInfo);
    /* dont forget to propagate the exception which was wrapped in the readInfo
     * struct!
     */
    DLRL_Exception_PROPAGATE(exception);
    DLRL_Exception_PROPAGATE_RESULT(
        exception,
        result,
        "%s: DCPS read failed.",
        ENTITY_NAME);
    size = Coll_List_getNrOfElements(&collReadInfo.dataSamples);
    for(count = 0; count < size; count++)
    {
        data = (DK_CollectionReadData*)Coll_List_popBack(
            &collReadInfo.dataSamples);
#ifndef NDEBUG
        assert(data->holder || (data->action == DK_READ_ACTION_CREATE
                            ||  data->action == DK_READ_ACTION_CREATE_DELETE));
        if(data->action == DK_READ_ACTION_DELETE  ||
           data->action == DK_READ_ACTION_GENERATION_DELETE)
        {
            /*data->ownerKeysArray); may be valid */
            assert(!data->targetKeysArray);
        } else if(data->action == DK_READ_ACTION_NO_WRITERS_CHANGE)
        {
            assert(!data->ownerKeysArray);
            assert(!data->targetKeysArray);
        } else
        {
            if(!data->holder ||
                !DK_Collection_us_getOwner((DK_Collection*)DK_ObjectHolder_us_getOwner(data->holder)) ||
                data->action == DK_READ_ACTION_GENERATION)
            {
                assert(((data->ownerKeysArray) && (keysSize > 0)) ||
                    ((!data->ownerKeysArray) && (keysSize == 0)));
            } else
            {
                assert(!data->ownerKeysArray);
            }
            assert(((data->targetKeysArray) && (targetKeysSize > 0)) ||
                ((!data->targetKeysArray) && (targetKeysSize == 0)));
        }
#endif
        DK_CollectionReader_us_processElement(
            _this,
            exception,
            userData,
            data,
            keysSize,
            targetKeysSize,
            collectionIndex);
        DLRL_Exception_PROPAGATE(exception);
        /* might be reset to NULL if an unresolved target was created */
        if(data->targetKeysArray)
        {
            DK_DCPSUtility_us_destroyValueArray(
                data->targetKeysArray,
                targetKeysSize);
        }
        if(data->ownerKeysArray)
        {
            DK_DCPSUtility_us_destroyValueArray(data->ownerKeysArray, keysSize);
        }
        os_free(data);
    }

 /* TODO ID: 178 - turned off   } */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* assumes the ObjectHome of the owner object is LOCKED as well as the accessed
 * target object home a lock on the cache admin must be present (updates)
 */
v_actionResult
DK_CollectionReader_us_readerCopy(
    c_object object,
    c_voidp arg)
{
    v_actionResult result = V_PROCEED;
    DLRL_Exception* exception = NULL;
    DK_ReadAction action = DK_READ_ACTION_DO_NOTHING;
    DK_CollectionReadInfo* collReadInfo;
    DK_CollectionReadData* data = NULL;
    LOC_long errorCode = COLL_OK;
    u_instanceHandle handle;
    v_public kernelPublic = NULL;
    v_readerSample sample = NULL;
    v_dataReaderInstance instance = NULL;
    v_message message = NULL;
    DK_ObjectHolder* holder = NULL;
    DK_ObjectAdmin* owner = NULL;
    void* sampleData = NULL;
    LOC_boolean entityExists;
    LOC_boolean noWritersCountChanged;
    LOC_boolean disposedCountChanged;

    DLRL_INFO(INF_ENTER);

    assert(arg);

    collReadInfo = (DK_CollectionReadInfo*)arg;
    exception = collReadInfo->exception;

    if(object)
    {
        sample = v_readerSample(object);
        assert(sample->instance);
        instance = v_dataReaderInstance(sample->instance);
        kernelPublic = v_public(instance);
        handle = u_instanceHandleNew(kernelPublic);
        holder = (DK_ObjectHolder*)kernelPublic->userDataPublic;

        if(holder)
        {
            entityExists = TRUE;
            if(DK_ObjectHolder_us_getNoWritersCount(holder) != instance->noWritersCount)
            {
                noWritersCountChanged = TRUE;
            } else {
                noWritersCountChanged = FALSE;
            }
            if(DK_ObjectHolder_us_getDisposedCount(holder) != instance->disposeCount)
            {
                disposedCountChanged = TRUE;
            } else
            {
                disposedCountChanged = FALSE;
            }
        } else
        {
            entityExists = FALSE;
            noWritersCountChanged = FALSE;
            disposedCountChanged = FALSE;
        }

        action = DK_ObjectReader_us_determineSampleAction(
            entityExists,
            sample->sampleState,
            instance->instanceState,
            noWritersCountChanged,
            disposedCountChanged);
        if(action != DK_READ_ACTION_DO_NOTHING)
        {
            assert(action < DK_ReadAction_elements);
            /* create the corresponding DLRL kernel data holder */
            DLRL_ALLOC(
                data,
                DK_CollectionReadData,
                exception,
                "Unable to complete operation. Out of resources");
            memset(data, 0, sizeof(DK_CollectionReadData));
            /* store the action for this data element */
            data->action = action;
            /* Store the DCPS sample instance handle */
            data->handle = handle;
            /* set the user data registered with the instance handle. */
            data->holder = holder;
            /* set the disposed and no writers counts */
            data->disposedCount = instance->disposeCount;
            data->noWritersCount = instance->noWritersCount;
            if(action == DK_READ_ACTION_DELETE ||
               action == DK_READ_ACTION_GENERATION_DELETE)
            {
                if(holder)
                {
                    owner = DK_Collection_us_getOwner((DK_Collection*)
                        DK_ObjectHolder_us_getOwner(data->holder));
                    /* if this represent an unresolved element we need the owner
                     * keys to try and locate the owner
                     */
                    if(!owner)
                    {
                        /* get topic data associated with the sample */
                        message = v_dataReaderSampleTemplate(object)->message;
                        sampleData = C_DISPLACE(message, collReadInfo->offset);
                        /* for a set this contains the owner and target fields,
                         * as both are keys in a multi place topic that
                         * represents a set. For a map this will contain the
                         * owner fields and the index field.
                         */
                        data->ownerKeysArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
                            exception,
                            collReadInfo->keyFields,
                            sampleData,
                            0);
                        DLRL_Exception_PROPAGATE(exception);
                    }
                }
            } else if(action != DK_READ_ACTION_NO_WRITERS_CHANGE)
            {
                /* explicitly assert that the action state must thus be one of
                 * the other states to ensure that if a new state is added it
                 * must be added here and thought will be put into what happens
                 * here!
                 */
                assert(data->action == DK_READ_ACTION_CREATE ||
                       data->action == DK_READ_ACTION_CREATE_DELETE ||
                       data->action == DK_READ_ACTION_MODIFY ||
                       data->action == DK_READ_ACTION_MODIFY_AND_DELETE ||
                       data->action == DK_READ_ACTION_GENERATION );

                /* get topic data associated with the sample */
                message = v_dataReaderSampleTemplate(object)->message;
                sampleData = C_DISPLACE(message, collReadInfo->offset);
                if(holder)
                {
                    owner = DK_Collection_us_getOwner((DK_Collection*)
                        DK_ObjectHolder_us_getOwner(data->holder));
                }
                /* if this represent an unresolved element we need the owner
                 * keys to try and locate the owner if this is a first time
                 * creation then we need the owner keys to find the owner for
                 * the first time if this is a generation and we already found
                 * an owner, then we need the owner keys so we can lookup the
                 * owner and determine if the owner also made a generational
                 * transition.
                 */
                if(!owner || (owner && (action == DK_READ_ACTION_GENERATION)))
                {
                    /* for a set this contains the owner and target fields,
                     * as both are keys in a multi place topic  that represents
                     * a set. For a map this will contain the owner fields and
                     * the index field.
                     */
                    data->ownerKeysArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
                        exception,
                        collReadInfo->keyFields,
                        sampleData,
                        0);
                    DLRL_Exception_PROPAGATE(exception);
                }
                if(!holder && (collReadInfo->type != DMM_BASIS_SET))
                {
                    assert(collReadInfo->indexField);
                    data->indexField = DK_DCPSUtility_us_getIndexFieldValueOfDataSample(
                        exception,
                        collReadInfo->indexField,
                        sampleData);
                    DLRL_Exception_PROPAGATE(exception);
                }
                data->targetKeysArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
                    exception,
                    collReadInfo->targetFields,
                    sampleData,
                    0);
                DLRL_Exception_PROPAGATE(exception);
            }
            errorCode = Coll_List_pushBack(
                &collReadInfo->dataSamples,
                (void*)data);
            if(errorCode != COLL_OK)
            {
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Unable to complete operation. Out of resources");
            }
        }
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        result = FALSE;
    }
    DLRL_INFO(INF_EXIT);
    return result;
}

void
DK_CollectionReader_us_processElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long targetKeysSize,
    LOC_unsigned_long collectionIndex)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(data->action < DK_ReadAction_elements);
    assert(!u_instanceHandleIsNil(data->handle));

    /* CREATION */
    if(data->action == DK_READ_ACTION_CREATE)
    {
        assert(!data->holder);
        DK_CollectionReader_us_processNewElement(
            _this,
            exception,
            userData,
            data,
            keysSize,
            targetKeysSize,
            collectionIndex,
            NULL);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* MODIFICATION */
    else if(data->action == DK_READ_ACTION_MODIFY)
    {
        assert(data->holder);
        data->indexField = DK_ObjectHolder_us_getUserData(data->holder);
        DK_CollectionReader_us_processModifiedElement(
            _this,
            exception,
            userData,
            data,
            targetKeysSize);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* DELETION (update and delete is always just seen as a delete only,
     * we dont update)
     */
    else if(data->action == DK_READ_ACTION_DELETE ||
            data->action == DK_READ_ACTION_MODIFY_AND_DELETE ||
            data->action == DK_READ_ACTION_GENERATION_DELETE)
    {
        assert(data->holder);
        data->indexField = DK_ObjectHolder_us_getUserData(data->holder);
        DK_CollectionReader_us_processDeletedElement(
            _this,
            exception,
            userData,
            data,
            keysSize,
            collectionIndex,
            TRUE);
        DLRL_Exception_PROPAGATE(exception);
    }
    else if(data->action == DK_READ_ACTION_CREATE_DELETE)
    {
        DK_CollectionReader_us_processNewDeletedElement(
            _this,
            exception,
            userData,
            data,
            keysSize,
            collectionIndex,
            NULL);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* NEW GENERATION */
    else if(data->action == DK_READ_ACTION_GENERATION )
    {
        assert(data->holder);
        assert(!u_instanceHandleIsNil(data->handle));
        DK_CollectionReader_us_processNewElementGeneration(
            _this,
            exception,
            userData,
            data,
            keysSize,
            targetKeysSize,
            collectionIndex);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* ignore the sample and take it away */
    else
    {
        assert(data->action == DK_READ_ACTION_NO_WRITERS_CHANGE);
        /* ignore this for now, see ticket dds295 */
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_processNewDeletedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex,
    DK_ObjectAdmin* owner)
{
    DMM_Basis type;
    DK_ObjectAdmin* target = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_Collection* collection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(!data->holder);
    assert(!u_instanceHandleIsNil(data->handle));
    assert((data->action == DK_READ_ACTION_GENERATION_DELETE) ||
        (data->action ==DK_READ_ACTION_CREATE_DELETE));
    /* owner may be NULL */

    /* Step 1: get target */
    target = DK_CollectionReader_us_getCollectionTarget(_this, exception, data);
    /* Step 2: get owner */
    if(!owner)
    {
        owner = DK_CollectionReader_us_getCollectionOwner(
            _this,
            exception,
            data);
        DLRL_Exception_PROPAGATE(exception);
    }
    if(target && owner)
    {
        /* Step 3: get collection */
        collection = DK_CollectionReader_us_getCollection(
            _this,
            exception,
            userData,
            owner,
            data,
            keysSize,
            collectionIndex);
        /* Step 4: Create holder */
        data->holder = DK_ObjectHolder_newResolved(
            exception,
            (DK_Entity*)collection,
            target,
            data->handle,
            collectionIndex);
        DLRL_Exception_PROPAGATE(exception);
        /* NOTE: do not have to set the userdata for the handle, as it would
         * be reset when the instance is taken anyways, so it has little use to
         * perform that step (as is done in other operations). The
         * registerIsRelatedFrom also does not have to be performed, as in a
         * normal deletion the unregister is done immediately...
         * the same goes for updating the no writers and disposed counts, it
         * serves little purpose to update them for a holder that will be
         * only temporarily used.
         */
        /* Step 5: insert element into collection as a removed element */
        /* first need to set the userData, for a set this will be NULL */
        DK_ObjectHolder_us_setUserData(data->holder, data->indexField);
        type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
        if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
        {
            /* I.E. we are dealing with a fully resolved element (which we just
             * add to the collection)
             */
            DK_MapAdmin_us_addRemovedElement(
                (DK_MapAdmin*)collection,
                exception,
                data->indexField,
                data->holder);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            assert(type == DMM_BASIS_SET);
            /* I.E. we are dealing with a fully resolved element (which we just
             * add to the collection)
             */
            DK_SetAdmin_us_addRemovedElement(
                (DK_SetAdmin*)collection,
                exception,
                data->holder);
            DLRL_Exception_PROPAGATE(exception);
        }

        /* Step 6: mark owner as modified if applicable */
        if(owner && owner->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
        {
            if(!ownerHome)
            {
                ownerHome = DK_ObjectAdmin_us_getHome(owner);
            }
            assert(ownerHome);
            DK_ObjectHomeAdmin_us_markObjectAsModified(
                ownerHome,
                exception,
                owner);
            DLRL_Exception_PROPAGATE(exception);
        }
    }/*ignore processing it, nothing to show.*/
    /* in the following op FALSE means do not reset user data */
    DK_DCPSUtility_us_takeInstanceFromDatabase(
        exception,
        _this->queryReader,
        data->handle,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_processNewElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long targetKeysSize,
    LOC_unsigned_long collectionIndex,
    DK_ObjectAdmin* owner)
{
    DK_ObjectAdmin* target = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    void* oldUserData = NULL;
    DK_Collection* collection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(!data->holder);
    assert(!u_instanceHandleIsNil(data->handle));
    assert((data->action != DK_READ_ACTION_GENERATION_DELETE) &&
        (data->action !=DK_READ_ACTION_CREATE_DELETE));
    /* owner may be NULL */

    /* Step 1: get target */
    target = DK_CollectionReader_us_getCollectionTarget(_this, exception, data);
    DLRL_Exception_PROPAGATE(exception);

    /* Step 2: get owner */
    if(!owner)
    {
        owner = DK_CollectionReader_us_getCollectionOwner(
            _this,
            exception,
            data);
        DLRL_Exception_PROPAGATE(exception);
    }

    /* Step 3: get collection */
    collection = DK_CollectionReader_us_getCollection(
        _this,
        exception,
        userData,
        owner,
        data,
        keysSize,
        collectionIndex);
    /* Step 4: Create holder */
    if(target)
    {
        data->holder = DK_ObjectHolder_newResolved(
            exception,
            (DK_Entity*)collection,
            target,
            data->handle,
            collectionIndex);
        DLRL_Exception_PROPAGATE(exception);
        DK_ObjectAdmin_us_registerIsRelatedFrom(target, exception, data->holder);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        data->holder = DK_ObjectHolder_newUnresolved(
            exception,
            (DK_Entity*)collection,
            data->targetKeysArray,
            targetKeysSize,
            data->handle,
            collectionIndex);
        DLRL_Exception_PROPAGATE(exception);
        data->targetKeysArray = NULL;
    }
    assert(data->holder);
    DK_ObjectHolder_us_setNoWritersCount(data->holder, data->noWritersCount);
    DK_ObjectHolder_us_setDisposedCount(data->holder, data->disposedCount);
    /* set the create object holder as user data in the instance handle, note
     * that the holder is not reference counted. this is because the only
     * reference to it are from the map and the instance handle user data and
     * not reference counting these elements will result in a performance gain
     * against a minimum loss in  maintainability of the code, as the places
     * where the holder is managed are limited to two places
     */
    oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
        exception,
        data->handle,
        data->holder);
    DLRL_Exception_PROPAGATE(exception);
    assert(!oldUserData);
    /* Step 5: insert element into collection */
    /* first need to set the userData, for a set this will be NULL */
    DK_ObjectHolder_us_setUserData(data->holder, data->indexField);
    if(DK_ObjectHolder_us_isResolved(data->holder))
    {
        DK_CollectionReader_us_addElementToCollection(
            _this,
            exception,
            data,
            collection);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        DK_CollectionReader_us_registerUnresolvedElement(
            _this,
            exception,
            userData,
            data);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* Step 6: mark owner as modified if applicable */
    if(target && owner && owner->readState ==
        DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
    {
        if(!ownerHome)
        {
            ownerHome = DK_ObjectAdmin_us_getHome(owner);
        }
        assert(ownerHome);
        DK_ObjectHomeAdmin_us_markObjectAsModified(ownerHome, exception, owner);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_processModifiedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long targetKeysSize)
{
    DK_ObjectAdmin* target = NULL;
    DK_ObjectAdmin* owner = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;
    DK_Collection* collection = NULL;
    DMM_DLRLClass* targetClass = NULL;
    DMM_Basis type;
    LOC_boolean resolved = FALSE;
    DK_ObjectAdmin* oldTarget = NULL;
    LOC_boolean modified = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(data->holder);
    assert(!u_instanceHandleIsNil(data->handle));

    /* Step 0: update the no writers count and disposed counts! */
    DK_ObjectHolder_us_setNoWritersCount(data->holder, data->noWritersCount);
    DK_ObjectHolder_us_setDisposedCount(data->holder, data->disposedCount);
    /* Step 1: Get target */
    target = DK_CollectionReader_us_getCollectionTarget(_this, exception, data);
    DLRL_Exception_PROPAGATE(exception);

    /* Step 2: Determine if holder is resolved or not */
    resolved = DK_ObjectHolder_us_isResolved(data->holder);

    /* Step 3: Determine what to do If the ObjectHolder was unresolved and we
     * were unable to locate a target this round AND we are dealing with a SET
     * then we dont have to do anything, as the target keys in a set dont change
     * which implies nothing in the unresolved would change anyway, so we can
     * just disregard the modification as its already properly managed.
     */
    type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
    if(resolved || target || (type != DMM_BASIS_SET))
    {
        /* Step 4: get collection */
        assert( DK_Entity_getClassID(DK_ObjectHolder_us_getOwner(data->holder)) == DK_CLASS_SET_ADMIN ||
                DK_Entity_getClassID(DK_ObjectHolder_us_getOwner(data->holder)) == DK_CLASS_MAP_ADMIN);
        collection = (DK_Collection*)DK_ObjectHolder_us_getOwner(data->holder);

        /* Step 5: If holder was unresolved, then unregister it from the
         * unresolved list Note that the unresolved status of an object holder
         * always refers to the target object. Since this piece of algorithm
         * deals with a modified element the chance is _very_ high that the
         * target object values have changed (not neccesary though, but we will
         * assume it has for performance optimization in 90% to 99% of the
         * cases).
         */
        targetClass = DMM_DLRLRelation_getType(
            (DMM_DLRLRelation*)_this->metaRelation);
        targetHome = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(
            targetClass);
        assert(targetHome);
        if(!resolved)
        {
            /* if the objectholder was previously unresolved, we have to
             * unregister it with the unresolved list of the target object home
             * before changing the target.
             */
            DK_ObjectHomeAdmin_us_unregisterUnresolvedElement(
                targetHome,
                userData,
                data->holder);
            /* this is not correct --
             * DK_Collection_us_decreaseNrOfUnresolvedElements(collection);
             */
        }
        /* now just mark it as modified in the collection if this object holder
         * previously represented a resolved target object then we see this as
         * a modification if it represented an unresolved target object then we
         * will see this modification as a newly added object
         */
        if(target && resolved)
        {
            oldTarget = DK_ObjectHolder_us_getTarget(data->holder);
            /* if the old target and the new target are the same objects, then
             * the DLRL wont notice this as a modification to the collection
             * and not do anything.
             */
            if(oldTarget != target)
            {
                DK_CollectionReader_us_commitElementModification(
                    exception,
                    data->holder,
                    target,
                    oldTarget,
                    type,
                    collection);
                DLRL_Exception_PROPAGATE(exception);
                /* must do the unregisterIsRelatedFrom outside the scope of the
                 * commitElementModification operation!
                 */
                if(oldTarget)
                {
                    assert(DK_ObjectAdmin_us_isAlive(oldTarget));
                    DK_ObjectAdmin_us_unregisterIsRelatedFrom(
                        oldTarget,
                        data->holder);
                }
                modified = TRUE;
            }
        } else if(target && !resolved)
        {
            /* just change the target object, this operation will clean/release
             * any resources needed (like the old  target) dont have to
             * unregister from the 'isRelatedFrom' list as the holder was
             * previously unresolved
             */
            DK_ObjectHolder_us_setTarget(data->holder, target);
            DK_ObjectAdmin_us_registerIsRelatedFrom(
                target,
                exception,
                data->holder);
            DLRL_Exception_PROPAGATE(exception);
            DK_CollectionReader_us_addElementToCollection(
                _this,
                exception,
                data,
                collection);
            DLRL_Exception_PROPAGATE(exception);
            modified = TRUE;
        } else
        {
            if(resolved)
            {
                assert(DK_ObjectHolder_us_getTarget(data->holder)->alive);
                DK_ObjectAdmin_us_unregisterIsRelatedFrom(
                    DK_ObjectHolder_us_getTarget(data->holder),
                    data->holder);
                data->holder = DK_CollectionReader_us_doModificationRemoval(
                    exception,
                    collection,
                    type,
                    data->targetKeysArray,
                    targetKeysSize,
                    data->holder);
                DLRL_Exception_PROPAGATE(exception);
                data->indexField = DK_ObjectHolder_us_getUserData(data->holder);
                data->targetKeysArray = NULL;/* prevent it from being freed */
                modified = TRUE;
            }
            /* now we just have to add this target object to it's home
             * unresolved list and the collection will be auto updated as soon
             * as the target object arrives.
             */
            DK_ObjectHomeAdmin_us_registerUnresolvedElement(
                targetHome,
                exception,
                userData,
                data->holder,
                0);
            DLRL_Exception_PROPAGATE(exception);
            DK_Collection_us_increaseNrOfUnresolvedElements(collection);
        }
        owner = DK_Collection_us_getOwner(collection);/* may return nill */
        if(owner && (owner->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED) &&
            modified)
        {
            if(!ownerHome)
            {
                ownerHome = DK_ObjectAdmin_us_getHome(owner);
            }
            assert(ownerHome);
            DK_ObjectHomeAdmin_us_markObjectAsModified(
                ownerHome,
                exception,
                owner);
            DLRL_Exception_PROPAGATE(exception);
        }
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}


void
DK_CollectionReader_us_processDeletedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData, DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex,
    LOC_boolean takeInstance)
{
    DK_ObjectAdmin* owner = NULL;
    DK_Collection* collection = NULL;
    LOC_boolean resolved = FALSE;
    DK_ObjectHomeAdmin* ownerHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(data->holder);
    assert(!u_instanceHandleIsNil(data->handle));

    /* step 0: update the no writers and disposed counts */
    DK_ObjectHolder_us_setNoWritersCount(data->holder, data->noWritersCount);
    DK_ObjectHolder_us_setDisposedCount(data->holder, data->disposedCount);
    /* Step 1: Get the collection object */
    assert(DK_Entity_getClassID(DK_ObjectHolder_us_getOwner(data->holder)) == DK_CLASS_SET_ADMIN ||
           DK_Entity_getClassID(DK_ObjectHolder_us_getOwner(data->holder)) == DK_CLASS_MAP_ADMIN);
    collection = (DK_Collection*)DK_ObjectHolder_us_getOwner(data->holder);
    DLRL_Exception_PROPAGATE(exception);
    /* Step 2: Determine if element if resolved */
    resolved = DK_ObjectHolder_us_isResolved(data->holder);
    if(!resolved)
    {
        /* Step 3-A: If the object holder represents an unresolved element then
         * this element will only be contained within the unresolved list
         * managed by the target object home. We must thus remove it from the
         * unresolved list first and then we can simply destroy the holder
         */
        DK_CollectionReader_us_cleanUnresolvedElement(
            _this,
            userData,
            data,
            collection);
    } else
    {
        /* Step 3-B: We are dealing with a fully resolved object holder, so
         9 just remove it from the collection */
        DK_CollectionReader_us_removeElementFromCollection(
            _this,
            exception,
            data,
            collection,
            takeInstance);
        DLRL_Exception_PROPAGATE(exception);
    }
    assert(!data->holder);
    assert(!data->indexField);
    /* Step 4: Now that we have removed an element we must validate that the
     * element didnt belong to an unresolved collection whose size has now
     * become zero. If thats the case we have to clean the collection as well.
     */
    DK_CollectionReader_us_tryUnregisterUnresolvedCollection(
        _this,
        userData,
        data,
        collection,
        keysSize,
        collectionIndex);
    /* Step 5: Take the instance from the database */
    if(takeInstance)
    {
        DK_DCPSUtility_us_takeInstanceFromDatabase(
            exception,
            _this->queryReader,
            data->handle,
            TRUE);/* TRUE = reset user data */
        DLRL_Exception_PROPAGATE(exception);
    }
    /* Step 6: Fetch the owner object of the collection in which the element
     * is contained
     */
    owner = DK_Collection_us_getOwner(collection);/* may return nill */
    /* Step 7: mark owner as modified if applicable only if we have an owner,
     * the object holder was resolved (otherwise it would just be removed from
     * an unresolved list and not be a modification to the collection) and if
     * the owners state wasnt already some sort of modification state then we
     * set the modification state to modified.
     */
    if(owner && owner->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED &&
        resolved)
    {
        if(!ownerHome)
        {
            ownerHome = DK_ObjectAdmin_us_getHome(owner);
        }
        assert(ownerHome);
        DK_ObjectHomeAdmin_us_markObjectAsModified(ownerHome, exception, owner);
        DLRL_Exception_PROPAGATE(exception);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_processNewElementGeneration(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long targetKeysSize,
    LOC_unsigned_long collectionIndex)
{
    DK_ObjectAdmin* collectionOwner = NULL;
    DK_ObjectAdmin* owner = NULL;
    void* keyValue = NULL;
    DMM_Basis type;
    DK_Collection* collection = NULL;
    void* oldUserData = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be NULL */
    assert(data);
    assert(data->holder);
    assert(u_instanceHandleIsEqual(
            data->handle,
            DK_ObjectHolder_us_getHandle(data->holder)));
    assert(data->action == DK_READ_ACTION_GENERATION);

    /* in case of a set the index field will still be null after the following
     * operation
     */
    data->indexField = DK_ObjectHolder_us_getUserData(data->holder);
    /* owner class must be a collection */
    assert(DK_Entity_getClassID(DK_ObjectHolder_us_getOwner(data->holder)) == DK_CLASS_SET_ADMIN ||
           DK_Entity_getClassID(DK_ObjectHolder_us_getOwner(data->holder)) == DK_CLASS_MAP_ADMIN);
    collection = (DK_Collection*)DK_ObjectHolder_us_getOwner(data->holder);
    collectionOwner = DK_Collection_us_getOwner(collection);
    if(!collectionOwner)
    {
        /* this is an element of an unresolved collection, this means we dont
         * need to see it as a new generation just a modification.
         */
        DK_CollectionReader_us_processModifiedElement(
            _this,
            exception,
            userData,
            data,
            targetKeysSize);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        /* find the new owner object, maybe it has changed! */
        owner = DK_CollectionReader_us_getCollectionOwner(
            _this,
            exception,
            data);
        DLRL_Exception_PROPAGATE(exception);
        if(owner == collectionOwner)
        {
            /*i.e. !owner*/
            /* the owner hasnt changed, this means that the owner hasnt made a
             * generational transition. We can thus treat this new generation
             * of the element as a modification or deletion to the collection
             * of the already known owner
             */
            DK_CollectionReader_us_processModifiedElement(
                _this,
                exception,
                userData,
                data,
                targetKeysSize);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            /* We have one option now:
             * 1: the owner has changed, this means that the owner has become a
             * new generation as well.
             *
             * In this case we must thus delete the element first, then
             * recreate it and treat it as a normal creation. So we need to
             * create a copy of it before calling the deletion function, we
             * could try and prevent the delete operation from deleting the key
             * but that would result in adding parameters to various operations
             * where it isnt logical why its there; making maintainance more
             * difficult. In that light we decided to just copy the value of the
             * indexfield (if any). Further reason is that this copy action is
             * only done in the event of a new generation for which the owner
             * has changed and for collections that represent maps, which we
             * suspect wont be a common use case.
             */
            type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
            if(type == DMM_BASIS_STR_MAP)
            {
                assert(data->indexField);
                DLRL_STRDUP(
                    keyValue,
                    ((LOC_string)data->indexField),
                    exception,
                    "Unable to allocate memory for copy of the indexfield "
                        "value");
            } else if (type == DMM_BASIS_INT_MAP)
            {
                assert(data->indexField);
                DLRL_ALLOC_WITH_SIZE(
                    keyValue,
                    sizeof(c_long),
                    exception,
                    "Unable to allocate memory for copy of the indexfield "
                        "value");
                *((c_long*)keyValue) = *((c_long*)data->indexField);
            }
            /* reset instance handle as we need to re-use it for the new g
             * eneration and we dont want it cleared  when all the modification
             * information is reset!
             */
            oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
                exception,
                data->handle,
                NULL);
            DLRL_Exception_PROPAGATE(exception);
            assert(data->holder == oldUserData);

            DK_ObjectHolder_us_setHandleNil(data->holder);
            DK_CollectionReader_us_processDeletedElement(
                _this,
                exception,
                userData,
                data,
                keysSize,
                collectionIndex,
                FALSE);
            DLRL_Exception_PROPAGATE(exception);
            /* expecting indexField to be set to NULL. */
            assert(!data->indexField);
            /* expecting holder to be set to null. */
            assert(!data->holder);
            /* should still be ok */
            assert(!u_instanceHandleIsNil(data->handle));
            data->indexField = keyValue;
            DK_CollectionReader_us_processNewElement(
                _this,
                exception,
                userData,
                data,
                keysSize,
                targetKeysSize,
                collectionIndex,
                owner);
            DLRL_Exception_PROPAGATE(exception);
        }
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_tryUnregisterUnresolvedCollection(
    DK_CollectionReader* _this,
    void* userData,
    DK_CollectionReadData* data,
    DK_Collection* collection,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex)
{
    DMM_Basis type;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_ObjectAdmin* owner = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* owner may be NULL */
    assert(data);
    assert(collection);

    /* basically this operation only does something if the owner of this
     * collection was not known. If an owner was known for this collection we
     * don't need to unregister it, as it isnt an unresolved collection. The
     * collection is the responsibility of the owning object admin in that case.
     */
    owner = DK_Collection_us_getOwner(collection);/* may return nill */
    type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
    if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
    {
        if(!owner &&
            (DK_MapAdmin_us_getLength((DK_MapAdmin*)collection) == 0) &&
            (DK_Collection_us_getNrOfUnresolvedElements(collection) == 0))
        {
            ownerHome = _this->topicInfo->owner;
            assert(ownerHome);
            DK_ObjectHomeAdmin_us_unregisterUnresolvedCollection(
                ownerHome,
                userData,
                data->ownerKeysArray,
                keysSize,
                collectionIndex);

            DK_MapAdmin_us_delete((DK_MapAdmin*)collection, userData);
            DK_Entity_ts_release((DK_Entity*)collection);
        }
    } else
    {
        assert(type == DMM_BASIS_SET);
        if(!owner &&
            DK_SetAdmin_us_getLength((DK_SetAdmin*)collection) == 0 &&
            (DK_Collection_us_getNrOfUnresolvedElements(collection) == 0))
        {
            /* if the collection is in the unresolved list and the length has
             * become 0, then remove it from the unresolved list! */
            ownerHome = _this->topicInfo->owner;
            assert(ownerHome);
            DK_ObjectHomeAdmin_us_unregisterUnresolvedCollection(
                ownerHome,
                userData,
                data->ownerKeysArray,
                keysSize,
                collectionIndex);

            DK_SetAdmin_us_delete((DK_SetAdmin*)collection, userData);
            DK_Entity_ts_release((DK_Entity*)collection);
        }
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_removeElementFromCollection(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data,
    DK_Collection* collection,
    LOC_boolean resetHandle)
{
    DMM_Basis type;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(collection);
    assert(data->holder);

    assert(DK_ObjectHolder_us_getTarget(data->holder));
    assert(DK_ObjectHolder_us_getTarget(data->holder)->alive);
    DK_ObjectAdmin_us_unregisterIsRelatedFrom(
        DK_ObjectHolder_us_getTarget(data->holder),
        data->holder);
    type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
    if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
    {
        assert(DK_ObjectHolder_us_getUserData(data->holder));
        DK_MapAdmin_us_doRemove(
            (DK_MapAdmin*)collection,
            exception,
            DK_ObjectHolder_us_getUserData(data->holder),
            data->holder);
        DLRL_Exception_PROPAGATE(exception);
        data->indexField = NULL;
    } else
    {
        assert(type == DMM_BASIS_SET);
        DK_SetAdmin_us_doRemove(
            (DK_SetAdmin*)collection,
            exception,
            data->holder);
        DLRL_Exception_PROPAGATE(exception);
    }
    if(resetHandle)
    {
        DK_ObjectHolder_us_setHandleNil(data->holder);
    }
    data->holder = NULL;
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_cleanUnresolvedElement(
    DK_CollectionReader* _this,
    void* userData,
    DK_CollectionReadData* data,
    DK_Collection* collection)
{
    DK_ObjectHomeAdmin* targetHome = NULL;
    DMM_DLRLClass* dlrlClass = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(data);
    assert(data->holder);
    assert(collection);

    dlrlClass = DMM_DLRLRelation_getType(
        (DMM_DLRLRelation*)_this->metaRelation);
    targetHome = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(dlrlClass);
    assert(targetHome);
    DK_ObjectHomeAdmin_us_unregisterUnresolvedElement(
        targetHome,
        userData,
        data->holder);
    /* this is not correct --
     * DK_Collection_us_decreaseNrOfUnresolvedElements(collection);
     */
    /* now we only need to destroy the object holder and the resources it
     * claims.
     */
    assert(DK_ObjectHolder_us_getUserData(data->holder) == data->indexField);
    if(data->indexField)
    {
        os_free(data->indexField);
        data->indexField = NULL;
    }
    DK_ObjectHolder_us_setUserData(data->holder, NULL);
    DK_ObjectHolder_us_destroy(data->holder);
    data->holder = NULL;

    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_registerUnresolvedElement(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CollectionReadData* data)
{
    DK_ObjectHomeAdmin* targetHome = NULL;
    DMM_DLRLClass* dlrlClass = NULL;
    DK_Collection* collection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(data->holder);

    collection = (DK_Collection*)DK_ObjectHolder_us_getOwner(data->holder);
    dlrlClass =DMM_DLRLRelation_getType((DMM_DLRLRelation*)_this->metaRelation);
    targetHome = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(dlrlClass);
    assert(targetHome);
    DK_ObjectHomeAdmin_us_registerUnresolvedElement(
        targetHome,
        exception,
        userData,
        data->holder,
        0);
    DLRL_Exception_PROPAGATE(exception);
    DK_Collection_us_increaseNrOfUnresolvedElements(collection);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionReader_us_addElementToCollection(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data,
    DK_Collection* collection)
{
    DMM_Basis type;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    assert(data->holder);
    assert(collection);

    type = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);
    if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
    {
        /* I.E. we are dealing with a fully resolved element (which we just
         * add to the collection)
         */
        DK_MapAdmin_us_doPut(
            (DK_MapAdmin*)collection,
            exception,
            data->indexField,
            data->holder);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        assert(type == DMM_BASIS_SET);
        /* I.E. we are dealing with a fully resolved element (which we just
         * add to the collection)
         */
        DK_SetAdmin_us_doAdd(
            (DK_SetAdmin*)collection,
            exception,
            data->holder);
        DLRL_Exception_PROPAGATE(exception);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DK_Collection*
DK_CollectionReader_us_getCollection(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* owner,
    DK_CollectionReadData* data,
    LOC_unsigned_long keysSize,
    LOC_unsigned_long collectionIndex)
{
    DK_Collection* collection = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);
    /* owner may be NULL */

    if(owner)
    {
        collection = DK_ObjectAdmin_us_getCollection(owner, collectionIndex);
    } else
    {
        /* if no owner was found, then register the collection as an unresolved
         * collection. If the collection was already known then we will get that
         * already registered collection
         */
        ownerHome = _this->topicInfo->owner;
        assert(ownerHome);
        collection = DK_ObjectHomeAdmin_us_registerUnresolvedCollection(
            ownerHome,
            exception,
            userData,
            data->ownerKeysArray,
            keysSize,
            _this->metaRelation,
            collectionIndex);
        DLRL_Exception_PROPAGATE(exception);
    }
    assert(collection);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return collection;
}

/* the remove operation of both collections eventually destroy the holder they
 * remove. Because of this we need to create a copy in this case of the object
 * holder which we can use after the remove. the reason we do not re use the
 * object holder but destroy it is because of the introduction of a bug
 * otherwise. The bug that would be introduced only happens in very special
 * cases but has to potential to disrupt the entire DLRL. The scenario is as
 * following: Element is resolved and added to the collection an update arrives
 * and we enter this operation. We find that the new target object cannot be
 * resolved this leads us to remove the object holder from the collection as a
 * collection may not contain unresolved elements. (the code below). Now if we
 * were not to destroy the object holder when we remove it then imagine the
 * flow continueing as follows. We place the object holder in the unresolved
 * list. At the same time it is still in the removed_elements list of the
 * collection. Once the removed elements is cleared the object holder should
 * be destroyed normally, but if we wish to reuse it we would only destroy it
 * if the object holder represents a resolved element (which indicates its a
 * normal removal and that the object holder did not migrate to an unresolved
 * list). But if the object home which represents the target elements of the
 * collection is processed AFTER this collection is processed then the
 * possibility exists that the unresolved element arrives anyway and the
 * target element is turned back into a resolved element in the same update
 * round! This is possible because we cannot yet shield update rounds by
 * coherent updates due to lacking DCPS functionality. Anyhow, when the target
 * element turns into a resolved element in the same update round we had the
 * same object holder pending in the removed_element list then we will see that
 * the object holder turns into a resolved object holder ofcourse. And we just
 * said that when reusing an object holder we would only destroy it if its
 * resolved. and because this is now the case the object holder is destroyed at
 * the end of the update round while the object holder is still apart of the
 * collection. Leading to unpredictable and unwanted behavior. Therefore we have
 * chosen not to re-use the object holder, and instead create a new one, taken
 * the two mallocs for granted. It should be noted that unresolved elements are
 * still a fault condition and not expected to occur often, and this scenario
 *  where we do the two malloc instead of reuse will probably occur in a few
 * percent of the examples
 */
DK_ObjectHolder*
DK_CollectionReader_us_doModificationRemoval(
    DLRL_Exception* exception,
    DK_Collection* collection,
    DMM_Basis type,
    void* targetKeysArray,
    LOC_unsigned_long targetKeysSize,
    DK_ObjectHolder* holder)
{
    DK_ObjectHolder* returnHolder = NULL;
    void* returnHolderIndexField = NULL;
    u_instanceHandle handle;
    void* oldUserData = NULL;
    void* keyValue = NULL;

    DLRL_INFO(INF_ENTER);

    handle = DK_ObjectHolder_us_getHandle(holder);
    assert(!u_instanceHandleIsNil(handle));
    returnHolder = DK_ObjectHolder_newUnresolved(
        exception,
        DK_ObjectHolder_us_getOwner(holder),
        targetKeysArray,
        targetKeysSize,
        handle,
        DK_ObjectHolder_us_getIndex(holder));
    DLRL_Exception_PROPAGATE(exception);
    /* dont forget to copy the user data and setting this holder into the
     * element handle user data! also reset to element handle of the holder to
     * NULL, as that holder is no longer represented by that instance handle.
     */
    DK_ObjectHolder_us_setHandleNil(holder);
    oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
        exception,
        handle,
        returnHolder);
    DLRL_Exception_PROPAGATE(exception);
    assert(oldUserData && oldUserData == holder);
    if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
    {
        keyValue = DK_ObjectHolder_us_getUserData(holder);
        assert(keyValue);
        if(type == DMM_BASIS_STR_MAP)
        {
            DLRL_STRDUP(
                returnHolderIndexField,
                ((LOC_string)keyValue),
                exception,
                "Unable to allocate memory for copy of the indexfield value");
        } else
        {
            DLRL_ALLOC_WITH_SIZE(
                returnHolderIndexField,
                sizeof(c_long),
                exception,
                "Unable to allocate memory for copy of the indexfield value");
            *((c_long*)returnHolderIndexField) = *((c_long*)keyValue);
        }
        DK_ObjectHolder_us_setUserData(returnHolder, returnHolderIndexField);
        DK_MapAdmin_us_doRemove(
            (DK_MapAdmin*)collection,
            exception,
            keyValue,
            holder);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        assert(type == DMM_BASIS_SET);
        DK_SetAdmin_us_doRemove(
            (DK_SetAdmin*)collection,
            exception,
            holder);
        DLRL_Exception_PROPAGATE(exception);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return returnHolder;
}

/* also used from the object reader class! When changing the target we do NOT
 * unregister the object holder from the isRelatedFrom list of the old target
 * object as this function is also used from the object reader where we are
 * iterating through the isRelatedFrom set and are clearing it. If we were to
 * do an unregister in this operation then we would cause problems there. The
 * reason why this was implemented as such is because we wanted to keep the
 * logic of how to perform an element modification in one place and the only
 * thing holding us back was the unregisterIsRelatedFrom call, so we moved it
 * outside the scope of this operation.
 */
void
DK_CollectionReader_us_commitElementModification(
    DLRL_Exception* exception,
    DK_ObjectHolder* holder,
    DK_ObjectAdmin* target,
    DK_ObjectAdmin* oldTarget,
    DMM_Basis type,
    DK_Collection* collection)
{
    void* keyValue = NULL;
    DK_ObjectHolder* removedHolder = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(holder);
    /* owner may be null */
    assert(target);
    assert(type < DMM_Basis_elements);
    assert(collection);
    assert(DK_ObjectHolder_us_isResolved(holder));

    if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
    {
        /* just change the target object, this operation will auto release any
         * value array present in the holder or release the previously targeted
         * object admin
         */
        DK_ObjectHolder_us_setTarget(holder, target);
        DK_ObjectAdmin_us_registerIsRelatedFrom(target, exception, holder);
        DLRL_Exception_PROPAGATE(exception);
        keyValue = DK_ObjectHolder_us_getUserData(holder);
        DK_MapAdmin_us_doMarkAsModified(
            (DK_MapAdmin*)collection,
            exception,
            keyValue,
            holder);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        assert(type == DMM_BASIS_SET);
        /* For the set we have to create a new object holder to store the "old"
         * information as we have to add that to the removed_elements list to
         * represent the removed object, we will reuse the existing object
         * holder for the "new" target object and this object holder will be
         * added to the added_elements list. The reason to keep the existing
         * object holder in the collection instead of moving it to the removed
         * list is to avoid having to reset userhandle in the handle. Basically
         * its a small performance optimization. We provide a nil pointer for
         * the element instance handle of the new holder, as this object holder
         * is not the object holder represented by this element handle
         */
        /* dont register it in the 'isRelatedFrom' list, as this holder should
         * be seen as a temp holder, do not have to update noWriterscount or
         * disposed count as this holder is just temporary.
         */
        removedHolder = DK_ObjectHolder_newResolved(
            exception,
            (DK_Entity*)collection,
            oldTarget,
            DK_DCPSUtility_ts_getNilHandle(),
            DK_ObjectHolder_us_getIndex(holder));
        DLRL_Exception_PROPAGATE(exception);
        /* Note that an object holder in a set doesnt utilize the user data as
         * an object holder in a map does. Therefore we dont need to copy user
         * data into the newly created removedHolder object. This is noted
         * explicitly to clarify why this step is not taken.
         *
         * Now that we have saved the previous target, we can changed the target
         * for the already existing object holder
         */
        DK_ObjectHolder_us_setTarget(holder, target);
        DK_ObjectAdmin_us_registerIsRelatedFrom(target, exception, holder);
        DLRL_Exception_PROPAGATE(exception);
        /* Because this branch represents a set, it is obvious that this
         * modification can only mean a new generation of a specific target.
         * therefore we do not need to update the main collection of the set
         * with the object holder, because it was already contained there (the
         * holder was previously resolved) we only need to update the added and
         * removed elements lists of the set
         */
        DK_SetAdmin_us_addAddedElement(
            (DK_SetAdmin*)collection,
            exception,
            holder);
        DLRL_Exception_PROPAGATE(exception);
        DK_SetAdmin_us_addRemovedElement(
            (DK_SetAdmin*)collection,
            exception,
            removedHolder);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* assumes the ObjectHome of the owner object is LOCKED! */
DK_ObjectAdmin*
DK_CollectionReader_us_getCollectionTarget(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data)
{
    DMM_DCPSTopic* topic = NULL;
    DMM_DLRLClass* theClass = NULL;
    DK_ObjectAdmin* object = NULL;
    DMM_Basis base;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(data);

    topic = DMM_DLRLRelation_getTargetTopic(
        (DMM_DLRLRelation*)_this->metaRelation);
    theClass = DMM_DLRLRelation_getType((DMM_DLRLRelation*)_this->metaRelation);
    base = DMM_DLRLMultiRelation_getBasis(_this->metaRelation);

    object = DK_CollectionReader_us_lookupObject(
        _this,
        exception,
        data->targetKeysArray,
        topic,
        theClass);
    DLRL_Exception_PROPAGATE(exception);

    /* we can never make a relation to an already deleted OA, the deleted
     * relation clean up algorithm is dependant on  this assumption.
     * (see object reader -
     * DK_ObjectReader_us_clearAllRelationsToDeletedObjects)
     */
    if(object &&
        ((!DK_ObjectAdmin_us_isAlive(object)) ||
        (DK_ObjectAdmin_us_getReadState(object) ==
        DK_OBJECT_STATE_OBJECT_DELETED)))
    {
        object = NULL;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return object;
}

/* assumes the ObjectHome of the owner object is LOCKED! */
DK_ObjectAdmin*
DK_CollectionReader_us_getCollectionOwner(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    DK_CollectionReadData* data)
{
    DMM_DCPSTopic* topic = NULL;
    DMM_DLRLClass* theClass = NULL;
    DK_ObjectAdmin* object = NULL;

    DLRL_INFO(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(data);

    topic = DMM_DLRLRelation_getOwnerTopic(
        (DMM_DLRLRelation*)_this->metaRelation);
    theClass = DMM_DLRLRelation_getOwner(
        (DMM_DLRLRelation*)_this->metaRelation);
    object = DK_CollectionReader_us_lookupObject(
        _this,
        exception,
        data->ownerKeysArray,
        topic,
        theClass);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return object;
}

DK_ObjectAdmin*
DK_CollectionReader_us_lookupObject(
    DK_CollectionReader* _this,
    DLRL_Exception* exception,
    void* keysArray,
    DMM_DCPSTopic* topic,
    DMM_DLRLClass* theClass)
{
    Coll_List* topicKeys = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DK_ObjectReader* reader = NULL;
    DK_TopicInfo* topicInfo = NULL;
    void* sample = NULL;
    u_instanceHandle handle = U_INSTANCEHANDLE_NIL;
    DK_ObjectAdmin* object = NULL;
    DK_DCPSUtility_copyValuesHolder holder;
    u_result result;

    DLRL_INFO(INF_ENTER);
    assert(_this);
    assert(exception);
    /* keysArray may be NULL */
    assert(topic);
    assert(theClass);

    /* get the key fields */
    topicKeys = DMM_DCPSTopic_getKeyFields(topic);
    /* fetch the ALREADY LOCKED object home */
    home = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(theClass);
    assert(home);
    /* get object reader, no duplicate done by the getter so no need to
     * release
     */
    reader = DK_ObjectHomeAdmin_us_getObjectReader(home);
    assert(reader);/* should be present */
    /* get the topic info, no duplicate done by the getter so no need to
     * release
     */
    topicInfo = DK_ObjectReader_us_getTopicInfo(reader);
    /* get the targetSample which we can use to resolve the object */
    sample = DK_TopicInfo_us_getDataSample(topicInfo);

    holder.topicKeys = topicKeys;
    holder.keysArray = keysArray;
    holder.sample = sample;
    result = u_entityWriteAction(
        u_entity(DK_TopicInfo_us_getTopic(topicInfo)),
        DK_DCPSUtility_copyValuesIntoDatabaseAction,
        &holder);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "An error occured while copying database values into a database v_message.");
    handle = DK_CollectionReader_us_lookupHandle(exception, topicInfo, reader);
    DLRL_Exception_PROPAGATE(exception);
    if(!u_instanceHandleIsNil(handle))
    {
        object = (DK_ObjectAdmin*)DK_DCPSUtility_us_getUserDataBasedOnHandle(
            exception,
            handle);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return object;
}

u_instanceHandle
DK_CollectionReader_us_lookupHandle(
    DLRL_Exception* exception,
    DK_TopicInfo* topicInfo,
    DK_ObjectReader* reader)
{
    /* on stack definition to prevent alloc */
    struct LookupInstanceHolder holder;
    u_result result = U_RESULT_OK;
    void* userReader = NULL;

    DLRL_INFO(INF_ENTER);
    assert(topicInfo);
    assert(reader);
    assert(exception);

    holder.topicInfo = topicInfo;
    holder.relationHandle = DK_DCPSUtility_ts_getNilHandle();
    userReader = (u_entity)DK_ObjectReader_us_getReader(reader);
    result = u_entityAction(
        userReader,
        DK_DCPSUtility_us_lookupInstance,
        &holder);
    DLRL_Exception_PROPAGATE_RESULT(
        exception,
        result,
        "An error occured while looking up an instance handle.");

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return holder.relationHandle;
}
