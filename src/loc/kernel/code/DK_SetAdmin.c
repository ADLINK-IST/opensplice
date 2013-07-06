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

/* OS abstraction layer includes */
#include "os_heap.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_CollectionBridge.h"
#include "DK_DCPSUtility.h"
#include "DK_ObjectHolder.h"
#include "DK_SetAdmin.h"
#include "DK_Utility.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

#define ENTITY_NAME "DLRL Kernel Set Admin"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;
static void
DK_SetAdmin_us_destroy(
    DK_Entity * _this);

static int
DK_SetAdmin_ObjectHolderTargetPointerIsLessThen(
    void *left,
    void *right);

static void
DK_SetAdmin_us_add(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* target);

int
DK_SetAdmin_ObjectHolderTargetPointerIsLessThen(
    void *left,
    void *right)
{
    int returnValue = 0;
    DK_ObjectHolder* leftHolder = NULL;
    DK_ObjectHolder* rightHolder = NULL;
    DK_ObjectAdmin* leftTarget = NULL;
    DK_ObjectAdmin* rightTarget = NULL;

    DLRL_INFO(INF_ENTER);

    assert(left);
    assert(right);

    leftHolder = (DK_ObjectHolder*)left;
    rightHolder = (DK_ObjectHolder*)right;
    /* no duplicates done, so nothing to release when calling the getTarget function. */
    leftTarget = DK_ObjectHolder_us_getTarget(leftHolder);
    rightTarget = DK_ObjectHolder_us_getTarget(rightHolder);
    if(leftTarget || rightTarget)
    {
        returnValue = (leftTarget < rightTarget);
    } else
    {
        returnValue = (leftHolder < rightHolder);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

DK_SetAdmin*
DK_SetAdmin_new(
    DLRL_Exception* exception,
    DK_ObjectAdmin* owner,
    DK_ObjectHomeAdmin* targetHome,
    DMM_DLRLMultiRelation* metaRelation,
    DK_ObjectHomeAdmin* ownerHome)
{
    DK_SetAdmin* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(targetHome);
    /* owner may be null */
    assert(ownerHome);
    assert(metaRelation);

    DLRL_ALLOC(_this, DK_SetAdmin, exception, allocError);

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

    Coll_Set_init(&(_this->objectHolders), DK_SetAdmin_ObjectHolderTargetPointerIsLessThen, FALSE);
    Coll_List_init(&(_this->base.addedElements));
    Coll_List_init(&(_this->base.removedElements));

    Coll_Set_init(&(_this->base.changedElements), DK_SetAdmin_ObjectHolderTargetPointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->base.deletedElements), DK_SetAdmin_ObjectHolderTargetPointerIsLessThen, FALSE);

    DK_Entity_us_init(&(_this->base.entity), DK_CLASS_SET_ADMIN, DK_SetAdmin_us_destroy);

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_SetAdmin_us_delete(_this, NULL);
        DK_Entity_ts_release(&(_this->base.entity));
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_SetAdmin_us_destroy(
    DK_Entity * _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        /* release the owner and target home here, no one has a reference to this set anymore and thus we do not need the */
        /* target or owner home for their mutex' and can release our references. We cannot release the target and owner */
        /* home before as this set is protected by the mutex located in the owner home and uses the mutex located in the */
        /* target home for some operations. */
        DK_Entity_ts_release((DK_Entity*)(((DK_Collection*)_this)->ownerHome));
        DK_Entity_ts_release((DK_Entity*)(((DK_Collection*)_this)->targetHome));
        if(((DK_Collection*)_this)->owner)
        {
            DK_Entity_ts_release((DK_Entity*)(((DK_Collection*)_this)->owner));
        }
        assert(((DK_Collection*)_this)->nrOfUnresolvedElements == 0);
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_SetAdmin*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_us_delete(
    DK_SetAdmin* _this,
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
        /* and owner references are released when this set is actually destroyed! */
         if(_this->base.nrOfUnresolvedElements > 0 && _this->base.targetHome->alive)
        {
             /* unresolved elements dont exist for collections that exist in cache accesses. */
            assert((!_this->base.owner) || ((_this->base.owner) && (!DK_ObjectAdmin_us_getAccess(_this->base.owner))));
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
            /* not the owner so dont have to delete, also not reference counted so dont have to release */
            _this->base.metaRelation = NULL;
        }
        DK_SetAdmin_us_resetModificationInformation(_this);
        assert(Coll_List_getNrOfElements(&(_this->base.addedElements)) == 0);
        assert(Coll_List_getNrOfElements(&(_this->base.removedElements)) == 0);
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
            /* elements in this set arent reference counted, so no release required */
            /* TODO ID: 150 */
            /* dont use the user data of the object holder, so dont have to reset it (which is a precondition of the */
            /* destroy operation) */
            assert(!DK_ObjectHolder_us_getUserData((DK_ObjectHolder*)aHolder));
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

LOC_unsigned_long
DK_SetAdmin_ts_getLength(
    DK_SetAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_unsigned_long length = 0;
    DK_Collection* collection = (DK_Collection*)_this;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_Collection_lockHome(collection);
    DK_Collection_us_checkAlive(collection, exception);
    DLRL_Exception_PROPAGATE(exception);
    length = DK_SetAdmin_us_getLength(_this);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockHome(collection);
    DLRL_INFO(INF_EXIT);
    return length;
}

LOC_unsigned_long
DK_SetAdmin_us_getLength(
    DK_SetAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return Coll_Set_getNrOfElements(&(_this->objectHolders));
}

Coll_Set*
DK_SetAdmin_us_getHolders(
    DK_SetAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->objectHolders);
}

void
DK_SetAdmin_ts_getLSValues(
    DK_SetAdmin* _this,
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

void
DK_SetAdmin_ts_getLSAddedElements(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_List* addedElements;
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

    addedElements = DK_SetAdmin_us_getAddedElements(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this->base.targetHome,
        Coll_List_getFirstElement(addedElements),
        Coll_List_getNrOfElements(addedElements),
        arg,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAll((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
void
DK_SetAdmin_ts_getLSRemovedElements(
    DK_SetAdmin* _this,
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
        Coll_List_getFirstElement(&(_this->base.removedElements)),
        Coll_List_getNrOfElements(&(_this->base.removedElements)),
        arg,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAll((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

/* no copy made of the set, just a direct pointer */
Coll_List*
DK_SetAdmin_us_getAddedElements(
    DK_SetAdmin* _this,
    DLRL_Exception* exception)
{
    void* anElement = NULL;
    Coll_Iter* iterator = NULL;
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

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
    /* Secondly making it best practice to retrieve the objectHolders collection instead of the addedElements collection */
    /* for collections belong to new objects makes processing updates for these collections faster. */
    iterator = Coll_Set_getFirstElement(&(_this->objectHolders));
    if(iterator && (DK_ObjectAdmin_us_getReadState(_this->base.owner) == DK_OBJECT_STATE_OBJECT_NEW) &&
                            (Coll_List_getNrOfElements(&(_this->base.addedElements))== 0))
    {
        while(iterator)
        {
            anElement = Coll_Iter_getObject(iterator);
            returnCode = Coll_List_pushBack(&(_this->base.addedElements), anElement);
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

LOC_boolean
DK_SetAdmin_ts_contains(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin)
{
    DK_Collection* collection = (DK_Collection*)_this;
    DK_ObjectHolder tempHolder;
    LOC_boolean contained = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);

    /* init the temp holder */
    tempHolder.ref.target = objectAdmin;
    tempHolder.resolved = TRUE;
    tempHolder.elementHandle = DK_DCPSUtility_ts_getNilHandle();
    tempHolder.owner = NULL;
    tempHolder.userData = NULL;

    DK_Collection_lockHome(collection);
    DK_Collection_us_checkAlive(collection, exception);
    DLRL_Exception_PROPAGATE(exception);

    contained = Coll_Set_contains(&(_this->objectHolders), (void*)&tempHolder);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockHome(collection);
    DLRL_INFO(INF_EXIT);
    return contained;
}

/* caller must ensure the object beind added is unique within the set! */
void
DK_SetAdmin_us_doAdd(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);

    returnCode = Coll_Set_addUniqueObject(&(_this->objectHolders), (void*)holder);
    if (returnCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to add an object to list of objects of a %s", ENTITY_NAME);
    }
    DK_SetAdmin_us_addAddedElement(_this, exception, holder);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_us_doRemove(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);

    Coll_Set_remove(&(_this->objectHolders), (void*)holder);
    DK_SetAdmin_us_addRemovedElement(_this, exception, holder);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_us_addAddedElement(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);

    /* only need to add elements to the added list if we have an owner, else the owner is unknown and this collection */
    /* represents an unresolved collection. Unresolved collections do not need to maintain the added/deleted */
    /* elements lists. */
    /* The check for the primary object state is to deal with the case that we had a previously unresolved collection */
    /* and we found the owner during the current update round and are now processing the collection updates in which we */
    /* find added/removed elements. As new objects may only have added elements, we must ensure this. For */
    /* collections belonging to new objectadmins the added elements list will be equal to the normal objectHolders */
    /* collection. I.E. have the same content. For this reason we dont need to maintain two lists. We also do not want an */
    /* element added while the collection owner was unresolved and which was removed (the element) in the update */
    /* round where the owner was resolved to appear in the removed elements list as this would be inconsistent */
    /* with what one would expect. For these reason we check for the object state of the owner and if its new we wont */
    /* utlitize the added/removed elements lists. */
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

/* any holder entering this operation will eventually be destroyed. It's possible this is done within this operation */
/* as well depending on the situation. */
void
DK_SetAdmin_us_addRemovedElement(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);

    /* only need to add elements to the added list if we have an owner, else the owner is unknown and this collection */
    /* represents an unresolved collection. Unresolved collections do not need to maintain the added/deleted */
    /* elements lists. */
    /* The check for the primary object state is to deal with the case that we had a previously unresolved collection */
    /* and we found the owner during the current update round and are now processing the collection updates in which we */
    /* find added/removed elements. As new objects may only have added elements, we must ensure this. For */
    /* collections belonging to new objectadmins the added elements list will be equal to the normal objectHolders */
    /* collection. I.E. have the same content. For this reason we dont need to maintain two lists. We also do not want an */
    /* element added while the collection owner was unresolved and which was removed (the element) in the update */
    /* round where the owner was resolved to appear in the removed elements list as this would be inconsistent */
    /* with what one would expect. For these reason we check for the object state of the owner and if its new we wont */
    /* utlitize the added/removed elements lists. */
    if(_this->base.owner)
    {
        /* object holder is destroy once its cleared from the removed elements */
        returnCode = Coll_List_pushBack(&(_this->base.removedElements), (void*)holder);
        if (returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add an object to list of removed objects of a %s", ENTITY_NAME);
        }
    } else
    {
        /* in case of a collection with an unresolved owner we can completely clean the element I.E. destroy the holder */
        /* otherwise we would do it when resetting the modification information. */
        assert(!DK_ObjectHolder_us_getUserData(holder));
        DK_ObjectHolder_us_destroy(holder);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_us_resetModificationInformation(
    DK_SetAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    while(Coll_List_getNrOfElements(&(_this->base.addedElements)) > 0)
    {
        /* elements are reference counted in this list, as they are contained within the object holders as well. */
        /* therefore we just remove the elements from the list and nothing more */
        Coll_List_popBack(&(_this->base.addedElements));
    }
    while(Coll_List_getNrOfElements(&(_this->base.removedElements)) > 0)
    {
        /* destroy all elements within the removed elements list, they are no longer of any use. */
        DK_ObjectHolder* holder = (DK_ObjectHolder*)Coll_List_popBack(&(_this->base.removedElements));
        assert(!DK_ObjectHolder_us_getUserData(holder));
        DK_ObjectHolder_us_destroy(holder);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_ts_add(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* target)
{
    DK_CacheAccessAdmin* access = NULL;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* target may be NIL */

    DK_Collection_lockAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_Collection_us_checkAliveAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
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

    DK_SetAdmin_us_add(_this, exception, userData, target);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAllForChanges((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_us_add(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* target)
{
    LOC_boolean destroyHolder = FALSE;
    DK_ObjectHolder* holder = NULL;
    LOC_unsigned_long index = 0;

    DLRL_INFO_OBJECT(INF_ENTER);


    index = _this->base.metaRelation->index;
    /* create an object holder. We can then use this holder to see if it is already */
    /* contained within the collection. This might mean we allocate the holder */
    /* unnecesarily for some cases. We can optimize that but the flipside */
    /* is a(small) performance degradation for the cases when the object was not yet contained in the collection.  */
    /* so the choice is to leave the code as it is right now.  */
    holder = DK_ObjectHolder_newResolved(exception, (DK_Entity*)_this, target, DK_DCPSUtility_ts_getNilHandle(),
                                        index);
    DLRL_Exception_PROPAGATE(exception);
    destroyHolder = DK_Collection_us_insertElement((DK_Collection*)_this, exception, userData, holder,
                                                    &(_this->objectHolders));
    DLRL_Exception_PROPAGATE(exception);
    if(destroyHolder)
    {
        DK_ObjectHolder_us_destroy(holder);
        holder = NULL;
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION && holder)
    {
        DK_ObjectHolder_us_destroy(holder);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_us_copyElementsFromCollection(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_SetAdmin* targetCollection)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    DK_ObjectAdmin* target = NULL;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetCollection);
    /* user data may be null*/

    iterator = Coll_Set_getFirstElement(&(targetCollection->objectHolders));
    while(iterator)
    {
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        target = DK_ObjectHolder_us_getTarget(holder);
        DK_SetAdmin_us_add(_this, exception, userData, target);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_SetAdmin_ts_clear(
    DK_SetAdmin* _this,
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

void
DK_SetAdmin_ts_remove(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHolder tmpHolder;/* on stack definition */
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* objectAdmin may be NULL */

    DK_Collection_lockAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_Collection_us_checkAliveAllForChanges((DK_Collection*)_this, exception);
    DLRL_Exception_PROPAGATE(exception);
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
    if(!objectAdmin)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to remove element from the collection. The "
        "object provided represents a NIL pointer.");
    }
    /* prepare the holder so we can use it to see if the object is contained...  */
    tmpHolder.resolved = TRUE;
    tmpHolder.ref.target = objectAdmin;/* dont use the setter, as that operation performs a duplicate we dont need here.. */

    DK_Collection_us_removeElement((DK_Collection*)_this, exception, userData, &tmpHolder, &(_this->objectHolders));
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAllForChanges((DK_Collection*)_this);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_SetAdmin_us_hasInvalidElements(
    DK_SetAdmin* _this)
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
