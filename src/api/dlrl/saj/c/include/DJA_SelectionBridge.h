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
#ifndef DJA_SELECTION_BRIDGE_H
#define DJA_SELECTION_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"
#include "DLRL_Exception.h"
#include "DLRL_Kernel.h"

DK_ObjectAdmin**
DJA_SelectionBridge_us_checkObjects(
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    DLRL_LS_object filterCriterion,
    DK_ObjectAdmin** objectArray,
    LOC_unsigned_long size,
    LOC_unsigned_long* passedAdminsArraySize);

void
DJA_SelectionBridge_us_triggerListenerInsertedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin);

void
DJA_SelectionBridge_us_triggerListenerModifiedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin);

void
DJA_SelectionBridge_us_triggerListenerRemovedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin);

#endif /* DJA_SELECTION_BRIDGE_H */
