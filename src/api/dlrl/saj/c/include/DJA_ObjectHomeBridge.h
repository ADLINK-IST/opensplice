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
#ifndef DJA_OBJECT_HOME_BRIDGE_H
#define DJA_OBJECT_HOME_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

LOC_boolean
DJA_ObjectHomeBridge_us_checkObjectForSelection(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    DK_ObjectAdmin* objectAdmin);

void
DJA_ObjectHomeBridge_us_loadMetamodel(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    void* userData);

void
DJA_ObjectHomeBridge_us_unregisterAdminWithLSHome(
    void* userData,
    DLRL_LS_object ls_home,
    LOC_boolean isRegistered);

void
DJA_ObjectHomeBridge_us_triggerListeners(
    DLRL_Exception* exception,
    void *userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* newSamples,
    Coll_List* modifiedSamples,
    Coll_List* deletedSamples);

void
DJA_ObjectHomeBridge_us_deleteUserData(
    void* userData,
    void* homeUserData);

DLRL_LS_object
DJA_ObjectHomeBridge_us_createTypedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_TopicInfo* topicInfo,
    DLRL_LS_object ls_topic,
    DK_ObjectAdmin* objectAdmin);

void
DJA_ObjectHomeBridge_us_doCopyInForTopicOfObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectWriter* objWriter,
    DK_ObjectAdmin* objectAdmin,
    void* message,
    void* dataSample);

/* className may be NIL */
void
DJA_ObjectHomeBridge_us_setDefaultTopicKeys(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_TopicInfo* topicInfo,
    DLRL_LS_object ls_object,
    DK_ObjectID* oid,
    LOC_string className);

void DJA_ObjectHomeBridge_us_createTypedObjectSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size);

void DJA_ObjectHomeBridge_us_addElementToTypedObjectSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);


void DJA_ObjectHomeBridge_us_createTypedSelectionSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size);

void DJA_ObjectHomeBridge_us_addElementToTypedSelectionSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);


void DJA_ObjectHomeBridge_us_createTypedListenerSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size);

void DJA_ObjectHomeBridge_us_addElementToTypedListenerSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

#endif /* DJA_OBJECT_HOME_BRIDGE_H */
