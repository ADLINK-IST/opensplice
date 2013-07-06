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

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Report.h"
#include "DLRL_Types.h"
#include "DLRL_Util.h"

/* Collection includes */
#include "Coll_Compare.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLMultiRelation.h"
#include "DMM_DLRLRelation.h"

/* DLRL kernel includes */
#include "DK_Collection.h"
#include "DK_DCPSUtility.h"
#include "DK_ObjectHolder.h"
#include "DK_ObjectRelationReaderBridge.h"
#include "DK_MapAdmin.h"
#include "DK_SetAdmin.h"
#include "DLRL_Kernel_private.h"

typedef enum DK_CallbackType_s
{
    DK_CALLBACKTYPE_COLLECTION_OWNER_NOT_FOUND,
    DK_CALLBACKTYPE_TARGET_NOT_FOUND,
    DK_CallbackType_elements
} DK_CallbackType;

typedef struct DK_UnresolvedTargetCallback_s
{
    DK_Entity* interestedEntity;
    DK_ObjectHolder* holder;
    LOC_unsigned_long relationIndex;
} DK_UnresolvedTargetCallback;

typedef struct DK_UnresolvedCollectionCallback_s
{
    DK_Collection* collection;
    LOC_unsigned_long collectionIndex;
} DK_UnresolvedCollectionCallback;

typedef union DK_CallBackData_u
{
    DK_UnresolvedTargetCallback target;
    DK_UnresolvedCollectionCallback collection;
} DK_CallBackData;

typedef struct DK_UnresolvedElement_s
{
    void* values;/* own copy which must be freed */
    LOC_unsigned_long valuesSize;
    Coll_Set callbacks;
} DK_UnresolvedElement;

typedef struct DK_UnresolvedElementCallback_s
{
    DK_CallbackType type;
    DK_CallBackData data;
} DK_UnresolvedElementCallback;

static DK_UnresolvedElement*
DK_UnresolvedObjectsUtility_us_createUnresolvedElement(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* ownerValues,
    LOC_unsigned_long valuesSize);

static DK_UnresolvedElement*
DK_UnresolvedObjectsUtility_us_getUnresolvedElement(
    Coll_Set* unresolvedElements,
    void* sourceValues,
    LOC_unsigned_long valuesSize);

static void
DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(
    DK_UnresolvedElement* element);

static void
DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(
    DK_UnresolvedElementCallback* elementCallback,
    void* userData);

void
DK_UnresolvedObjectsUtility_us_clear(
    Coll_Set* unresolvedElements,
    void* userData)
{
    Coll_Iter* callbackIterator = NULL;
    Coll_Iter* iterator = NULL;
    DK_UnresolvedElement* anElement = NULL;
    DK_UnresolvedElementCallback* elementCallback = NULL;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    /* userData may be null */

    iterator = Coll_Set_getFirstElement(unresolvedElements);
    while(iterator)
    {
        anElement = (DK_UnresolvedElement*)Coll_Iter_getObject(iterator);
        callbackIterator = Coll_Set_getFirstElement(&(anElement->callbacks));
        while(callbackIterator)
        {
            elementCallback = (DK_UnresolvedElementCallback*)Coll_Iter_getObject(callbackIterator);
            callbackIterator = Coll_Iter_getNext(callbackIterator);
            Coll_Set_remove(&(anElement->callbacks), elementCallback);
            DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(elementCallback, userData);
        }
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(unresolvedElements, anElement);
        DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(anElement);
    }
    DLRL_INFO(INF_EXIT);
}

/* assumes all homes that play a role in the creation of a new (deleted) object are locked. */
void
DK_UnresolvedObjectsUtility_us_resolveUnresolvedElements(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin,
    void* keyArray,
    LOC_boolean resolveTargets,/* temporary var until proper fix for dds439, read text in comments there when solving this nicely */
    LOC_boolean resolveCollectionOwners)/* temporary var until proper fix for dds439, read text in comments there when solving this nicely */
{
    Coll_Iter* iterator = NULL;
    DK_UnresolvedElement* element = NULL;
    LOC_boolean done = FALSE;
    Coll_Iter* callbackIterator = NULL;
    Coll_List* ownerKeys = NULL;
    void* values = NULL;
    LOC_unsigned_long arraySize = 0;
    void* keyValue = NULL;
    DK_UnresolvedElementCallback* callback= NULL;
    DK_ObjectHolder* holder = NULL;
    DK_Collection* collection = NULL;
    DMM_DLRLMultiRelation* metaCollection = NULL;
    DMM_Basis base;
    DK_ObjectAdmin* elementOwner = NULL;
    DK_ObjectHomeAdmin* elementOwnerHome = NULL;
    DK_CacheAccessAdmin* access = NULL;
    DK_Class typeOfEntity;
    LOC_boolean destroyCallback = FALSE;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert(exception);
    assert(home);
    assert(objectAdmin);

    ownerKeys = DMM_DCPSTopic_getKeyFields(DMM_DLRLClass_getMainTopic(home->meta_representative));
    /* values may be null after the convert call */
    values = (void*)DK_DCPSUtility_us_cloneKeys(exception,
                                                ownerKeys,
                                                keyArray,
                                                NULL);
    DLRL_Exception_PROPAGATE(exception);
    arraySize = Coll_List_getNrOfElements(ownerKeys);
    /* iterate through the unresolved list and check if unresolved elements exist */
    iterator = Coll_Set_getFirstElement(unresolvedElements);
    while(iterator && !done)
    {
        element = (DK_UnresolvedElement*)Coll_Iter_getObject(iterator);
        if(DK_DCPSUtility_us_areValueArraysEqual(element->values,
                                                 values,
                                                 element->valuesSize))
        {
            /* this unresolved element represents the object just (re)constructed! */
             callbackIterator = Coll_Set_getFirstElement(&(element->callbacks));
             while(callbackIterator)
            {
                 callback = (DK_UnresolvedElementCallback*)Coll_Iter_getObject(callbackIterator);
                 destroyCallback = FALSE;
                 if((callback->type == DK_CALLBACKTYPE_TARGET_NOT_FOUND) &&
                    (DK_ObjectAdmin_us_getReadState(objectAdmin) != DK_OBJECT_STATE_OBJECT_DELETED) )
                {
                    if(resolveTargets)
                    {/* temporary if statement until proper fix for dds439 */
                        holder = callback->data.target.holder;
                        typeOfEntity = DK_Entity_getClassID(callback->data.target.interestedEntity);
                        assert((DK_ObjectAdmin_us_isAlive(objectAdmin)) ||
                               (DK_ObjectAdmin_us_getReadState(objectAdmin) != DK_OBJECT_STATE_OBJECT_DELETED));
                        assert(!DK_ObjectHolder_us_isResolved(holder));
                        DK_ObjectHolder_us_setTarget(holder, objectAdmin);                                                      /* RELATED HOME */
                        DK_ObjectAdmin_us_registerIsRelatedFrom(objectAdmin, exception, holder);
                        DLRL_Exception_PROPAGATE(exception);
                        /* based upon what the type of class of the interested entity is we will perform a specific */
                        /* notification algorithm */
                        if(typeOfEntity == DK_CLASS_OBJECT_ADMIN)
                        {
                            /* we also have to inform the owner of the collection that something changed. */
                            elementOwner = (DK_ObjectAdmin*)callback->data.target.interestedEntity;
                            elementOwnerHome = DK_ObjectAdmin_us_getHome(elementOwner);                                         /* RELATED HOME */
                            assert(elementOwnerHome);
                            /* only if we have an owner (could be we resolved a target element for a collection that is */
                            /* unresolved itself) and if the owners state wasnt already some sort of modificationstate */
                            /* then we set the modification state to modified. */
                            if(elementOwner->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
                            {
                                access = DK_ObjectAdmin_us_getAccess(elementOwner);                                             /* RELATED HOME */
                                if(access)
                                {
                                    DK_ObjectAdmin_us_setWriteState(elementOwner, userData, DK_OBJECT_STATE_OBJECT_MODIFIED);
                                    DK_CacheAccessAdmin_us_markObjectAsChanged(access, exception, elementOwner);
                                    DLRL_Exception_PROPAGATE(exception);
                                    DK_ObjectAdmin_us_relationHasChanged(elementOwner, callback->data.target.relationIndex);
                                } else
                                {
                                    DK_ObjectHomeAdmin_us_markObjectAsModified(elementOwnerHome, exception, elementOwner);      /* RELATED HOME */
                                    DLRL_Exception_PROPAGATE(exception);
                                }
                            }
                            /* change the relation on language specific level */
                            objectRelationReaderBridge.setRelatedObjectForObject(
                                userData,
                                elementOwnerHome,/* RELATED HOME */
                                elementOwner,
                                callback->data.target.relationIndex,
                                objectAdmin,
                                TRUE);
                            /* dont need to add holder as a single relation, (addSingleRelation) this was already done in */
                            /* the algorithm that orginally registered this unresolved element. */
                        } else
                        {
                            assert((typeOfEntity == DK_CLASS_SET_ADMIN) || (typeOfEntity == DK_CLASS_MAP_ADMIN));
                            assert(!DK_ObjectAdmin_us_getAccess(objectAdmin));
                            collection = (DK_Collection*)callback->data.target.interestedEntity;
                            /* an element was just resolved, so decrease number of pending unresolved elements for the collection */
                            /*not needed, done in destruction of the callback element -- DK_Collection_us_decreaseNrOfUnresolvedElements(collection);*/
                            /* now we will add the resolved element to the collection */
                            metaCollection = DK_Collection_us_getMetaRepresentative(collection);
                            base = DMM_DLRLMultiRelation_getBasis(metaCollection);
                            if(base == DMM_BASIS_STR_MAP || base == DMM_BASIS_INT_MAP)
                            {
                                keyValue = DK_ObjectHolder_us_getUserData(holder);
                                assert(keyValue);
                                DK_MapAdmin_us_doPut((DK_MapAdmin*)collection, exception, keyValue, holder);
                                DLRL_Exception_PROPAGATE(exception);
                            } else
                            {
                                assert(base == DMM_BASIS_SET);
                                DK_SetAdmin_us_doAdd((DK_SetAdmin*)collection, exception, holder);
                                DLRL_Exception_PROPAGATE(exception);
                            }
                            /* we also have to inform the owner of the collection that something changed. */
                            elementOwner = DK_Collection_us_getOwner(collection);
                            /* only if we have an owner (could be we resolved a target element for a collection that is */
                            /* unresolved itself) and if the owners state wasnt already some sort of modificationstate */
                            /* then we set the modification state to modified. */
                            if(elementOwner && elementOwner->readState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
                            {
                                elementOwnerHome = DK_ObjectAdmin_us_getHome(elementOwner);
                                assert(elementOwnerHome);
                                DK_ObjectHomeAdmin_us_markObjectAsModified(elementOwnerHome, exception, elementOwner);
                                DLRL_Exception_PROPAGATE(exception);
                            }
                        }
                        destroyCallback = TRUE;
                    }
                } else if(callback->type == DK_CALLBACKTYPE_COLLECTION_OWNER_NOT_FOUND)
                {
                    if(resolveCollectionOwners)
                    {/* temporary if statement until proper fix for dds439 */
                    /*deleted or not for the object admin is irrelevant here*/
                    assert(!DK_ObjectAdmin_us_getAccess(objectAdmin));
                    collection = callback->data.collection.collection;
                    callback->data.collection.collection = NULL;/* prevents the release in the destroy of the callback */
                    /* !!!!dont release the collection found in the callback, the object admin will take over the ref count */
                    /* and administer it from the unresolved list. This is a performance optimization. */
                    DK_ObjectAdmin_us_addCollection(objectAdmin, exception, collection,
                                                                             callback->data.collection.collectionIndex);
                    DLRL_Exception_PROPAGATE(exception);
                    /* we do not need to update the owner object admin in language specific level, that is covered in the */
                    /* calling operation and not the respondsibility here. The only thing we want to accomplish here is */
                    /* that we match unresolved collections in the unresolved list with the newly received objects */

                    /* set the resolved owner as owner of the collection */
                    DK_Collection_us_setOwner(collection, objectAdmin);/* does the duplicate                     */
                    destroyCallback = TRUE;
                    }
                }
#ifndef NDEBUG
                else
                {
                    assert((callback->type == DK_CALLBACKTYPE_TARGET_NOT_FOUND) &&
                                (DK_ObjectAdmin_us_getReadState(objectAdmin) == DK_OBJECT_STATE_OBJECT_DELETED));
                }
#endif
                callbackIterator = Coll_Iter_getNext(callbackIterator);
                /* only destroy the callback if it was actually resolved. If the newly created object admin
                 * for which we are resolving elements is actually created as a DELETED admin, then we may
                 * not resolve all callbacks waiting for it, as we cannot make relations to deleted objects..
                 */
                if(destroyCallback)
                {
                    Coll_Set_remove(&(element->callbacks), callback);
                    DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(callback, userData);/* takes care of release & free */
                }
            }
            /* get next iterator before removing the current element */
            iterator = Coll_Iter_getNext(iterator);
            /* if the object admin we resolved was newly created as a deleted object admin and we found
             * a DK_CALLBACKTYPE_TARGET_NOT_FOUND callback, then that callback wont be removed and
             * will remain unresolved, making us unable to destroy the element naturally.
             */
            if(!Coll_Set_getFirstElement(&(element->callbacks)))
            {
                Coll_Set_remove(unresolvedElements, element);
                DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(element);
            }
            done = TRUE;
        } else
        {
            /* just get the next iterator, we already did that in the other branch as well */
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    DLRL_Exception_EXIT(exception);
    if(values)
    {
        DK_DCPSUtility_us_destroyValueArray(values, arraySize);
    }
    DLRL_INFO(INF_EXIT);
}


/* one owner may register multiple callbacks if wanted under one element.(IE two relations of the owner object point to */
/* the same object which cant be resolved. A very valid use case. For each call to the register operation */
/* a call to either the unregister operation must be made or a callback must have been received */
/* relation index is only used if the entity is a object admin, in other cases it may be any value */
void
DK_UnresolvedObjectsUtility_us_registerUnresolvedElement(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* holder,
    LOC_unsigned_long relationIndex)
{
    DK_UnresolvedElement* element = NULL;
    DK_UnresolvedElementCallback* elementCallback = NULL;
    LOC_long returnCode = COLL_OK;
    DK_Entity* owner = NULL;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert(exception);
    assert(holder);

    /* only register unresolved holders */
    if(!DK_ObjectHolder_us_isResolved(holder))
    {
        element = DK_UnresolvedObjectsUtility_us_getUnresolvedElement(unresolvedElements, DK_ObjectHolder_us_getValues(holder),
                                                            DK_ObjectHolder_us_getValuesSize(holder));
        if(!element)
        {
            element = DK_UnresolvedObjectsUtility_us_createUnresolvedElement(unresolvedElements, exception,
                                        DK_ObjectHolder_us_getValues(holder),DK_ObjectHolder_us_getValuesSize(holder));
            DLRL_Exception_PROPAGATE(exception);
        }
        /* create callback information */
        DLRL_ALLOC(elementCallback, DK_UnresolvedElementCallback, exception, "Unable to allocate an unresolved "
                                                                            "element callback container");
        owner= DK_ObjectHolder_us_getOwner(holder);
        elementCallback->type = DK_CALLBACKTYPE_TARGET_NOT_FOUND;
        elementCallback->data.target.interestedEntity = DK_Entity_ts_duplicate(owner);
        elementCallback->data.target.holder = holder;
        elementCallback->data.target.relationIndex = relationIndex;
        returnCode = Coll_Set_add(&(element->callbacks), elementCallback);
        if(returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add an unresolved element callback container"
                                                        " to the list of callback containers of an unresolved element");
        }
    }/* else do nothing */

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        /* do rollback, last possible exception occurs when element callback is added to the list of element callbacks */
        /* this means we never have to remove the callback from the element's list of callbacks as this is the last */
        /* possible place an exception occured. Meaning it would never been added to the callbacks list of the element */
        /* if an exception has occured. */
        if(elementCallback)
        {
            assert(elementCallback->type = DK_CALLBACKTYPE_TARGET_NOT_FOUND);
            DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(elementCallback, userData);
            elementCallback = NULL;
        }
        if(element)
        {
            if(Coll_Set_getNrOfElements(&(element->callbacks)) == 0)
            {
                /* this element was create for the first time here, so lets delete it. Note that the last element */
                /* callback could have never been inserted in the element callbacks list, as it was the last action */
                /* that could generate an exception */
                Coll_Set_remove(unresolvedElements, element);
                DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(element);
                element = NULL;
            }
        }
    }
    DLRL_INFO(INF_EXIT);
}

/* one owner may register multiple callbacks if wanted. no reason to forbid it. For each call to the register operation */
/* a call to either the unregister operation must be made or a callback must have been received */
/* returns the collection if already exists as an unresolved element, if it didnt yet exist its created and then */
/* added to the unresolved list and returned. */
/* Caller may not release the pointer of the collection */
DK_Collection*
DK_UnresolvedObjectsUtility_us_registerUnresolvedCollection(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    DMM_DLRLMultiRelation* metaRelation,
    LOC_unsigned_long collectionIndex,
    DK_ObjectHomeAdmin* ownerHome)
{
    DK_UnresolvedElement* element = NULL;
    DK_UnresolvedElementCallback* elementCallback = NULL;
    DK_Collection* collection = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;
    DMM_Basis base;
    Coll_Iter* iterator = NULL;
    LOC_long returnCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert(exception);
    assert((valuesSize > 0 && ownerValues) || (valuesSize == 0 && !ownerValues));
    assert(metaRelation);
    assert(ownerHome);

    element = DK_UnresolvedObjectsUtility_us_getUnresolvedElement(unresolvedElements, ownerValues, valuesSize);
    if(!element)
    {
        element = DK_UnresolvedObjectsUtility_us_createUnresolvedElement(unresolvedElements, exception, ownerValues, valuesSize);
        DLRL_Exception_PROPAGATE(exception);
    }
    assert(element);
    /* maybe the collection for which we cant find the owner already exists in the callbacks of the element */
    iterator = Coll_Set_getFirstElement(&(element->callbacks));
    while(iterator && !collection)
    {
        elementCallback = (DK_UnresolvedElementCallback*)Coll_Iter_getObject(iterator);
        if(elementCallback->type == DK_CALLBACKTYPE_COLLECTION_OWNER_NOT_FOUND)
        {
            if(elementCallback->data.collection.collectionIndex == collectionIndex)
            {
                collection = elementCallback->data.collection.collection;
            }
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    /* create a callback element and the collection if we were unable to locate the collection */
    if(!collection)
    {
        targetHome = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(DMM_DLRLRelation_getType((DMM_DLRLRelation*)metaRelation));
        base = DMM_DLRLMultiRelation_getBasis(metaRelation);
        if(base == DMM_BASIS_STR_MAP || base == DMM_BASIS_INT_MAP)
        {
            collection = (DK_Collection*)DK_MapAdmin_new(exception, NULL, targetHome, metaRelation, ownerHome);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            assert(base == DMM_BASIS_SET);
            collection = (DK_Collection*)DK_SetAdmin_new(exception, NULL, targetHome, metaRelation, ownerHome);
            DLRL_Exception_PROPAGATE(exception);
        }

        DLRL_ALLOC(elementCallback, DK_UnresolvedElementCallback, exception, "Unable to allocate an unresolved "
                                                                            "element callback container");
        elementCallback->type = DK_CALLBACKTYPE_COLLECTION_OWNER_NOT_FOUND;
        elementCallback->data.collection.collection = collection;
        elementCallback->data.collection.collectionIndex = collectionIndex;
        returnCode = Coll_Set_add(&(element->callbacks), elementCallback);
        if(returnCode != COLL_OK)
        {
            /* release for the new done by the destroy function. Will also delete the created collection. */
            DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(elementCallback, userData);
            collection = NULL;
            elementCallback = NULL;
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add an unresolved element callback container"
                                                        " to the list of callback containers of an unresolved element");
        }
    }
    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        /* rollback of the element */
        if(element)
        {
            if(Coll_Set_getNrOfElements(&(element->callbacks)) == 0)
            {
                /* this element was create for the first time here, so lets delete it. Note that the last element */
                /* callback could have never been inserted in the element callbacks list, as it was the last action */
                /* that could generated an exception */
                Coll_Set_remove(unresolvedElements, element);
                DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(element);
                element = NULL;
            }
        }
    }
    DLRL_INFO(INF_EXIT);
    return collection;
}

/* caller must handle the collection, (eventually destroy & release it) */
DK_Collection*
DK_UnresolvedObjectsUtility_us_unregisterUnresolvedCollection(
    Coll_Set* unresolvedElements,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    LOC_unsigned_long collectionIndex)
{
    DK_UnresolvedElement* element = NULL;
    DK_UnresolvedElementCallback* elementCallback = NULL;
    Coll_Iter* iterator = NULL;
    DK_Collection* collection = NULL;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert((valuesSize > 0 && ownerValues) || (valuesSize == 0 && !ownerValues));

    /* find the element under which the collection is registered */
    element = DK_UnresolvedObjectsUtility_us_getUnresolvedElement(unresolvedElements, ownerValues, valuesSize);
    assert(element);
    iterator = Coll_Set_getFirstElement(&(element->callbacks));
    while(iterator && !collection)
    {
        elementCallback = (DK_UnresolvedElementCallback*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        if(elementCallback->type == DK_CALLBACKTYPE_COLLECTION_OWNER_NOT_FOUND)
        {
            if(elementCallback->data.collection.collectionIndex == collectionIndex)
            {
                /* we found the element callback we need to remove */
                collection = elementCallback->data.collection.collection;
                /* dont release the collection, we will pass it to the caller of the operation whom has to release it */
                Coll_Set_remove(&(element->callbacks), elementCallback);
                /* we can now free the element callback */
                /* reset the collection to null though else it will also be destroyed! */
                elementCallback->data.collection.collection = NULL;
                DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(elementCallback, userData);
            }
        }
    }
    DLRL_INFO(INF_EXIT);
    return collection;
}

void
DK_UnresolvedObjectsUtility_us_unregisterUnresolvedElement(
    Coll_Set* unresolvedElements,
    void* userData,
    DK_ObjectHolder* holder)
{
    LOC_boolean done = FALSE;
    Coll_Iter* iterator = NULL;
    DK_UnresolvedElement* element = NULL;
    DK_UnresolvedElementCallback* elementCallback = NULL;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert(holder);

    element = DK_UnresolvedObjectsUtility_us_getUnresolvedElement(unresolvedElements, DK_ObjectHolder_us_getValues(holder),
                                                        DK_ObjectHolder_us_getValuesSize(holder));
    if(element)
    {
        iterator = Coll_Set_getFirstElement(&(element->callbacks));
        while(iterator && !done)
        {
            elementCallback = (DK_UnresolvedElementCallback*)Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);/* do the get next before the (possible)remove in the next part */
            /* if the callback element represents a not found target element, the holder matches */
            /* then we can remove it. Remember multiple holders of 1 owner can be registered under one element. */
            if( (elementCallback->type == DK_CALLBACKTYPE_TARGET_NOT_FOUND)      &&
                                                                         elementCallback->data.target.holder == holder)
            {
                Coll_Set_remove(&(element->callbacks), elementCallback);
                DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(elementCallback, userData);
                done = TRUE;
            }
        }
        /* if no more callbacks remain, remove the element from the unresolved elements set and free the memory */
        if(Coll_Set_getNrOfElements(&(element->callbacks)) == 0)
        {
            Coll_Set_remove(unresolvedElements, element);
            DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(element);
        }
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(
    DK_UnresolvedElementCallback* elementCallback,
    void* userData)
{
    LOC_long classID = 0;
    DK_Entity* entity= NULL;
    DK_Collection* collection = NULL;

    DLRL_INFO(INF_ENTER);

    assert(elementCallback);

    if(elementCallback->type == DK_CALLBACKTYPE_TARGET_NOT_FOUND)
    {
        entity = elementCallback->data.target.interestedEntity;
        classID = DK_Entity_getClassID(entity);
        if(classID == DK_CLASS_SET_ADMIN || classID == DK_CLASS_MAP_ADMIN)
        {
            /* an callback element was just deleted, so decrease number of pending unresolved elements for the collection */
            DK_Collection_us_decreaseNrOfUnresolvedElements((DK_Collection*)entity);
        }
        /* not the owner of the holder, so dont destroy it. */
        DK_Entity_ts_release(entity);
    } else
    {
        assert(elementCallback->type == DK_CALLBACKTYPE_COLLECTION_OWNER_NOT_FOUND);
        collection = elementCallback->data.collection.collection;
        if(collection)
        {
            DK_Class classID = DK_Collection_us_getClassID(elementCallback->data.collection.collection);
            if(classID == DK_CLASS_SET_ADMIN)
            {
                /* the delete requires a lock of the owning home, which we have in this operation */
                DK_SetAdmin_us_delete((DK_SetAdmin*)collection, userData);

            } else
            {
                assert(classID == DK_CLASS_MAP_ADMIN);
                /* delete requires a lock on the home, which we have in this operation */
                DK_MapAdmin_us_delete((DK_MapAdmin*)collection, userData);
            }
            DK_Entity_ts_release((DK_Entity*)collection);
        }
    }
    os_free(elementCallback);

    DLRL_INFO(INF_EXIT);
}

void
DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(
    DK_UnresolvedElement* element)
{
    DLRL_INFO(INF_ENTER);

    assert(element);

    if(element->values)
    {
        DK_DCPSUtility_us_destroyValueArray(element->values, element->valuesSize);
    }
    os_free(element);

    DLRL_INFO(INF_EXIT);
}

void
DK_UnresolvedObjectsUtility_us_unregisterAllUnresolvedElementsForEntity(
    Coll_Set* unresolvedElements,
    void* userData,
    DK_Entity* entity)
{
    Coll_Iter* iterator = NULL;
    DK_UnresolvedElement* element = NULL;
    Coll_Iter* callBackIterator = NULL;
    DK_UnresolvedElementCallback* elementCallback = NULL;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert(entity);

    iterator = Coll_Set_getFirstElement(unresolvedElements);
    while(iterator)
    {
        element = (DK_UnresolvedElement*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        callBackIterator = Coll_Set_getFirstElement(&(element->callbacks));
        while(callBackIterator)
        {
            elementCallback = (DK_UnresolvedElementCallback*)Coll_Iter_getObject(callBackIterator);
            callBackIterator = Coll_Iter_getNext(callBackIterator);
            if((elementCallback->type == DK_CALLBACKTYPE_TARGET_NOT_FOUND) &&
                                                    (elementCallback->data.target.interestedEntity == entity))
            {
                if(entity->classID == DK_CLASS_SET_ADMIN || entity->classID == DK_CLASS_MAP_ADMIN)/*TODO this is a temporary fix for the fact that collection elements are not transfered to new generations */
                {
                    if(DK_ObjectHolder_us_getUserData((DK_ObjectHolder*)elementCallback->data.target.holder))
                    {
                        os_free(DK_ObjectHolder_us_getUserData((DK_ObjectHolder*)elementCallback->data.target.holder));
                        DK_ObjectHolder_us_setUserData((DK_ObjectHolder*)elementCallback->data.target.holder, NULL);
                    }
                    DK_ObjectHolder_us_destroy((DK_ObjectHolder*)elementCallback->data.target.holder);
                }
                Coll_Set_remove(&(element->callbacks), elementCallback);
                DK_UnresolvedObjectsUtility_us_destroyUnresolvedElementCallback(elementCallback, userData);
            }
        }

        /* if no more callbacks remain, remove the element from the unresolved elements set and free the memory */
        if(Coll_Set_getNrOfElements(&(element->callbacks)) == 0)
        {
            Coll_Set_remove(unresolvedElements, element);
            DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(element);
        }
    }

    DLRL_INFO(INF_EXIT);
}

DK_UnresolvedElement*
DK_UnresolvedObjectsUtility_us_getUnresolvedElement(
    Coll_Set* unresolvedElements,
    void* sourceValues,
    LOC_unsigned_long valuesSize)
{
    Coll_Iter* iterator = NULL;
    DK_UnresolvedElement* element = NULL;
    DK_UnresolvedElement* anElement = NULL;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert((valuesSize > 0 && sourceValues) || (valuesSize == 0 && !sourceValues));

    iterator = Coll_Set_getFirstElement(unresolvedElements);
    while(iterator && !element)
    {
        anElement = (DK_UnresolvedElement*)Coll_Iter_getObject(iterator);
        if(DK_DCPSUtility_us_areValueArraysEqual(sourceValues, anElement->values, valuesSize))
        {
            element = anElement;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
    return element;
}

DK_UnresolvedElement*
DK_UnresolvedObjectsUtility_us_createUnresolvedElement(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* ownerValues,
    LOC_unsigned_long valuesSize)
{
    DK_UnresolvedElement* element = NULL;
    LOC_long returnCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(unresolvedElements);
    assert(exception);
    /* owner values may be null */

    DLRL_ALLOC(element, DK_UnresolvedElement, exception, "Unable to allocate an unresolved element container");
    Coll_Set_init(&(element->callbacks), pointerIsLessThen, TRUE);
    /* copy it completely (may set values to null) */
    element->values = (void*)DK_DCPSUtility_us_cloneValueArray(exception, ownerValues, valuesSize);
    DLRL_Exception_PROPAGATE(exception);
    element->valuesSize = valuesSize;
    returnCode = Coll_Set_add(unresolvedElements, element);
    if(returnCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add an unresolved element container"
                                                     " to the list of unresolved elements");
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION && element)
    {
        DK_UnresolvedObjectsUtility_us_destroyUnresolvedElement(element);
        element = NULL;
        DLRL_Exception_PROPAGATE(exception);
    }
    DLRL_INFO(INF_EXIT);
    return element;
}
