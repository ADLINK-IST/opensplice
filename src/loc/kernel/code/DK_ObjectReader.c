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

/* Kernel includes */
#include "v_observer.h"
#include "v_public.h"
#include "v_state.h"

/* User layer includes */
#include "u_instanceHandle.h"
#include "u_dataReader.h"
#include "u_entity.h"
#include "u_query.h"

/* User layer 'private'  includes */
#include "u__entity.h"

/* Collection includes */
#include "Coll_Compare.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL meta model includes */
#include "DMM_DCPSField.h"
#include "DMM_DLRLRelation.h"
#include "DMM_InheritanceTable.h"

/* DLRL kernel includes */
#include "DK_Collection.h"
#include "DK_CollectionBridge.h"
#include "DK_CollectionReader.h"
#include "DK_DCPSUtility.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_ObjectAdmin.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_ObjectReader.h"
#include "DK_ObjectReaderBridge.h"
#include "DK_ObjectRelationReader.h"
#include "DK_ObjectRelationReaderBridge.h"
#include "DK_MapAdmin.h"
#include "DK_SetAdmin.h"
#include "DK_Types.h"
#include "DK_Utility.h"
#include "DK_UnresolvedObjectsUtility.h"

#define ENTITY_NAME "DLRL Kernel ObjectReader"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

typedef enum DK_SampleAction_e
{
    DK_SampleAction_Choice_1,
    DK_SampleAction_Choice_2,
    DK_SampleAction_Choice_3,
    DK_SampleAction_Choice_4,
    DK_SampleAction_Choice_5,
    DK_SampleAction_Choice_6,
    DK_SampleAction_Choice_7,
    DK_SampleAction_Choice_8,
    DK_SampleAction_Choice_9,
    DK_SampleAction_Choice_10,
    DK_SampleAction_Choice_11,
    DK_SampleAction_Choice_12,
    DK_SampleAction_Choice_13,
    DK_SampleAction_Choice_14,
    DK_SampleAction_TERMINATE
} DK_SampleAction;

static v_actionResult
DK_ObjectReader_us_readerCopy(
    c_object object,
    c_voidp arg);

/* NOT IN DESIGN - moved to other class, thus removed here */
/* static void DK_ObjectReader_us_createOIDForObjectAdmin(DLRL_Exception* exception, void* userData,  */
/*                                                             DK_ObjectHomeAdmin* home, DK_ReadData* data); */

/* assumes all homes that play a role in the creation of a new object are locked. */
static void
DK_ObjectReader_us_processNewObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data);

static void
DK_ObjectReader_us_processModifiedObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data);

/* assumes all homes that play a role in the deletion of an object are locked. */
static void
DK_ObjectReader_us_processDisposedObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data);

static void
DK_ObjectReader_us_updateObjectAdmin(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data);

/* assumes all homes needed for creating collections are locked
NOT IN DESIGN
static void
DK_ObjectReader_us_createCollections(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data);
*/
/* NOT IN DESIGN*/
static void
DK_ObjectReader_us_copyAllRelationsToNewGeneration(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* prevGenObject,
    DK_ObjectAdmin* nextGenObject);

static void
DK_ObjectReader_us_loadObjectContent(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data);

DK_ObjectHolder*
DK_ObjectReader_us_createGenerationHolderClone(
    DLRL_Exception* exception,
    DK_Collection* newGenColl,
    DK_ObjectHolder* holder,
    DMM_Basis type,
    void** keyValue);

/* NOT IN DESIGNstatic void DK_ObjectReader_us_takeSampleFromDatabase(DK_ObjectReader* _this, DLRL_Exception* exception,  */
               /*                                      u_instanceHandle handle); */

static c_bool
DK_ObjectReader_us_getObjectFromHandle(
    c_object object,
    c_voidp arg);

static void
DK_ObjectReader_us_destroy(
    DK_Entity * _this);

static void
DK_ObjectReader_us_processObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data,
    LOC_unsigned_long keysSize);

static LOC_boolean
DK_ObjectReader_us_determineDefaultRelationIsValid(
    DMM_DLRLRelation* relation,
    void* foreignKeyValueArray);

void
DK_ObjectReader_us_performGenerationalCollectionCopy(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_Collection* oldGenColl,
    DK_Collection* newGenColl);

/* ls reader may be null */
DK_ObjectReader*
DK_ObjectReader_new(
    DLRL_Exception* exception,
    u_reader reader,
    u_reader queryReader,
    DLRL_LS_object ls_reader,
    DK_TopicInfo* topicInfo)
{
    DK_ObjectReader* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(reader);
    assert(queryReader);
    assert(topicInfo);
    /* ls_reader may be null */

    DLRL_ALLOC(_this, DK_ObjectReader, exception, "%s", allocError);

    _this->alive = TRUE;
    _this->reader = reader;
    _this->queryReader = queryReader;
    _this->ls_reader = ls_reader;
    _this->topicInfo = (DK_TopicInfo*)DK_Entity_ts_duplicate((DK_Entity*)topicInfo);
    Coll_List_init(&(_this->newSamples));
    Coll_List_init(&(_this->modifiedSamples));
    Coll_List_init(&(_this->deletedSamples));
    Coll_List_init(&(_this->collectionReaders));
    Coll_Set_init(&(_this->unresolvedElements), pointerIsLessThen, TRUE);
    DK_Entity_us_init(&(_this->entity), DK_CLASS_OBJECT_READER, DK_ObjectReader_us_destroy);
    DLRL_INFO(INF_ENTITY, "Created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        /* set user layer reader to null to prevent double free, as caller
         * of this operation will still assume ownership if this function has
         * failed!
         */
        _this->reader = NULL;
        DK_ObjectReader_us_delete(_this, NULL);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }

    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_ObjectReader_us_destroy(
    DK_Entity * _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_ObjectReader*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

/* cache must have update lock, administration, home must be locked */
void
DK_ObjectReader_us_delete(
    DK_ObjectReader* _this,
    void* userData)
{
    DK_CollectionReader* collReader = NULL;
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    u_result result = U_RESULT_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    if(_this->alive)
    {
        DLRL_Exception_init(&exception);
        /* clear unresolved list */
        DK_UnresolvedObjectsUtility_us_clear(&(_this->unresolvedElements), userData);
        if(_this->queryReader)
        {
            result = u_queryFree(u_query(_this->queryReader));
            if(result != U_RESULT_OK)
            {
                DLRL_Exception_transformResultToException(&exception, result, "Unable to free the query reader");
               DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to delete the DCPS query datareader\n%s",
                        DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
            _this->queryReader = NULL;
        }
        if(_this->reader)
        {
            /* no duplicate done */
            cache = DK_ObjectHomeAdmin_us_getCache(DK_TopicInfo_us_getOwner(_this->topicInfo));
            dcpsUtilityBridge.deleteDataReader(&exception, userData, cache, _this->reader,
                                                                                _this->ls_reader);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to delete the DCPS datareader\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
                /* reset the exception, maybe it's used again later in this deletion function. We dont propagate the */
                /* exception here anyway, so it can do no harm as we already logged the exception directly above. */
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
            /* ls_reader and userReader pointer become invalid once the datareader has been deleted. */
            _this->ls_reader = NULL;
        }
        if(_this->topicInfo)
        {
            DK_Entity_ts_release((DK_Entity*)_this->topicInfo);
            _this->topicInfo = NULL;
        }
        while(Coll_List_getNrOfElements(&(_this->newSamples)) > 0)
        {
            DK_Entity_ts_release((DK_Entity*)Coll_List_popBack(&(_this->newSamples)));
        }
        while(Coll_List_getNrOfElements(&(_this->modifiedSamples)) > 0)
        {
            DK_Entity_ts_release((DK_Entity*)Coll_List_popBack(&(_this->modifiedSamples)));
        }
        while(Coll_List_getNrOfElements(&(_this->deletedSamples)) > 0)
        {
            DK_Entity_ts_release((DK_Entity*)Coll_List_popBack(&(_this->deletedSamples)));
        }
        while(Coll_List_getNrOfElements(&(_this->collectionReaders)) > 0)
        {
            collReader = (DK_CollectionReader*)Coll_List_popBack(&(_this->collectionReaders));
            DK_CollectionReader_us_delete(collReader,  userData);
            DK_Entity_ts_release((DK_Entity*)collReader);
        }
        _this->alive = FALSE;
    }
    DLRL_INFO(INF_EXIT);
}

/* no duplicate done! */
DK_TopicInfo*
DK_ObjectReader_us_getTopicInfo(
    DK_ObjectReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicInfo;
}

DLRL_LS_object
DK_ObjectReader_us_getLSReader(
    DK_ObjectReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_reader;
}

Coll_List*
DK_ObjectReader_us_getCollectionReaders(
    DK_ObjectReader* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->collectionReaders);
}

/* no copy made */
Coll_List*
DK_ObjectReader_getNewObjects(
    DK_ObjectReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->newSamples);
}

/* no copy made */
Coll_List*
DK_ObjectReader_getModifiedObjects(
    DK_ObjectReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->modifiedSamples);
}

/* no copy made */
Coll_List*
DK_ObjectReader_getDeletedObjects(
    DK_ObjectReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->deletedSamples);
}

void
DK_ObjectReader_us_enable(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator  = NULL;
    DK_CollectionReader* collectionReader = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    dcpsUtilityBridge.enableEntity(exception, userData, _this->ls_reader);
    DLRL_Exception_PROPAGATE(exception);

    iterator = Coll_List_getFirstElement(&(_this->collectionReaders));
    while(iterator)
    {
        collectionReader = (DK_CollectionReader*)Coll_Iter_getObject(iterator);
        DK_CollectionReader_us_enable(collectionReader, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_createCollectionReader(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    DMM_DLRLMultiRelation* relation)
{
    u_reader reader = NULL;
    u_reader queryReader = NULL;
    DLRL_LS_object ls_reader = NULL;
    DK_CollectionReader* collectionReader = NULL;
    long errorCode = 0;
    u_result result;
    DLRL_Exception exception2;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(topicInfo);
    assert(relation);
    /* userData may be null */
    DLRL_Exception_init(&exception2);

    reader = dcpsUtilityBridge.createDataReader(exception, userData, topicInfo, &ls_reader);
    DLRL_Exception_PROPAGATE(exception);
    assert(reader);
    queryReader = DK_DCPSUtility_us_createTakeDisposedNotNewInstanceReader(exception, reader);
    DLRL_Exception_PROPAGATE(exception);
    DK_DCPSUtility_us_resolveDCPSDatabaseTopicInfo(topicInfo, reader, exception);
    DLRL_Exception_PROPAGATE(exception);
    collectionReader = DK_CollectionReader_new(exception, reader, queryReader, ls_reader, topicInfo, relation);
    DLRL_Exception_PROPAGATE(exception);
    assert(collectionReader);

    errorCode = Coll_List_pushBack(&(_this->collectionReaders), collectionReader);
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add a collection reader to the list of collection"
                            " readers");
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        if(collectionReader)
        {
            DK_CollectionReader_us_delete(collectionReader, userData);/* also takes care off the delete of reader*/
            DK_Entity_ts_release((DK_Entity*)collectionReader);
        } else if(reader)
        {
#if 1
            result = u_entityFree(u_entity(reader));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception2,
                   result,
                   "Unable to free the user layer reader!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer datareader\n%s",
                    DLRL_Exception_exceptionIDToString(exception2.exceptionID),
                   exception2.exceptionMessage);
               DLRL_Exception_init(&exception2);
            }
#endif
        }
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_collectObjectUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadInfo* info)
{
    DMM_DLRLClass* metaClass = NULL;
    DMM_DCPSTopic* mainTopic = NULL;
/* TODO ID: 178 - turned off    tatusKindMask mask; */

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(info);
    assert(!info->dstInfo);
    /* userData may be null */

/* TODO ID: 178 - turned off    mask = ataReader_get_status_changes(_this->reader); */
/* TODO ID: 178 - turned off   if((mask & ATA_AVAILABLE_STATUS) == ATA_AVAILABLE_STATUS){ */
        /* need to retrieve the mainTopic so we can read its key fields and foreign key fields list. we need them during  */
        /* the read */
        metaClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(home);
        mainTopic = DMM_DLRLClass_getMainTopic(metaClass);

        info->exception = exception;
        info->userData = userData;
        info->keyFields = DMM_DCPSTopic_getKeyFields(mainTopic);
        info->foreignKeyFields = DMM_DCPSTopic_getForeignKeyFields(mainTopic);
        info->validityFields = DMM_DCPSTopic_getValidityFields(mainTopic);
        info->offset = _this->topicInfo->topicDataSampleOffset;

        /* The bridge needs to set the dstInfo (if its needed during the language specific processing part of */
        /* the object update). To allow the bridge to allocate the needed info on stack the bridge needs to call the */
        /* DK_ObjectReader_us_doRead(...) operation */
        objectReaderBridge.doLSReadPreProcessing(info, _this);
        /* dont forget to propagate the exception which was wrapped in the readInfo struct! */
        DLRL_Exception_PROPAGATE(exception);
        info->dstInfo = NULL;/* was only temp available */

/* TODO ID: 178 - turned off    }else nothing to read so do nothing */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_processObjectUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* dataSamples)
{
    DMM_DLRLClass* metaClass = NULL;
    DMM_DCPSTopic* mainTopic = NULL;
    LOC_unsigned_long keysSize = 0;
    DK_ReadData* data = NULL;
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long foreignKeysSize;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    metaClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(home);
    mainTopic = DMM_DLRLClass_getMainTopic(metaClass);
    keysSize = Coll_List_getNrOfElements(DMM_DCPSTopic_getKeyFields(mainTopic));

    iterator = Coll_List_getFirstElement(dataSamples);
    while(iterator)
    {
        data = (DK_ReadData*)Coll_Iter_getObject(iterator);
        assert(data->action != DK_READ_ACTION_DO_NOTHING);
#ifndef NDEBUG
        foreignKeysSize = Coll_List_getNrOfElements(DMM_DCPSTopic_getForeignKeyFields(mainTopic)) +
                            Coll_List_getNrOfElements(DMM_DCPSTopic_getValidityFields(mainTopic));
        if(data->action != DK_READ_ACTION_DELETE &&
           data->action != DK_READ_ACTION_NO_WRITERS_CHANGE)
        {
            assert(((data->keyValueArray) && (keysSize > 0)) || ((!data->keyValueArray) && (keysSize == 0)));
            assert(((data->foreignKeyValueArray) && (foreignKeysSize > 0)) ||
                                                        ((!data->foreignKeyValueArray) && (foreignKeysSize == 0)));
        } else
        {
            assert(!data->keyValueArray);
            assert(!data->foreignKeyValueArray);
        }
#endif
        DK_ObjectReader_us_processObject(_this, exception, userData, home, data, keysSize);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_processRelationUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* dataSamples)
{
    Coll_List* relations = NULL;
    DMM_DLRLRelation* aRelation = NULL;
    LOC_unsigned_long relationsCount = 0;
    Coll_Iter* iterator = NULL;
    DMM_DLRLClass* metaClass = NULL;
    LOC_unsigned_long foreignKeysSize = 0;
    DMM_DCPSTopic* mainTopic = NULL;
    DK_ReadData* data = NULL;
    Coll_Iter* sampleIterator = NULL;
    LOC_boolean* validityArray;
    LOC_boolean isValid;
    DMM_DCPSField* validityField = NULL;
    DMM_DLRLClass* targetType;
    DMM_Mapping targetMapping;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(dataSamples);
    /* userData my be NULL */

    metaClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(home);
    mainTopic = DMM_DLRLClass_getMainTopic(metaClass);
    relations = DMM_DLRLClass_getRelations(metaClass);
    foreignKeysSize = Coll_List_getNrOfElements(DMM_DCPSTopic_getForeignKeyFields(mainTopic));
    /* for each object */
    sampleIterator = Coll_List_getFirstElement(dataSamples);
    while(sampleIterator)
    {
        data = (DK_ReadData*)Coll_Iter_getObject(sampleIterator);
        assert(data->action != DK_READ_ACTION_DO_NOTHING);
        if(data->action != DK_READ_ACTION_DELETE &&
           data->action != DK_READ_ACTION_NO_WRITERS_CHANGE)
        {
            /* only resolve relations for objects that where just created by create action,
             * Do not do anything if action is create_deleted as one cant make relations to deleted objects
             * Also do nothing for generations, as it's impossible to have unresolved targets for generations
             */
            if(data->action == DK_READ_ACTION_CREATE)
            {
                /* only resolve targets now, collections where already resolved */
                DK_UnresolvedObjectsUtility_us_resolveUnresolvedElements(
                    &(_this->unresolvedElements),
                    exception,
                    userData,
                    home,
                    data->objectAdmin,
                    data->keyValueArray,
                    TRUE,
                    FALSE);
                DLRL_Exception_PROPAGATE(exception);
            }
            /* explicitly assert that the action state must thus be one of the other states to ensure that
             * if a new state is added it must be added here and thought will be put into what happens here!
             */
            assert( data->action == DK_READ_ACTION_CREATE || data->action == DK_READ_ACTION_CREATE_DELETE ||
                    data->action == DK_READ_ACTION_MODIFY || data->action == DK_READ_ACTION_MODIFY_AND_DELETE ||
                    data->action == DK_READ_ACTION_GENERATION || data->action == DK_READ_ACTION_GENERATION_DELETE);
            /* iterate through all single (1-1) relations */
            relationsCount = 0;
            validityArray = (LOC_boolean*)((data->foreignKeyValueArray)+foreignKeysSize);
            iterator = Coll_List_getFirstElement(relations);
            while(iterator)
            {
                aRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(iterator);
                /* determine if we should interprete the foreign keyfields or not */
                validityField = DMM_DLRLRelation_getValidityField(aRelation);
                /* if a validity field is defined, then it's leading (the kernel thus allows default mapping
                 * (of the target object) & a validity field combo). If unwanted this should be prevented at another
                 * stage (ie content validation within the code generator). The kernel will allow a (0,0,0) oid while
                 * the validity field says the relation should be interpreted, it would result in an unresolved object
                 */
                if(validityField)
                {
                    assert(validityArray);
                    assert(DMM_DCPSField_getUserDefinedIndex(validityField) >= 0);
                    isValid = validityArray[DMM_DCPSField_getUserDefinedIndex(validityField)];
                } else
                {
                    targetType = DMM_DLRLRelation_getType(aRelation);
                    targetMapping = DMM_DLRLClass_getMapping(targetType);
                    if(targetMapping == DMM_MAPPING_DEFAULT)
                    {
                        isValid = DK_ObjectReader_us_determineDefaultRelationIsValid(aRelation, data->foreignKeyValueArray);
                    } else
                    {
                        isValid = TRUE;/* predefined and no validity field defaults to this */
                    }
                }
                DK_ObjectRelationReader_us_processSingleRelation(_this, exception, userData, aRelation, relationsCount,
                                                                        home, data, isValid);
                DLRL_Exception_PROPAGATE(exception);
                relationsCount++;
                iterator = Coll_Iter_getNext(iterator);
            }
        }
        if(data->foreignKeyValueArray)
        {
            DK_DCPSUtility_us_destroyValueArray(data->foreignKeyValueArray, foreignKeysSize);
            data->foreignKeyValueArray = NULL;
        }
        sampleIterator = Coll_Iter_getNext(sampleIterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_ObjectReader_us_determineDefaultRelationIsValid(
    DMM_DLRLRelation* relation,
    void* foreignKeyValueArray)
{
    Coll_List* targetKeys;
    Coll_Iter* iterator;
    LOC_unsigned_long count = 0;
    DMM_DCPSField* targetKey;

    assert(relation);
    assert(foreignKeyValueArray);

    targetKeys = DMM_DLRLRelation_getOwnerKeys(relation);
    assert(Coll_List_getNrOfElements(targetKeys) == 3 || Coll_List_getNrOfElements(targetKeys) == 4);
    /* start at the back, because this list is either 3 or 4 elements long. But the last 3 elements
     * are always the OID long elements! In the case we have 4 elements in the list it means
     * the first element is a string, which is not relevant for determining validity
     */
    iterator = Coll_List_getLastElement(targetKeys);
    while(iterator && count < 3)
    {
        targetKey = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        /* if any of the fields is not equal to 0, then return true, because that indicates validity...*/
        if(DK_DCPSUtility_us_getLongValueFromArray(foreignKeyValueArray, DMM_DCPSField_getUserDefinedIndex(targetKey)) != 0)
        {
            return TRUE;
        }
        iterator = Coll_Iter_getPrev(iterator);
        count++;
    }
    return FALSE;
}

void
DK_ObjectReader_us_resetAllRelationsToObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin,
    void* array,
    LOC_unsigned_long arraySize)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* isRelatedFromHolder = NULL;
    DK_Entity* owner = NULL;
    LOC_unsigned_long index = 0;
    DK_Class classID;
    void* newArray = NULL;
    DK_ObjectAdmin* ownerAdmin = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DMM_Basis type;
    DK_ObjectHolder* unresolvedHolder = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    assert(objectAdmin);
    assert((array && (arraySize > 0))|| (!array && (arraySize == 0)));
    iterator = Coll_Set_getFirstElement(&(objectAdmin->isRelatedFrom));
    while(iterator)
    {
        isRelatedFromHolder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        /* all OHs must be resolved, otherwise they shouldnt be in this list! */
        assert(DK_ObjectHolder_us_isResolved(isRelatedFromHolder));
        /* Depending if the holders owner is a object admin (we are dealing with a 1-1 relation) or a collection */
        /* (we are dealing with a 1-n relation) we take the appropiate action to update the owner */
        owner = DK_ObjectHolder_us_getOwner(isRelatedFromHolder);
        index = DK_ObjectHolder_us_getIndex(isRelatedFromHolder);
        classID = DK_Entity_getClassID(owner);
        newArray = DK_DCPSUtility_us_cloneValueArray(exception, array, arraySize);
        DLRL_Exception_PROPAGATE(exception);
        if(classID == DK_CLASS_OBJECT_ADMIN)
        {
            /* the owner is an object admin, the only thing we need to do is reset the objectholder on language */
            /* specific layer. */
            ownerAdmin = (DK_ObjectAdmin*)owner;
            ownerHome = DK_ObjectAdmin_us_getHome(ownerAdmin);
            objectRelationReaderBridge.setRelatedObjectForObject(
                userData,
                ownerHome,
                ownerAdmin,
                index,
                NULL,
                TRUE);

            DK_ObjectHolder_us_setValues(isRelatedFromHolder, newArray, arraySize);
            unresolvedHolder = isRelatedFromHolder;
        } else
        {
        /* BEWARE !!!!: The following code WILL lead to a newly instantiate ObjectHolder object! The
         * 'doModificationRemoval' operation actually transfers ownership of the holder we just retrieved to
         * the removal list of the collection, where it is either immediately destroyed or will be destroyed
         * once the modification info of this update round is cleared. The holder returned by the operation
         * is thus a different pointer then we put into it. We thus cannot use this pointer to clear the 'isRelatedFrom'
         * set!
         */
            /* the owner is a collection, we need to  remove element from the collection*/
            assert(classID == DK_CLASS_SET_ADMIN || classID == DK_CLASS_MAP_ADMIN);
            ownerAdmin = DK_Collection_us_getOwner((DK_Collection*)owner);
            if(ownerAdmin)
            {
                ownerHome = DK_ObjectAdmin_us_getHome(ownerAdmin);
            } else
            {
                /* MUST RESET TO NULL!!, else when updating the modified list at the end we might end up updating a
                 * home belonging to a previous element!!
                 */
                ownerHome = NULL;
            }
            type = DMM_DLRLMultiRelation_getBasis(((DK_Collection*)owner)->metaRelation);
            /* an OH that is being removed can NEVER be already added, modified OR removed, as the DLRL does */
            /* not allow relations being made to deleted objects! These relations are automatically broken */
            /* and unregistered from the target object, which means it wouldnt show up in this operation */
            unresolvedHolder = DK_CollectionReader_us_doModificationRemoval(exception, (DK_Collection*)owner,
                                                            type, newArray, arraySize, isRelatedFromHolder);
            DLRL_Exception_PROPAGATE(exception);
            /* after this set we will make the OH an unresolved element, so we need to increase the number */
            /* of unresolved element of this collection as well! */
            DK_Collection_us_increaseNrOfUnresolvedElements((DK_Collection*)owner);
        }
        /* since the target of the OH is being deleted we have to turn the OH into an unresolved OH */
        /* to accomplish that we first need to clone the keys of the target object, then make the OH */
        /* unresolved and finally insert it into the unresolved list */
        DK_ObjectHomeAdmin_us_registerUnresolvedElement(home, exception, userData, unresolvedHolder, index);
        DLRL_Exception_PROPAGATE(exception);
        /* if the owner admin was not modified, then we need to notify its home that the object has been */
        /* modified */
        if(ownerAdmin && (DK_ObjectAdmin_us_getReadState(ownerAdmin) == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED))
        {
            assert(ownerHome);
            DK_ObjectHomeAdmin_us_markObjectAsModified(ownerHome, exception, ownerAdmin);
            DLRL_Exception_PROPAGATE(exception);
        }
        /* we need to switch to the next element. */
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(objectAdmin->isRelatedFrom), (void*)isRelatedFromHolder);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* This operation also release the taken over ref counts of the user data of the instance handles in case
 * that the previousGeneration of the DK_ReadData has a value, and it also resets the userData of the handle
 * in case the objectAdmin was deleted.
 */
void
DK_ObjectReader_us_clearAllRelationsToDeletedObjects(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* dataSamples)
{
    Coll_Iter* sampleIterator = NULL;
    DK_ReadData* data = NULL;
    void* oldUserData = NULL;
    DMM_DLRLClass* metaClass = NULL;
    DMM_DCPSTopic* mainTopic = NULL;
    void* array = NULL;
    LOC_unsigned_long arraySize = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);


    metaClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(home);
    mainTopic = DMM_DLRLClass_getMainTopic(metaClass);
    arraySize = Coll_List_getNrOfElements(DMM_DCPSTopic_getKeyFields(mainTopic));
    sampleIterator = Coll_List_getFirstElement(dataSamples);
    while(sampleIterator)
    {
        data = (DK_ReadData*)Coll_Iter_getObject(sampleIterator);
        assert(data);
        assert(data->objectAdmin);
        array = DK_ObjectAdmin_us_getKeyValueArray(data->objectAdmin);
        if(data->previousGeneration &&
           (DK_ObjectAdmin_us_getReadState(data->objectAdmin) != DK_OBJECT_STATE_OBJECT_DELETED))
        {
            /* In this case a new generation occured of the object. The keys
             * of the topic that the two objects are based upon are the same.
             * This means that we should transfer the relations pointing to
             * the to-be-deleted object (previousGeneration) should now
             * automatically point to the new generation (objectAdmin).
             * This needs to be done only in this very special case, as if
             * we were to simply destroy the relations pointing towards the
             * to-be-deleted object, then those relations would enter the
             * unresolved list and as you may (or may not) know unresolved
             * lists are only evaluated when a new object that was never
             * seen before enters the DLRL, and as we already saw the new
             * object generation it would mean that the unresolved list is
             * never evaluated and thus that any unresolved object will
             * never turn back into a normal relation (unless the object
             * maintain the relation is updated or in case of collections if
             * the element topic is updated). And this is wrong, therefore
             * we must transfer the relations of the old generation to the
             * new generation.
             */
            assert(DK_ObjectAdmin_us_getReadState(data->previousGeneration) == DK_OBJECT_STATE_OBJECT_DELETED);
            DK_ObjectReader_us_copyAllRelationsToNewGeneration(
                _this,
                exception,
                userData,
                data->previousGeneration,
                data->objectAdmin);
            DLRL_Exception_PROPAGATE(exception);
            /* no more objects should point to the previous generation
             * object
             */
            assert(!Coll_Set_getFirstElement(&(data->previousGeneration->isRelatedFrom)));
            /* release the previous generation, as it was reference counted!*/
            DK_ObjectAdmin_us_setHandle(
                data->previousGeneration,
                DK_DCPSUtility_ts_getNilHandle());
            DK_Entity_ts_release((DK_Entity*)data->previousGeneration);
            data->previousGeneration = NULL;
            /* do NOT release the ref count of the data->objectAdmin, it is
             * not deleted, so it's ref count is still owned by the user
             * data of the handle...
             */
        } else
        {
            if(data->previousGeneration)
            {
                /*always has read state deleted, if defined */
                assert(DK_ObjectAdmin_us_getReadState(data->previousGeneration) == DK_OBJECT_STATE_OBJECT_DELETED);
                DK_ObjectReader_us_resetAllRelationsToObject(
                    _this,
                    exception,
                    userData,
                    home,
                    data->previousGeneration,
                    array,
                    arraySize);
                DLRL_Exception_PROPAGATE(exception);
                /* no more objects should point to this object*/
                assert(!Coll_Set_getFirstElement(&(data->previousGeneration->isRelatedFrom)));
                /* prevent that user data is reset later on, by setting the
                 * handle of the previous generation to nil
                 */
                DK_ObjectAdmin_us_setHandle(
                    data->previousGeneration,
                    DK_DCPSUtility_ts_getNilHandle());
                /* release the previous generation, as it was reference
                 * counted!
                 */
                DK_Entity_ts_release((DK_Entity*)data->previousGeneration);
                data->previousGeneration = NULL;
            } /*else do nothing*/
            if(DK_ObjectAdmin_us_getReadState(data->objectAdmin) == DK_OBJECT_STATE_OBJECT_DELETED)
            {
                DK_ObjectReader_us_resetAllRelationsToObject(
                    _this,
                    exception,
                    userData,
                    home,
                    data->objectAdmin,
                    array,
                    arraySize);
                DLRL_Exception_PROPAGATE(exception);
                /* no more objects should point to this object*/
                assert(!Coll_Set_getFirstElement(&(data->objectAdmin->isRelatedFrom)));
                /* finally reset the userdata in the instance handle, to
                 * allow the object to be deleted
                 */
                assert(!u_instanceHandleIsNil(data->handle));
                assert(u_instanceHandleIsEqual(
                    data->handle,
                    DK_ObjectAdmin_us_getHandle(data->objectAdmin)));
                oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
                    exception,
                    data->handle,
                    NULL);
                DLRL_Exception_PROPAGATE(exception);
                assert(oldUserData == data->objectAdmin);
                DK_Entity_ts_release((DK_Entity*)oldUserData);
                data->objectAdmin = NULL;
            }/*else do nothing*/
        }

        sampleIterator = Coll_Iter_getNext(sampleIterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_copyAllRelationsToNewGeneration(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* prevGenObject,
    DK_ObjectAdmin* nextGenObject)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* isRelatedFromHolder = NULL;
    DK_Entity* owner = NULL;
    LOC_unsigned_long index = 0;
    DK_Class classID;
    DK_ObjectAdmin* ownerAdmin = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DMM_Basis type;
    LOC_unsigned_long collectionsSize;
    DK_Collection** newGenCollections;
    DK_Collection** oldGenCollections;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(prevGenObject);
    assert(nextGenObject);

    iterator = Coll_Set_getFirstElement(&(prevGenObject->isRelatedFrom));
    while(iterator)
    {
        isRelatedFromHolder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        /* all OHs must be resolved, otherwise they shouldnt be in this list! */
        assert(DK_ObjectHolder_us_isResolved(isRelatedFromHolder));
        owner = DK_ObjectHolder_us_getOwner(isRelatedFromHolder);
        index = DK_ObjectHolder_us_getIndex(isRelatedFromHolder);
        classID = DK_Entity_getClassID(owner);
        if(classID == DK_CLASS_OBJECT_ADMIN)
        {
            ownerAdmin = (DK_ObjectAdmin*)owner;
            ownerHome = DK_ObjectAdmin_us_getHome(ownerAdmin);
            DK_ObjectHolder_us_setTarget(isRelatedFromHolder, nextGenObject);
            DK_ObjectAdmin_us_registerIsRelatedFrom(nextGenObject, exception, isRelatedFromHolder);
            DLRL_Exception_PROPAGATE(exception);
            /* when changing the relation target from the prev gen to the next gen object the relation is always seen as
             * valid, because the relation first pointed to the prev gen object, and therefore must have been valid
             * and now points to the next gen object... ie why would it suddenly not be valid?
             */
            objectRelationReaderBridge.setRelatedObjectForObject(
                userData,
                ownerHome,
                ownerAdmin,
                index,
                nextGenObject,
                TRUE);

        } else
        {
            /* the owner is a collection */
            assert(classID == DK_CLASS_SET_ADMIN || classID == DK_CLASS_MAP_ADMIN);
            ownerAdmin = DK_Collection_us_getOwner((DK_Collection*)owner);
            /* ownerAdmin could be null, if this is an unresolved collection */
            if(ownerAdmin)
            {
                ownerHome = DK_ObjectAdmin_us_getHome(ownerAdmin);
            }
            type = DMM_DLRLMultiRelation_getBasis(((DK_Collection*)owner)->metaRelation);
            DK_CollectionReader_us_commitElementModification(exception, isRelatedFromHolder, nextGenObject,
                                                        prevGenObject, type, (DK_Collection*)owner);
            DLRL_Exception_PROPAGATE(exception);
        }
        if(ownerAdmin && (DK_ObjectAdmin_us_getReadState(ownerAdmin) == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED))
        {
            /* this condition being  not true in the past caused a bug, so assert it for safety */
            assert(ownerHome == DK_ObjectAdmin_us_getHome(ownerAdmin));
            DK_ObjectHomeAdmin_us_markObjectAsModified(ownerHome, exception, ownerAdmin);
            DLRL_Exception_PROPAGATE(exception);
        }
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(prevGenObject->isRelatedFrom), (void*)isRelatedFromHolder);
    }
    collectionsSize = DK_ObjectAdmin_us_getCollectionsSize(prevGenObject);
    assert(collectionsSize == DK_ObjectAdmin_us_getCollectionsSize(nextGenObject));
    newGenCollections = DK_ObjectAdmin_us_getCollections(nextGenObject);
    oldGenCollections = DK_ObjectAdmin_us_getCollections(prevGenObject);
    for(count = 0; count < collectionsSize; count++)
    {
        DK_Collection* oldGenColl;
        DK_Collection* newGenColl;

        newGenColl = newGenCollections[count];
        oldGenColl = oldGenCollections[count];

        DK_ObjectReader_us_performGenerationalCollectionCopy(
            _this,
            exception,
            userData,
            oldGenColl,
            newGenColl);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_performGenerationalCollectionCopy(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_Collection* oldGenColl,
    DK_Collection* newGenColl)
{
    DMM_DLRLMultiRelation* metaRelation;
    DMM_Basis type;
    Coll_Set* holders;
    Coll_Iter* iterator;
    DK_ObjectHolder* holder;
    DK_ObjectHolder* newHolder;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /*userData mye be null*/
    assert(oldGenColl);
    assert(newGenColl);

    metaRelation = DK_Collection_us_getMetaRepresentative(newGenColl);
    type = DMM_DLRLMultiRelation_getBasis(metaRelation);

    if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
    {
        holders = DK_MapAdmin_us_getObjectHolders((DK_MapAdmin*)oldGenColl);
        iterator = Coll_Set_getFirstElement(holders);
        while(iterator)
        {
            void* keyValue = NULL;

            holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
            newHolder = DK_ObjectReader_us_createGenerationHolderClone(
                exception,
                newGenColl,
                holder,
                type,
                &keyValue);
            DLRL_Exception_PROPAGATE(exception);
            DK_MapAdmin_us_doPut(
                (DK_MapAdmin*)newGenColl,
                exception,
                keyValue,
                newHolder);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    else {
        assert(type == DMM_BASIS_SET);
        holders = DK_SetAdmin_us_getHolders((DK_SetAdmin*)oldGenColl);
        iterator = Coll_Set_getFirstElement(holders);
        while(iterator)
        {
            holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
            newHolder = DK_ObjectReader_us_createGenerationHolderClone(
                exception,
                newGenColl,
                holder,
                type,
                NULL);
            DLRL_Exception_PROPAGATE(exception);
            DK_SetAdmin_us_doAdd(
                (DK_SetAdmin*)newGenColl,
                exception,
                newHolder);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    /* do not copy over removed elements, this was a wrong decision. After talks
     * between erik and emiel we decided that removed elements only belong to
     * the 'old' generational collection owner

    removedElements = DK_Collection_us_getRemovedElements(oldGenColl);
    iterator = Coll_List_getFirstElement(removedElements);
    while(iterator)
    {
        void* keyValue = NULL;

        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);

        newHolder = DK_ObjectReader_us_createGenerationHolderClone(
            exception,
            newGenColl,
            holder,
            type,
            &keyValue);
        DLRL_Exception_PROPAGATE(exception);
        if(type == DMM_BASIS_STR_MAP || type == DMM_BASIS_INT_MAP)
        {
            DK_MapAdmin_us_addRemovedElement(
                (DK_MapAdmin*)newGenColl,
                exception,
                &keyValue,
                holder);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            assert(type == DMM_BASIS_SET);
            DK_SetAdmin_us_addRemovedElement(
                (DK_SetAdmin*)newGenColl,
                exception,
                newHolder);
            DLRL_Exception_PROPAGATE(exception);
        }
        iterator = Coll_Iter_getNext(iterator);
    }*/

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DK_ObjectHolder*
DK_ObjectReader_us_createGenerationHolderClone(
    DLRL_Exception* exception,
    DK_Collection* newGenColl,
    DK_ObjectHolder* holder,
    DMM_Basis type,
    void** keyValue)
{
    DK_ObjectAdmin* target;
    LOC_unsigned_long index;
    u_instanceHandle handle;
    void* key;
    DK_ObjectHolder* newHolder;
    void* oldUserData;

    DLRL_INFO(INF_EXIT);

    assert(exception);
    assert(newGenColl);
    assert(holder);
    assert((keyValue && !*keyValue) || !keyValue);

    assert(holder);
    assert(DK_ObjectHolder_us_isResolved(holder));

    target = DK_ObjectHolder_us_getTarget(holder);
    index = DK_ObjectHolder_us_getIndex(holder);
    handle = DK_ObjectHolder_us_getHandle(holder);
    key = DK_ObjectHolder_us_getUserData(holder);

    /* create a new holder for the new generation collection */
    newHolder = DK_ObjectHolder_newResolved(
        exception,
        (DK_Entity*)newGenColl,
        target,
        handle,
        index);
    DLRL_Exception_PROPAGATE(exception);
    /* copy over the nowriter and disposed counts */
    DK_ObjectHolder_us_setNoWritersCount(newHolder, DK_ObjectHolder_us_getNoWritersCount(holder));
    DK_ObjectHolder_us_setDisposedCount(newHolder, DK_ObjectHolder_us_getDisposedCount(holder));
    if(type == DMM_BASIS_STR_MAP)
    {
        DLRL_STRDUP(
            (*keyValue),
            ((LOC_string)key),
            exception,
            "Unable to allocate memory for copy of the key value");
        assert(*keyValue);
        DK_ObjectHolder_us_setUserData(newHolder, *keyValue);
    } else if(type == DMM_BASIS_INT_MAP)
    {
        DLRL_ALLOC_WITH_SIZE(
            *keyValue,
            sizeof(c_long),
            exception,
            "Unable to allocate memory for copy of the key value");
        *((c_long*)(*keyValue)) = *((c_long*)key);
        assert(*keyValue);
        DK_ObjectHolder_us_setUserData(newHolder, *keyValue);
    }/* else do nothing */
    if(!u_instanceHandleIsNil(handle))
    {
        oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
            exception,
            handle,
            newHolder);
        DLRL_Exception_PROPAGATE(exception);
        assert(oldUserData == holder);
        DK_ObjectHolder_us_setHandleNil(holder);
    }
    DK_ObjectAdmin_us_registerIsRelatedFrom(
        target,
        exception,
        newHolder);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return newHolder;
}

/* process updates in the sequence the collections were added in the meta model. Because the collection readers */
/* were added to the list of collection readers in the same sequence as the multi relations were added we can simply */
/* use a counter, which is performance wise faster. This collection index also indicates the index of the collection */
/* in an object admin instance */
/* Also take note that its important that collections are only processed AFTER the 'main'  objects are processed. */
/* This is to prevent the unresolved list coming into action and to prevent problems as changes to a collection */
/* lead to the read state of the owning object to be adjusted as well. If one were to process the collections first */
/* then it could happen that the state of an owning object is set to modified. If you were then to process the main */
/* objects it would assume no modification status are set yet, if there were then one risks a deleted object ending */
/* up in the modified objects list as well. Which is very much incorrect. */
void
DK_ObjectReader_us_processCollectionUpdates(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_CollectionReader* collectionReader = NULL;
    LOC_unsigned_long count = 0;
    Coll_Iter* iterator = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be NULL */

    iterator = Coll_List_getFirstElement(&(_this->collectionReaders));
    while(iterator)
    {
        collectionReader = Coll_Iter_getObject(iterator);
        DK_CollectionReader_us_processDCPSUpdates(collectionReader, exception, userData, count);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

u_reader
DK_ObjectReader_us_getReader(
    DK_ObjectReader* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->reader;
}

/* assumes the owning home of this data reader is locked as well as any related homes needed to manage relations. */
void
DK_ObjectReader_us_doRead(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    DK_ReadInfo* readInfo)
{
    u_result result = U_RESULT_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    /* TODO ID: 30 */
    assert(_this);
    assert(exception);
    assert(readInfo);

    DLRL_INFO(INF_DCPS, "u_readerRead(...)");
    result =  u_readerRead(_this->reader, DK_ObjectReader_us_readerCopy, (c_voidp)readInfo);
    /* dont forget to propagate the exception which was wrapped in the readInfo struct! */
    DLRL_Exception_PROPAGATE(readInfo->exception);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "%s: DCPS read failed.", ENTITY_NAME);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
v_actionResult
DK_ObjectReader_us_readerCopy(
    c_object object,
    c_voidp arg)
{
    v_actionResult result = V_PROCEED;
    LOC_long errorCode = COLL_OK;
    LOC_unsigned_long nrOfValidityFields = 0;
    LOC_boolean* validityArray = NULL;
    DK_ReadInfo* readInfo = NULL;
    DLRL_Exception* exception = NULL;
    u_instanceHandle handle;
    v_public kernelPublic = NULL;
    v_readerSample sample = NULL;
    v_dataReaderInstance instance = NULL;
    v_message message = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    DK_ReadAction action = DK_READ_ACTION_DO_NOTHING;
    void* sampleData = NULL;
    DK_ReadData* data = NULL;
    LOC_boolean entityExists;
    LOC_boolean noWritersCountChanged;
    LOC_boolean disposedCountChanged;

    DLRL_INFO(INF_ENTER);

    assert(arg);

    readInfo = (DK_ReadInfo*)arg;
    exception = readInfo->exception;
    if(object)
    {
        assert(object);

        sample = v_readerSample(object);
        assert(sample->instance);
        instance = v_dataReaderInstance(sample->instance);
        kernelPublic = v_public(instance);
        handle = u_instanceHandleNew(kernelPublic);
        objectAdmin = (DK_ObjectAdmin*)kernelPublic->userDataPublic;/*may be null*/
        if(objectAdmin)
        {
            entityExists = TRUE;
            if(DK_ObjectAdmin_us_getNoWritersCount(objectAdmin) != instance->noWritersCount)
            {
                noWritersCountChanged = TRUE;
            } else {
                noWritersCountChanged = FALSE;
            }
            if(DK_ObjectAdmin_us_getDisposedCount(objectAdmin) != instance->disposeCount)
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
            /* get the corresponding DLRL kernel data holder */
            DLRL_ALLOC(
                data,
                DK_ReadData,
                exception,
                "Unable to complete operation. Out of resources");
            memset(data, 0, sizeof(DK_ReadData));
            /* store the action for this data element */
            data->action = action;
            /* Store the DCPS sample instance handle */
            data->handle = handle;
            /* set the user data registered with the instance handle. */
            data->objectAdmin = objectAdmin;
            /* set the disposed and no writers counts */
            data->disposedCount = instance->disposeCount;
            data->noWritersCount = instance->noWritersCount;
            if(action != DK_READ_ACTION_DELETE &&
               action != DK_READ_ACTION_NO_WRITERS_CHANGE)
            {
                /* explicitly assert that the action state must thus be one of
                 * the other states to ensure that if a new state is added it
                 * must be added here and thought will be put into what happens
                 * here!
                 */
                assert( data->action == DK_READ_ACTION_CREATE ||
                        data->action == DK_READ_ACTION_CREATE_DELETE ||
                        data->action == DK_READ_ACTION_MODIFY ||
                        data->action == DK_READ_ACTION_MODIFY_AND_DELETE ||
                        data->action == DK_READ_ACTION_GENERATION ||
                        data->action == DK_READ_ACTION_GENERATION_DELETE);
                assert(action < DK_ReadAction_elements);
                /* get topic data associated with the sample */
                message = v_dataReaderSampleTemplate(object)->message;
                sampleData = C_DISPLACE(message, readInfo->offset);
                /* create the ls_topic object, if the action was determined to
                 * be a new generation we must not ensure a new topic instance
                 * is created. Otherwise problems may occur when re-using the
                 * previous topic of the 'old' generation.
                 */
                if(data->action == DK_READ_ACTION_GENERATION ||
                   data->action == DK_READ_ACTION_GENERATION_DELETE)
                {
                    data->ls_topic = objectReaderBridge.createLSTopic(
                        exception,
                        NULL,
                        readInfo->dstInfo,
                        readInfo->copyOut,
                        sampleData);
                } else
                {
                    data->ls_topic = objectReaderBridge.createLSTopic(
                        exception,
                        objectAdmin,
                        readInfo->dstInfo,
                        readInfo->copyOut,
                        sampleData);
                }
                if(!data->ls_topic)
                {
                    DLRL_Exception_THROW(
                        exception,
                        DLRL_DCPS_ERROR,
                        "%s failed to copy a 'C' DCPS data sample into a "
                        "language specific DCPS data sample. Check DCPS error "
                        "log file for (possibly) more information.",
                        ENTITY_NAME);
                }
                /* load all relevant DCPS database values */
                if(data->action == DK_READ_ACTION_CREATE ||
                   data->action == DK_READ_ACTION_CREATE_DELETE)
                {
                    /* only convert key fields if this is a create, otherwise
                     * they are already stored. remember this may return null
                     *(if we are dealing with a singleton)
                     */
                    data->keyValueArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
                        exception,
                        readInfo->keyFields,
                        sampleData,
                        0);
                } else
                {/* just fetch it */
                    assert(data->objectAdmin);
                    data->keyValueArray = DK_ObjectAdmin_us_getKeyValueArray(
                        data->objectAdmin);/*may return null*/
                }
                /* foreign fields can change at any time, so we convert them
                 * always. Allocate some extra capacity so we can append the
                 * validity booleans at the end of the array.
                 */
                nrOfValidityFields = Coll_List_getNrOfElements(
                    readInfo->validityFields);
                data->foreignKeyValueArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(
                    exception,
                    readInfo->foreignKeyFields,
                    sampleData,
                    (nrOfValidityFields*sizeof(LOC_boolean)));
                DLRL_Exception_PROPAGATE(exception);
                /* now we have to append the validity booleans at the end of the array */
                if(nrOfValidityFields > 0)
                {
                    /* shift pointer */
                    validityArray = (LOC_boolean*)((data->foreignKeyValueArray) +
                        Coll_List_getNrOfElements(readInfo->foreignKeyFields));
                    DK_DCPSUtility_us_fillValidityArray(
                        validityArray,
                        readInfo->validityFields,
                        sampleData);
                }
            }
            errorCode = Coll_List_pushBack(&readInfo->dataSamples, (void*)data);
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
        v_actionResultClear(result, V_PROCEED);
    }
    DLRL_INFO(INF_EXIT);
    return result;
}

DK_ReadAction
DK_ObjectReader_us_determineSampleAction(
    LOC_boolean entityExists,
    v_state sampleState,
    v_state instanceState,
    LOC_boolean noWritersCountChanged,
    LOC_boolean disposedCountChanged)
{
    DK_SampleAction choice = DK_SampleAction_Choice_1;
    DK_ReadAction action = DK_READ_ACTION_DO_NOTHING;/* default */

    DLRL_INFO(INF_ENTER);

    while(choice != DK_SampleAction_TERMINATE)
    {
        switch (choice)
        {
        case DK_SampleAction_Choice_1:
            if(entityExists)
            {
                choice = DK_SampleAction_Choice_4;
            } else
            {
                choice = DK_SampleAction_Choice_2;
            }
            break;
        case DK_SampleAction_Choice_2:
            if(v_stateTest(sampleState, L_VALIDDATA))
            {
                choice = DK_SampleAction_Choice_3;
            } else
            {
                action = DK_READ_ACTION_DO_NOTHING;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        case DK_SampleAction_Choice_3:
            if(v_stateTest(instanceState, L_DISPOSED))
            {
                action = DK_READ_ACTION_CREATE_DELETE;
                choice = DK_SampleAction_TERMINATE;
            } else
            {
                action = DK_READ_ACTION_CREATE;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        case DK_SampleAction_Choice_4:
            if(v_stateTest(sampleState, L_NEW))
            {
                choice = DK_SampleAction_Choice_8;
            } else
            {
                choice = DK_SampleAction_Choice_5;
            }
            break;
        case DK_SampleAction_Choice_5:
            if(v_stateTest(instanceState, L_DISPOSED))
            {
                choice = DK_SampleAction_Choice_10;
            } else
            {
                choice = DK_SampleAction_Choice_6;
            }
            break;
        case DK_SampleAction_Choice_6:
            if(!v_stateTest(sampleState, L_READ))
            {
                choice = DK_SampleAction_Choice_9;
            } else
            {
                choice = DK_SampleAction_Choice_7;
            }
            break;
        case DK_SampleAction_Choice_7:
            if(noWritersCountChanged)
            {
                action = DK_READ_ACTION_NO_WRITERS_CHANGE;
                choice = DK_SampleAction_TERMINATE;
            } else
            {
                action = DK_READ_ACTION_DO_NOTHING;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        case DK_SampleAction_Choice_8:
            if(disposedCountChanged)
            {
                choice = DK_SampleAction_Choice_12;
            } else
            {
                choice = DK_SampleAction_Choice_9;
            }
            break;
        case DK_SampleAction_Choice_9:
            if(v_stateTest(sampleState, L_VALIDDATA))
            {
                action = DK_READ_ACTION_MODIFY;
                choice = DK_SampleAction_TERMINATE;
            } else
            {
                choice = DK_SampleAction_Choice_7;
            }
            break;
        case DK_SampleAction_Choice_10:
            if(!v_stateTest(sampleState, L_READ))
            {
                choice = DK_SampleAction_Choice_11;
            } else
            {
                action = DK_READ_ACTION_DELETE;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        case DK_SampleAction_Choice_11:
            if(v_stateTest(sampleState, L_VALIDDATA))
            {
                action = DK_READ_ACTION_MODIFY_AND_DELETE;
                choice = DK_SampleAction_TERMINATE;
            } else
            {
                action = DK_READ_ACTION_DELETE;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        case DK_SampleAction_Choice_12:
            if(v_stateTest(instanceState, L_DISPOSED))
            {
                choice = DK_SampleAction_Choice_14;
            } else
            {
                choice = DK_SampleAction_Choice_13;
            }
            break;
        case DK_SampleAction_Choice_13:
            if(v_stateTest(sampleState, L_VALIDDATA))
            {
                action = DK_READ_ACTION_GENERATION;
                choice = DK_SampleAction_TERMINATE;
            } else
            {
                action = DK_READ_ACTION_DELETE;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        case DK_SampleAction_Choice_14:
            if(v_stateTest(sampleState, L_VALIDDATA))
            {
                action = DK_READ_ACTION_GENERATION_DELETE;
                choice = DK_SampleAction_TERMINATE;
            } else
            {
                action = DK_READ_ACTION_DELETE;
                choice = DK_SampleAction_TERMINATE;
            }
            break;
        default:
            /*TODO error*/
            break;
        }
    }
    DLRL_INFO(INF_EXIT);
    return action;
}

#if 0
/*
 if todays date is after august 2008, then feel free to delete the code within
 this 'if 0' statement, just keeping the code now so i can re-use it if i
 change my mind :)
*/
DK_ReadAction
DK_ObjectReader_us_determineSampleAction(
    LOC_boolean entityExists,
    v_state sampleState,
    v_state instanceState
    LOC_boolean noWritersCountChanged,
    LOC_boolean disposedCountChanged)
{
    DK_ReadAction action = DK_READ_ACTION_DO_NOTHING;/* default */

    DLRL_INFO(INF_ENTER);

    /* The following state machine is documented in a magicdraw design */

    if(entityExists)
    {
        if(v_stateTest(sampleState, L_NEW))
        {
            if(disposedCountChanged)
            {
                if(v_stateTest(instanceState, L_DISPOSED))
                {
                    if(v_stateTest(sampleState, L_VALIDDATA))
                    {
                        action = DK_READ_ACTION_GENERATION_DELETE;
                    } else
                    {
                        action = DK_READ_ACTION_DELETE;
                    }
                } else
                {
                    if(v_stateTest(sampleState, L_VALIDDATA))
                    {
                        action = DK_READ_ACTION_GENERATION;
                    } else
                    {
                        action = DK_READ_ACTION_DELETE;
                    }
                }
            } else
            {
                if(v_stateTest(sampleState, L_VALIDDATA))
                {
                    action = DK_READ_ACTION_MODIFY;
                } else
                {
                    if(noWritersCountChanged)
                    {
                        /*option yes*/
                        action = DK_READ_ACTION_NO_WRITERS_CHANGE;
                    } else
                    {
                        action = DK_READ_ACTION_DO_NOTHING;
                    }
                }
            }
        } else
        {
            if(v_stateTest(instanceState, L_DISPOSED))
            {
                if(!v_stateTest(sampleState, L_READ))
                {
                    if(v_stateTest(sampleState, L_VALIDDATA))
                    {
                        action = DK_READ_ACTION_MODIFY_AND_DELETE;
                    } else
                    {
                        action = DK_READ_ACTION_DELETE;
                    }
                } else
                {
                    action = DK_READ_ACTION_DELETE;
                }
            } else
            {
                if(!v_stateTest(sampleState, L_READ))
                {
                    if(v_stateTest(sampleState, L_VALIDDATA))
                    {
                        ation = DK_READ_ACTION_MODIFY;
                    } else
                    {
                        if(noWritersCountChanged)
                        {
                            /*option yes*/
                            action = DK_READ_ACTION_NO_WRITERS_CHANGE;
                        } else
                        {
                            action = DK_READ_ACTION_DO_NOTHING;
                        }
                    }
                } else
                {
                    if(noWritersCountChanged)
                    {
                        /*option yes*/
                        action = DK_READ_ACTION_NO_WRITERS_CHANGE;
                    } else
                    {
                        action = DK_READ_ACTION_DO_NOTHING;
                    }
                }
            }
        }
    } else
    {
        if(v_stateTest(sampleState, L_VALIDDATA))
        {
            if(v_stateTest(instanceState, L_DISPOSED))
            {
                action = DK_READ_ACTION_CREATE_DELETE;
            } else
            {
                action = DK_READ_ACTION_CREATE;
            }
        } else
        {
            action = DK_READ_ACTION_DO_NOTHING;
        }
    }
    return action
}
#endif

void
DK_ObjectReader_us_processObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data,
    LOC_unsigned_long keysSize)
{
    void* oldUserData = NULL;
    DK_ObjectState readState;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be NULL */
    assert(data);
    assert(data->action < DK_ReadAction_elements);
    /*this action state shouldnt be encountered here*/
    assert(data->action != DK_READ_ACTION_DO_NOTHING);
    assert(!u_instanceHandleIsNil(data->handle));
    assert(home);

    /* the following sequence of action events is checked, starting with the
     * most common action event and ending with the least common one. Exception
     * are the create_delete and generation_delete events they are combined with
     * the create and generation events respectively for code maintainability
     * reasons.
     * The sequence is as follows:
     * DK_READ_ACTION_MODIFY
     * DK_READ_ACTION_CREATE || DK_READ_ACTION_CREATE_DELETE
     * DK_READ_ACTION_DELETE
     * DK_READ_ACTION_MODIFY_AND_DELETE
     * DK_READ_ACTION_GENERATION || DK_READ_ACTION_GENERATION_DELETE
     * DK_READ_ACTION_NO_WRITERS_CHANGE
     */
    /* MODIFICATION */
    if(data->action == DK_READ_ACTION_MODIFY)
    {
        assert(data->objectAdmin);
        assert(data->ls_topic);

        DK_ObjectReader_us_processModifiedObject(
            _this,
            exception,
            userData,
            home,
            data);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* CREATION or CREATE DELETED*/
    else if((data->action == DK_READ_ACTION_CREATE) ||
            (data->action == DK_READ_ACTION_CREATE_DELETE))
    {
        assert(!data->objectAdmin);
        assert(data->ls_topic);

        if(data->action == DK_READ_ACTION_CREATE)
        {
            readState = DK_OBJECT_STATE_OBJECT_NEW;
        } else
        {
            assert(data->action == DK_READ_ACTION_CREATE_DELETE);
            readState = DK_OBJECT_STATE_OBJECT_DELETED;
        }
        data->objectAdmin = DK_ObjectAdmin_new(
            exception,
            userData,
            home,
            data->handle,
            data->keyValueArray,
            keysSize,
            readState,
            DK_OBJECT_STATE_OBJECT_VOID,
            NULL,
            TRUE);
        DLRL_Exception_PROPAGATE(exception);
        DK_ObjectReader_us_processNewObject(
            _this,
            exception,
            userData,
            home,
            data);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* DELETION */
    else if(data->action == DK_READ_ACTION_DELETE ||
            data->action == DK_READ_ACTION_MODIFY_AND_DELETE)
    {
        assert(data->objectAdmin);
        assert(!data->ls_topic ||
               (data->ls_topic && (data->action == DK_READ_ACTION_MODIFY_AND_DELETE)));

        DK_ObjectReader_us_processDisposedObject(
            _this,
            exception,
            userData,
            home,
            data);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* NEW GENERATION or NEW DELETED GENERATION*/
    else if((data->action == DK_READ_ACTION_GENERATION) ||
               (data->action == DK_READ_ACTION_GENERATION_DELETE))
    {
        assert(data->objectAdmin);
        assert(data->ls_topic);
        assert(data->keyValueArray ==
            DK_ObjectAdmin_us_getKeyValueArray(data->objectAdmin));

        if(data->action == DK_READ_ACTION_GENERATION)
        {
            readState = DK_OBJECT_STATE_OBJECT_NEW;
        } else
        {
            assert(data->action == DK_READ_ACTION_GENERATION_DELETE);
            readState = DK_OBJECT_STATE_OBJECT_DELETED;
        }
        DK_ObjectReader_us_processDisposedObject(
            _this,
            exception,
            userData,
            home,
            data);
        DLRL_Exception_PROPAGATE(exception);
        /* we need to reset the instance handle, so it can be used when creating
         * the new object admin we will also allow the new generation to take
         * over the key value array of the 'old' generation. we will store the
         * (ref counted) object admin in a 'previous generation' value in the
         * read data, so it can be properly cleared when all deleted objects are
         * cleaned.
         */
        assert(u_instanceHandleIsEqual(data->handle,
            DK_ObjectAdmin_us_getHandle(data->objectAdmin)));
        oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
            exception,
            data->handle,
            NULL);
        DLRL_Exception_PROPAGATE(exception);
        assert(oldUserData == data->objectAdmin);
        DK_ObjectAdmin_us_setKeyValueArray(data->objectAdmin, NULL, 0);
        /* DK_Entity_ts_release((DK_Entity*)oldUserData); no release, let it
         * be taken over!
         */
        /* takes over the ref count of the instance handle!!! */
        data->previousGeneration = data->objectAdmin;
        data->objectAdmin = NULL;
        /* disposed object allow the new generation to take over the key value
         * array! It has been set to null in the to be destroyed object so the
         * key value array can be used in the new generation.
         */
        assert(!data->objectAdmin);
        data->objectAdmin = DK_ObjectAdmin_new(
            exception,
            userData,
            home,
            data->handle,
            data->keyValueArray,
            keysSize,
            readState,
            DK_OBJECT_STATE_OBJECT_VOID,
            NULL,
            TRUE);
        DLRL_Exception_PROPAGATE(exception);
        assert(data->objectAdmin);
        assert(data->ls_topic);
        DK_ObjectReader_us_processNewObject(
            _this,
            exception,
            userData,
            home,
            data);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        assert(data->action == DK_READ_ACTION_NO_WRITERS_CHANGE);
        /* we currently ignore this, see dds295 */
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/*******************************************************************************
******************** Processing new DLRL object operations *********************
*******************************************************************************/

/* assumes all homes that play a role in the creation of a new object are
 * locked.
 */
void
DK_ObjectReader_us_processNewObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data)
{
    void* oldUserData = NULL;
    LOC_long errorCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    assert(data);

     /* Sets the object admin as the user data of the instance handle.
     * Takes over the ref count of the new of the object admin.
     */
    oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(
        exception,
        data->handle,
        (void *)data->objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
    assert(!oldUserData);

    /* Update/create the object */
    DK_ObjectReader_us_updateObjectAdmin(
        _this,
        exception,
        userData,
        home,
        data);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_setNoWritersCount(data->objectAdmin, data->noWritersCount);
    DK_ObjectAdmin_us_setDisposedCount(data->objectAdmin, data->disposedCount);

    /* This operation just matches any unresolved collections with this owner,
     * it doesnt make the language specific links that is covered in the
     * createCollections step
     */
    DK_UnresolvedObjectsUtility_us_resolveUnresolvedElements(
        &(_this->unresolvedElements),
        exception,
        userData,
        home,
        data->objectAdmin,
        data->keyValueArray,
        FALSE,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    /* This operation creates any collection that were not contained within the
     * unresolved list (which is ussually empty) it also make links with the
     * language specific objects for all applications
     */
    DK_ObjectAdmin_us_createCollections(data->objectAdmin, exception, userData);
    DLRL_Exception_PROPAGATE(exception);

    /* finally the newly created object is stored in the new samples list */
    if(DK_ObjectAdmin_us_getReadState(data->objectAdmin) ==
        DK_OBJECT_STATE_OBJECT_NEW)
    {
        errorCode = Coll_List_pushBack(
            &(_this->newSamples),
            (void *)DK_Entity_ts_duplicate((DK_Entity*)data->objectAdmin));
    } else
    {
        assert(DK_ObjectAdmin_us_getReadState(data->objectAdmin) ==
            DK_OBJECT_STATE_OBJECT_DELETED);
        errorCode = Coll_List_pushBack(
            &(_this->deletedSamples),
            (void *)DK_Entity_ts_duplicate((DK_Entity*)data->objectAdmin));
    }
    if(errorCode != COLL_OK)
    {
        DK_Entity_ts_release((DK_Entity*)data->objectAdmin);
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add a "
            "processed new sample to the samples list within %s '%s'. "
            "Allocation error when adding the processed sample.",
            "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(home->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/*********************************************************************************************
************************** Processing modified DLRL object operations ************************
*********************************************************************************************/
/* assumes all homes that play a role in the modification of an object are locked. */
void
DK_ObjectReader_us_processModifiedObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be NULL */
    assert(home);
    assert(data);

    DK_ObjectReader_us_updateObjectAdmin(_this, exception, userData, home, data);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_setNoWritersCount(data->objectAdmin, data->noWritersCount);
    DK_ObjectAdmin_us_setDisposedCount(data->objectAdmin, data->disposedCount);
    if(data->objectAdmin->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
    {
        assert(data->objectAdmin->home->objectReader == _this);
        DK_ObjectAdmin_us_setReadState(data->objectAdmin, DK_OBJECT_STATE_OBJECT_MODIFIED);
        errorCode = Coll_List_pushBack(&(_this->modifiedSamples),
                                                (void *)DK_Entity_ts_duplicate((DK_Entity*)data->objectAdmin));
        if(errorCode != COLL_OK)
        {
            DK_Entity_ts_release((DK_Entity*)data->objectAdmin);
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add a processed modified sample to the modified samples list within %s '%s'. "
                "Allocation error when adding the processed sample.","DLRL Kernel ObjectHomeAdmin",
                    DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(DK_TopicInfo_us_getOwner(_this->topicInfo))));
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/*********************************************************************************************
************************** Processing disposed DLRL object operations ************************
*********************************************************************************************/
/* assumes all homes that play a role in the deletion of an object are locked. */
void
DK_ObjectReader_us_processDisposedObject(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    /* userData may be NULL */
    assert(data);

    /* ONLY UPDATE IF ITS A NEW SAMPLE */
    if(data->action == DK_READ_ACTION_MODIFY_AND_DELETE)
    {
        assert(data->ls_topic);
        DK_ObjectReader_us_updateObjectAdmin(_this, exception, userData, home, data);
    }
#ifndef NDEBUG
    else if(data->action == DK_READ_ACTION_GENERATION || data->action == DK_READ_ACTION_GENERATION_DELETE)
    {
        assert(data->ls_topic);
    } else
    {
        assert(!data->ls_topic);
    }
#endif
    assert(data->objectAdmin->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
    DK_ObjectAdmin_us_setNoWritersCount(data->objectAdmin, data->noWritersCount);
    DK_ObjectAdmin_us_setDisposedCount(data->objectAdmin, data->disposedCount);

    DK_ObjectAdmin_us_setReadState(data->objectAdmin, DK_OBJECT_STATE_OBJECT_DELETED);
    errorCode = Coll_List_pushBack(&(_this->deletedSamples),
                                            (void *)DK_Entity_ts_duplicate((DK_Entity*)data->objectAdmin));
    if(errorCode != COLL_OK)
    {
        DK_Entity_ts_release((DK_Entity*)data->objectAdmin);
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to add a processed deleted sample to the deleted samples list within %s '%s'. "
            "Allocation error when adding the processed sample.", "DLRL Kernel ObjectHomeAdmin",
            DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(DK_TopicInfo_us_getOwner(_this->topicInfo))));
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* assumes all homes that play a role in the updating of an object are locked. */
void
DK_ObjectReader_us_updateObjectAdmin(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    assert(data);

    if(DK_ObjectAdmin_us_getLSObject(data->objectAdmin))
    {
        objectReaderBridge.updateObject(exception, userData, home, data->objectAdmin, data->ls_topic);
        DLRL_Exception_PROPAGATE(exception);
    /* load the object content. Could be that the auto deref boolean changed between */
    /* update round, making this neccesary */
    } else if(DK_ObjectHomeAdmin_us_getAutoDeref(home))
    {
        DK_ObjectReader_us_loadObjectContent(_this, exception, userData, home, data);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_loadObjectContent(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data)
{
    DLRL_LS_object ls_object = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    assert(data);
    /* userData may be NULL */

    /* cant modified state information here, should be done when this operation is called and */
    /* when the sample state is determined else we have a chance of overwritten the secondary state if this operation */
    /* is called from the modified sample process operation and an secondary object was already created for the */
    /* object belonging to the sample being processed */
    ls_object = objectHomeBridge.createTypedObject(exception, userData, home, _this->topicInfo, data->ls_topic,
                                                                                                data->objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_setLSObject(data->objectAdmin, userData, ls_object);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectReader_us_markObjectAsModified(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* modifiedObject)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(modifiedObject);

    if(modifiedObject->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
    {
        assert(modifiedObject->home->objectReader == _this);
        DK_ObjectAdmin_us_setReadState(modifiedObject, DK_OBJECT_STATE_OBJECT_MODIFIED);
        errorCode = Coll_List_pushBack(&(_this->modifiedSamples), DK_Entity_ts_duplicate((DK_Entity*)modifiedObject));
        if(errorCode != COLL_OK)
        {
            DK_Entity_ts_release((DK_Entity*)modifiedObject);
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add a processed modified sample to the modified samples list within %s '%s'. "
                "Allocation error when adding the processed sample.","DLRL Kernel ObjectHomeAdmin",
                    DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(DK_TopicInfo_us_getOwner(_this->topicInfo))));
        }

    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* requires update & admin locks on owner home and related homes! */
void
DK_ObjectReader_us_resetObjectModificationInformation(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_ObjectAdmin* anObjectAdmin = NULL;
    u_instanceHandle handle = DK_DCPSUtility_ts_getNilHandle();

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    while(Coll_List_getNrOfElements(&(_this->newSamples)) > 0)
    {
        anObjectAdmin = (DK_ObjectAdmin*)Coll_List_popBack(&(_this->newSamples));
        DK_ObjectAdmin_us_resetModificationInfoOnCollections(anObjectAdmin);
        DK_ObjectAdmin_us_setReadState(anObjectAdmin, DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
        DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
    }
    while(Coll_List_getNrOfElements(&(_this->modifiedSamples)) > 0)
    {
        anObjectAdmin = (DK_ObjectAdmin*)Coll_List_popBack(&(_this->modifiedSamples));
        objectReaderBridge.resetLSModificationInfo(exception, userData, anObjectAdmin);
        DK_ObjectAdmin_us_resetModificationInfoOnCollections(anObjectAdmin);
        DK_ObjectAdmin_us_setReadState(anObjectAdmin, DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
        DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
    }
    while(Coll_List_getNrOfElements(&(_this->deletedSamples)) > 0)
    {
        anObjectAdmin = (DK_ObjectAdmin*)Coll_List_popBack(&(_this->deletedSamples));
        DK_ObjectAdmin_us_resetModificationInfoOnCollections(anObjectAdmin);
        handle = DK_ObjectAdmin_us_getHandle(anObjectAdmin);
        /* handle can be set to NIL when re-used for a new generation (also means we dont have to take anything) */
        if(!u_instanceHandleIsNil(handle))
        {
            /* do NOT reset the user data on the handle!! This is already done during updates. The reasoning that this is */
            /* done during updates and not here is to ensure that we never see deleted objects in the object listings */
            assert(DK_DCPSUtility_us_getUserDataBasedOnHandle(exception, handle) != anObjectAdmin);
            DK_DCPSUtility_us_takeInstanceFromDatabase(exception, _this->queryReader, handle, FALSE);
            DLRL_Exception_PROPAGATE(exception);
        }
        DK_ObjectAdmin_us_delete(anObjectAdmin, userData);
        DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* array holder internal object array may be null after this operation!. if so the size will be indicated as 0 */
void
DK_ObjectReader_us_getAllObjects(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    DK_ObjectArrayHolder* arrayHolder)
{
    v_dataReader kernelReader = NULL;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(arrayHolder);

    arrayHolder->maxSize =0;
    arrayHolder->size = 0;
    arrayHolder->objectArray = NULL;

    /* get kernel reader and lock it */
    /* internal user layer header file defined op... not so nice to use */
    result = u_entityReadClaim(u_entity(_this->reader), (v_entity*)&kernelReader);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Could not claim the DCPS (kernel) data reader.");
    v_observerLock(v_observer(kernelReader));

    /* dataReader->emptyList is created with c_tableNew(...) */
    /* datareader->index->objects is created with c_tableNew(...) */

#ifdef _EL_
    arrayHolder->maxSize = c_count(kernelReader->index->objects) + c_count(kernelReader->emptyList);
#else
    arrayHolder->maxSize = c_count(kernelReader->index->objects);
#endif
    if(arrayHolder->maxSize > 0)
    {
        /* now allocate an array of object admin points that is sure to be able to hold all object admins */
        /* this is more effcient then using the Coll_List as that alloc member per element. */
        DLRL_ALLOC_WITH_SIZE(arrayHolder->objectArray, (sizeof(DK_ObjectAdmin*)*arrayHolder->maxSize), exception,
                                                               "Unable to allocate array container for object admins!");
        /* get object admins from objects table */
        c_walk(kernelReader->index->objects, DK_ObjectReader_us_getObjectFromHandle, (c_voidp)arrayHolder);
#ifdef _EL_
        /* get object admins from emptylist table. */
        c_walk(kernelReader->emptyList, DK_ObjectReader_us_getObjectFromHandle, (c_voidp)arrayHolder);
#endif
    }

    /* unlock reader */
    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && arrayHolder->objectArray)
    {
        os_free(arrayHolder->objectArray);
        arrayHolder->objectArray = NULL;
        arrayHolder->maxSize = 0;
        arrayHolder->size = 0;
    }
    if(kernelReader)
    {
        v_observerUnlock(v_observer(kernelReader));
        u_entityRelease((u_entity)_this->reader);
    }
    DLRL_INFO(INF_EXIT);
}

c_bool
DK_ObjectReader_us_getObjectFromHandle(
    c_object object,
    c_voidp arg)
{
    DK_ObjectAdmin* userData = NULL;
    DK_ObjectArrayHolder* arrayHolder = (DK_ObjectArrayHolder*)arg;

    DLRL_INFO(INF_ENTER);

    assert(object);
    assert(arg);

    /* get user data */
    userData = (DK_ObjectAdmin*)v_publicGetUserData(v_public(object));
    /* if user data was found, then add the object admin (aka the user data) to the array at the correct position */
    if(userData)
    {
        /* dont duplicate, gives to most flexibility for this algoritm to be used effeciently for multiple purposes */
        assert(userData->alive);/* object admins in user data are always alive. */
        arrayHolder->objectArray[arrayHolder->size] = userData;
        arrayHolder->size++;
        assert(arrayHolder->size <= arrayHolder->maxSize);
    }/* else do nothing */

    DLRL_INFO(INF_EXIT);
    return TRUE;
}
