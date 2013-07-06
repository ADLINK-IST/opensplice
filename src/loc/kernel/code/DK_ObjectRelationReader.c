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

/* user layer includes */
#include "u_instanceHandle.h"
#include "u_entity.h"

 /* DLRL util includes */
#include "DLRL_Report.h"

/* DLRL meta model includes */
#include "DMM_DLRLClass.h"
#include "DMM_DLRLRelation.h"

/* DLRL kernel includes */
#include "DK_DCPSUtility.h"
#include "DK_ObjectAdmin.h"
#include "DK_ObjectHolder.h"
#include "DK_ObjectHomeBridge.h"
#include "DK_ObjectReader.h"
#include "DK_ObjectRelationReader.h"
#include "DK_ObjectRelationReaderBridge.h"
#include "DK_TopicInfo.h"

static u_instanceHandle
DK_ObjectRelationReader_us_getHandleForRelatedObject(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* relatedHome,
    DMM_DLRLRelation* relation,
    DK_ReadData* data);

/* requires a lock on the home and an administrative or higher(update) lock on the cache */
/* assumes all homes that play a role in the creation of a new object are locked. */
void
DK_ObjectRelationReader_us_processSingleRelation(
    DK_ObjectReader* reader,
    DLRL_Exception* exception,
    void* userData,
    DMM_DLRLRelation* relation,
    LOC_unsigned_long relationIndex,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data,
    LOC_boolean isValid)
{
    u_instanceHandle relationHandle = U_INSTANCEHANDLE_NIL;
    Coll_List* relationKeys = NULL;
    DK_ObjectHomeAdmin* relatedHome = NULL;
    DK_ObjectAdmin* relationObjectAdmin= NULL;
    DK_ObjectHolder* relationObjectHolder = NULL;
    DK_ObjectAdmin* oldTarget = NULL;
    void* values = NULL;
    LOC_unsigned_long arraySize = 0;

    DLRL_INFO(INF_ENTER);

    assert(reader);
    assert(exception);
    assert(relation);
    assert(home);
    assert(data);
    assert(data->objectAdmin);

    /* Get the home that manages instances of the related object */
    relatedHome = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(DMM_DLRLRelation_getType(relation));
    assert(relatedHome);
    if(isValid)
    {/* only need to look up the related object if the foreign keys are valid */
        relationHandle = DK_ObjectRelationReader_us_getHandleForRelatedObject(exception, relatedHome, relation, data);
        DLRL_Exception_PROPAGATE(exception);
        if(!u_instanceHandleIsNil(relationHandle))
        {
            relationObjectAdmin = (DK_ObjectAdmin*)DK_DCPSUtility_us_getUserDataBasedOnHandle(exception, relationHandle);
            DLRL_Exception_PROPAGATE(exception);
            /* we can never make a relation to an already deleted OA, the deleted relation clean up algorithm is dependant on  */
            /* this assumption. (see object reader - DK_ObjectReader_us_clearAllRelationsToDeletedObjects) */
            if(relationObjectAdmin && (!DK_ObjectAdmin_us_isAlive(relationObjectAdmin) ||
                                  (DK_ObjectAdmin_us_getReadState(relationObjectAdmin) == DK_OBJECT_STATE_OBJECT_DELETED)))
            {
                relationObjectAdmin = NULL;/* we can never make a relation to an already deleted OA */
            }
        }
    }


    relationObjectHolder = DK_ObjectAdmin_us_getSingleRelation(data->objectAdmin, relationIndex);
    if(relationObjectHolder)
    {
        /* if the object holder was an unresolved holder in the previous update round, then unregister it now. */
        if(!DK_ObjectHolder_us_isResolved(relationObjectHolder))
        {
            DK_ObjectHomeAdmin_us_unregisterUnresolvedElement(relatedHome, userData, relationObjectHolder);
        } else
        {
            oldTarget = DK_ObjectHolder_us_getTarget(relationObjectHolder);
            if(oldTarget && (oldTarget != relationObjectAdmin))
            {
                assert(DK_ObjectAdmin_us_isAlive(oldTarget));
                DK_ObjectAdmin_us_unregisterIsRelatedFrom(oldTarget, relationObjectHolder);
            }
        }
        /* if we found and instance handle and a corresponding related object, then set it as the target for the holder */
        if(relationObjectAdmin)
        {
            oldTarget = DK_ObjectHolder_us_getTarget(relationObjectHolder);
            if(!oldTarget || (oldTarget != relationObjectAdmin))
            {
                DK_ObjectHolder_us_setTarget(relationObjectHolder, relationObjectAdmin);
                DK_ObjectAdmin_us_registerIsRelatedFrom(relationObjectAdmin, exception, relationObjectHolder);
                DLRL_Exception_PROPAGATE(exception);
            }
        /* if we could not find the object admin for the (valid) relation, then register ourselves as an unresolved object */
        /*  to the related object home */
        } else if(isValid)
        {
            /* first copy the values that represent the relations into a value array we can use to register to the */
            /* unresolved list of the related object home relations owner keys describe the keys in the owner sample */
            relationKeys = DMM_DLRLRelation_getOwnerKeys(relation);
            values = DK_DCPSUtility_us_cloneKeys(exception, relationKeys, data->keyValueArray,
                                                                                            data->foreignKeyValueArray);
            DLRL_Exception_PROPAGATE(exception);
            arraySize = Coll_List_getNrOfElements(relationKeys);
            DK_ObjectHolder_us_setValues(relationObjectHolder, values, arraySize);
            DK_ObjectHomeAdmin_us_registerUnresolvedElement(relatedHome, exception, userData, relationObjectHolder,
                                                                                                        relationIndex);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            DK_ObjectHolder_us_setTarget(relationObjectHolder, NULL);
        }
    } else
    {
        if(relationObjectAdmin)
        {
            relationObjectHolder = DK_ObjectHolder_newResolved(exception, (DK_Entity*)data->objectAdmin,
                                                relationObjectAdmin, DK_DCPSUtility_ts_getNilHandle(), relationIndex);
            DLRL_Exception_PROPAGATE(exception);
            DK_ObjectAdmin_us_registerIsRelatedFrom(relationObjectAdmin, exception, relationObjectHolder);
            DLRL_Exception_PROPAGATE(exception);
        } else if(isValid)
        {
            relationKeys = DMM_DLRLRelation_getOwnerKeys(relation);
            values = DK_DCPSUtility_us_cloneKeys(exception, relationKeys, data->keyValueArray,
                                                                                            data->foreignKeyValueArray);
            DLRL_Exception_PROPAGATE(exception);
            arraySize = Coll_List_getNrOfElements(relationKeys);
            relationObjectHolder = DK_ObjectHolder_newUnresolved(exception, (DK_Entity*)data->objectAdmin, values,
                                                            arraySize, DK_DCPSUtility_ts_getNilHandle(), relationIndex);
            DLRL_Exception_PROPAGATE(exception);
            DK_ObjectHomeAdmin_us_registerUnresolvedElement(relatedHome, exception, userData, relationObjectHolder,
                                                                                                        relationIndex);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            relationObjectHolder = DK_ObjectHolder_newResolved(exception, (DK_Entity*)data->objectAdmin,
                                                NULL, DK_DCPSUtility_ts_getNilHandle(), relationIndex);
            DLRL_Exception_PROPAGATE(exception);
        }
        DK_ObjectAdmin_us_addSingleRelation(data->objectAdmin, relationObjectHolder, relationIndex);
    }
    /* change the relation on language specific level */
    objectRelationReaderBridge.setRelatedObjectForObject(
        userData,
        home,
        data->objectAdmin,
        relationIndex,
        relationObjectAdmin,
        isValid);


    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

u_instanceHandle
DK_ObjectRelationReader_us_getHandleForRelatedObject(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    DMM_DLRLRelation* relation,
    DK_ReadData* data)
{
    struct LookupInstanceHolder holder;
    DK_DCPSUtility_relationKeysCopyDataHolder dataHolder;
    u_result result = U_RESULT_OK;
    u_entity userReader;
    c_object sampleDatabaseObject;
    DK_ObjectReader* reader = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(home);
    assert(relation);
    assert(data);

    reader = DK_ObjectHomeAdmin_us_getObjectReader(home);
    holder.topicInfo = DK_ObjectReader_us_getTopicInfo(reader);
    holder.relationHandle = DK_DCPSUtility_ts_getNilHandle();
    sampleDatabaseObject = DK_TopicInfo_us_getDataSample(holder.topicInfo);
    dataHolder.exception = exception;
    dataHolder.relation = relation;
    dataHolder.data = data;
    dataHolder.sampleDatabaseObject = sampleDatabaseObject;
    userReader = (u_entity)DK_ObjectReader_us_getReader(reader);
    result = u_entityWriteAction(userReader, DK_DCPSUtility_us_copyRelationKeysAction, &dataHolder);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "An error occured while trying to copy relation keys into a database sample.");
    DLRL_Exception_PROPAGATE(exception);/* an exception was passed along in the dataHolder, it might have been set */

    result = u_entityAction(userReader, DK_DCPSUtility_us_lookupInstance, &holder);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Unable to look up the DCPS instance handle for a related "
                                                         "object");

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return holder.relationHandle;
}
