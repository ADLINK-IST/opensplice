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
#ifndef DLRL_META_MODEL_DCPS_FIELD_H
#define DLRL_META_MODEL_DCPS_FIELD_H

/* DLRL meta model includes */
#include "DMM_Types.h"
#include "DMM_KeyType.h"
#include "DMM_AttributeType.h"

/* DCPS database includes */
#include "c_field.h"

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

struct DMM_DCPSField_s {
    LOC_string name;
    DMM_KeyType fieldType;
    DMM_AttributeType type;
    DMM_DCPSTopic* owner;
    c_field databaseField;
    LOC_long udIndex;/* user defined index for this field */
};

OS_API DMM_DCPSField*
DMM_DCPSField_new(
    DLRL_Exception* exception,
    LOC_string name,
    DMM_KeyType fieldType,
    DMM_AttributeType type,
    DMM_DCPSTopic* owner);

OS_API void
DMM_DCPSField_delete(
    DMM_DCPSField* _this);

OS_API LOC_string
DMM_DCPSField_getName(
    DMM_DCPSField* _this);

OS_API DMM_KeyType
DMM_DCPSField_getFieldType(
    DMM_DCPSField* _this);

OS_API DMM_AttributeType
DMM_DCPSField_getAttributeType(
    DMM_DCPSField* _this);

OS_API DMM_DCPSTopic*
DMM_DCPSField_getOwner(
    DMM_DCPSField* _this);

OS_API void
DMM_DCPSField_setDatabaseField(
    DMM_DCPSField* _this,
    c_field databaseField);

OS_API c_field
DMM_DCPSField_getDatabaseField(
    DMM_DCPSField* _this);

OS_API void
DMM_DCPSField_setUserDefinedIndex(
    DMM_DCPSField* _this,
    LOC_long udIndex);

OS_API LOC_long
DMM_DCPSField_getUserDefinedIndex(
    DMM_DCPSField* _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DCPS_FIELD_H */
