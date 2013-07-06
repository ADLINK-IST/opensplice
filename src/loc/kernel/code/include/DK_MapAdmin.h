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
#ifndef DLRL_KERNEL_MAP_ADMIN_H
#define DLRL_KERNEL_MAP_ADMIN_H

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_Map.h"
#include "Coll_Set.h"

/* DLRL kernel includes */
#include "DK_Collection.h"
#include "DK_ObjectAdmin.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_MapAdmin_s
{
    DK_Collection base;
    Coll_List modifiedElements;
    Coll_Set objectHolders;/* should be a map, but we are missing an iterator implementation for the map so... */
};

DK_MapAdmin*
DK_MapAdmin_new(
    DLRL_Exception* exception,
    DK_ObjectAdmin* owner,
    DK_ObjectHomeAdmin* targetHome,
    DMM_DLRLMultiRelation* metaRelation,
    DK_ObjectHomeAdmin* ownerHome);

void
DK_MapAdmin_us_delete(
    DK_MapAdmin* _this,
    void* userData);

void
DK_MapAdmin_us_doPut(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder);

void
DK_MapAdmin_us_doMarkAsModified(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder);

void
DK_MapAdmin_us_doRemove(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder);

void
DK_MapAdmin_us_addRemovedElement(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* keyValue,
    DK_ObjectHolder* holder);

LOC_unsigned_long
DK_MapAdmin_us_getLength(
    DK_MapAdmin* _this);

void
DK_MapAdmin_us_resetModificationInformation(
    DK_MapAdmin* _this);

/* NOT IN DESIGN */
LOC_boolean
DK_MapAdmin_us_hasInvalidElements(
    DK_MapAdmin* _this);
/* NOT IN DESIGN */

void
DK_MapAdmin_us_copyElementsFromCollection(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_MapAdmin* targetCollection);
#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_MAP_ADMIN_H */
