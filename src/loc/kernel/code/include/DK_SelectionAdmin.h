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
#ifndef DLRL_KERNEL_SELECTION_ADMIN_H
#define DLRL_KERNEL_SELECTION_ADMIN_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_List.h"
#include "Coll_Set.h"

/* DLRL kernel includes */
#include "DK_Entity.h"
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_SelectionAdmin_s
{
    DK_Entity entity;
    DK_ObjectHomeAdmin* home;
    LOC_boolean alive;
    LOC_boolean autoRefresh;
    LOC_boolean concernsContained;
    Coll_Set membership;
    Coll_List newMembers;
    Coll_List modifiedMembers;
    Coll_List removedMembers;
    DLRL_LS_object ls_selection;
    DLRL_LS_object listener;
    DK_CriterionKind criterionKind;
    DK_SelectionCriterion criterion;
};

DK_SelectionAdmin*
DK_SelectionAdmin_new(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object ls_selection,
    DK_SelectionCriterion* criterion,
    DK_CriterionKind kind,
    LOC_boolean autoRefresh,
    LOC_boolean concernsContainedObjects);

void
DK_SelectionAdmin_us_triggerListener(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

void
DK_SelectionAdmin_us_delete(
    DK_SelectionAdmin* _this,
    void* userData);

LOC_boolean
DK_SelectionAdmin_us_getAutoRefresh(
    DK_SelectionAdmin* _this);

void
DK_SelectionAdmin_us_addMember(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin,
    LOC_boolean belongs);

void
DK_SelectionAdmin_us_resetModificationInformation(
    DK_SelectionAdmin* selection);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_SELECTION_ADMIN_H */
