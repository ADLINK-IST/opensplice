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
#ifndef DLRL_KERNEL_OBJECT_BRIDGE_H
#define DLRL_KERNEL_OBJECT_BRIDGE_H

#include "DLRL_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* NOT IN DESIGN */
typedef void (*DK_ObjectBridge_us_setIsRegistered)(
    void* userData,
    DLRL_LS_object ls_object,
    LOC_boolean val);

/* NOT IN DESIGN */
typedef void (*DK_ObjectBridge_us_notifyWriteStateChange)(
    void* userData,
    DLRL_LS_object ls_object,
    DK_ObjectState val);

/* NOT IN DESIGN */
typedef void (*DK_ObjectBridge_us_setIsAlive)(
    void* userData,
    DLRL_LS_object ls_object,
    LOC_boolean val);

typedef void (*DK_ObjectBridge_us_clearLSObjectAdministration)(
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object ls_object);

typedef struct DK_ObjectBridge_s{
    DK_ObjectBridge_us_setIsAlive setIsAlive;/* may be set to NIL pointer */
    DK_ObjectBridge_us_setIsRegistered setIsRegistered;/* may be set to NIL pointer */
    DK_ObjectBridge_us_notifyWriteStateChange notifyWriteStateChange;/* may be set to NIL pointer */
    DK_ObjectBridge_us_clearLSObjectAdministration clearLSObjectAdministration;/* may be set to NIL pointer */
} DK_ObjectBridge;

extern DK_ObjectBridge objectBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_BRIDGE_H */
