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
#ifndef DLRL_META_MODEL_DLRL_MULTI_ATTRIBUTE_H
#define DLRL_META_MODEL_DLRL_MULTI_ATTRIBUTE_H

/* DLRL Metamodel includes */
#include "DMM_Types.h"
#include "DMM_Basis.h"
#include "DMM_DLRLAttribute.h"
#include "DMM_DLRLClass.h"


/* DLRL includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_LOC_METAMODEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

struct DMM_DLRLMultiAttribute_s {
    DMM_DLRLAttribute base;
    DMM_Basis basis;
    DMM_DCPSField* indexField;

};

OS_API DMM_DLRLMultiAttribute*
DMM_DLRLMultiAttribute_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type,
    DMM_Basis basis,
    DMM_DLRLClass* owner);

OS_API void
DMM_DLRLMultiAttribute_delete(
    DMM_DLRLMultiAttribute* _this);

OS_API DMM_Basis
DMM_DLRLMultiAttribute_getBasis(
    DMM_DLRLMultiAttribute* _this);

OS_API void
DMM_DLRLMultiAttribute_setIndexField(
    DMM_DLRLMultiAttribute* _this,
    DMM_DCPSField* indexField);

OS_API DMM_DCPSField*
DMM_DLRLMultiAttribute_getIndexField(
    DMM_DLRLMultiAttribute* _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DLRL_MULTI_ATTRIBUTE_H */
