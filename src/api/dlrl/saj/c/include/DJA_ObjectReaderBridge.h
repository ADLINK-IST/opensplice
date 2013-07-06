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
#ifndef DJA_OBJECT_READER_BRIDGE_H
#define DJA_OBJECT_READER_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

void
DJA_ObjectReaderBridge_us_doLSReadPreProcessing(
    DK_ReadInfo* readInfo,
    DK_ObjectReader* objReader);

DLRL_LS_object
DJA_ObjectReaderBridge_us_createLSTopic(
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin,
    void* dstInfo,
    void (*ls_copyOut)(void*, void*),
    void* sampleData);

void
DJA_ObjectReaderBridge_us_setCollectionToLSObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin,
    DK_Collection* collection,
    LOC_unsigned_long collectionIndex);

void
DJA_ObjectReaderBridge_us_updateObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* object,
    DLRL_LS_object ls_topic);

void
DJA_ObjectReaderBridge_us_resetLSModificationInfo(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

#endif /* DJA_OBJECT_READER_BRIDGE_H */
