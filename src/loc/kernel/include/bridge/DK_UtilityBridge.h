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
#ifndef DLRL_KERNEL_UTILITY_BRIDGE_H
#define DLRL_KERNEL_UTILITY_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Should return <code>TRUE</code> when the two object pointers represent the same object and <code>FALSE</code>
 * otherwise
 *
 * This utility function is used by multiple kernel entities to determine if two language specific objects represent
 * the same object or not. In that light no garantees can be given as to which kernel entities are locked and which are
 * not.
 *
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param obj1 A language specific object pointer
 * \param obj2 A language specific object pointer
 *
 * \return <code>TRUE</code> when the two object pointer represent the same object and <code>FALSE</code> otherwise.
 */
typedef LOC_boolean (*DK_UtilityBridge_us_AreSameLSObjects)(
    void* userData,
    DLRL_LS_object obj1,
    DLRL_LS_object obj2);

/* not in design */
typedef DLRL_LS_object (*DK_UtilityBridge_us_duplicateLSValuetypeObject)(
    void* userData,
    DLRL_LS_object object);

/* not in design */
typedef void (*DK_UtilityBridge_us_releaseLSValuetypeObject)(
    void* userData,
    DLRL_LS_object object);

/* not in design */
typedef DLRL_LS_object (*DK_UtilityBridge_us_duplicateLSInterfaceObject)(
    void* userData,
    DLRL_LS_object object);

/* not in design */
typedef void (*DK_UtilityBridge_us_releaseLSInterfaceObject)(
    void* userData,
    DLRL_LS_object object);

/* not in design */
typedef DLRL_LS_object (*DK_UtilityBridge_us_localDuplicateLSInterfaceObject)(
    void* userData,
    DLRL_LS_object object);

/* not in design */
typedef DLRL_LS_object (*DK_UtilityBridge_us_localDuplicateLSValuetypeObject)(
    void* userData,
    DLRL_LS_object object);

/* not in design - removedDLRL_LS_object DK_UtilityBridge_us_createOIDStructure(DLRL_Exception* exception, void* userData, void* oidData); */

/* not in design - removedvoid DK_UtilityBridge_us_deleteOIDStructure(void* userData, DLRL_LS_object oidStructure); */

typedef void (*DK_UtilityBridge_us_getThreadCreateUserData)(
    DLRL_Exception* exception,
    void* userData,
    void** retVal);

typedef void (*DK_UtilityBridge_us_doThreadAttach)(
    DLRL_Exception* exception,
    void* userData);

typedef void (*DK_UtilityBridge_us_doThreadDetach)(
    DLRL_Exception* exception,
    void* userData);

typedef void* (*DK_UtilityBridge_us_getThreadSessionUserData)();

typedef void (*DK_UtilityBridge_us_createIntegerSeq)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size);

typedef void (*DK_UtilityBridge_us_createStringSeq)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size);

typedef void (*DK_UtilityBridge_us_addElementToStringSeq)(
    DLRL_Exception* exception,
    void* userData,
    void* arg,
    LOC_unsigned_long count,
    void* keyUserData);

typedef void (*DK_UtilityBridge_us_addElementToIntegerSeq)(
    DLRL_Exception* exception,
    void* userData,
    void* arg,
    LOC_unsigned_long count,
    void* keyUserData);

typedef struct DK_UtilityBridge_s{
    DK_UtilityBridge_us_getThreadCreateUserData getThreadCreateUserData;/*may be NIL pointer*/
    DK_UtilityBridge_us_doThreadAttach doThreadAttach;/*may be NIL pointer*/
    DK_UtilityBridge_us_doThreadDetach doThreadDetach;/*may be NIL pointer*/
    DK_UtilityBridge_us_getThreadSessionUserData getThreadSessionUserData;/*may be NIL pointer*/
    DK_UtilityBridge_us_AreSameLSObjects areSameLSObjects;
    DK_UtilityBridge_us_createIntegerSeq createIntegerSeq;
    DK_UtilityBridge_us_createStringSeq createStringSeq;
    DK_UtilityBridge_us_addElementToStringSeq addElementToStringSeq;
    DK_UtilityBridge_us_addElementToIntegerSeq addElementToIntegerSeq;

    /* following dup / release operations are used by the kernel when it needs to
     * store a language specific object in the kernel for a long period of time
     */
    DK_UtilityBridge_us_duplicateLSValuetypeObject duplicateLSValuetypeObject;
    DK_UtilityBridge_us_releaseLSValuetypeObject releaseLSValuetypeObject;
    DK_UtilityBridge_us_duplicateLSInterfaceObject duplicateLSInterfaceObject;
    DK_UtilityBridge_us_releaseLSInterfaceObject releaseLSInterfaceObject;
    /* Following dup operations are used when the kernel returns a reference to a
     * language specific object it is maintaining. This is handy when the language binding
     * requires a different kind of dup mechanism for locally available objects then globally
     * stored objects, such as a Java language binding
     */
    DK_UtilityBridge_us_localDuplicateLSInterfaceObject localDuplicateLSInterfaceObject;
    DK_UtilityBridge_us_localDuplicateLSValuetypeObject localDuplicateLSValuetypeObject;
}DK_UtilityBridge;

extern DK_UtilityBridge utilityBridge;

/* the following operations and function cache is used per object type, it is
 * thus not available as a global function cache like the generic utilityBridge
 * function cache. Instead it should be maintained in the ObjectHomeAdmin
 * for example.
 */
typedef void (*DK_UtilityBridge_us_createTypedObjectSeq)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size);

typedef void (*DK_UtilityBridge_us_addToTypedObjectSeq)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long count,
    DLRL_LS_object notDuppedObject);

typedef struct DK_UtilityBridgeFunctionCache_s{
    DK_UtilityBridge_us_createTypedObjectSeq createTypedObjectSeq;
    DK_UtilityBridge_us_addToTypedObjectSeq addToTypedObjectSeq;
} DK_UtilityBridgeFunctionCache;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_UTILITY_BRIDGE_H */
