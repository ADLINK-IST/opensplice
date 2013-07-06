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

/* DLRL utilities includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_Collection.h"
#include "DK_CollectionWriter.h"
#include "DK_DCPSUtility.h"
#include "DK_DCPSUtilityBridge.h"

static void
DK_CollectionWriter_us_destroy(
    DK_Entity * _this);

static void
DK_CollectionWriter_us_writeCollectionElements(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    DK_Collection* collection,
    Coll_Set* elements,
    void (*action)( v_entity,
                    c_voidp),
    LOC_boolean destroyWrittenHolders,
    LOC_boolean unregisterHolders);

#define ENTITY_NAME "DLRL Kernel CollectionWriter"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

DK_CollectionWriter*
DK_CollectionWriter_new(
    DLRL_Exception* exception,
    u_writer writer,
    DLRL_LS_object ls_writer,
    DK_TopicInfo* topicInfo)
{
    DK_CollectionWriter* _this = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(writer);
    assert(topicInfo);
    /* ls_writer may be null */

    DLRL_ALLOC(_this, DK_CollectionWriter, exception, "%s", allocError);

    _this->alive = TRUE;
    _this->ls_writer = ls_writer;
    _this->topicInfo = (DK_TopicInfo*)DK_Entity_ts_duplicate((DK_Entity*)topicInfo);
    _this->writer = writer;

    DK_Entity_us_init(&(_this->entity), DK_CLASS_COLLECTION_WRITER, DK_CollectionWriter_us_destroy);

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        /* set user layer writer to null to prevent double free, as caller
         * of this operation will still assume ownership if this function has
         * failed!
         */
        _this->writer = NULL;
        DK_CollectionWriter_us_delete(_this, NULL);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_CollectionWriter_us_delete(
    DK_CollectionWriter* _this,
    void* userData)
{
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    /* userData may be null */

    if(_this->alive)
    {
        DLRL_Exception_init(&exception);
        if(_this->writer)
        {
            /* no duplicate done */
            cache = DK_ObjectHomeAdmin_us_getCache(DK_TopicInfo_us_getOwner(_this->topicInfo));
            dcpsUtilityBridge.deleteDataWriter(&exception, userData, cache, _this->writer,
                                                                                    _this->ls_writer);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to delete the DCPS datawriter\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
                /* reset the exception, maybe it's used again later in this deletion function. We dont propagate the */
                /* exception here anyway, so it can do no harm as we already logged the exception directly above. */
                DLRL_Exception_init(&exception);
            }
            result = u_entityFree(u_entity(_this->writer));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception,
                   result,
                   "Unable to free the user layer writer!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer datawriter\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
            _this->writer = NULL;
            /* ls_reader and userReader pointer become invalid once the datareader has been deleted. */
            _this->ls_writer = NULL;
        }
        if(_this->topicInfo)
        {
            DK_Entity_ts_release((DK_Entity*)_this->topicInfo);
            _this->topicInfo = NULL;
        }
        _this->alive = FALSE;
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionWriter_us_enable(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    void* userData)
{

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);

    dcpsUtilityBridge.enableEntity(exception, userData, _this->ls_writer);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* no duplicate done! */
DK_TopicInfo*
DK_CollectionWriter_us_getTopicInfo(
    DK_CollectionWriter* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicInfo;
}

DLRL_LS_object
DK_CollectionWriter_us_getLSWriter(
    DK_CollectionWriter* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_writer;
}


void
DK_CollectionWriter_us_destroy(
    DK_Entity * _this)
{

    DLRL_INFO_OBJECT(INF_ENTER);
    /* _this may be null */

    if(_this)
    {
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_CollectionWriter*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionWriter_us_write(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    DK_Collection* collection)
{
    Coll_Set* changedElements = NULL;
    Coll_Set* deletedElements = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(collection);

    changedElements = DK_Collection_us_getChangedElements(collection);
    DK_CollectionWriter_us_writeCollectionElements(
        _this,
        exception,
        collection,
        changedElements,
        DK_DCPSUtility_us_writeMessage,
        FALSE,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);
    deletedElements = DK_Collection_us_getDeletedElements(collection);
    DK_CollectionWriter_us_writeCollectionElements(
        _this,
        exception,
        collection,
        deletedElements,
        DK_DCPSUtility_us_disposeMessage,
        TRUE,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CollectionWriter_us_dispose(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    DK_Collection* collection)
{
    Coll_Set* elements = NULL;
    DMM_DLRLMultiRelation* metaCollection = NULL;
    DMM_Basis base;
    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(collection);

    metaCollection = DK_Collection_us_getMetaRepresentative(collection);
    base = DMM_DLRLMultiRelation_getBasis(metaCollection);
    if(base == DMM_BASIS_SET)
    {
        elements = DK_SetAdmin_us_getHolders((DK_SetAdmin*)collection);
    } else
    {
        assert((base == DMM_BASIS_STR_MAP) || (base == DMM_BASIS_INT_MAP));
        elements = DK_MapAdmin_us_getObjectHolders((DK_MapAdmin*)collection);
    }
    assert(elements);

    DK_CollectionWriter_us_writeCollectionElements(
        _this,
        exception,
        collection,
        elements,
        DK_DCPSUtility_us_disposeMessage,
        TRUE,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO_OBJECT(INF_EXIT);
}


/*writes and removes elements contained within the elements set */
void
DK_CollectionWriter_us_writeCollectionElements(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    DK_Collection* collection,
    Coll_Set* elements,
    void (*action)( v_entity,
                    c_voidp),
    LOC_boolean destroyWrittenHolders,
    LOC_boolean unregisterHolders)
{
    DMM_DLRLMultiRelation* metaCollection = NULL;
    Coll_List* targetKeys = NULL;
    Coll_List* ownerKeys = NULL;
    DMM_Basis base;
    Coll_List* collectionOwnerFields = NULL;
    Coll_List* collectionTargetFields = NULL;
    DK_ObjectAdmin* collectionOwner = NULL;
    c_value* ownerKeyValues = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    v_message message = NULL;
    c_long offset = 0;
    void* dataSample = NULL;
    DK_ObjectAdmin* elementTarget = NULL;
    c_value* targetKeyValues = NULL;
    void* keyValue = NULL;
    DMM_DCPSField* collectionIndexField = NULL;
    DK_DCPSUtilityWriteMessageArg messageArg;/*  on stack def...     */
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(collection);
    assert(action);

    metaCollection = DK_Collection_us_getMetaRepresentative(collection);
    targetKeys = DMM_DLRLRelation_getTargetKeys((DMM_DLRLRelation*)metaCollection);
    ownerKeys = DMM_DLRLRelation_getOwnerKeys((DMM_DLRLRelation*)metaCollection);
    base = DMM_DLRLMultiRelation_getBasis(metaCollection);
    collectionOwnerFields = DMM_DLRLMultiRelation_getRelationTopicOwnerFields(metaCollection);
    collectionTargetFields = DMM_DLRLMultiRelation_getRelationTopicTargetFields(metaCollection);
    collectionOwner = DK_Collection_us_getOwner(collection);
    assert(collectionOwner);
    ownerKeyValues = DK_ObjectAdmin_us_getKeyValueArray(collectionOwner);/* may return NULL */

    iterator = Coll_Set_getFirstElement(elements);
    while(iterator)
    {
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        message = DK_DCPSUtility_ts_createMessageForDataWriter(_this->writer, exception, &offset);
        DLRL_Exception_PROPAGATE(exception);
        dataSample = C_DISPLACE(message, offset);
        /* now we have a data sample in which we can copy the key values and then write it... */
        DK_DCPSUtility_us_copyFromSource(exception, ownerKeys, ownerKeyValues, collectionOwnerFields, dataSample);
        DLRL_Exception_PROPAGATE(exception);
        elementTarget = DK_ObjectHolder_us_getTarget(holder);
        assert(elementTarget);
        targetKeyValues = DK_ObjectAdmin_us_getKeyValueArray(elementTarget);
        DK_DCPSUtility_us_copyFromSource(exception, targetKeys, targetKeyValues, collectionTargetFields, dataSample);
        DLRL_Exception_PROPAGATE(exception);
        if(base == DMM_BASIS_STR_MAP)
        {
            keyValue = DK_ObjectHolder_us_getUserData(holder);/*used later on for freeing purposes!*/
            assert(keyValue);
            collectionIndexField = DMM_DLRLMultiRelation_getIndexField(metaCollection);/* may return NULL */
            DK_DCPSUtility_us_copyStringIntoDatabaseSample((LOC_string)keyValue, collectionIndexField, dataSample);
        } else if(base == DMM_BASIS_INT_MAP)
        {
            keyValue = DK_ObjectHolder_us_getUserData(holder);/*used later on for freeing purposes!*/
            assert(keyValue);
            collectionIndexField = DMM_DLRLMultiRelation_getIndexField(metaCollection);/* may return NULL */
            DK_DCPSUtility_us_copyIntegerIntoDatabaseSample((LOC_long*)keyValue, collectionIndexField, dataSample);
        }
#ifndef NDEBUG
         else
        {
            /* else do nothing at all */
            assert(base == DMM_BASIS_SET);
        }
#endif

        /* now write the message... */
        messageArg.message = message;
        messageArg.exception = exception;
        result = u_entityWriteAction((u_entity)_this->writer, action, (void*)&messageArg);
        DLRL_Exception_PROPAGATE_RESULT(exception, result, "An error occured while trying to write a message into the system.");
        DLRL_Exception_PROPAGATE(exception);/* propagate the exception wrapped in the messageArg struct... */
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(elements, holder);
        if(unregisterHolders && elementTarget)
        {
#if 0
            if(DK_ObjectAdmin_us_getWriteState(elementTarget) == DK_OBJECT_STATE_OBJECT_DELETED)
            {
                DK_CacheAccessAdmin_us_decreaseInvalidLinks(elementTarget->access);
            }
#endif
            DK_ObjectAdmin_us_unregisterIsRelatedFrom(elementTarget, holder);
        }
        if(destroyWrittenHolders)
        {
            if(keyValue)
            {
                os_free(keyValue);
                DK_ObjectHolder_us_setUserData(holder, NULL);
                keyValue = NULL;/* must set to null to prevent mistakes in the next iteration */
            }
            DK_ObjectHolder_us_destroy(holder);
        }
        if(message)
        {
            result = u_entityAction((u_entity)_this->writer, DK_DCPSUtility_us_freeMessage, (void*)message);
            DLRL_Exception_PROPAGATE_RESULT(exception, result, "An error occured while trying to free a v_message object.");
            message = NULL;
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}



