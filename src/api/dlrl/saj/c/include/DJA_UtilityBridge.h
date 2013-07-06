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
#ifndef DJA_UTILITY_BRIDGE_H
#define DJA_UTILITY_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

LOC_boolean
DJA_UtilityBridge_us_AreSameLSObjects(
    void* userData,
    DLRL_LS_object obj1,
    DLRL_LS_object obj2);

DLRL_LS_object
DJA_UtilityBridge_us_duplicateLSObject(
    void* userData,
    DLRL_LS_object object);

void
DJA_UtilityBridge_us_releaseLSObject(
    void* userData,
    DLRL_LS_object object);

DLRL_LS_object
DJA_UtilityBridge_us_localDuplicateLSObject(
    void* userData,
    DLRL_LS_object object);

void
DJA_UtilityBridge_us_getThreadCreateUserData(
    DLRL_Exception* exception,
    void* userData,
    void** retVal);

void
DJA_UtilityBridge_us_doThreadAttach(
    DLRL_Exception* exception,
    void* userData);

void
DJA_UtilityBridge_us_doThreadDetach(
    DLRL_Exception* exception,
    void* userData);

void*
DJA_UtilityBridge_us_getThreadSessionUserData(
    );

void DJA_UtilityBridge_us_createIntegerSeq(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size);

void DJA_UtilityBridge_us_createStringSeq(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size);

void
DJA_UtilityBridge_us_addElementToStringSeq(
    DLRL_Exception* exception,
    void* userData,
    void* arg,
    LOC_unsigned_long count,
    void* keyUserData);

void
DJA_UtilityBridge_us_addElementToIntegerSeq(
    DLRL_Exception* exception,
    void* userData,
    void* arg,
    LOC_unsigned_long count,
    void* keyUserData);


#endif /* DJA_UTILITY_BRIDGE_H */
