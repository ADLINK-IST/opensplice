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
#ifndef DLRL_KERNEL_CONTRACT_H
#define DLRL_KERNEL_CONTRACT_H

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* DLRL Kernel includes */
#include "DK_Entity.h"

#if defined (__cplusplus)
extern "C" {
#endif


struct DK_Contract_s
{
    /* The base class of the <code>DK_Collection</code> class which manages the reference count.
     */
    DK_Entity entity;
    LOC_long depth;
    DK_ObjectScope scope;
};

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CONTRACT_H */
