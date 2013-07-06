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
#ifndef DLRL_META_MODEL_DCPS_TOPIC_H
#define DLRL_META_MODEL_DCPS_TOPIC_H

/* DLRL Metamodel includes */
#include "DMM_Types.h"

/* DCPS database include */
#include "c_metabase.h"

/* Collection includes */
#include "Coll_List.h"

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

struct DMM_DCPSTopic_s {
    LOC_char* name;
    LOC_char* typeName;
    LOC_long size;
    Coll_List fields;
    Coll_List keyFields;
    Coll_List foreignKeyFields;
    Coll_List validityFields;
/* NOT IN DESIGN    c_type databaseTopic; */
};

OS_API DMM_DCPSTopic*
DMM_DCPSTopic_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName);

OS_API void
DMM_DCPSTopic_delete(
    DMM_DCPSTopic* _this);

OS_API LOC_char*
DMM_DCPSTopic_getTopicName(
    DMM_DCPSTopic* _this);

OS_API LOC_char*
DMM_DCPSTopic_getTopicTypeName(
    DMM_DCPSTopic* _this);

OS_API void
DMM_DCPSTopic_setSize(
    DMM_DCPSTopic* _this,
    LOC_long size);

OS_API LOC_long
DMM_DCPSTopic_getSize(
    DMM_DCPSTopic* _this);

OS_API Coll_List*
    DMM_DCPSTopic_getFields(
    DMM_DCPSTopic* _this);

/* also implicity adds to key fields */
OS_API void
DMM_DCPSTopic_addField(
    DMM_DCPSTopic* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* field);

/* no add function counterpart, add to key fields done in the add field implicitly */
OS_API Coll_List*
DMM_DCPSTopic_getKeyFields(
    DMM_DCPSTopic* _this);

/* returns null if nothing found */
OS_API DMM_DCPSField*
DMM_DCPSTopic_findFieldByName(
    DMM_DCPSTopic* _this,
    LOC_string fieldName);

/* may return NIL */
/* NOT IN DESIGNc_type DMM_DCPSTopic_getDatabaseTopic(DMM_DCPSTopic* _this); */

/* NOT IN DESIGNvoid DMM_DCPSTopic_setDatabaseTopic(DMM_DCPSTopic* _this, c_type databaseTopic); */
/* NOT IN DESIGN*/
OS_API Coll_List*
DMM_DCPSTopic_getForeignKeyFields(
    DMM_DCPSTopic* _this);

/* NOT IN DESIGN*/
OS_API Coll_List*
DMM_DCPSTopic_getValidityFields(
    DMM_DCPSTopic* _this);

/* NOT IN DESIGN*/
OS_API void
DMM_DCPSTopic_addValidityField(
    DMM_DCPSTopic* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* validityField);
#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DCPS_TOPIC_H */
