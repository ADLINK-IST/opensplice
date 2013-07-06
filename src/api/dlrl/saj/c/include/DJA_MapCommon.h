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
#ifndef DJA_MAP_COMMON_H
#define DJA_MAP_COMMON_H

#include <jni.h>
#include "DLRL_Exception.h"
#include "DLRL_Kernel_private.h"

typedef enum DJA_MapKeyType_e{
    DJA_MAP_KEY_TYPE_STRING,
    DJA_MAP_KEY_TYPE_INT,
    DJA_MapKeyType_elements
} DJA_MapKeyType;

typedef enum DJA_MapElementType_e{
    DJA_MAP_ELEMENT_TYPE_ADDED,
    DJA_MAP_ELEMENT_TYPE_MODIFIED,
    DJA_MAP_ELEMENT_TYPE_REMOVED,
    DJA_MAP_ELEMENT_TYPE_MAIN,
    DJA_MapElementType_elements
} DJA_MapElementType;

jobject DJA_MapCommon_ts_getValues(JNIEnv* env, DK_Collection* collection, DLRL_Exception* exception);

jobject DJA_MapCommon_ts_fillElementsArray(JNIEnv* env, DK_Collection* collection, DLRL_Exception* exception,
                                                   DJA_MapElementType elementType, DJA_MapKeyType collectionBase);

#endif
