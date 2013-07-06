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
#ifndef DLRL_KERNEL_UTILITY_H
#define DLRL_KERNEL_UTILITY_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL kernel includes */
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* lock two homes, sequence of locking is determined by the index of the homes */
/* both homes must be registered to the same cache. homes must be registered to a cache as well. */
void
DK_Utility_lockAdminForTwoHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2);

/* unlock two homes, sequence of unlocking is determined by the index of the homes and is reverse that of the locking  */
/* counter part operation to this operation. both homes must be registered to the same cache. homes must be registered  */
/* to a cache as well. */
void
DK_Utility_unlockAdminForTwoHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2);

/* nOT IN DESIGN */
LOC_boolean
DK_Utility_us_isRelationInvalid(
    DK_ObjectHolder* holder,
    LOC_boolean isOptional,
    DK_ObjectState ownerWriteState);

LOC_boolean
DK_Utility_us_isCollectionElementInvalid(
    DK_ObjectHolder* holder);

/* NOT IN DESIGN void DK_Utility_us_createOIDForObjectAdmin(DLRL_Exception* exception, void* userData,  */
    /*                                                         DK_ObjectHomeAdmin* home, DK_ObjectAdmin* objectAdmin); */


/* nOT IN DESIGN */
void
DK_Utility_us_unlockThreeHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2,
    DK_ObjectHomeAdmin* home3);

/* nOT IN DESIGN */
void
DK_Utility_us_lockThreeHomesInSequence(
    DK_ObjectHomeAdmin* home1,
    DK_ObjectHomeAdmin* home2,
    DK_ObjectHomeAdmin* home3);
/* nOT IN DESIGN */
void
DK_Utility_us_copyObjectsIntoTypedObjectSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    Coll_Iter* iterator,
    LOC_unsigned_long size,
    void** arg,
    LOC_boolean hasIndirection);
/* nOT IN DESIGN */
void
DK_Utility_us_copySelectionsIntoTypedSelectionSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    const Coll_Set* selections,
    void** arg);
/* nOT IN DESIGN */
void
DK_Utility_us_copyListenersIntoTypedListenerSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    const Coll_Set* listeners,
    void** arg);
#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_UTILITY_H */
