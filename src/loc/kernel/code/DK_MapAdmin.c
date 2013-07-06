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
#include <string.h>

/* OS abstraction layer includes */
#include "os_heap.h"
#include "os_stdlib.h"

/* DLRL Utilities includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL MetaModel includes */
#include "DMM_Basis.h"

/* DLRL Kernel includes */
#include "DK_CollectionBridge.h"
#include "DK_DCPSUtility.h"
#include "DK_ObjectHolder.h"
#include "DK_MapAdmin.h"
#include "DK_Utility.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

static void
DK_MapAdmin_us_destroy(
    DK_Entity* _this);

/* NOT IN DESIGN */
static DK_ObjectHolder*
DK_MapAdmin_us_createTempHolder(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    const void* key,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
static void
DK_MapAdmin_us_destroyHolder(
    DK_ObjectHolder* holder);

/* NOT IN DESIGN */

static void
DK_MapAdmin_us_put(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key,
    DK_ObjectAdmin* target);

/* Some standard definitions */
#define ENTITY_NAME "DLRL Kernel Map Admin"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

/* The compare function needed for maps which are created as string maps */
int
ObjectHolderStringKeyCompare(
    void *left,
    void *right)
{
    LOC_string leftKey;
    LOC_string rightKey;
    DK_ObjectHolder* leftHolder = (DK_ObjectHolder*)left;
    DK_ObjectHolder* rightHolder = (DK_ObjectHolder*)right;

    DLRL_INFO(INF_ENTER);

    assert(left);
    assert(right);

    leftKey = (LOC_string)DK_ObjectHolder_us_getUserData(leftHolder);
    rightKey = (LOC_string)DK_ObjectHolder_us_getUserData(rightHolder);
    assert(leftKey);
    assert(rightKey);
    DLRL_INFO(INF_EXIT);
    return (strcmp(leftKey, rightKey) < 0);
}

/* The compare function needed for maps which are created as integer maps */
int
ObjectHolderIntKeyCompare(
    void *left,
    void *right)
{
    LOC_long leftKey;
    LOC_long rightKey;
    DK_ObjectHolder* leftHolder = (DK_ObjectHolder*)left;
    DK_ObjectHolder* rightHolder = (DK_ObjectHolder*)right;

    DLRL_INFO(INF_ENTER);

    assert(left);
    assert(right);

    /* no duplicates done, so nothing to release when calling the getTarget function. */
    leftKey = *((LOC_long*)DK_ObjectHolder_us_getUserData(leftHolder));
    rightKey = *((LOC_long*)DK_ObjectHolder_us_getUserData(rightHolder));
    DLRL_INFO(INF_EXIT);
    return (leftKey < rightKey);
}

DK_MapAdmin*
DK_MapAdmin_new(
    DLRL_Exception* exception,
    DK_ObjectAdmin* owner,
    DK_ObjectHomeAdmin* targetHome,
    DMM_DLRLMultiRelation* metaRelation,
    DK_ObjectHomeAdmin* ownerHome)
{
    DK_MapAdmin* _this = NULL;
    DMM_Basis base;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(targetHome);
    assert(ownerHome);
    assert(metaRelation);
    /* owner may be null */

    DLRL_ALLOC(_this, DK_MapAdmin, exception, allocError);

    if(owner)
    {
        _this->base.owner = (DK_ObjectAdmin*)DK_Entity_ts_duplicate((DK_Entity*)owner);
    } else
    {
        _this->base.owner = NULL;
    }
    _this->base.targetHome = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)targetHome);
    _this->base.ownerHome = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)ownerHome);
    _this->base.alive = TRUE;
    _this->base.ls_collection = NULL;
    _this->base.metaRelation = metaRelation;
    _this->base.nrOfUnresolvedElements = 0;

    Coll_List_init(&(_this->base.addedElements));
    Coll_List_init(&(_this->base.removedElements));
    Coll_List_init(&(_this->modifiedElements));

    base = DMM_DLRLMultiRelation_getBasis(metaRelation);
    if(base == DMM_BASIS_STR_MAP)
    {
        Coll_Set_init(&(_this->objectHolders), ObjectHolderStringKeyCompare, TRUE);
        Coll_Set_init(&(_this->base.changedElements), ObjectHolderStringKeyCompare, TRUE);
        Coll_Set_init(&(_this->base.deletedElements), ObjectHolderStringKeyCompare, FALSE);
    } else
    {
        assert(base == DMM_BASIS_INT_MAP);
        Coll_Set_init(&(_this->objectHolders), ObjectHolderIntKeyCompare, TRUE);
        Coll_Set_init(&(_this->base.changedElements), ObjectHolderIntKeyCompare, TRUE);
        Coll_Set_init(&(_this->base.deletedElements), ObjectHolderIntKeyCompare, FALSE);
    }

    DK_Entity_us_init(&(_this->base.entity), DK_CLASS_MAP_ADMIN, DK_MapAdmin_us_destroy);

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_MapAdmin_us_delete(_this, NULL);
        DK_Entity_ts_release(&(_this->base.entity));
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

Coll_Set*
DK_MapAdmin_us_getObjectHolders(
    DK_MapAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->objectHolders);
}

void
DK_MapAdmin_ts_getLSValues(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(arg);

    /* Locks owner and target home, and next operation checks if collection,
     * owner home and target homes are alive. The owner home must be locked
     * because it protects the collection itself and the target home must be
     * locked because it protects the elements in the collection.
     */
    DK_Collection_lockAll((DK_Collection*)_this);
    DK_Collection_us_checkAliveAll((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this->base.targetHome,
        Coll_Set_getFirstElement(&(_this->objectHolders)),
        Coll_Set_getNrOfElements(&(_this->objectHolders)),
        arg,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAll((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

/* a generic function which allows a language specific layer to retrieve
 * the key values for elements in this map for either added elements,
 * modified elements, deleted elements or the currently contained objects.
 */
void
DK_MapAdmin_ts_getLSElementsGeneric(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    DK_MapAdmin_ElementType type)
{
    DMM_Basis base;
    Coll_Iter* iterator;
    DK_ObjectHolder* holder;
    void* keyUserData;
    LOC_unsigned_long count = 0;/* must init to 0*/
    LOC_unsigned_long size = 0;

    DLRL_INFO(INF_ENTER);

    DK_Collection_lockHome((DK_Collection*)_this);
    DK_Collection_us_checkAlive((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    base = DMM_DLRLMultiRelation_getBasis(_this->base.metaRelation);

    if(type == DK_MAPADMIN_ELEMENTTYPE_ADDED)
    {
        Coll_List* elements;

        elements = DK_MapAdmin_us_getAddedElements(_this, exception);
        DLRL_Exception_PROPAGATE(exception);
        size = Coll_List_getNrOfElements(elements);
        iterator = Coll_List_getFirstElement(elements);
    } else if(type == DK_MAPADMIN_ELEMENTTYPE_MODIFIED)
    {
        Coll_List* elements;

        elements = DK_MapAdmin_us_getModifiedElements(_this);
        size = Coll_List_getNrOfElements(elements);
        iterator = Coll_List_getFirstElement(elements);
    } else if(type == DK_MAPADMIN_ELEMENTTYPE_REMOVED)
    {
        Coll_List* elements;

        elements = DK_Collection_us_getRemovedElements((DK_Collection*)_this);
        size = Coll_List_getNrOfElements(elements);
        iterator = Coll_List_getFirstElement(elements);
    } else
    {
        Coll_Set* elements;

        assert(type == DK_MAPADMIN_ELEMENTTYPE_KEYS);
        elements = DK_MapAdmin_us_getObjectHolders(_this);
        size = Coll_Set_getNrOfElements(elements);
        iterator = Coll_Set_getFirstElement(elements);
    }

    /* create the correct sequence for this map */
    if(base == DMM_BASIS_STR_MAP)
    {
        utilityBridge.createStringSeq(
            exception,
            userData,
            arg,
            size);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        assert(base == DMM_BASIS_INT_MAP);
        utilityBridge.createIntegerSeq(
            exception,
            userData,
            arg,
            size);
        DLRL_Exception_PROPAGATE(exception);
    }

    /* now iterate over the elements and insert each one into the sequence */
    while(iterator)
    {
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        keyUserData = DK_ObjectHolder_us_getUserData(holder);
        assert(keyUserData);
        if(base == DMM_BASIS_STR_MAP)
        {
            utilityBridge.addElementToStringSeq(
                exception,
                userData,
                *arg,
                count,
                keyUserData);
            DLRL_Exception_PROPAGATE(exception);
        } else
        {
            assert(base == DMM_BASIS_INT_MAP);
            utilityBridge.addElementToIntegerSeq(
                exception,
                userData,
                *arg,
                count,
                keyUserData);
            DLRL_Exception_PROPAGATE(exception);
        }
        count++;
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockHome((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_destroy(
    DK_Entity* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        /* release the owner and target home here, no one has a reference to this map anymore and thus we do not need the */
        /* target or owner home for their mutex' and can release our references. We cannot release the target and owner */
        /* home before as this map is protected by the mutex located in the owner home and uses the mutex located in the */
        /* target home for some operations. */
        DK_Entity_ts_release((DK_Entity*)(((DK_Collection*)_this)->ownerHome));
        DK_Entity_ts_release((DK_Entity*)(((DK_Collection*)_this)->targetHome));
        if(((DK_Collection*)_this)->owner)
        {
            DK_Entity_ts_release((DK_Entity*)(((DK_Collection*)_this)->owner));
        }
        assert(((DK_Collection*)_this)->nrOfUnresolvedElements == 0);
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_MapAdmin*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_delete(
    DK_MapAdmin* _this,
    void* userData)
{
    Coll_Iter* iterator = NULL;
    void* aHolder = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    if(_this->base.alive)
    {
        /* may not release ownerhome, targethome or owner here! They must always remain valid! The target home,owner home */
        /* and owner references are released when this map is actually destroyed! */
        if(_this->base.nrOfUnresolvedElements > 0 && _this->base.targetHome->alive)
        {
            assert(!_this->base.owner || ((_this->base.owner) && (!DK_ObjectAdmin_us_getAccess(_this->base.owner))));
            DK_ObjectHomeAdmin_us_unregisterAllUnresolvedElementsForEntity(_this->base.targetHome, userData,
                                                                                                    (DK_Entity*)_this);
            assert(_this->base.nrOfUnresolvedElements == 0);
        }
        if(_this->base.ls_collection)
        {
            utilityBridge.releaseLSValuetypeObject(userData, _this->base.ls_collection);
        }
        if(_this->base.metaRelation)
        {
            /* isnt the owner so dont have to destroy it, and the meta model objects arent referneced counted */
            /* so dont have to lower a ref count either. */
            _this->base.metaRelation = NULL;
        }
        DK_MapAdmin_us_resetModificationInformation(_this);
        assert(Coll_List_getNrOfElements(&(_this->base.addedElements)) == 0);
        assert(Coll_List_getNrOfElements(&(_this->base.removedElements)) == 0);
        assert(Coll_List_getNrOfElements(&(_this->modifiedElements)) == 0);

        iterator = Coll_Set_getFirstElement(&(_this->base.changedElements));
        while(iterator)
        {
            aHolder = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->base.changedElements), aHolder);
            /* not the owner, so do not have to delete the holder... (done when iterating through the objectHolders set */
        }
        iterator = Coll_Set_getFirstElement(&(_this->objectHolders));
        while(iterator)
        {
            DK_ObjectAdmin* target;

            aHolder = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->objectHolders), aHolder);
            target = DK_ObjectHolder_us_getTarget((DK_ObjectHolder*)aHolder);
            if(target)
            {
#if 0
                if(DK_ObjectAdmin_us_getWriteState(target) == DK_OBJECT_STATE_OBJECT_DELETED)
                {
                    DK_CacheAccessAdmin_us_decreaseInvalidLinks(target->access);
                }
#endif
                DK_ObjectAdmin_us_unregisterIsRelatedFrom(target, (DK_ObjectHolder*)aHolder);
            }
            /* free the key element stored in the user data of the holder AFTER removing it from the map */
            os_free(DK_ObjectHolder_us_getUserData((DK_ObjectHolder*)aHolder));
            DK_ObjectHolder_us_setUserData((DK_ObjectHolder*)aHolder, NULL);
            /* elements in this set arent reference counted, so no release required */
            DK_ObjectHolder_us_destroy((DK_ObjectHolder*)aHolder);
        }
        iterator = Coll_Set_getFirstElement(&(_this->base.deletedElements));
        while(iterator)
        {
            aHolder = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->base.deletedElements), aHolder);
            /* free the key element stored in the user data of the holder AFTER removing it from the map */
            os_free(DK_ObjectHolder_us_getUserData(aHolder));
            DK_ObjectHolder_us_setUserData(aHolder, NULL);
            /* elements in this set arent reference counted, so no release required */
            DK_ObjectHolder_us_destroy((DK_ObjectHolder*)aHolder);
        }

        _this->base.alive = FALSE;
    }
    DLRL_INFO(INF_EXIT);
}

/* no copy made of the set, just a direct pointer */
Coll_List*
DK_MapAdmin_us_getAddedElements(
    DK_MapAdmin* _this,
    DLRL_Exception* exception)
{
    void* anElement = NULL;
    Coll_Iter* iterator = NULL;
    LOC_long returnCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);

    /* assert we have an owner, otherwise this operation may not be called. And assert that if the owners */
    /* primary object state is object_new that the added elements list is empty. */
    assert(_this->base.owner);

    /* if the object state is new, then it means the added elements list equals the object holders collection */
    /* used within this map. However because this operation requires the return type List, we must copy each element */
    /* into the addedElements list. Naturally we only need to perform the copy action if the addedElements list is still */
    /* empty and if there are actually values in the values list (iterator is not null). Now this is done only during  */
    /* the condition where the object admin is new */
    /* and its done to correctly deal with unresolved owners of collections. Now this might seem as a bad choice */
    /* as we will now introduce additional processing during the getAddedElements operation just to deal with the */
    /* unresolved collections correctly. But this is not the case. For one we are moving the processing, either its */
    /* done during the processing of the updates, or its done here. But it has to be done anyway. Its also logical */
    /* to assume that in most cases applications using the DLRL and collections will utilize this operation for new */
    /* objects. Although they should just get the objectHolders content when dealing with a collection belonging */
    /* to a new object admin, as its more efficient. */
    /* The reason why we copy the object holders values in the added elements list is for the following: */
    /* The added elements collection is a list. Meaning we can only add or remove elements at the end. If we were */
    /* to maintain the added elements collections for unresolved collections as well we would encounter problems as */
    /* elements in the middle of the list can suddenly dissapear, leading to expensive resorts of the added elements */
    /* list. */
    /* Secondly making it best practice to retrieve the obejctHolders collection instead of the addedElements collection */
    /* for collections belong to new objects makes processing updates for these collections faster. */
    iterator = Coll_Set_getFirstElement(&(_this->objectHolders));
    if(iterator && (DK_ObjectAdmin_us_getReadState(_this->base.owner) == DK_OBJECT_STATE_OBJECT_NEW) &&
                                                        (Coll_List_getNrOfElements(&(_this->base.addedElements))== 0))
    {
        while(iterator)
        {
            anElement = Coll_Iter_getObject(iterator);
            returnCode = Coll_List_pushBack(&(_this->base.addedElements), (void*)anElement);
            if (returnCode != COLL_OK)
            {
                DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                    "Unable to add an object to list of added objects of a %s", ENTITY_NAME);
            }
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return &(_this->base.addedElements);
}

Coll_List*
DK_MapAdmin_us_getModifiedElements(
    DK_MapAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    /* assert we have an owner, otherwise this operation may not be called. And assert that if the owners */
    /* primary object state is object_new that the modified elements list is empty. */
    assert(_this->base.owner);
    assert(((DK_ObjectAdmin_us_getReadState(_this->base.owner) == DK_OBJECT_STATE_OBJECT_NEW) &&
                (Coll_List_getNrOfElements(&(_this->modifiedElements))== 0)) ||
            (DK_ObjectAdmin_us_getReadState(_this->base.owner) != DK_OBJECT_STATE_OBJECT_NEW));

    DLRL_INFO(INF_EXIT);
    return &(_this->modifiedElements);
}

LOC_unsigned_long
DK_MapAdmin_ts_getLength(
    DK_MapAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_unsigned_long length = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_Collection_lockHome((DK_Collection*)_this);
    DK_Collection_us_checkAlive((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    length = DK_MapAdmin_us_getLength(_this);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockHome((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
    return length;
}

LOC_unsigned_long
DK_MapAdmin_us_getLength(
    DK_MapAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return Coll_Set_getNrOfElements(&(_this->objectHolders));
}

DLRL_LS_object
DK_MapAdmin_ts_get(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key)
{
    DLRL_LS_object retVal = NULL;
    DK_ObjectAdmin* targetAdmin = NULL;
    DK_ObjectHolder* aHolder = NULL;
    Coll_Iter* iterator = NULL;
    DMM_Basis base;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(key);
    /* userData may be NULL */

    DK_Collection_lockAll((DK_Collection*)_this);
    DK_Collection_us_checkAliveAll((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    base = DMM_DLRLMultiRelation_getBasis(_this->base.metaRelation);
    iterator = Coll_Set_getFirstElement(&(_this->objectHolders));
    while(iterator && !targetAdmin)
    {
        aHolder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        if(((base == DMM_BASIS_STR_MAP) &&
            (0 == strcmp((LOC_const_string)key, (LOC_string)DK_ObjectHolder_us_getUserData(aHolder)))) ||
            ((base == DMM_BASIS_INT_MAP) &&
            (*((LOC_long*)DK_ObjectHolder_us_getUserData(aHolder))) == (*(LOC_long*)key)))
        {
            assert(DK_ObjectHolder_us_isResolved(aHolder));
            targetAdmin = DK_ObjectHolder_us_getTarget(aHolder);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    if(!targetAdmin)
    {
        DLRL_Exception_THROW(exception, DLRL_NO_SUCH_ELEMENT,
            "There is no element within this collection for the specified key.");
    }
    DK_ObjectAdmin_us_checkAlive(targetAdmin, exception);
    DLRL_Exception_PROPAGATE(exception);
    retVal = utilityBridge.localDuplicateLSValuetypeObject(
        userData,
        DK_ObjectAdmin_us_getLSObject(targetAdmin));
    if(!retVal)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Out of resources.");
    }

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAll((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_MapAdmin_us_doPut(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(keyValue);
    assert(holder);
    assert(DK_ObjectHolder_us_getUserData(holder));

    returnCode = Coll_Set_add(&(_this->objectHolders), (void*)holder);
    if (returnCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to add an object to list of objects of a %s", ENTITY_NAME);
    }
    /* only need to add elements to the added list if we have an owner, else the owner is unknown and this collection */
    /* represents an unresolved collection. Unresolved collections do not need to maintain the added/modified/deleted */
    /* elements lists. */
    /* The check for the primary object state is to deal with the case that we had a previously unresolved collection */
    /* and we found the owner during the current update round and are now processing the collection updates in which we */
    /* find added/modified/removed elements. As new objects may only have added elements, we must ensure this. For */
    /* collections belonging to new objectadmins the added elements list will be equal to the normal objectHolders */
    /* collection. I.E. have the same content. For this reason we dont need to maintain two lists. We also do not want an */
    /* element added while the collection owner was unresolved and which was modified/removed (the element) in the update */
    /* round where the owner was resolved to appear in the modified/removed elements list as this would be inconsistent */
    /* with what one would expect. For these reason we check for the object state of the owner and if its new we wont */
    /* utlitize the added/modified/removed elements lists. */
    if(_this->base.owner && (DK_ObjectAdmin_us_getReadState(_this->base.owner) != DK_OBJECT_STATE_OBJECT_NEW))
    {
        returnCode = Coll_List_pushBack(&(_this->base.addedElements), (void*)holder);
        if (returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add an object to list of added objects of a %s", ENTITY_NAME);
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_doMarkAsModified(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(keyValue);
    assert(holder);

    /* only need to add elements to the added list if we have an owner, else the owner is unknown and this collection */
    /* represents an unresolved collection. Unresolved collections do not need to maintain the added/modified/deleted */
    /* elements lists. */
    /* The check for the primary object state is to deal with the case that we had a previously unresolved collection */
    /* and we found the owner during the current update round and are now processing the collection updates in which we */
    /* find added/modified/removed elements. As new objects may only have added elements, we must ensure this. For */
    /* collections belonging to new objectadmins the added elements list will be equal to the normal objectHolders */
    /* collection. I.E. have the same content. For this reason we dont need to maintain two lists. We also do not want an */
    /* element added while the collection owner was unresolved and which was modified/removed (the element) in the update */
    /* round where the owner was resolved to appear in the modified/removed elements list as this would be inconsistent */
    /* with what one would expect. For these reason we check for the object state of the owner and if its new we wont */
    /* utlitize the added/modified/removed elements lists. */
    if(_this->base.owner && (DK_ObjectAdmin_us_getReadState(_this->base.owner) != DK_OBJECT_STATE_OBJECT_NEW))
    {
        returnCode = Coll_List_pushBack(&(_this->modifiedElements), (void*)holder);
        if (returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add an object to list of removed objects of a %s", ENTITY_NAME);
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_doRemove(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(keyValue);
    assert(holder);

    Coll_Set_remove(&(_this->objectHolders), (void*)holder);
    DK_MapAdmin_us_addRemovedElement(_this, exception, keyValue, holder);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_addRemovedElement(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(keyValue);
    assert(holder);
    /* only need to add elements to the added list if we have an owner, else the owner is unknown and this collection */
    /* represents an unresolved collection. Unresolved collections do not need to maintain the added/modified/deleted */
    /* elements lists. */
    /* The check for the primary object state is to deal with the case that we had a previously unresolved collection */
    /* and we found the owner during the current update round and are now processing the collection updates in which we */
    /* find added/modified/removed elements. As new objects may only have added elements, we must ensure this. For */
    /* collections belonging to new objectadmins the added elements list will be equal to the normal objectHolders */
    /* collection. I.E. have the same content. For this reason we dont need to maintain two lists. We also do not want an */
    /* element added while the collection owner was unresolved and which was modified/removed (the element) in the update */
    /* round where the owner was resolved to appear in the modified/removed elements list as this would be inconsistent */
    /* with what one would expect. For these reason we check for the object state of the owner and if its new we wont */
    /* utlitize the added/modified/removed elements lists. */
    if(_this->base.owner)
    {
        returnCode = Coll_List_pushBack(&(_this->base.removedElements), (void*)holder);
        if (returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add an object to list of removed objects of a %s", ENTITY_NAME);
        }
    } else
    {
        /* in case of a collection with an unresolved owner we can completely clean the element I.E. free the key value */
        /* & destroy the holder. Otherwise we would do it when resetting the modification information. */
        assert(keyValue == DK_ObjectHolder_us_getUserData(holder));/* pointers should be equal. */
        assert(DK_ObjectHolder_us_getUserData(holder));
        os_free(DK_ObjectHolder_us_getUserData(holder));
        DK_ObjectHolder_us_setUserData(holder, NULL);
        DK_ObjectHolder_us_destroy(holder);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_resetModificationInformation(
    DK_MapAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    while(Coll_List_getNrOfElements(&(_this->base.addedElements)) > 0)
    {
        /* elements are not reference counted in this list, as they are contained within the object holders as well. */
        /* therefore we just remove the elements from the list and nothing more */
        Coll_List_popBack(&(_this->base.addedElements));
    }
    while(Coll_List_getNrOfElements(&(_this->modifiedElements)) > 0)
    {
        /* elements are not reference counted in this list, as they are contained within the object holders as well. */
        /* therefore we just remove the elements from the list and nothing more */
        Coll_List_popBack(&(_this->modifiedElements));
    }
    while(Coll_List_getNrOfElements(&(_this->base.removedElements)) > 0)
    {
        /* destroy all elements within the removed elements list, they are no longer of any use. */
        DK_ObjectHolder* aHolder = (DK_ObjectHolder*)Coll_List_popBack(&(_this->base.removedElements));
        assert(DK_ObjectHolder_us_getUserData(aHolder));
        os_free(DK_ObjectHolder_us_getUserData(aHolder));
        DK_ObjectHolder_us_setUserData(aHolder, NULL);
        DK_ObjectHolder_us_destroy(aHolder);

    }
    DLRL_INFO(INF_EXIT);
}

DK_ObjectHolder*
DK_MapAdmin_us_createTempHolder(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    const void* key,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHolder* holder = NULL;
    DMM_Basis type;
    void* keyValue = NULL;
    LOC_unsigned_long index = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(key);
    assert(objectAdmin);

    index = _this->base.metaRelation->index;
    /* create an object holder and set the key as its user data. We can then use this holder to see if it is already */
    /* contained within the collection (ie an element with the same key). This might mean we allocate the holder */
    /* unnecesarily for some cases, but this is only if a key was already in use. We can optimize that but the flipside */
    /* is a(small) performance degradation for the cases when the key was not yet contained in the collection.  */
    /* so the choice is to leave the code as it is right now.  */
    holder = DK_ObjectHolder_newResolved(exception, (DK_Entity*)_this, objectAdmin, DK_DCPSUtility_ts_getNilHandle(),
                                        index);
    DLRL_Exception_PROPAGATE(exception);

    type = DMM_DLRLMultiRelation_getBasis(_this->base.metaRelation);
    if(type == DMM_BASIS_STR_MAP)
    {
        DLRL_STRDUP(keyValue, ((LOC_string)key), exception,"Unable to allocate memory for copy of the key value");
    } else
    {
        assert(type == DMM_BASIS_INT_MAP);
        DLRL_ALLOC_WITH_SIZE(keyValue, sizeof(c_long), exception,"Unable to allocate memory for copy of the key value");
        *((c_long*)keyValue) = *((c_long*)key);
    }
    assert(keyValue);
    DK_ObjectHolder_us_setUserData(holder, keyValue);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION && holder)
    {
        /* if we arrive here, then the keyValue will always be NULL, as an exception should have occured during  */
        /* allocation of the key value (last operation to be able to throw an exception) */
        DK_MapAdmin_us_destroyHolder(holder);
        holder = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return holder;
}

void
DK_MapAdmin_us_destroyHolder(
    DK_ObjectHolder* holder)
{
    void* keyValue = NULL;

    DLRL_INFO(INF_ENTER);

    assert(holder);

    keyValue = DK_ObjectHolder_us_getUserData(holder);
    if(keyValue)
    {
        os_free(keyValue);
        DK_ObjectHolder_us_setUserData(holder, NULL);
    }
    DK_ObjectHolder_us_destroy(holder);

    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_ts_put(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key,
    DK_ObjectAdmin* target)
{
    DK_CacheAccessAdmin* access = NULL;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData, key, target may all be NIL */

    DK_Collection_lockAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_Collection_us_checkAliveAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    if(!key)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to insert element into the collection. The "
        "key provided represents a NIL pointer. It's not valid to insert elements with a NIL pointer key to a "
        "collection.");
    }
    if(!target)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to insert element into the collection. The "
        "object being inserted represents a NIL pointer. It's not valid to insert NIL pointers to a collection.");
    }
    access = DK_ObjectAdmin_us_getAccess(_this->base.owner);
    assert(access);
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to insert element into the collection. The "
        "collection is not located in a writeable cache access");
    }
    if(!DK_ObjectAdmin_us_getIsRegistered(_this->base.owner))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to insert element into the collection. The "
                    "'owner' of the collection has not yet been registered to it's cache access.");
    }
    DK_MapAdmin_us_put(_this, exception, userData, key, target);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAllForChanges((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_put(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key,
    DK_ObjectAdmin* target)
{
    LOC_boolean destroyHolder = FALSE;
    DK_ObjectHolder* holder = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData, key, target may all be NIL */

    holder = DK_MapAdmin_us_createTempHolder(_this, exception, key, target);
    DLRL_Exception_PROPAGATE(exception);
    destroyHolder = DK_Collection_us_insertElement((DK_Collection*)_this, exception, userData, holder,
                                                    &(_this->objectHolders));
    DLRL_Exception_PROPAGATE(exception);
    if(destroyHolder)
    {/* it was never inserted anywhere */
        DK_MapAdmin_us_destroyHolder(holder);
        holder = NULL;
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION && holder)
    {
        DK_MapAdmin_us_destroyHolder(holder);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_us_copyElementsFromCollection(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_MapAdmin* targetCollection)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    void* key = NULL;
    DK_ObjectAdmin* target = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetCollection);
    /* userData may be NIL */

    iterator = Coll_Set_getFirstElement(&(targetCollection->objectHolders));
    while(iterator)
    {
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        key = DK_ObjectHolder_us_getUserData(holder);
        target = DK_ObjectHolder_us_getTarget(holder);
        DK_MapAdmin_us_put(_this, exception, userData, key, target);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_MapAdmin_ts_remove(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key)
{
    DK_ObjectHolder tmpHolder;/* on stack definition */
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* key may be nil */

    DK_Collection_lockAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_Collection_us_checkAliveAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    access = DK_ObjectAdmin_us_getAccess(_this->base.owner);
    assert(access);
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to remove element from the map. The map is "
                    "not located in a writeable cache access");
    }
    if(!key)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to remove element from the collection. The "
        "key provided represents a NIL pointer.");
    }
    access = DK_ObjectAdmin_us_getAccess(_this->base.owner);
    assert(access);
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to remove element from the collection. The "
        "collection is not located in a writeable cache access");
    }
    if(!DK_ObjectAdmin_us_getIsRegistered(_this->base.owner))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to remove element from the collection. The "
                    "'owner' of the collection has not yet been registered to it's cache access.");
    }
    /* prepare the holder so we can use it to see if the object is contained...  */
    DK_ObjectHolder_us_setUserData(&tmpHolder, (void*)key);

    DK_Collection_us_removeElement((DK_Collection*)_this, exception, userData, &tmpHolder, &(_this->objectHolders));
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAllForChanges((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}


void
DK_MapAdmin_ts_clear(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_Collection_lockAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_Collection_us_checkAliveAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    access = DK_ObjectAdmin_us_getAccess(_this->base.owner);
    assert(access);
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to clear elements from the collection. The "
                    "collection is not located in a writeable cache access");
    }
    if(!DK_ObjectAdmin_us_getIsRegistered(_this->base.owner))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to clear elements from the collection. The "
                    "'owner' of the collection has not yet been registered to it's cache access.");
    }
    DK_Collection_us_clear((DK_Collection*)_this, exception, userData, &(_this->objectHolders));
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAllForChanges((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_MapAdmin_us_hasInvalidElements(
    DK_MapAdmin* _this)
{
    Coll_Iter* elementIterator = NULL;
    LOC_boolean hasInvalidRelations = FALSE;
    DK_ObjectHolder* aHolder = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    elementIterator = Coll_Set_getFirstElement(&(_this->objectHolders));
    while(elementIterator && !hasInvalidRelations)
    {
        aHolder = Coll_Iter_getObject(elementIterator);
        hasInvalidRelations = DK_Utility_us_isCollectionElementInvalid(aHolder);
        elementIterator = Coll_Iter_getNext(elementIterator);
    }

    DLRL_INFO(INF_EXIT);
    return hasInvalidRelations;
}
