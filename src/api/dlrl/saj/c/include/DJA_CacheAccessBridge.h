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
#ifndef DJA_CACHE_ACCESS_BRIDGE_H
#define DJA_CACHE_ACCESS_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"
#include "Coll_Set.h"
#include "Coll_List.h"

void DJA_CacheAccessBridge_us_containedTypesAction(DLRL_Exception* exception, void* userData, LOC_long* indexes, 
                                                                        LOC_unsigned_long totalSize, void** arg);

void DJA_CacheAccessBridge_us_containedTypeNamesAction(DLRL_Exception* exception, void* userData, 
                            LOC_unsigned_long totalSize, LOC_unsigned_long index, LOC_string name, void** arg);

void DJA_CacheAccessBridge_us_objectsAction(DLRL_Exception* exception, void* userData, void** arg, 
                                        LOC_unsigned_long size, LOC_unsigned_long* elementIndex, Coll_Set* objects);

void DJA_CacheAccessBridge_us_invalidObjectsAction(DLRL_Exception* exception, void* userData, void** arg, 
                                                                                            Coll_List* invalidObjects);

#endif /* DJA_CACHE_ACCESS_BRIDGE_H */
