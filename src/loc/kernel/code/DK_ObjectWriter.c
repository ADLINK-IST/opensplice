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

/* use rlayer includes */
#include "u_writer.h"
#include "u_time.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_CollectionWriter.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_DCPSUtility.h"
#include "DK_ObjectAdmin.h"
#include "DK_ObjectWriter.h"
#include "DK_ObjectWriterBridge.h"

#define ENTITY_NAME "DLRL Kernel ObjectWriter"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_ObjectWriter_us_destroy(
    DK_Entity * _this);

static c_bool
DK_ObjectWriter_us_copyKeyValues(
    c_type type,
    void *data,
    void *to);

DK_ObjectWriter*
DK_ObjectWriter_new(
    DLRL_Exception* exception,
    u_writer writer,
    DLRL_LS_object ls_writer,
    DK_TopicInfo* topicInfo)
{
    DK_ObjectWriter* _this = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(writer);
    assert(topicInfo);
    /* ls_writer may be null */

    DLRL_ALLOC(_this, DK_ObjectWriter, exception, "%s", allocError);

    _this->alive = TRUE;
    _this->ls_writer = ls_writer;
    Coll_List_init(&(_this->collectionWriters));

    _this->topicInfo = (DK_TopicInfo*)DK_Entity_ts_duplicate((DK_Entity*)topicInfo);
    _this->writer = writer;
    DK_Entity_us_init(&(_this->entity), DK_CLASS_OBJECT_WRITER, DK_ObjectWriter_us_destroy);
    DLRL_INFO(INF_ENTITY, "Created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        /* set user layer writer to null to prevent double free, as caller
         * of this operation will still assume ownership if this function has
         * failed!
         */
        _this->writer = NULL;
        DK_ObjectWriter_us_delete(_this, NULL);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }

    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_ObjectWriter_us_destroy(
    DK_Entity * _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_ObjectWriter*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectWriter_us_delete(
    DK_ObjectWriter* _this,
    void* userData)
{
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    DK_CollectionWriter* collWriter = NULL;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    /* userData may be null */

    if(_this->alive)
    {
        DLRL_Exception_init(&exception);
        if(_this->writer)
        {
            cache = DK_ObjectHomeAdmin_us_getCache(DK_TopicInfo_us_getOwner(_this->topicInfo));
            dcpsUtilityBridge.deleteDataWriter(&exception, userData, cache, _this->writer, _this->ls_writer);
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
                   "Unable to free the user layer topic!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer topic\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
            _this->writer = NULL;
            _this->ls_writer = NULL;
        }
        if(_this->topicInfo)
        {
            DK_Entity_ts_release((DK_Entity*)_this->topicInfo);
            _this->topicInfo = NULL;
        }
        while(Coll_List_getNrOfElements(&(_this->collectionWriters)) > 0)
        {
            collWriter = (DK_CollectionWriter*)Coll_List_popBack(&(_this->collectionWriters));
            DK_CollectionWriter_us_delete(collWriter, userData);
            DK_Entity_ts_release((DK_Entity*)collWriter);
        }
        _this->alive = FALSE;
    }

    DLRL_INFO(INF_EXIT);
}

DK_TopicInfo*
DK_ObjectWriter_us_getTopicInfo(
    DK_ObjectWriter* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicInfo;
}

Coll_List*
DK_ObjectWriter_us_getCollectionWriters(
    DK_ObjectWriter* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->collectionWriters);
}

void
DK_ObjectWriter_us_enable(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator  = NULL;
    DK_CollectionWriter* collectionWriter = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);

    dcpsUtilityBridge.enableEntity(exception, userData, _this->ls_writer);
    DLRL_Exception_PROPAGATE(exception);

    iterator = Coll_List_getFirstElement(&(_this->collectionWriters));
    while(iterator)
    {
        collectionWriter = (DK_CollectionWriter*)Coll_Iter_getObject(iterator);
        DK_CollectionWriter_us_enable(collectionWriter, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DLRL_LS_object
DK_ObjectWriter_us_getLSWriter(
    DK_ObjectWriter* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_writer;
}

void
DK_ObjectWriter_us_createCollectionWriter(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo)
{
    u_writer writer = NULL;
    DLRL_LS_object ls_writer = NULL;
    long errorCode = 0;
    DK_CollectionWriter* collectionWriter = NULL;
    u_result result;
    DLRL_Exception exception2;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(topicInfo);
    /* userData may be null */

    DLRL_Exception_init(&exception2);

    writer = dcpsUtilityBridge.createDataWriter(exception, userData, topicInfo, &ls_writer);
    DLRL_Exception_PROPAGATE(exception);
    assert(writer);
    collectionWriter = DK_CollectionWriter_new(exception, writer, ls_writer, topicInfo);
    DLRL_Exception_PROPAGATE(exception);
    assert(collectionWriter);

    errorCode = Coll_List_pushBack(&(_this->collectionWriters), collectionWriter);
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add a collection writer to the list of collection"
                            " writers");
    }
    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        if(collectionWriter)
        {
            DK_CollectionWriter_us_delete(collectionWriter, userData);/* also takes care off the delete of writer */
            DK_Entity_ts_release((DK_Entity*)collectionWriter);
        } else if(writer)
        {
            result = u_entityFree(u_entity(writer));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception2,
                   result,
                   "Unable to free the user layer writer!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer datawriter\n%s",
                    DLRL_Exception_exceptionIDToString(exception2.exceptionID),
                   exception2.exceptionMessage);
               DLRL_Exception_init(&exception2);
            }
        }
    }
    DLRL_INFO(INF_EXIT);
}

u_instanceHandle
DK_ObjectWriter_us_registerInstance(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin,
    DK_WriterData* writerData)
{
    u_instanceHandle handle;
    /* on stack def */
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(DK_ObjectAdmin_us_isAlive(objectAdmin));
    /* writerData may be NULL */

    /* if we dont have sample data goto the language specific layer as a copy
     * in needs to be done, but if we already have sample data we can bypass
     * that and directly call the user layer writer register instance
     */
    if(!writerData)
    {
        handle = objectWriterBridge.registerInstance(exception,
                                                     userData,
                                                     _this,
                                                     objectAdmin);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
#ifndef NDEBUG
        printf("NDEBUG- optimize usage of u_timeGet()\n");
#endif
        result = u_writerRegisterInstanceTMP(_this->writer,
                                            writerData,
                                            u_timeGet(),
                                            (u_instanceHandle*)&handle,
                                            DK_ObjectWriter_us_copyKeyValues);
        /* exception (in writerdata struct) superseeds any result value */
        DLRL_Exception_PROPAGATE(exception);
        DLRL_Exception_PROPAGATE_RESULT(exception, result, "Register instance failed!");
    }

    if(u_instanceHandleIsNil(handle))
    {
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "The registeration of object '%p' failed. DCPS "
                             "failed to allocate an instance handle for this "
                             "object. Check DCPS error log file for (possibly) "
                             "more information.", objectAdmin);
    }
    DK_DCPSUtility_us_registerObjectToWriterInstance(exception, handle, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return handle;
}

c_bool
DK_ObjectWriter_us_copyKeyValues(
    c_type type,
    void *data,
    void *to)
{
    DK_WriterData* wData = (DK_WriterData*)data;
    c_value* value;
    Coll_Iter* iterator;
    LOC_unsigned_long size, count = 0;
    DMM_DCPSField* aField;
    c_value* valueArray;

    assert(data);
    assert(to);

    size = Coll_List_getNrOfElements(wData->keyFields);
    assert(size == 4 || size == 3);
    DLRL_ALLOC_WITH_SIZE(valueArray,
                        (sizeof(struct c_value)*size),
                        wData->exception,
                        "Unable to allocate memory");
    iterator = Coll_List_getLastElement(wData->keyFields);/* start at the end*/
    while(iterator)
    {
        aField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        value = &(valueArray[size-1-count]);
        if(count == 0)
        {
            value->kind = V_LONG;
            value->is.Long = (c_long)os_timeGet().tv_nsec;
        } else if(count == 1)
        {
            value->kind = V_LONG;
            value->is.Long = (c_long)wData->objectAdmin;
        } else if(count == 2)
        {
            value->kind = V_LONG;
            value->is.Long = u_entitySystemId(u_entity(wData->writer));
        } else
        {
            assert(count == 3);
            value->kind = V_STRING;
            DLRL_STRDUP(value->is.String,
                        (DMM_DCPSTopic_getTopicTypeName(wData->mainTopic)),
                        (wData->exception),
                        "Unable to copy string for c_value");
        }
        DK_DCPSUtility_us_copyValueIntoDatabaseSample(wData->exception,
                                                      *value,
                                                      to,
                                                      aField);
        DLRL_Exception_PROPAGATE(wData->exception);
        iterator = Coll_Iter_getPrev(iterator);
        count++;
    }
    DK_ObjectAdmin_us_setKeyValueArray(wData->objectAdmin, valueArray, size);

    DLRL_Exception_EXIT(wData->exception);

    return TRUE;
}

/* NOT IN DESIGN */
u_writer
DK_ObjectWriter_us_getWriter(
    DK_ObjectWriter* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->writer;
}

LOC_boolean
DK_ObjectWriter_us_isAlive(
    DK_ObjectWriter* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->alive;
}

void
DK_ObjectWriter_us_write(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object)
{
    LOC_unsigned_long nrOfCollections = 0;
    DK_Collection** collections = NULL;
    LOC_boolean* changedCollections = NULL;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be NULL */
    assert(object);
    assert(DK_ObjectAdmin_us_getWriteState(object) != DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
    assert(DK_ObjectAdmin_us_getWriteState(object) != DK_OBJECT_STATE_OBJECT_DELETED);

    /* in the future this should be a loop for each topic... */
    if(DK_ObjectAdmin_us_hasTopicChanged(object))
    {
        objectWriterBridge.write(exception, userData, _this, object);
        DLRL_Exception_PROPAGATE(exception);
    }
    /* Now check which collections, if any, have changes and commit those changes. */
    nrOfCollections = DK_ObjectAdmin_us_getCollectionsSize(object);
    collections = DK_ObjectAdmin_us_getCollections(object);
    changedCollections = DK_ObjectAdmin_us_getChangedCollections(object);
    for(count = 0; count < nrOfCollections; count++)
    {
        if(changedCollections[count])
        {
            /* TODO make collection writers an array */
            DK_CollectionWriter_us_write(Coll_List_getObject(&(_this->collectionWriters), count), exception, collections[count]);
            DLRL_Exception_PROPAGATE(exception);

        }/* else do nothing */
    }
    DK_ObjectAdmin_us_resetChangedFlags(object);
    DK_ObjectAdmin_us_setWriteState(object, userData, DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectWriter_us_dispose(
    DK_ObjectWriter* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object)
{
    LOC_unsigned_long nrOfCollections =0;
    DK_Collection** collections = NULL;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be NULL */
    assert(object);
    assert(DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_DELETED);

    objectWriterBridge.destroy(exception, userData, _this, object);
    DLRL_Exception_PROPAGATE(exception);
    nrOfCollections = DK_ObjectAdmin_us_getCollectionsSize(object);
    collections = DK_ObjectAdmin_us_getCollections(object);
    for(count = 0; count < nrOfCollections; count++)
    {
        /* TODO make collection writers an array */
        DK_CollectionWriter_us_dispose(Coll_List_getObject(&(_this->collectionWriters), count), exception, collections[count]);
        DLRL_Exception_PROPAGATE(exception);
    }
    DK_DCPSUtility_us_unregisterObjectFromWriterInstance(exception, object, _this->writer);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_delete(object, userData);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
