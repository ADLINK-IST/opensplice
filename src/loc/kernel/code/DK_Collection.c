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

/* DLRL kernel includes */
#include "DK_Collection.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_CacheAccessAdmin.h"
#include "DK_ObjectBridge.h"
#include "DK_Utility.h"
#include "DLRL_Kernel_private.h"

#define ENTITY_NAME "DLRL Kernel Collection"

/* no duplicate done */
DK_ObjectHomeAdmin*
DK_Collection_us_getTargetHome(
    DK_Collection* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->targetHome;
}

/* no duplicate done */
DK_ObjectAdmin*
DK_Collection_us_getOwner(
    DK_Collection* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

/* no copy made of the set, just a direct pointer */
Coll_List*
DK_Collection_us_getRemovedElements(
    DK_Collection* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);
    /* assert we have an owner, otherwise this operation may not be called.*/
    assert(_this->owner);

    DLRL_INFO(INF_EXIT);
    return &(_this->removedElements);
}

DK_Class
DK_Collection_us_getClassID(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->entity.classID;
}

void
DK_Collection_us_setLSObject(
    DK_Collection* _this,
    DLRL_LS_object ls_collection)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* ls_collection may be null */

    _this->ls_collection = ls_collection;

    DLRL_INFO(INF_EXIT);
}

DLRL_LS_object
DK_Collection_us_getLSObject(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_collection;
}

Coll_Set*
DK_Collection_us_getChangedElements(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->changedElements);
}

Coll_Set*
DK_Collection_us_getDeletedElements(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->deletedElements);
}

DMM_DLRLMultiRelation*
DK_Collection_us_getMetaRepresentative(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->metaRelation;
}

void
DK_Collection_us_increaseNrOfUnresolvedElements(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->nrOfUnresolvedElements++;

    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_us_decreaseNrOfUnresolvedElements(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->nrOfUnresolvedElements--;

    DLRL_INFO(INF_EXIT);
}

LOC_long
DK_Collection_us_getNrOfUnresolvedElements(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->nrOfUnresolvedElements;
}

/* owner of the collection must be null when calling this. */
void
DK_Collection_us_setOwner(
    DK_Collection* _this,
    DK_ObjectAdmin* owner)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(owner);
    assert(!(_this->owner));

    _this->owner = (DK_ObjectAdmin*)DK_Entity_ts_duplicate((DK_Entity*)owner);

    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_lockHome(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->ownerHome);

    DK_ObjectHomeAdmin_lockAdmin(_this->ownerHome);
    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_unlockHome(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->ownerHome);

    DK_ObjectHomeAdmin_unlockAdmin(_this->ownerHome);

    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_lockAll(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->ownerHome);
    assert(_this->targetHome);

    DK_Utility_lockAdminForTwoHomesInSequence(
        _this->ownerHome,
        _this->targetHome);

    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_unlockAll(
    DK_Collection* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->ownerHome);
    assert(_this->targetHome);

    DK_Utility_unlockAdminForTwoHomesInSequence(
        _this->ownerHome,
        _this->targetHome);

    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_us_checkAliveAll(
    DK_Collection* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(_this->ownerHome);
    assert(_this->targetHome);

    DK_Collection_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectHomeAdmin_us_checkAlive(_this->ownerHome, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectHomeAdmin_us_checkAlive(_this->targetHome, exception);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_lockAllForChanges(
    DK_Collection* _this,
    DLRL_Exception* exception)
{
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->ownerHome);
    assert(_this->targetHome);
    /* this operation should never be called in a flow where a collection has
     * no owner (I.E. unresolved collections)
     */
    assert(_this->owner);

    /* locks the access & verifies its alive. If no access is found a
     * precondition not met is raised
     */
    access = DK_ObjectAdmin_us_getAccess(_this->owner);
    if(!access)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to change collection. The %s %p is not located in a "
            "CacheAccess.",
            ENTITY_NAME,
            _this);
    }
    DK_CacheAccessAdmin_lock(access);
#ifndef NDEBUG
    if(DK_CacheAccessAdmin_us_isAlive(access))
    {
        assert(_this->ownerHome->alive);
        assert(_this->targetHome->alive);
    }
#endif
    DK_Utility_lockAdminForTwoHomesInSequence(
        _this->ownerHome,
        _this->targetHome);

    DLRL_Exception_EXIT(exception);
    /* no rollback, the unlock takes care of all situations (IE only cache
     * access locked and not the homes locked)
     */
    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_unlockAllForChanges(
    DK_Collection* _this)
{
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->ownerHome);
    assert(_this->targetHome);
    /* this operation should never be called in a flow where a collection has
     * no owner (I.E. unresolved collections)
     */
    assert(_this->owner);

    access = DK_ObjectAdmin_us_getAccess(_this->owner);
    if(access)
    {
        DK_Utility_unlockAdminForTwoHomesInSequence(
            _this->ownerHome,
            _this->targetHome);
        DK_CacheAccessAdmin_unlock(access);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_us_checkAliveAllForChanges(
    DK_Collection* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(_this->ownerHome);
    assert(_this->targetHome);
    /* this operation should never be called in a flow where a collection has
     * no owner (I.E. unresolved collections)
     */
    assert(_this->owner);

    if(!_this->alive)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!",
            ENTITY_NAME,
            _this);
    }
    /* if the collection is alive, then its owning access must also be alive */
    assert(DK_CacheAccessAdmin_us_isAlive(
        DK_ObjectAdmin_us_getAccess(_this->owner)));
    /* if the access is alive, then all homes must be alive */
    assert(_this->ownerHome->alive);
    assert(_this->targetHome->alive);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}


void
DK_Collection_us_checkAlive(
    DK_Collection* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_Collection_us_insertElement(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* holder,
    Coll_Set* objectHolders)
{
    DK_CacheAccessAdmin* access = NULL;
    DK_ObjectAdmin* target = NULL;
    Coll_Iter* existingElement = NULL;
    DK_ObjectHolder* aHolder = NULL;
    LOC_boolean destroyHolder = FALSE;
    long errorCode = COLL_OK;
    LOC_unsigned_long collectionIndex = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);
    assert(objectHolders);
    assert(_this->owner);
    /* userData may be NIL */

    access = DK_ObjectAdmin_us_getAccess(_this->owner);
    assert(access);
    /* verify that we are allowed to change this collection, if not we raise
     * an exception.
     */
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) ==
        DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to insert element into the collection. The collection is "
            "not located in a writeable cache access");
    }
    target = DK_ObjectHolder_us_getTarget(holder);
    assert(target);
    if(!DK_ObjectAdmin_us_isAlive(target))
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to insert element into the collection. The object being "
            "inserted has already been deleted. It's not valid to insert "
            "already deleted objects to a collection.");
    }
    if(DK_ObjectAdmin_us_getWriteState(target) ==
        DK_OBJECT_STATE_OBJECT_DELETED)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to insert element into the collection. The object being "
            "inserted is marked for destruction in the next write action. It's "
            "not valid to insert an object which is marked for destruction to "
            "a collection.");
    }
    existingElement = Coll_Set_find(objectHolders, (void*)holder);
    if(!existingElement)
    {
        /* it could be that the element represented by the key was removed
         * earlier. If so we need to undo this action.
         */
        aHolder = Coll_Set_remove(&(_this->deletedElements), holder);
        if(aHolder)
        {
            /* transform oldholder into the holder, clean up the holder */
            assert(DK_ObjectHolder_us_getTarget(aHolder) == target);
            destroyHolder = TRUE;
            holder = aHolder;
        }/* else it wasnt contained in the removed elements set. */
        /* add it to the object holders list, it isnt contained there yet. */
        errorCode = Coll_Set_add(objectHolders, holder);
        if(errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation. Out of resources.");
        }
        /* add the element to the set of changed elements, it isnt contained
         * there yet.
         */
        errorCode = Coll_Set_add(&(_this->changedElements), holder);
        if(errorCode != COLL_OK)
        {
           DLRL_Exception_THROW(
               exception,
               DLRL_OUT_OF_MEMORY,
               "Unable to complete operation. Out of resources.");
        }
        /* if the owner of this collection was still marked as not modified,
         * we should now change it to modified.
         */
        if(DK_ObjectAdmin_us_getWriteState(_this->owner) ==
            DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
        {
            DK_ObjectAdmin_us_setWriteState(
                _this->owner,
                userData,
                DK_OBJECT_STATE_OBJECT_MODIFIED);
            DK_CacheAccessAdmin_us_markObjectAsChanged(
                DK_ObjectAdmin_us_getAccess(_this->owner),
                exception,
                _this->owner);
            DLRL_Exception_PROPAGATE(exception);
        }
        collectionIndex =DMM_DLRLMultiRelation_us_getIndex(_this->metaRelation);
        DK_ObjectAdmin_us_collectionHasChanged(_this->owner, collectionIndex);
        /* we need to update the isRelatedFrom list in the target objectAdmin. */
        DK_ObjectAdmin_us_registerIsRelatedFrom(target, exception, holder);
        DLRL_Exception_PROPAGATE(exception);
    } else
    {
        DK_Collection_us_changeExistingElement(
            _this,
            exception,
            userData,
            Coll_Iter_getObject(existingElement),
            target,
            access);
        DLRL_Exception_PROPAGATE(exception);
        /* clean up the holder we created in this operation, we no longer
         * need it.
         */
        destroyHolder = TRUE;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return destroyHolder;
}

void
DK_Collection_us_changeExistingElement(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* existingElement,
    DK_ObjectAdmin* objectAdmin,
    DK_CacheAccessAdmin* access)
{
    DK_ObjectAdmin* oldTarget = NULL;
    long errorCode = COLL_OK;
    LOC_unsigned_long collectionIndex = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(existingElement);
    assert(objectAdmin);
    assert(access);

    /* only resolved holders may be contained, therefore the existing element
     * must be resolved.
     */
    assert(DK_ObjectHolder_us_isResolved(existingElement));
    /* get the 'old' target of the object holder */
    oldTarget = DK_ObjectHolder_us_getTarget(existingElement);
    /* NIL pointers arent allowed in a map, so we shouldn't be seeing this. */
    assert(oldTarget);
    /* if the old target doesnt match the objectAdmin which was provided as
     * param then we have a real change on our hands, lets process it by all
     * means! :)
     */
    if(oldTarget != objectAdmin)
    {
#if 0
        /* if the target was marked for deletion, then it means this OH was one
         * of the invalid links marked in the  cache access. We can (well must)
         * now decrease this number by one.
         */
        if(DK_ObjectAdmin_us_getWriteState(oldTarget) ==
            DK_OBJECT_STATE_OBJECT_DELETED)
        {
            DK_CacheAccessAdmin_us_decreaseInvalidLinks(access);
        }
#endif
#ifndef NDEBUG
        printf("NDEBUG - might need to lock the home of the oldTarget in case of inheritance!\n");
#endif
        /* update the is related from lists in the oldTarget and the new
         * target objectAdmin.
         */
        DK_ObjectAdmin_us_unregisterIsRelatedFrom(oldTarget, existingElement);
        DK_ObjectHolder_us_setTarget(existingElement, objectAdmin);
        DK_ObjectAdmin_us_registerIsRelatedFrom(
            objectAdmin,
            exception,
            existingElement);
        DLRL_Exception_PROPAGATE(exception);
        /* add the element to the set of changed elements, if it was already
         * contained, nothing will happen.
         */
        errorCode = Coll_Set_add(&(_this->changedElements), existingElement);
        if(errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation. Out of resources.");
        }
        /* if the owner of this collection was still marked as not modified,
         * we should now change it to modified.
         */
        if(DK_ObjectAdmin_us_getWriteState(_this->owner) ==
            DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
        {
            DK_ObjectAdmin_us_setWriteState(
                _this->owner,
                userData,
                DK_OBJECT_STATE_OBJECT_MODIFIED);
            DK_CacheAccessAdmin_us_markObjectAsChanged(
                DK_ObjectAdmin_us_getAccess(_this->owner),
                exception,
                _this->owner);
            DLRL_Exception_PROPAGATE(exception);
        }
        collectionIndex =DMM_DLRLMultiRelation_us_getIndex(_this->metaRelation);
        DK_ObjectAdmin_us_collectionHasChanged(_this->owner, collectionIndex);
    }/* else we do nothing.... */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_us_removeElement(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* tmpHolder,
    Coll_Set* objectHolders)
{
    DK_ObjectHolder* holder = NULL;
    long errorCode = COLL_OK;
    DK_ObjectAdmin* target = NULL;
    LOC_unsigned_long collectionIndex = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(tmpHolder);
    assert(objectHolders);
    assert(_this->owner);

    holder = Coll_Set_remove(objectHolders, tmpHolder);
    if(holder)
    {
        /* TODO ID: ? performance optimalisation : Know if a holder is contained
         * in the changedElements list, so we dont  try to remove for nothing
         * try and remove element from changedElements, might remove something,
         * might not. Doesnt matter.
         */
        Coll_Set_remove(&(_this->changedElements), holder);
        /* add the holder to the removed elements list, so we can destroy it
         * during the write.
         */
        errorCode = Coll_Set_add(&(_this->deletedElements), holder);
        if(errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation. Out of resources.");
        }

        target = DK_ObjectHolder_us_getTarget(holder);
        assert(target);
#if 0
        /* if the target was marked for deletion, then it means this OH was one
         * of the invalid links marked in the  cache access. We can (well must)
         * now decrease this number by one.
         */
        if(DK_ObjectAdmin_us_getWriteState(target) ==
            DK_OBJECT_STATE_OBJECT_DELETED)
        {
            assert(_this->owner->access);
            DK_CacheAccessAdmin_us_decreaseInvalidLinks(_this->owner->access);
        }
#endif
        /* We are breaking the link to the target object, therefore we need to
         * unregister ourselves from the target as a relatedFrom holder. If we
         * do not do this, then we are incorrectly seen as an object which is
         * pointing towards the target.  */
        DK_ObjectAdmin_us_unregisterIsRelatedFrom(target, holder);
        /* if the owner of this collection was still marked as not modified,
         * we should now change it to modified.
         */
        if(DK_ObjectAdmin_us_getWriteState(_this->owner) ==
            DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
        {
            DK_ObjectAdmin_us_setWriteState(
                _this->owner,
                userData,
                DK_OBJECT_STATE_OBJECT_MODIFIED);
            DK_CacheAccessAdmin_us_markObjectAsChanged(
                DK_ObjectAdmin_us_getAccess(_this->owner),
                exception,
                _this->owner);
            DLRL_Exception_PROPAGATE(exception);
        }
        collectionIndex = DMM_DLRLMultiRelation_us_getIndex(
            _this->metaRelation);
        DK_ObjectAdmin_us_collectionHasChanged(_this->owner, collectionIndex);
    }/* else wasnt contained, do nothing. */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_Collection_us_clear(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    Coll_Set* objectHolders)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    long errorCode = COLL_OK;
    DK_ObjectAdmin* target = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long collectionIndex = 0;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectHolders);
    assert(_this->owner);

    iterator = Coll_Set_getFirstElement(&(_this->changedElements));
    while(iterator)
    {
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->changedElements), holder);
        /* changedElements is not the owner of any holder, so dont have to
         * clean it or anything... We just remove it...
         */
    }
    size = Coll_Set_getNrOfElements(objectHolders);
    iterator = Coll_Set_getFirstElement(objectHolders);
    while(iterator)
    {
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(objectHolders, holder);

        /* add the holder to the removed elements list, so we can destroy it
         * during the write. the deletedElements list becomes the owner of the
         * holder, and is responsible for cleaning it...
         */
        errorCode = Coll_Set_add(&(_this->deletedElements), holder);
        if(errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation. Out of resources.");
        }
        target = DK_ObjectHolder_us_getTarget(holder);
        assert(target);
#if 0
        /* if the target was marked for deletion, then it means this OH was one
         * of the invalid links marked in the  cache access. We can (well must)
         * now decrease this number by one.
         */
        if(DK_ObjectAdmin_us_getWriteState(target) ==
            DK_OBJECT_STATE_OBJECT_DELETED)
        {
            assert(_this->owner->access);
            DK_CacheAccessAdmin_us_decreaseInvalidLinks(_this->owner->access);
        }
#endif
        /* We are breaking the link to the target object, therefore we need to
         * unregister ourselves from the target as a relatedFrom holder. If we
         * do not do this, then we are incorrectly seen as an object which is
         * pointing towards the target.
         */
        DK_ObjectAdmin_us_unregisterIsRelatedFrom(target, holder);
    }
    if(size > 0)
    {
        /* this means actual changes have been made, so we need to inform the
         * collection owner of this event if the owner of this collection was
         * still marked as not modified, we should now change it to modified.
         */
        if(DK_ObjectAdmin_us_getWriteState(_this->owner) ==
            DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
        {
            DK_ObjectAdmin_us_setWriteState(
                _this->owner,
                userData,
                DK_OBJECT_STATE_OBJECT_MODIFIED);
            DK_CacheAccessAdmin_us_markObjectAsChanged(
                DK_ObjectAdmin_us_getAccess(_this->owner),
                exception,
                _this->owner);
            DLRL_Exception_PROPAGATE(exception);
        }
        collectionIndex = DMM_DLRLMultiRelation_us_getIndex(
            _this->metaRelation);
        DK_ObjectAdmin_us_collectionHasChanged(_this->owner, collectionIndex);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
