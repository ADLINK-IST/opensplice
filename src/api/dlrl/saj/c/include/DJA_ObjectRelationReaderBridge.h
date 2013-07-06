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
#ifndef DJA_OBJECT_RELATION_READER_BRIDGE_H
#define DJA_OBJECT_RELATION_READER_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

void DJA_ObjectRelationReaderBridge_us_setRelatedObjectForObject(
    void* userData,
    DK_ObjectHomeAdmin* ownerObjectHome,
    DK_ObjectAdmin* owner,
    LOC_unsigned_long relationIndex,
    DK_ObjectAdmin* relationObjectAdmin,
    LOC_boolean isValid);

#endif /* DJA_OBJECT_RELATION_READER_BRIDGE_H */
