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
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* OS abstraction layer include */
#include "os_heap.h"
#include "os_stdlib.h"

/* collection includes */
#include "Coll_Compare.h"

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL Metamodel includes */
#include "DMM_DLRLClass.h"
#include "DMM_DLRLRelation.h"

/* DLRL kernel includes */
#include "DK_CollectionBridge.h"
#include "DK_DCPSUtility.h"
#include "DK_ObjectAdmin.h"
#include "DK_ObjectBridge.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_ObjectReaderBridge.h"
#include "DK_MapAdmin.h"
#include "DK_SetAdmin.h"
#include "DK_Utility.h"
#include "DK_UtilityBridge.h"
#include "DLRL_Kernel_private.h"

#define ENTITY_NAME "DLRL Kernel ObjectAdmin"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_ObjectAdmin_us_destroy(
    DK_Entity* _this);

static void
DK_ObjectAdmin_us_validateRelationChange(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DMM_DLRLRelation* metaRelation,
    DK_ObjectAdmin* target);

/**********************************************************************************************************************
*************************************** DLRL Kernel API calls of the ObjectAdmin **************************************
**********************************************************************************************************************/

/* assumes home is locked during call. */
DK_ObjectAdmin*
DK_ObjectAdmin_new(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    u_instanceHandle handle,
    void* keyArray,
    LOC_unsigned_long keyArraysize,
    DK_ObjectState readState,
    DK_ObjectState writeState,
    DK_CacheAccessAdmin* access,
    LOC_boolean isRegistered)
{
    DK_ObjectAdmin* _this;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(readState < DK_ObjectState_elements);
    assert(writeState < DK_ObjectState_elements);
/*TODO 416 this assertion should work again once we have a final solution for dds416
 *   assert((!u_instanceHandleIsNil(handle) && isRegistered) || (!isRegistered && u_instanceHandleIsNil(handle)));
 */
    assert(home);
    assert(isRegistered || access);/* registered may only be false if there is an access */
    /* keyArray may be NULL */
    /* access may be NULL */
    /* handle may be NULL */

    DLRL_ALLOC(_this, DK_ObjectAdmin, exception,  "%s", allocError);

    _this->alive = TRUE;
    _this->keyArray = keyArray;
    _this->keyArraySize = keyArraysize;
    _this->ls_object = NULL;
    _this->noWritersCount = 0;
    _this->disposedCount = 0;
    _this->handle = handle;
    _this->readState = readState;
    DK_ObjectAdmin_us_setWriteState(_this, userData, writeState);
    _this->isRegistered = isRegistered;
    _this->home = NULL;
    if(access)
    {
        _this->access = (DK_CacheAccessAdmin*)DK_Entity_ts_duplicate((DK_Entity*)access);
        _this->topicHasChanged = TRUE;
    } else
    {
        _this->access = NULL;
        _this->topicHasChanged = FALSE;
    }
    Coll_Set_init(&(_this->isRelatedFrom), pointerIsLessThen, FALSE);
    /* TODO: relations should be filled in when creating an object. Right now we just leave it as NIL */
    /* But this is not 'nice' we should allocate an array of object holder structs and fill them on the go.. */
    /* this isnt done now due to lack of time...  */
    _this->relations = NULL;
    _this->relationsSize = 0;
    _this->collections = NULL;
    _this->collectionsSize = 0;
    /* booleans which indicate which collections have changed. Their indexes in the array coincide with the collections */
    /* on meta level. */
    _this->changedCollections = NULL;


    DK_Entity_us_init(&(_this->entity), DK_CLASS_OBJECT_ADMIN, DK_ObjectAdmin_us_destroy);
    _this->home = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)home);
    _this->collectionsSize = Coll_List_getNrOfElements(DMM_DLRLClass_getMultiRelations(DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home)));
    /* allocate an array of collection pointers */
    if( _this->collectionsSize > 0)
    {
        DLRL_ALLOC_WITH_SIZE(_this->collections, ((sizeof(DK_Collection*))*(_this->collectionsSize)), exception,
                                        "Unable to allocate array container for collections of %s!", ENTITY_NAME);
        memset(_this->collections, 0, ((sizeof(DK_Collection*))*(_this->collectionsSize)));

        DLRL_ALLOC_WITH_SIZE(_this->changedCollections, ((sizeof(LOC_boolean))*(_this->collectionsSize)), exception,
                                "Unable to allocate array container for collections modify infoof %s!", ENTITY_NAME);
        memset(_this->changedCollections, 0, ((sizeof(LOC_boolean))*(_this->collectionsSize)));
    }/* else let it remain null */
    _this->relationsSize = Coll_List_getNrOfElements(DMM_DLRLClass_getRelations(
                                                            DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home)));
    if( _this->relationsSize > 0)
    {
        DLRL_ALLOC_WITH_SIZE(_this->relations, ((sizeof(DK_ObjectHolder*))*(_this->relationsSize)), exception,
                                        "Unable to allocate array container for relations of %s!", ENTITY_NAME);
        memset(_this->relations, 0, ((sizeof(DK_ObjectHolder*))*(_this->relationsSize)));
    } /* else let it remain null */

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_ObjectAdmin_us_delete(_this, NULL);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

DK_ObjectState
DK_ObjectAdmin_ts_getReadState(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    DK_ObjectState readState = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    readState = DK_ObjectAdmin_us_getReadState(_this);

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return readState;
}

DK_ObjectState
DK_ObjectAdmin_ts_getWriteState(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    DK_ObjectState writeState = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    writeState = _this->writeState;

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return writeState;
}

void
DK_ObjectAdmin_ts_getObjectID(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectID* oid)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(oid);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    DK_ObjectAdmin_us_getObjectID(_this, oid);

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_getObjectID(
    DK_ObjectAdmin* _this,
    DK_ObjectID* oid)
{
    DMM_Mapping mapping;
    LOC_unsigned_long index = 0;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(oid);
    assert(_this->home);
    assert(DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home));
    mapping = DMM_DLRLClass_getMapping(DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home));
    if(mapping == DMM_MAPPING_PREDEFINED)
    {
        if(!u_instanceHandleIsNil(_this->handle))
        {
            oid->oid[0] = u_instanceHandleServerId(_this->handle);
            oid->oid[1] = u_instanceHandleIndex(_this->handle);
            oid->oid[2] = u_instanceHandleSerial(_this->handle);
        } else
        {
            oid->oid[0] = 0;
            oid->oid[1] = 0;
            oid->oid[2] = 0;
        }
    } else
    {
        assert(mapping == DMM_MAPPING_DEFAULT);
        if(_this->keyArraySize == 0)
        {
            oid->oid[0] = 0;
            oid->oid[1] = 0;
            oid->oid[2] = 0;
        } else
        {
            assert(_this->keyArray);
            assert(_this->keyArraySize == 3 || _this->keyArraySize == 4);
            if(_this->keyArraySize == 4)
            {
                index++;/* skip the name field*/
            }
            oid->oid[0] = DK_DCPSUtility_us_getLongValueFromArray(_this->keyArray, index);
            index++;
            oid->oid[1] = DK_DCPSUtility_us_getLongValueFromArray(_this->keyArray, index);
            index++;
            oid->oid[2] = DK_DCPSUtility_us_getLongValueFromArray(_this->keyArray, index);
        }
    }
    DLRL_INFO(INF_EXIT);
}

LOC_long
DK_ObjectAdmin_ts_getHomeIndex(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_long index = -1;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    assert(DK_ObjectHomeAdmin_us_isAlive(_this->home));

    index = DK_ObjectHomeAdmin_us_getRegistrationIndex(_this->home);

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return index;

}

/* caller must release the objecthome pointer, may return null if the primary state is initial */
DK_ObjectHomeAdmin*
DK_ObjectAdmin_ts_getHome(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    DK_ObjectHomeAdmin* home = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    home = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)(DK_ObjectAdmin_us_getHome(_this)));

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return home;
}


LOC_boolean
DK_ObjectAdmin_ts_isModified(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectScope scope)
{
    LOC_boolean isModified = FALSE;
    DK_ObjectHolder* aHolder = NULL;
    DK_ObjectAdmin* relatedAdmin = NULL;
    DK_ObjectState relatedObjectReadState = DK_ObjectState_elements;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(scope < DK_ObjectScope_elements);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->readState != DK_OBJECT_STATE_OBJECT_NOT_MODIFIED && _this->readState != DK_OBJECT_STATE_OBJECT_VOID)
    {
        /* aka state is new, deleted or modified */
        isModified = TRUE;
    } else if(scope == DK_OBJECT_SCOPE_RELATED_OBJECTS_SCOPE && _this->relations)
    {/* only if we have relations... */
        for(count =0; count < (_this->relationsSize) && !isModified; count++)
        {
            aHolder = _this->relations[count];/* holder may be nil... */
            if(aHolder)
            {
                relatedAdmin = DK_ObjectHolder_us_getTarget(aHolder);
            }
            if(relatedAdmin)
            {
                relatedObjectReadState = relatedAdmin->readState;/* this is threadsafe, just reading a value! */
                assert(relatedObjectReadState < DK_ObjectState_elements);
                /* if a related objectadmin is no longer alive, then we no longer consider it for modification determination */
                /* else if this related, but deleted object is never reset it will result in this owning object admin to */
                /* be always viewed as modified, which cant be whats meant. Note that objects only become deleted once */
                /* an update round has ended! */
                if(relatedObjectReadState != DK_OBJECT_STATE_OBJECT_NOT_MODIFIED &&
                                                                    _this->readState != DK_OBJECT_STATE_OBJECT_VOID)
                {
                    assert(relatedAdmin->alive);
                    isModified = TRUE;
                }
            }/* else do nothing */
        }
    } else if(scope == DK_OBJECT_SCOPE_CONTAINED_OBJECTS_SCOPE && _this->relations)
    {
        /* is the related objects scope also taking contained relations into account?! */
        /* TODO ID: 124 take locking strategy into note when taking other related objects in account, see solution above */
    }
    #ifndef NDEBUG
    else if(_this->relations)
    {
        assert(scope == DK_OBJECT_SCOPE_SIMPLE_OBJECT_SCOPE);
    } else
    {
        assert(!_this->relations);
    }
    #endif
    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return isModified;
}

LOC_unsigned_long
DK_ObjectAdmin_us_getCollectionsSize(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->collectionsSize;
}

LOC_long
DK_ObjectAdmin_us_getNoWritersCount(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->noWritersCount;
}

void
DK_ObjectAdmin_us_setNoWritersCount(
    DK_ObjectAdmin* _this,
    LOC_long noWritersCount)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->noWritersCount = noWritersCount;

    DLRL_INFO(INF_EXIT);
}

LOC_long
DK_ObjectAdmin_us_getDisposedCount(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->disposedCount;
}

void
DK_ObjectAdmin_us_setDisposedCount(
    DK_ObjectAdmin* _this,
    LOC_long disposedCount)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->disposedCount = disposedCount;

    DLRL_INFO(INF_EXIT);
}

DK_Collection**
DK_ObjectAdmin_us_getCollections(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(_this);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->collections;
}

LOC_boolean*
DK_ObjectAdmin_us_getChangedCollections(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->changedCollections;
}

void
DK_ObjectAdmin_us_relationHasChanged(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long relationIndex)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* TODO relate relation change to the topic the relation belongs too and marked only that topic */
    /* as changed... */
    _this->topicHasChanged = TRUE;
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_resetChangedFlags(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->topicHasChanged = FALSE;
    memset(_this->changedCollections, 0, ((sizeof(LOC_boolean))*(_this->collectionsSize)));

    DLRL_INFO(INF_EXIT);
}

DK_CacheBase*
DK_ObjectAdmin_ts_getOwner(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    DK_CacheBase* cacheBase = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->access)
    {
        cacheBase = (DK_CacheBase*)DK_Entity_ts_duplicate((DK_Entity*)_this->access);
    } else
    {
        cacheBase = (DK_CacheBase*)DK_Entity_ts_duplicate((DK_Entity*)DK_ObjectHomeAdmin_us_getCache(_this->home));
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return cacheBase;
}

DLRL_LS_object
DK_ObjectAdmin_ts_getLSObject(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->ls_object)
    {
        retVal = utilityBridge.localDuplicateLSValuetypeObject(userData, _this->ls_object);
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_ObjectAdmin_ts_getLSHome(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;
    DLRL_LS_object lsHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    lsHome = DK_ObjectHomeAdmin_us_getLSHome(_this->home);
    if(lsHome)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(userData, lsHome);
        if(!retVal)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_ObjectAdmin_ts_destroy(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
#if 0
    LOC_unsigned_long i;
    DK_ObjectHolder* holder;
    DK_ObjectAdmin* relatedObject;
#endif

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->access)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to destroy object. The %s %p is not located in a Cache Access.",
            ENTITY_NAME,
            _this);
    }
    DK_CacheAccessAdmin_lock(_this->access);
    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_ALREADY_DELETED,
            "Unable to destroy object. The %s '%p' entity has already been deleted!",
            ENTITY_NAME,
            _this);
    }
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this->access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to destroy object. The %s %p is located in a READ ONLY "
            "Cache Access instead of a required WRITE ONLY or READ WRITE Cache Access.",
            ENTITY_NAME,
            _this);
    }
    DK_CacheAccessAdmin_us_checkAlive(_this->access, exception);
    DLRL_Exception_PROPAGATE(exception);

    /* only if the object was not already marked for deletion */
    if(_this->writeState != DK_OBJECT_STATE_OBJECT_DELETED)
    {
        if(_this->isRegistered)
        {
            if(_this->writeState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
            {
               DK_CacheAccessAdmin_us_markObjectAsChanged(
                   _this->access,
                   exception,
                   _this);
               DLRL_Exception_PROPAGATE(exception);
            }
            _this->topicHasChanged = TRUE;
#if 0
            /* everyone that is still pointing to this object now has an invalid
             * link towards this object so we mark that in the cache access.
             */
            DK_CacheAccessAdmin_us_increaseInvalidLinksWithAmount(
                _this->access,
                Coll_Set_getNrOfElements(&(_this->isRelatedFrom)));
            /* now if this object is pointing to objects that were also
             * deleted, we must update the invalid link amount accordingly.
             * because if both objects will be deleted during the next write
             * we do not have to worry about any invalid links at all!
             */
            for(i = 0; i < _this->relationsSize; i++)
            {
                holder = _this->relations[i];
                if(holder)
                {
                    relatedObject = DK_ObjectHolder_us_getTarget(holder);
                    if(relatedObject &&
                        relatedObject->writeState == DK_OBJECT_STATE_OBJECT_DELETED)
                    {
                        DK_CacheAccessAdmin_us_decreaseInvalidLinks(_this->access);
                    }
                }
            }
            /* now the same goes for the collections */
/* code for this part was not yet written, but should be done though if this code
 * is enabled ever ...
            for(i = 0; i < _this->relationsSize; i++)
            {
                holder = _this->relations[i];
                if(holder)
                {
                    relatedObject = DK_ObjectHolder_us_getTarget(holder);
                    if(relatedObject &&
                        relatedObject->writeState == DK_OBJECT_STATE_OBJECT_DELETED)
                    {
                        DK_CacheAccessAdmin_us_decreaseInvalidLinks(_this->access);
                    }
                }
            }
*/
#endif
        } else
        {
            DK_CacheAccessAdmin_us_removeUnregisteredObject(
                _this->access,
                exception,
                userData,
                _this);
            DLRL_Exception_PROPAGATE(exception);
            DK_ObjectAdmin_us_delete(_this, userData);
        }
        DK_ObjectAdmin_us_setWriteState(
            _this,
            userData,
            DK_OBJECT_STATE_OBJECT_DELETED);
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    if(_this->access)
    {
        DK_CacheAccessAdmin_unlock(_this->access);
    }
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
void
DK_ObjectAdmin_lockForRelationChange(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    LOC_unsigned_long index,
    DK_ObjectAdmin* newTargetObjectAdmin)
{
    DK_ObjectHomeAdmin* relatedHome = NULL;
    DK_ObjectHomeAdmin* aHome;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* newTargetObjectAdmin may be NULL */

    if(!_this->access)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change relation. "
                  "The %s %p is not located in a Cache Access.", ENTITY_NAME, _this);
    }
    DK_CacheAccessAdmin_lock(_this->access);
    DK_CacheAccessAdmin_us_checkAlive(_this->access, exception);
    DLRL_Exception_PROPAGATE(exception);

    /* if the cacheAccess is alive, then the home (and relatedHome) is alive! */
    assert(_this->home->alive);
    relatedHome = DK_ObjectHomeAdmin_us_getRelatedHome(_this->home, index);
    assert(relatedHome->alive);
    DK_Utility_lockAdminForTwoHomesInSequence(_this->home, relatedHome);
    if(newTargetObjectAdmin && (newTargetObjectAdmin->home != relatedHome) &&
                             (newTargetObjectAdmin->home != _this->home))
    {
        assert(DK_ObjectHomeAdmin_us_getRegistrationIndex(relatedHome) >
                                            DK_ObjectHomeAdmin_us_getRegistrationIndex(newTargetObjectAdmin->home));
#ifndef NDEBUG
        /* ensure that the home of the relatedObjectAdmin is in fact a child of the base home of this relation! */
        aHome = newTargetObjectAdmin->home;
        while(aHome != relatedHome)
        {
            aHome = DK_ObjectHomeAdmin_us_getParent(aHome);
        }
        assert(aHome && aHome == relatedHome);
#endif
        DK_ObjectAdmin_lockHome(newTargetObjectAdmin);
    }
    DLRL_Exception_EXIT(exception);
    /* no rollback, the unlock takes care of all situations (IE only cache access locked and not the homes locked) */
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
void
DK_ObjectAdmin_unlockForRelationChange(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long index,
    DK_ObjectAdmin* newTargetObjectAdmin)
{
    DK_ObjectHomeAdmin* relatedHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* newTargetObjectAdmin may be NULL */

    if(_this->access)
    {
        if(DK_CacheAccessAdmin_us_isAlive(_this->access))
        {
            assert(_this->home->alive);
            relatedHome = DK_ObjectHomeAdmin_us_getRelatedHome(_this->home, index);
            assert(relatedHome->alive);
            DK_Utility_unlockAdminForTwoHomesInSequence(_this->home, relatedHome);
            if(newTargetObjectAdmin && (newTargetObjectAdmin->home != relatedHome) &&
                                     (newTargetObjectAdmin->home != _this->home))
            {
                assert(DK_ObjectHomeAdmin_us_getRegistrationIndex(relatedHome) >
                                                DK_ObjectHomeAdmin_us_getRegistrationIndex(newTargetObjectAdmin->home));
                DK_ObjectAdmin_unlockHome(newTargetObjectAdmin);
            }
        }
        DK_CacheAccessAdmin_unlock(_this->access);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_ts_changeRelation(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long index,
    DK_ObjectAdmin* relatedObjectAdmin)
{
    DK_ObjectHolder* holder = NULL;
    DK_ObjectAdmin* target = NULL;
    DMM_DLRLRelation* relation = NULL;
    LOC_boolean relationIsOptional;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(_this->relations);/* this operation should only be called for objects that have relations... */
    /* userData, relatedObjectAdmin may be NULL */

    /* ensure cache is alive and all (2 or 3) homes */
    DK_ObjectAdmin_lockForRelationChange(_this, exception, index, relatedObjectAdmin);
    DLRL_Exception_PROPAGATE(exception);

    assert(_this->home->meta_representative);
    relation = DMM_DLRLClass_getSingleRelation(_this->home->meta_representative, index);
    assert(relation);
    relationIsOptional = DMM_DLRLRelation_isOptional(relation);

    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "Unable to change relation. The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(!_this->isRegistered)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change relation. The 'owning' "
             "%s '%p' entity is still unregistered. Relations can only be made between objects which are "
             "registered and thus have an identity.");
    }
    if(relatedObjectAdmin)
    {
        if(!relatedObjectAdmin->alive)
        {
            DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
                "Unable to change relation. The intended target %s '%p' entity has already been deleted and it is not "
                "allowed to change a relation to an AlreadyDeleted object.", ENTITY_NAME, _this);
        }
        if(!relatedObjectAdmin->access)
        {
            DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
                "Unable to change relation. The intended target %s '%p' entity is not created in the scope of a"
                " CacheAccess. You can only relate two objects that both belong to a (same) CacheAccess.", ENTITY_NAME,
                _this);
        }
        if(_this->access != relatedObjectAdmin->access)
        {
            DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
                "Unable to change relation. The intended target %s '%p' entity is belongs to a different CacheAccess."
                "You can only relate two objects that belong to the same CacheAccess!", ENTITY_NAME, _this);
        }
        if(!DK_ObjectAdmin_us_getIsRegistered(relatedObjectAdmin))
        {
            DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change relation. The intended target "
                 "%s '%p' entity is still unregistered. Relations can only be made between objects which are "
                 "registered and thus have an identity.");
        }
        DK_ObjectAdmin_us_validateRelationChange(_this, exception, relation, relatedObjectAdmin);
        DLRL_Exception_PROPAGATE(exception);
    } else if(!relationIsOptional)
    {
        /* If a relation is mandatory, we may not set it to NULL */
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change relation. This relation is "
                    "modeled as a mandatory relationship and may therefore not be set to NIL.");

    }
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this->access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change relation. The %s %p is located "
                    "in a READ ONLY Cache Access instead of a required WRITE ONLY or READ WRITE Cache Access.",
                    ENTITY_NAME, _this);
    }

    holder = _this->relations[index];
    if(!holder)
    {
        holder = DK_ObjectHolder_newResolved(exception,(DK_Entity*)_this, NULL, DK_DCPSUtility_ts_getNilHandle(),index);
        DLRL_Exception_PROPAGATE(exception);
        DK_ObjectAdmin_us_addSingleRelation(_this, holder, index);
    } else
    {
        target = DK_ObjectHolder_us_getTarget(holder);
        if(target && target != relatedObjectAdmin)
        {
            assert(target->alive);/* cant have relations to already deleted objects */
#if 0
            /* if the target is marked for destruction, then we can decrease the invalid links, as we are changing the
             * target
             */
            if(DK_ObjectAdmin_us_getWriteState(target) == DK_OBJECT_STATE_OBJECT_DELETED)
            {
                DK_CacheAccessAdmin_us_decreaseInvalidLinks(_this->access);
            }
#endif
            DK_ObjectAdmin_us_unregisterIsRelatedFrom(target, holder);
        }
    }
    /* NOTE: it doesnt matter if the holder is resolved or not, no (special) management is done for unresolved holders */
    /* that exist within a CacheAccess, unlike holders that exist in a main cache. */

#if 0
    /* if the old target was null , and the relation was mandatory and the new target is not null, then we can  */
    /* decrease the invalid links because we are setting it to a valid relatedObjectAdmin (ie a not nil one) */
    if(!target && !relationIsOptional && relatedObjectAdmin)
    {
        DK_CacheAccessAdmin_us_decreaseInvalidLinks(_this->access);
    }
#endif
    /* a relation is only changed, if there was an actual change... */
    if(target != relatedObjectAdmin)
    {
        DK_ObjectHolder_us_setTarget(holder, relatedObjectAdmin);
        if(relatedObjectAdmin)
        {
            DK_ObjectAdmin_us_registerIsRelatedFrom(relatedObjectAdmin, exception, holder);
            DLRL_Exception_PROPAGATE(exception);
        }
     /*   if(DMM_DLRLClass_getMapping(DMM_DLRLRelation_getType(relation)) == DMM_MAPPING_DEFAULT){*/
            /* note that it is impossible to encounter shared keys in this flow, obviously */
      /*      if(Coll_List_getNrOfElements(DMM_DLRLRelation_getTargetKeys(relation)) == 4){
                targetTopicName = DMM_DCPSTopic_getTopicTypeName(DMM_DLRLRelation_getTargetTopic(relation));
            }
            objectBridge.changeRelationFieldsInTopic(userData, exception, _this->home,
                                _this->ls_object, relatedObjectAdmin->handle.server,
                                relatedObjectAdmin->handle.index, relatedObjectAdmin->handle.serial, targetTopicName);
        }*/
        /* A transition to the modified state only happens when the object is not modified. If its new or deleted then  */
        /* those states are seen as being 'stronger'. */
        if(_this->writeState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
        {
           DK_ObjectAdmin_us_setWriteState(_this, userData, DK_OBJECT_STATE_OBJECT_MODIFIED);
           DK_CacheAccessAdmin_us_markObjectAsChanged(_this->access, exception, _this);
           DLRL_Exception_PROPAGATE(exception);
        }
        _this->topicHasChanged = TRUE;
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockForRelationChange(_this, index, relatedObjectAdmin);

    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_validateRelationChange(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DMM_DLRLRelation* metaRelation,
    DK_ObjectAdmin* target)
{
    DMM_DLRLClass* typeClass = NULL;
    void* ownerKeyArray = NULL;
    void* targetKeyArray = NULL;
    Coll_List* metaOwnerKeys = NULL;
    Coll_List* metaTargetKeys = NULL;
    Coll_Iter* iterator = NULL;
    Coll_Iter* targetKeysIterator = NULL;
    DMM_DCPSField* anOwnerField = NULL;
    DMM_DCPSField* aTargetField = NULL;
    LOC_long targetFieldIndex = 0;
    LOC_long ownerFieldIndex = 0;
    c_value* ownerKey = NULL;
    c_value* targetKey = NULL;
    DMM_KeyType keyType;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(metaRelation);
    assert(target);

    typeClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home);
    if(DMM_DLRLRelation_hasSharedKeys(metaRelation))
    {
        ownerKeyArray = DK_ObjectAdmin_us_getKeyValueArray(_this);
        targetKeyArray = DK_ObjectAdmin_us_getKeyValueArray(target);
        metaOwnerKeys = DMM_DLRLRelation_getOwnerKeys(metaRelation);
        metaTargetKeys = DMM_DLRLRelation_getTargetKeys(metaRelation);
        iterator = Coll_List_getFirstElement(metaOwnerKeys);
        targetKeysIterator = Coll_List_getFirstElement(metaTargetKeys);
        while(iterator)
        {
            anOwnerField = Coll_Iter_getObject(iterator);
            keyType = DMM_DCPSField_getFieldType(anOwnerField);
            if(keyType == DMM_KEYTYPE_SHARED_KEY)
            {
                aTargetField = (DMM_DCPSField*)Coll_Iter_getObject(targetKeysIterator);
                targetFieldIndex = DMM_DCPSField_getUserDefinedIndex(aTargetField);
                ownerFieldIndex = DMM_DCPSField_getUserDefinedIndex(anOwnerField);
                ownerKey = &(((c_value*)ownerKeyArray)[ownerFieldIndex]);
                targetKey = &(((c_value*)targetKeyArray)[targetFieldIndex]);
                if(!DK_DCPSUtility_us_areValuesEqual(ownerKey, targetKey))
                {
                    DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change relation. "
                            "The keys of the target object do not match the shared keys of the owning object.");
                }
            }
            iterator = Coll_Iter_getNext(iterator);
            targetKeysIterator = Coll_Iter_getNext(targetKeysIterator);
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_ts_changeCollection(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long index,
    DK_Collection* targetCollection)
{
    DK_ObjectHomeAdmin* sourceHome = NULL;
    DK_ObjectHomeAdmin* targetHome= NULL;
    DMM_DLRLMultiRelation* metaCollection = NULL;
    DMM_Basis base;
    Coll_Set* objectHolders = NULL;
    DK_Collection* collection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetCollection);

    if(!_this->access)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The %s %p is not located "
                    "in a Cache Access.", ENTITY_NAME, _this);
    }
    DK_CacheAccessAdmin_lock(_this->access);
    sourceHome = targetCollection->ownerHome;
    targetHome = targetCollection->targetHome;
    DK_Utility_us_lockThreeHomesInSequence(sourceHome, targetHome, _this->home);

    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "Unable to change relation. The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this->access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The %s %p is located "
                    "in a READ ONLY Cache Access instead of a required WRITE ONLY or READ WRITE Cache Access.",
                    ENTITY_NAME, _this);
    }
    if(!_this->isRegistered)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The %s %p has not yet "
                    "been registered to the CacheAccess.", ENTITY_NAME, _this);
    }
    assert(targetCollection->owner);
    if(!targetCollection->owner->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The source collection "
                    "belongs to an AlreadyDeleted ObjectRoot.");
    }
    if(targetCollection->owner->access != _this->access)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The source collection "
                    "is located in a different Cache Access then the destination collection.");
    }

    if(!targetCollection->owner->isRegistered)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The source collection "
                    "belongs to an ObjectRoot which is not yet registered to the CacheAccess.");
    }
    DK_CacheAccessAdmin_us_checkAlive(_this->access, exception);
    DLRL_Exception_PROPAGATE(exception);

    assert(_this->collections[index]);
    collection = _this->collections[index];
    metaCollection = DK_Collection_us_getMetaRepresentative(collection);
    base = DMM_DLRLMultiRelation_getBasis(metaCollection);
    if(base == DMM_BASIS_SET)
    {
        objectHolders = DK_SetAdmin_us_getHolders((DK_SetAdmin*)collection);
        DK_Collection_us_clear(collection, exception, userData, objectHolders);
        DLRL_Exception_PROPAGATE(exception);
        DK_SetAdmin_us_copyElementsFromCollection((DK_SetAdmin*)collection, exception, userData, (DK_SetAdmin*)targetCollection);
    } else
    {
        assert(base == DMM_BASIS_STR_MAP || base == DMM_BASIS_INT_MAP);
        objectHolders = DK_MapAdmin_us_getObjectHolders((DK_MapAdmin*)collection);
        DK_Collection_us_clear(collection, exception, userData, objectHolders);
        DLRL_Exception_PROPAGATE(exception);
        DK_MapAdmin_us_copyElementsFromCollection((DK_MapAdmin*)collection, exception, userData, (DK_MapAdmin*)targetCollection);
    }

    if(_this->writeState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
    {
       DK_ObjectAdmin_us_setWriteState(_this, userData, DK_OBJECT_STATE_OBJECT_MODIFIED);
       DK_CacheAccessAdmin_us_markObjectAsChanged(_this->access, exception, _this);
       DLRL_Exception_PROPAGATE(exception);
    }
    _this->changedCollections[index] = TRUE;

    DLRL_Exception_EXIT(exception);
    if(_this->access)
    {
        DK_Utility_us_unlockThreeHomesInSequence(sourceHome, targetHome, _this->home);
        DK_CacheAccessAdmin_unlock(_this->access);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_ts_stateHasChanged(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_boolean isImmutable)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->access)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The %s %p is not located "
                    "in a Cache Access.", ENTITY_NAME, _this);
    }
    DK_CacheAccessAdmin_lock(_this->access);
    DK_ObjectAdmin_lockHome(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "Unable to change object. The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this->access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The %s %p is located "
                    "in a READ ONLY Cache Access instead of a required WRITE ONLY or READ WRITE Cache Access.",
                    ENTITY_NAME, _this);
    }
    DK_CacheAccessAdmin_us_checkAlive(_this->access, exception);
    DLRL_Exception_PROPAGATE(exception);
    if(isImmutable && _this->isRegistered)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to change object. The %s %p is already "
                    "registered and the field to be changed represents an immutable field.", ENTITY_NAME, _this);
    }

    /* A transition to the modified state only happens when the object is not modified. If its new or deleted then those */
    /* states are seen as being 'stronger'. */
    if(_this->writeState == DK_OBJECT_STATE_OBJECT_NOT_MODIFIED)
    {
       DK_ObjectAdmin_us_setWriteState(_this, userData, DK_OBJECT_STATE_OBJECT_MODIFIED);
       DK_CacheAccessAdmin_us_markObjectAsChanged(_this->access, exception, _this);
       DLRL_Exception_PROPAGATE(exception);
    }
    _this->topicHasChanged = TRUE;

    DLRL_Exception_EXIT(exception);

    if(_this->access)
    {
        DK_ObjectAdmin_unlockHome(_this);
        DK_CacheAccessAdmin_unlock(_this->access);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_string
DK_ObjectAdmin_ts_getMainTopicName(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    LOC_string retVal = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_ObjectAdmin_lockHome(_this);
    DK_ObjectAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_STRDUP(
        retVal,
        DK_ObjectAdmin_us_getMainTopicName(_this),
        exception,
        "Unable to allocate memory");

    DLRL_Exception_EXIT(exception);
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}
void
DK_ObjectAdmin_us_checkRelationIsFound(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    LOC_unsigned_long index)
{
    DK_ObjectHolder* holder;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(_this->relations);

    holder = _this->relations[index];
    if(holder && !DK_ObjectHolder_us_isResolved(holder))
    {
        DLRL_Exception_THROW(exception, DLRL_NOT_FOUND, "This relationship "
            "cannot be resolved yet by the DLRL.");
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}


/* following operation should be phased out when java api no longer needs it */
LOC_string
DK_ObjectAdmin_us_getMainTopicName(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);
    DLRL_INFO_OBJECT(INF_EXIT);
    return DMM_DCPSTopic_getTopicTypeName(
                DMM_DLRLClass_getMainTopic(
                    DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home)));
}

DK_ObjectState
DK_ObjectAdmin_us_getWriteState(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->writeState;
}

void
DK_ObjectAdmin_us_collectionHasChanged(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long collectionIndex)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->changedCollections[collectionIndex] = TRUE;

    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_ObjectAdmin_us_hasTopicChanged(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->topicHasChanged;
}

void
DK_ObjectAdmin_ts_getInvalidRelations(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_List* invalidRelations = NULL;
    LOC_string name;
    LOC_unsigned_long count = 0; /* must init to 0*/
    LOC_unsigned_long size = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectAdmin_lockHome(_this);
    DK_ObjectAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    invalidRelations = DK_ObjectAdmin_us_getInvalidRelations(
        _this,
        exception);
    DLRL_Exception_PROPAGATE(exception);

    utilityBridge.createStringSeq(
        exception,
        userData,
        arg,
        Coll_List_getNrOfElements(invalidRelations));
    DLRL_Exception_PROPAGATE(exception);

    size = Coll_List_getNrOfElements(invalidRelations);
    while(count < size)
    {
        name = (LOC_string)Coll_List_popBack(invalidRelations);
        utilityBridge.addElementToStringSeq(
            exception,
            userData,
            *arg,
            count,
            (void*)name);
        DLRL_Exception_PROPAGATE(exception);
        count++;
    }
    Coll_List_delete(invalidRelations);
    invalidRelations = NULL;

    DLRL_Exception_EXIT(exception);
    if(invalidRelations)
    {
        while(Coll_List_getNrOfElements(invalidRelations) > 0)
        {
            Coll_List_popBack(invalidRelations);
        }
        Coll_List_delete(invalidRelations);
        invalidRelations = NULL;
    }
    DK_ObjectAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
}



/* The caller of this operation must delete the returned list (and thus pop back all elements).
 * But the elements itself do not have to be freed, as they are direct references to
 * strings maintained by the kernel.
 */
Coll_List*
DK_ObjectAdmin_us_getInvalidRelations(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    Coll_List* invalidRelations = NULL;
    long errorCode = COLL_OK;
    DMM_DLRLClass* typeClass = NULL;
    Coll_List* metaRelations = NULL;
    Coll_Iter* metaRelationsIterator = NULL;
    DMM_DLRLRelation* metaRelation = NULL;
    DK_ObjectHolder* holder = NULL;
    LOC_boolean hasInvalidRelations = FALSE;
    DMM_DLRLMultiRelation* metaCollection = NULL;
    DMM_Basis base;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    invalidRelations = Coll_List_new();
    if(!invalidRelations)
    {
         DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to retrieve the invalid relation names of %s '%p'. "
            "Couldn't allocate a list object to hold the relation names.",
            ENTITY_NAME, _this);
    }
    typeClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home);
    metaRelations = DMM_DLRLClass_getRelations(typeClass);
    metaRelationsIterator = Coll_List_getFirstElement(metaRelations);
    while(metaRelationsIterator)
    {
        assert(_this->relations);
        metaRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(metaRelationsIterator);
        holder = (DK_ObjectHolder*)_this->relations[count];
        if(DK_Utility_us_isRelationInvalid(holder, DMM_DLRLRelation_isOptional(metaRelation), _this->writeState))
        {
            errorCode = Coll_List_pushBack(invalidRelations, DMM_DLRLRelation_getName(metaRelation));
            if (errorCode != COLL_OK)
            {
                DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                    "Unable to add the relation name to list of relation names");
            }
        }
        metaRelationsIterator = Coll_Iter_getNext(metaRelationsIterator);
        count++;
    }
    if(_this->writeState != DK_OBJECT_STATE_OBJECT_DELETED)
    {
        metaRelations = DMM_DLRLClass_getMultiRelations(typeClass);
        metaRelationsIterator = Coll_List_getFirstElement(metaRelations);
        count = 0;
        while(metaRelationsIterator)
        {
            metaCollection = (DMM_DLRLMultiRelation*)Coll_Iter_getObject(metaRelationsIterator);
            hasInvalidRelations = FALSE;
            base = DMM_DLRLMultiRelation_getBasis(metaCollection);
            if(base == DMM_BASIS_SET)
            {
                hasInvalidRelations = DK_SetAdmin_us_hasInvalidElements((DK_SetAdmin*)_this->collections[count]);
            } else
            {
                assert((base == DMM_BASIS_STR_MAP) || (base == DMM_BASIS_INT_MAP));
                hasInvalidRelations = DK_MapAdmin_us_hasInvalidElements((DK_MapAdmin*)_this->collections[count]);
            }
            if(hasInvalidRelations)
            {
                errorCode = Coll_List_pushBack(invalidRelations, DMM_DLRLRelation_getName((DMM_DLRLRelation*)metaCollection));
                if (errorCode != COLL_OK)
                {
                    DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                        "Unable to add the relation name to list of relation names");
                }
            }
            metaRelationsIterator = Coll_Iter_getNext(metaRelationsIterator);
            count++;
        }
    }

    DLRL_Exception_EXIT(exception);
    /* clean up in case of an exception */
    if(exception->exceptionID != DLRL_NO_EXCEPTION && invalidRelations)
    {
        while(Coll_List_getNrOfElements(invalidRelations) > 0)
        {
            Coll_List_popBack(invalidRelations);
        }
        Coll_List_delete(invalidRelations);
        invalidRelations = NULL;

    }
    DLRL_INFO(INF_EXIT);
    return invalidRelations;
}

LOC_boolean
DK_ObjectAdmin_us_hasInvalidRelations(
    DK_ObjectAdmin* _this)
{
    DMM_DLRLClass* typeClass = NULL;
    Coll_List* metaRelations = NULL;
    Coll_Iter* metaRelationsIterator = NULL;
    DMM_DLRLRelation* metaRelation = NULL;
    DK_ObjectHolder* holder = NULL;
    LOC_boolean hasInvalidRelations = FALSE;
    DMM_DLRLMultiRelation* metaCollection = NULL;
    DMM_Basis base;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    typeClass = DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home);
    /* check 1-1 relations */
    metaRelations = DMM_DLRLClass_getRelations(typeClass);
    metaRelationsIterator = Coll_List_getFirstElement(metaRelations);
    while(metaRelationsIterator && !hasInvalidRelations)
    {
        metaRelation = (DMM_DLRLRelation*)Coll_Iter_getObject(metaRelationsIterator);
        assert(_this->relations);
        holder = (DK_ObjectHolder*)_this->relations[count];
        hasInvalidRelations =  DK_Utility_us_isRelationInvalid(holder, DMM_DLRLRelation_isOptional(metaRelation), _this->writeState);
        metaRelationsIterator = Coll_Iter_getNext(metaRelationsIterator);
        count++;
    }
    if(!hasInvalidRelations && _this->writeState != DK_OBJECT_STATE_OBJECT_DELETED)
    {
        /* check collections */
        count = 0;
        metaRelations = DMM_DLRLClass_getMultiRelations(typeClass);
        metaRelationsIterator = Coll_List_getFirstElement(metaRelations);
        while(metaRelationsIterator && !hasInvalidRelations)
        {
            metaCollection = (DMM_DLRLMultiRelation*)Coll_Iter_getObject(metaRelationsIterator);
            base = DMM_DLRLMultiRelation_getBasis(metaCollection);
            if(base == DMM_BASIS_SET)
            {
                hasInvalidRelations = DK_SetAdmin_us_hasInvalidElements((DK_SetAdmin*)_this->collections[count]);
            } else
            {
                assert((base == DMM_BASIS_STR_MAP) || (base == DMM_BASIS_INT_MAP));
                hasInvalidRelations = DK_MapAdmin_us_hasInvalidElements((DK_MapAdmin*)_this->collections[count]);
            }
            metaRelationsIterator = Coll_Iter_getNext(metaRelationsIterator);
            count++;
        }
    }
    DLRL_INFO(INF_EXIT);
    return hasInvalidRelations;
}

/**********************************************************************************************************************
************************* Kernel Thread unsafe calls of the ObjectAdmin (Internal kernel API) *************************
**********************************************************************************************************************/
/* no duplicate done */
DK_ObjectHomeAdmin*
DK_ObjectAdmin_us_getHome(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->home;
}

DK_CacheAccessAdmin*
DK_ObjectAdmin_us_getAccess(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->access;
}

void*
DK_ObjectAdmin_us_getKeyValueArray(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->keyArray;
}

void
DK_ObjectAdmin_us_registerIsRelatedFrom(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder)
{
    LOC_long returnCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    returnCode = Coll_Set_addUniqueObject(&(_this->isRelatedFrom), holder);
    if(returnCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add ObjectHolder %p to 'isRelatedFrom' set of "
                            "%s %p. Out of resources", holder, ENTITY_NAME, _this);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);

}

void
DK_ObjectAdmin_us_unregisterIsRelatedFrom(
    DK_ObjectAdmin* _this,
    DK_ObjectHolder* holder)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(holder);

    Coll_Set_remove(&(_this->isRelatedFrom), holder);
    DLRL_INFO(INF_EXIT);
}

LOC_unsigned_long
DK_ObjectAdmin_us_getKeyValueArraySize(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->keyArraySize;
}

void
DK_ObjectAdmin_us_setKeyValueArray(
    DK_ObjectAdmin* _this,
    void* keyArray,
    LOC_unsigned_long keysSize)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* keyArray may be null, even if size is not. */

    _this->keyArray = keyArray;
    _this->keyArraySize = keysSize;

    DLRL_INFO(INF_EXIT);

}

DK_Collection*
DK_ObjectAdmin_us_getCollection(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long collectionIndex)
{
    DK_Collection* returnValue = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(collectionIndex < _this->collectionsSize)
    {
        returnValue = _this->collections[collectionIndex];
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

void
DK_ObjectAdmin_us_resetModificationInfoOnCollections(
    DK_ObjectAdmin* _this)
{
    LOC_unsigned_long count = 0;
    DK_Collection* aCollection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    for(count = 0; count < _this->collectionsSize; count++)
    {
        aCollection = _this->collections[count];
        if(DMM_DLRLMultiRelation_getBasis(aCollection->metaRelation) == DMM_BASIS_SET)
        {
            DK_SetAdmin_us_resetModificationInformation((DK_SetAdmin*)aCollection);
        } else
        {
            assert( DMM_DLRLMultiRelation_getBasis(aCollection->metaRelation) == DMM_BASIS_STR_MAP ||
                    DMM_DLRLMultiRelation_getBasis(aCollection->metaRelation) == DMM_BASIS_INT_MAP);
            DK_MapAdmin_us_resetModificationInformation((DK_MapAdmin*)aCollection);
        }
    }

    DLRL_INFO(INF_EXIT);
}

/* assumes safe access to the meta models */
void
DK_ObjectAdmin_us_createCollections(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_List* relations = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DLRLMultiRelation* aRelation  = NULL;
    DMM_Basis base;
    DK_Collection* collection = NULL;
    DK_ObjectHomeAdmin* targetHome  = NULL;
    DLRL_LS_object ls_collection = NULL;
    LOC_unsigned_long relationsCount = 0;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    /* iterate through all multi relations */
    relations = DMM_DLRLClass_getMultiRelations(DK_ObjectHomeAdmin_us_getMetaRepresentative(_this->home));
    iterator = Coll_List_getFirstElement(relations);
    while(iterator)
    {
        /* ensure the collection does already exist(IE added through the unresolved list) */
        aRelation = (DMM_DLRLMultiRelation*)Coll_Iter_getObject(iterator);
        base = DMM_DLRLMultiRelation_getBasis(aRelation);
        collection = DK_ObjectAdmin_us_getCollection(_this, relationsCount);
        /*in the read side of the DLRL a collection may already exist due to unresolved collections
         * being matched to newly created object admin. The resolving of unresolved elements is then done between the
         * steps of creating a new object admin and create the collections.*/
        if(!collection)
        {
            targetHome = (DK_ObjectHomeAdmin*)DMM_DLRLClass_getUserData(DMM_DLRLRelation_getType(
                                                                                        (DMM_DLRLRelation*)aRelation));
            if(base == DMM_BASIS_STR_MAP || base == DMM_BASIS_INT_MAP)
            {
                collection = (DK_Collection*)DK_MapAdmin_new(exception, _this, targetHome, aRelation, _this->home);
                DLRL_Exception_PROPAGATE(exception);
            } else
            {
                assert(base == DMM_BASIS_SET);
                collection = (DK_Collection*)DK_SetAdmin_new(exception, _this, targetHome, aRelation, _this->home);
                DLRL_Exception_PROPAGATE(exception);
            }
            DK_ObjectAdmin_us_addCollection(_this, exception, collection, relationsCount);
            DLRL_Exception_PROPAGATE(exception);
        }
        if(_this->home->autoDeref)
        {
            ls_collection = collectionBridge.createLSCollection(exception, userData, collection, (DK_RelationType)base);
            DLRL_Exception_PROPAGATE(exception);
            assert(!DK_Collection_us_getLSObject(collection));
            DK_Collection_us_setLSObject(collection, ls_collection);
            objectReaderBridge.setCollectionToLSObject(
                exception,
                userData,
                _this->home,
                _this,
                collection,
                relationsCount);
            DLRL_Exception_PROPAGATE(exception);
        }
        relationsCount++;
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_addCollection(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_Collection* collection,
    LOC_unsigned_long collectionIndex)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(collection);
    if(collectionIndex < _this->collectionsSize)
    {
        _this->collections[collectionIndex] = collection;
    } else
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add collection '%p' to an list of collections "
                                                        "of Object %p. The index if out of range.", collection, _this);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DK_ObjectHolder*
DK_ObjectAdmin_us_getSingleRelation(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long index)
{
    DK_ObjectHolder* holder = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(index < _this->relationsSize)
    {
        holder = _this->relations[index];
    }/* else return null */

    DLRL_INFO(INF_EXIT);
    return holder;
}

DLRL_LS_object
DK_ObjectAdmin_us_getLSObject(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_object;
}

u_instanceHandle
DK_ObjectAdmin_us_getHandle(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->handle;
}

void
DK_ObjectAdmin_us_setHandle(
    DK_ObjectAdmin* _this,
    u_instanceHandle handle)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* handle may be nil */
    _this->handle = handle;
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_setReadState(
    DK_ObjectAdmin* _this,
    DK_ObjectState readState)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(readState < DK_ObjectState_elements);

    _this->readState = readState;
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_setWriteState(
    DK_ObjectAdmin* _this,
    void* userData,
    DK_ObjectState writeState)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(writeState < DK_ObjectState_elements);

    _this->writeState = writeState;
    if(_this->ls_object && objectBridge.notifyWriteStateChange)
    {
        objectBridge.notifyWriteStateChange(userData, _this->ls_object, writeState);
    }


    DLRL_INFO(INF_EXIT);
}

DK_ObjectState
DK_ObjectAdmin_us_getReadState(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->readState;
}

void
DK_ObjectAdmin_us_setIsRegistered(
    DK_ObjectAdmin* _this,
    void* userData,
    LOC_boolean isRegistered)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->isRegistered = isRegistered;
    if(_this->ls_object && objectBridge.setIsRegistered)
    {
        objectBridge.setIsRegistered(userData, _this->ls_object, _this->isRegistered);
    }

    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_ObjectAdmin_us_getIsRegistered(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->isRegistered;
}

void
DK_ObjectAdmin_us_addSingleRelation(
    DK_ObjectAdmin* _this,
    DK_ObjectHolder* holder,
    LOC_unsigned_long index)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(holder);
    assert(_this->relations);

    _this->relations[index] = holder;

    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_setLSObject(
    DK_ObjectAdmin* _this,
    void* userData,
    DLRL_LS_object ls_object)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* ls_object may be null */

    _this->ls_object = ls_object;
    if(ls_object)
    {
        if(objectBridge.setIsRegistered)
        {
            objectBridge.setIsRegistered(userData, _this->ls_object, _this->isRegistered);
        }
        if(objectBridge.notifyWriteStateChange)
        {
            objectBridge.notifyWriteStateChange(userData, _this->ls_object, _this->writeState);
        }
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_lockHome(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_lockAdmin(_this->home);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_unlockHome(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_unlockAdmin(_this->home);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectAdmin_us_checkAlive(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO_OBJECT(INF_ENTER);

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
DK_ObjectAdmin_us_isAlive(
    DK_ObjectAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->alive;
}

int
isObjectAdminCacheAccessLessThen(
    void *left,
    void *right)
{
    int returnValue = 0;
    DK_CacheAccessAdmin* leftAccess = NULL;
    DK_CacheAccessAdmin* rightAccess = NULL;

    DLRL_INFO(INF_ENTER);

    assert(left);
    assert(right);

    leftAccess = ((DK_ObjectAdmin*)left)->access;
    rightAccess = ((DK_ObjectAdmin*)right)->access;

    returnValue = (leftAccess < rightAccess);

    DLRL_INFO(INF_EXIT);
    return returnValue;
}

/**********************************************************************************************************************
****************************** Kernel Thread unsafe internal calls of the ObjectHomeAdmin *****************************
**********************************************************************************************************************/

void
DK_ObjectAdmin_us_destroy(
    DK_Entity* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        if(((DK_ObjectAdmin*)_this)->keyArray)
        {
            DK_DCPSUtility_us_destroyValueArray(((DK_ObjectAdmin*)_this)->keyArray, ((DK_ObjectAdmin*)_this)->keyArraySize);
            ((DK_ObjectAdmin*)_this)->keyArray= NULL;
            ((DK_ObjectAdmin*)_this)->keyArraySize = 0;
        }
        if(((DK_ObjectAdmin*)_this)->home)
        {
            DK_Entity_ts_release((DK_Entity*)(((DK_ObjectAdmin*)_this)->home));
        }
        if(((DK_ObjectAdmin*)_this)->access)
        {
            DK_Entity_ts_release((DK_Entity*)(((DK_ObjectAdmin*)_this)->access));
        }
        assert(Coll_Set_getNrOfElements(&(((DK_ObjectAdmin*)_this)->isRelatedFrom)) == 0);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_ObjectAdmin*)_this);
    }
    DLRL_INFO(INF_EXIT);
}

/* assumes the owning home of this object is locked as well as any related homes needed to manage relations when */
/* this object admin lives in a main cache or that the owning home is locked and all meta models are still in tact */
/* if this object lives in a cache access */
void
DK_ObjectAdmin_us_delete(
    DK_ObjectAdmin* _this,
    void* userData)
{
    LOC_unsigned_long count = 0;
    DK_ObjectHolder* holder = NULL;
    DMM_DLRLRelation* relation = NULL;
    DMM_DLRLClass* targetClass = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;
    DK_Entity* collection = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* target = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */
    if(_this->alive)
    {
        /* may NEVER release the home OR access pointer, this may only be released during the destroy operation when no  */
        /* one has a reference to this object admin anymore. As the home and access are needed for its respective mutex. */
        if(_this->relations)
        {
            for(count = 0; count < _this->relationsSize; count++)
            {
                assert(_this->home);
                holder = _this->relations[count];
                /* the holder might be referenced from an unresolved list, check it before destroying the holder! */
                if(holder)
                {
                    if(!DK_ObjectHolder_us_isResolved(holder))
                    {
                        /* first determine the object home of the relation target */
                        /* remember we have to get the last element first as we are unwinding the list from the end */
                        relation = DMM_DLRLClass_getSingleRelation(_this->home->meta_representative, count);
                        assert(relation);
                        targetClass = DMM_DLRLRelation_getType(relation);
                        targetHome = DMM_DLRLClass_getUserData(targetClass);
                        /* if the home is still alive, unregister unresolved element, could be the home was deleted before */
                        /* this home and thus cleared the unresolved elements list */
                        if(targetHome->alive)
                        {
                            if(_this->access)
                            {
                                DK_CacheAccessAdmin_us_unregisterUnresolvedElement(_this->access, userData, targetHome, holder);
                            } else
                            {
                                DK_ObjectHomeAdmin_us_unregisterUnresolvedElement(targetHome, userData, holder);
                            }
                        }
                    }
                    target = DK_ObjectHolder_us_getTarget(holder);
                    if(target)
                    {
                        DK_ObjectAdmin_us_unregisterIsRelatedFrom(target, holder);
                    }
                    /* dont use the user data of the object holder, so dont have to reset it (which is a precondition of the */
                    /* destroy operation) */
                    assert(!DK_ObjectHolder_us_getUserData(holder));
                    /* holder is not reference counted, so just destroy it. */
                    DK_ObjectHolder_us_destroy(holder);
                }
            }
            os_free(_this->relations);
            _this->relations = NULL;
        }
        iterator = Coll_Set_getFirstElement(&(_this->isRelatedFrom));
        while(iterator)
        {
            void* holder = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->isRelatedFrom), holder);
            /* dont have to free the holder, we arent the owner! We dont let the owner know we are unregistering */
            /* because that should have been done already. */
        }
        if(_this->changedCollections)
        {
            os_free(_this->changedCollections);
            _this->changedCollections = NULL;
        }
        if(_this->collections)
        {
            for(count = 0; count < _this->collectionsSize; count++)
            {
                collection = (DK_Entity*)_this->collections[count];
                if(collection)
                {
                    if(DK_Entity_getClassID(collection) == DK_CLASS_SET_ADMIN)
                    {
                        DK_SetAdmin_us_delete((DK_SetAdmin*)collection, userData);
                    } else
                    {
                        assert(DK_Entity_getClassID(collection) == DK_CLASS_MAP_ADMIN);
                        DK_MapAdmin_us_delete((DK_MapAdmin*)collection, userData);
                    }
                    DK_Entity_ts_release(collection);
                }
            }
            os_free(_this->collections);
            _this->collections = NULL;
            _this->collectionsSize = 0;
        }
        if(!u_instanceHandleIsNil(_this->handle))
        {
            /* not owner of the handle so dont have to reset handles userdata (should be done by home) */
            _this->handle = DK_DCPSUtility_ts_getNilHandle();
        }
        if(objectBridge.clearLSObjectAdministration)
        {
            objectBridge.clearLSObjectAdministration(_this->home, _this->ls_object);
        }
        if(_this->ls_object)
        {
            /* always do this as one of the last delete actions. */
            if(objectBridge.setIsAlive)
            {
                objectBridge.setIsAlive(userData, _this->ls_object, FALSE);
            }
            utilityBridge.releaseLSValuetypeObject(userData, _this->ls_object);

            _this->ls_object = NULL;
        }

        _this->alive = FALSE;
    }
    DLRL_INFO(INF_EXIT);
}
