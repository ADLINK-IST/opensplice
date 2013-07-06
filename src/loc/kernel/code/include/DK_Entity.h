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
#ifndef DLRL_KERNEL_ENTITY_H
#define DLRL_KERNEL_ENTITY_H

/* OS abstraction layer includes */
#include "os_defs.h"

/* DLRL util includes */
#include "DLRL_Types.h"

/* DLRL util includes */
#include "DLRL_Exception.h"

/* DLRL kernel includes  */
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_Entity_s
{
/*    os_mutex mutex;*/
    os_uint32 ref_count;
    void (* destroy)(DK_Entity *);
    DK_Class classID;
};

void
DK_Entity_us_init(
    DK_Entity* _this,
    DK_Class classID,
    void (* destroy)(
                    DK_Entity *));

DK_Class DK_Entity_getClassID(DK_Entity* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_ENTITY_H */
