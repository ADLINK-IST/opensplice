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
#ifndef DLRL_KERNEL_SECONDARY_OBJECT_ADMIN_H
#define DLRL_KERNEL_SECONDARY_OBJECT_ADMIN_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_Set.h"

/* DLRL kernel includes */
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_SecondaryObjectAdmin_s
{
    /* inherits DK_Entity struct from parent */
    DK_ObjectAdmin base;
    LOC_boolean isRegistered;
    LOC_long maxClonedContainedDepth;
    LOC_long maxClonedRelatedDepth;
    DK_CacheAccessAdmin* access;
    DK_ObjectAdmin* original;
    Coll_Set changedTopics;
};

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_SECONDARY_OBJECT_ADMIN_H */
