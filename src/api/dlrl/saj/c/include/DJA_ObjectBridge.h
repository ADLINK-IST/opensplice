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
#ifndef DJA_OBJECT_BRIDGE_H
#define DJA_OBJECT_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"
void DJA_ObjectBridge_us_setIsAlive(void* userData, DLRL_LS_object ls_object, LOC_boolean val);

void DJA_ObjectBridge_us_setIsRegistered(void* userData, DLRL_LS_object ls_object, LOC_boolean val);

void DJA_ObjectBridge_us_notifyWriteStateChange(void* userData, DLRL_LS_object ls_object, DK_ObjectState val);

#endif /* DJA_OBJECT_BRIDGE_H */
