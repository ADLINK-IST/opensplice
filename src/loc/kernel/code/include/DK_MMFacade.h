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
#ifndef DLRL_KERNEL_META_MODEL_FACADE_H
#define DLRL_KERNEL_META_MODEL_FACADE_H

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"


/* DLRL MetaModel includes */
#include "DMM_DCPSTopic.h"
#include "DMM_DLRLClass.h"
#include "DMM_DLRLRelation.h"

/* DLRL kernel includes */
#include "DK_ObjectHomeAdmin.h"

#if defined (__cplusplus)
extern "C" {
#endif

void
DK_MMFacade_us_resolveMetaModel(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_META_MODEL_FACADE_H */
