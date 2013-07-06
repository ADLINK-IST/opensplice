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
#ifndef DLRL_KERNEL_UNRESOLVED_LIST_UTILITY_H
#define DLRL_KERNEL_UNRESOLVED_LIST_UTILITY_H

/* DLRL util includes */
#include "DLRL_Exception.h"

/* collection includes */
#include "Coll_List.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLMultiRelation.h"

/* DLRL Kernel includes */
#include "DK_Collection.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

void
DK_UnresolvedObjectsUtility_us_clear(
    Coll_Set* unresolvedElements,
    void* userData);

void
DK_UnresolvedObjectsUtility_us_unregisterUnresolvedElement(
    Coll_Set* unresolvedElements,
    void* userData,
    DK_ObjectHolder* holder);

void
DK_UnresolvedObjectsUtility_us_unregisterAllUnresolvedElementsForEntity(
    Coll_Set* unresolvedElements,
    void* userData,
    DK_Entity* entity);

DK_Collection*
DK_UnresolvedObjectsUtility_us_unregisterUnresolvedCollection(
    Coll_Set* unresolvedElements,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    LOC_unsigned_long collectionIndex);

DK_Collection*
DK_UnresolvedObjectsUtility_us_registerUnresolvedCollection(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    DMM_DLRLMultiRelation* metaRelation,
    LOC_unsigned_long collectionIndex,
    DK_ObjectHomeAdmin* ownerHome);

void
DK_UnresolvedObjectsUtility_us_registerUnresolvedElement(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* holder,
    LOC_unsigned_long relationIndex);

/* NOT IN DESIGN removed and added parameters */
void
DK_UnresolvedObjectsUtility_us_resolveUnresolvedElements(
    Coll_Set* unresolvedElements,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin,
    void* keyArray,
    LOC_boolean resolveTargets,/* temporary var until proper fix for dds439 */
    LOC_boolean resolveCollectionOwners);/* temporary var until proper fix for dds439 */

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_UNRESOLVED_LIST_UTILITY_H */
