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
#ifndef DLRL_KERNEL_SET_ADMIN_H
#define DLRL_KERNEL_SET_ADMIN_H

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_Set.h"

/* DLRL kernel includes */
#include "DK_Collection.h"
#include "DK_ObjectAdmin.h"
#include "DK_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_SetAdmin_s
{
    DK_Collection base;
    Coll_Set objectHolders;
};

DK_SetAdmin*
DK_SetAdmin_new(
    DLRL_Exception* exception,
    DK_ObjectAdmin* owner,
    DK_ObjectHomeAdmin* targetHome,
    DMM_DLRLMultiRelation* metaRelation,
    DK_ObjectHomeAdmin* ownerHome);

void
DK_SetAdmin_us_delete(
    DK_SetAdmin* _this,
    void* userData);

void
DK_SetAdmin_us_doAdd(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder);

void
DK_SetAdmin_us_doRemove(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder);

LOC_unsigned_long
DK_SetAdmin_us_getLength(
    DK_SetAdmin* _this);

void
DK_SetAdmin_us_addAddedElement(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder);

void
DK_SetAdmin_us_addRemovedElement(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder);

void
DK_SetAdmin_us_resetModificationInformation(
    DK_SetAdmin* _this);

/* NOT IN DESIGN */
LOC_boolean
DK_SetAdmin_us_hasInvalidElements(
    DK_SetAdmin* _this);

/* NOT IN DESIGN */
void
DK_SetAdmin_us_copyElementsFromCollection(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_SetAdmin* targetCollection);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_SET_ADMIN_H */
