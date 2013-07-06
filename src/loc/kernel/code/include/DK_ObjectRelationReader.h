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
#ifndef DLRL_KERNEL_OBJECT_RELATION_READER_H
#define DLRL_KERNEL_OBJECT_RELATION_READER_H

/* DLRL util includes */
#include "DLRL_Exception.h"

/* DLRL MetaModel includes */
#include "DMM_Types.h"

/* DLRL kernel includes */
#include "DK_Types.h"
#include "DLRL_Kernel_private.h"

#if defined (__cplusplus)
extern "C" {
#endif

void
DK_ObjectRelationReader_us_processSingleRelation(
    DK_ObjectReader* reader,
    DLRL_Exception* exception,
    void* userData,
    DMM_DLRLRelation* relation,
    LOC_unsigned_long relationIndex,
    DK_ObjectHomeAdmin* home,
    DK_ReadData* data,
    LOC_boolean isValid);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_RELATION_READER_H */
