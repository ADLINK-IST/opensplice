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
#include "DLRL_Report.h"

/* DLRL kernel includes */
#include "DK_ObjectAdmin.h"
#include "DK_Collection.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_ObjectHomeBridge.h"
#include "DK_Utility.h"
#include "DK_UtilityBridge.h"
#include "DLRL_Kernel_private.h"

/* lock two homes, sequence of locking is determined by the index of the homes */
/* both homes must be registered to the same cache. homes must be registered to a cache as well. */
/* the home with the lowest index MUST be locked first, this is the strategy used elsewhere in the code as well */
/* changing this WILL result in deadlock scenarios becoming reality. */
void
DK_Utility_lockAdminForTwoHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2)
{
    DLRL_INFO(INF_ENTER);

    assert(home1);
    assert(home2);
    /* function may not be used for unregistered homes */
    assert(home1->cache && (home1->cache == home2->cache));

    /* could be that the same homes are provided, we can deal with that by using the following check */
    if(home1 == home2)
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
    } else if(home1->index < home2->index)
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home2);
    } else
    {
        assert(home1->index > home2->index);
        DK_ObjectHomeAdmin_lockAdmin(home2);
        DK_ObjectHomeAdmin_lockAdmin(home1);
    }
    DLRL_INFO(INF_EXIT);
}

/* unlock two homes, sequence of unlocking is determined by the index of the homes and is reverse that of the locking */
/* counter part operation to this operation. both homes must be registered to the same cache. homes must be registered */
/* to a cache as well. */
void
DK_Utility_unlockAdminForTwoHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2)
{
    DLRL_INFO(INF_ENTER);

    assert(home1);
    assert(home2);
    /* function may not be used for unregistered homes */
    assert(home1->cache && (home1->cache == home2->cache));

    /* could be that the same homes are provided, we can deal with that by using the following check */
    if(home1 == home2)
    {
        DK_ObjectHomeAdmin_unlockAdmin(home1);
    } else
    {
        DK_ObjectHomeAdmin_unlockAdmin(home1);
        DK_ObjectHomeAdmin_unlockAdmin(home2);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_Utility_us_lockThreeHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2,
    DK_ObjectHomeAdmin* home3)
{
    DLRL_INFO(INF_ENTER);

    assert(home1);
    assert(home2);
    assert(home3);

    /*TODO this should dissappear after a redesign of locking, this is more or less
    a quick fix for locking 3 homes in sequence*/
    if((home1 == home2) && (home1 == home3))
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
    } else if((home1 == home2) && (home1->index < home3->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home3);
        DK_ObjectHomeAdmin_lockAdmin(home1);
    } else if((home1->index < home2->index) && ((home1 == home3) || (home2 == home3)))
    {
        DK_ObjectHomeAdmin_lockAdmin(home2);
        DK_ObjectHomeAdmin_lockAdmin(home1);
    } else if((home1->index > home2->index) && ((home2 == home3) || (home1 == home3)))
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home2);
    } else if((home1 == home2) && (home1->index > home3->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home3);
    } else if((home1->index < home2->index) && (home2->index < home3->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home3);
        DK_ObjectHomeAdmin_lockAdmin(home2);
        DK_ObjectHomeAdmin_lockAdmin(home1);
    } else if((home1->index > home3->index) && (home1->index < home2->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home2);
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home3);
    } else if((home1->index > home3->index) && (home2->index < home3->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home3);
        DK_ObjectHomeAdmin_lockAdmin(home2);
    } else if((home1->index > home2->index) && (home2->index > home3->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home2);
        DK_ObjectHomeAdmin_lockAdmin(home3);
    } else if((home1->index < home3->index) && (home2->index > home3->index))
    {
        DK_ObjectHomeAdmin_lockAdmin(home2);
        DK_ObjectHomeAdmin_lockAdmin(home3);
        DK_ObjectHomeAdmin_lockAdmin(home1);
    } else
    {
        assert((home1->index < home3->index) && (home1->index > home2->index));
        DK_ObjectHomeAdmin_lockAdmin(home3);
        DK_ObjectHomeAdmin_lockAdmin(home1);
        DK_ObjectHomeAdmin_lockAdmin(home2);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_Utility_us_unlockThreeHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2,
    DK_ObjectHomeAdmin* home3)
{
    DLRL_INFO(INF_ENTER);

    assert(home1);
    assert(home2);
    assert(home3);

    if((home1 == home2) && (home1 == home3))
    {
        DK_ObjectHomeAdmin_unlockAdmin(home1);
    } else if(home1 != home2 && ((home3 == home1) || (home3 == home2)))
    {
        DK_ObjectHomeAdmin_unlockAdmin(home1);
        DK_ObjectHomeAdmin_unlockAdmin(home2);
    } else if((home1 != home3) && ((home2 == home1) || (home2 == home3)))
    {
        DK_ObjectHomeAdmin_unlockAdmin(home1);
        DK_ObjectHomeAdmin_unlockAdmin(home3);
    } else
    {
        DK_ObjectHomeAdmin_unlockAdmin(home1);
        DK_ObjectHomeAdmin_unlockAdmin(home2);
        DK_ObjectHomeAdmin_unlockAdmin(home3);
    }
    DLRL_INFO(INF_EXIT);
}


LOC_boolean
DK_Utility_us_isRelationInvalid(
    DK_ObjectHolder* holder,
    LOC_boolean isOptional,
    DK_ObjectState ownerWriteState)
{
    LOC_boolean isInvalid = FALSE;
    DK_ObjectAdmin* target = NULL;

    DLRL_INFO(INF_ENTER);

    /* holder may be NULL */

    if(holder)
    {
        target = DK_ObjectHolder_us_getTarget(holder);
    }
    /* if we do not have a holder, but the relation is mandatory */
    if(!holder && !isOptional)
    {
        isInvalid = TRUE;
    /* if we do not have a target, but the relation is mandatory */
    } else if(!target && !isOptional)
    {
        isInvalid = TRUE;
    /* If the relation owner is not deleted, but the target we are pointing to
     * is
     */
    } else if(  (ownerWriteState != DK_OBJECT_STATE_OBJECT_DELETED) &&
                target &&
                (DK_ObjectAdmin_us_getWriteState(target) == DK_OBJECT_STATE_OBJECT_DELETED))
    {
        isInvalid = TRUE;
    }

    DLRL_INFO(INF_EXIT);
    return isInvalid;
}

LOC_boolean
DK_Utility_us_isCollectionElementInvalid(
    DK_ObjectHolder* holder)
{
    LOC_boolean isInvalid = FALSE;
    DK_ObjectAdmin* target = NULL;

    DLRL_INFO(INF_ENTER);

    assert(holder);
    /* if the collection owner is already deleted, then this operation should
     * never have been invoked!
     */
    assert(DK_ObjectAdmin_us_getWriteState(((DK_Collection*)holder->owner)->owner) != DK_OBJECT_STATE_OBJECT_DELETED);

    target = DK_ObjectHolder_us_getTarget(holder);
    assert(target);
    if(DK_ObjectAdmin_us_getWriteState(target) == DK_OBJECT_STATE_OBJECT_DELETED)
    {
        isInvalid = TRUE;
    }

    DLRL_INFO(INF_EXIT);
    return isInvalid;
}

/* Assumes the ObjectHomeAdmin for the ObjectAdmin in the list is locked
 * with an admin lock. Also assumes that the ObjectHomeAdmin for the elements
 * in the list is locked. Also assumes access to the 'objects' list is safe
 * and that no 'free' actions need to be done on the list or its elements.
 */
void
DK_Utility_us_copyObjectsIntoTypedObjectSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    Coll_Iter* iterator,
    LOC_unsigned_long size,
    void** arg,
    LOC_boolean hasIndirection)
{
    LOC_unsigned_long count = 0;
    DK_ObjectAdmin* objectAdmin;
    DLRL_LS_object lsObject;
    DK_ObjectHolder* holder;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(targetLockedHome);
    assert((iterator && size > 0) || (!iterator && size == 0));
    assert(arg);

    /* initialize the language specific typed object sequence if applicable. */
    if(!(*arg))
    {
        objectHomeBridge.createTypedObjectSeq(
            exception,
            userData,
            targetLockedHome,
            arg,
            size);
    }
    /* iterate through all elements and assigned each element to the
     * typed object sequence.
     */
    while(iterator)
    {
        if (hasIndirection)
        {
            holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
            objectAdmin = DK_ObjectHolder_us_getTarget(holder);
        } else
        {
            objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        }
        lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);/* dont dup */
        /* assign the object to the seq */
        objectHomeBridge.addElementToTypedObjectSeq(
            exception,
            userData,
            targetLockedHome,
            *arg,
            lsObject,
            count);
        DLRL_Exception_PROPAGATE(exception);
        /* move to the next object */
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(exception);
    /* dont translate exception, not an API routine */
    DLRL_INFO(INF_EXIT);
}

/* Assumes the ObjectHomeAdmin for the ObjectAdmin in the list is locked
 * with an admin lock. Also assumes that the ObjectHomeAdmin for the elements
 * in the list is locked. Also assumes access to the 'objects' list is safe
 * and that no 'free' actions need to be done on the list or its elements.
 */
void
DK_Utility_us_copySelectionsIntoTypedSelectionSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    const Coll_Set* selections,
    void** arg)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long count = 0;
    DK_SelectionAdmin* selection;
    DLRL_LS_object lsObject;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(targetLockedHome);
    assert(selections);
    assert(arg);

    /* initialize the language specific typed object sequence if applicable. */
    if(!(*arg))
    {
        objectHomeBridge.createTypedSelectionSeq(
            exception,
            userData,
            targetLockedHome,
            arg,
            Coll_Set_getNrOfElements(selections));
    }
    /* iterate through all elements and assigned each element to the
     * typed object sequence.
     */
    iterator = Coll_Set_getFirstElement(selections);
    while(iterator)
    {
        selection = (DK_SelectionAdmin*)Coll_Iter_getObject(iterator);
        lsObject = DK_SelectionAdmin_us_getLSSelection(selection);/* dont dup */
        /* assign the object to the seq */
        objectHomeBridge.addElementToTypedSelectionSeq(
            exception,
            userData,
            targetLockedHome,
            *arg,
            lsObject,
            count);
        DLRL_Exception_PROPAGATE(exception);
        /* move to the next object */
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(exception);
    /* dont translate exception, not an API routine */
    DLRL_INFO(INF_EXIT);
}

/* Assumes the ObjectHomeAdmin for the ObjectAdmin in the list is locked
 * with an admin lock. Also assumes that the ObjectHomeAdmin for the elements
 * in the list is locked. Also assumes access to the 'objects' list is safe
 * and that no 'free' actions need to be done on the list or its elements.
 */
void
DK_Utility_us_copyListenersIntoTypedListenerSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    const Coll_Set* listeners,
    void** arg)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long count = 0;
    DLRL_LS_object* listener;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(targetLockedHome);
    assert(listeners);
    assert(arg);

    /* initialize the language specific typed object sequence if applicable. */
    if(!(*arg))
    {
        objectHomeBridge.createTypedListenerSeq(
            exception,
            userData,
            targetLockedHome,
            arg,
            Coll_Set_getNrOfElements(listeners));
    }
    /* iterate through all elements and assigned each element to the
     * typed object sequence.
     */
    iterator = Coll_Set_getFirstElement(listeners);
    while(iterator)
    {
        listener = (DLRL_LS_object)Coll_Iter_getObject(iterator);
        /* assign the object to the seq */
        objectHomeBridge.addElementToTypedListenerSeq(
            exception,
            userData,
            targetLockedHome,
            *arg,
            listener,
            count);
        DLRL_Exception_PROPAGATE(exception);
        /* move to the next object */
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(exception);
    /* dont translate exception, not an API routine */
    DLRL_INFO(INF_EXIT);
}
