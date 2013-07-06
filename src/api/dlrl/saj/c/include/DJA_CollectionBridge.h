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
#ifndef DJA_COLLECTION_BRIDGE_H
#define DJA_COLLECTION_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

DLRL_LS_object DJA_CollectionBridge_us_createLSCollection(DLRL_Exception* exception, void* userData,
                                                            DK_Collection* collection, DK_RelationType relationType);

#endif /* DJA_COLLECTION_BRIDGE_H */
