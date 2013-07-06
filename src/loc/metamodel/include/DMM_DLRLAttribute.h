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
#ifndef DLRL_META_MODEL_DLRL_ATTRIBUTE_H
#define DLRL_META_MODEL_DLRL_ATTRIBUTE_H

/* DLRL Metamodel includes */
#include "DMM_Types.h"
#include "DMM_AttributeType.h"
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

struct DMM_DLRLAttribute_s {
    LOC_long classID;
    LOC_boolean isImmutable;
    LOC_string name;
    DMM_AttributeType type;
    DMM_DLRLClass* owner;
    DMM_DCPSField* field;
    DMM_DCPSTopic* dcpsTopic;
};

OS_API DMM_DLRLAttribute*
DMM_DLRLAttribute_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type,
    DMM_DLRLClass* owner);

OS_API void
DMM_DLRLAttribute_delete(
    DMM_DLRLAttribute* _this);

OS_API LOC_long
DMM_DLRLAttribute_getClassID(
    DMM_DLRLAttribute* _this);

OS_API LOC_boolean
DMM_DLRLAttribute_isImmutable(
    DMM_DLRLAttribute* _this);

OS_API LOC_string
DMM_DLRLAttribute_getName(
    DMM_DLRLAttribute* _this);

OS_API DMM_AttributeType
DMM_DLRLAttribute_getAttributeType(
    DMM_DLRLAttribute* _this);

OS_API DMM_DLRLClass*
    DMM_DLRLAttribute_getOwner(
    DMM_DLRLAttribute* _this);

OS_API void
DMM_DLRLAttribute_setOwner(
    DMM_DLRLAttribute* _this,
    DMM_DLRLClass* owner);

OS_API DMM_DCPSField*
DMM_DLRLAttribute_getField(
    DMM_DLRLAttribute* _this);

OS_API void
DMM_DLRLAttribute_setField(
    DMM_DLRLAttribute* _this,
    DMM_DCPSField* field);

OS_API DMM_DCPSTopic*
DMM_DLRLAttribute_getTopic(
    DMM_DLRLAttribute* _this);

OS_API void
DMM_DLRLAttribute_setTopic(
    DMM_DLRLAttribute* _this,
    DMM_DCPSTopic* dcpsTopic);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DLRL_ATTRIBUTE_H */
