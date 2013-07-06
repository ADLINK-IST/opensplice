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
#ifndef DLRL_META_MODEL_DLRL_MULTI_RELATION_H
#define DLRL_META_MODEL_DLRL_MULTI_RELATION_H

#include "DMM_Types.h"
#include "DMM_Basis.h"
#include "DMM_DLRLRelation.h"
#include "DMM_DLRLClass.h"
#include "DMM_DCPSTopic.h"
#include "DMM_DCPSField.h"

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

struct DMM_DLRLMultiRelation_s{
    DMM_DLRLRelation base;
    DMM_Basis basis;
    DMM_DCPSTopic* relationTopic;
    DMM_DCPSField* relationTopicIndexField;
    Coll_List relationTopicOwnerFields;
    Coll_List relationTopicTargetFields;
    LOC_unsigned_long index;/* NOT IN DESIGN */
};

OS_API DMM_DLRLMultiRelation*
    DMM_DLRLMultiRelation_new(
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    DMM_Basis basis,
    DMM_DLRLClass* owner,
    LOC_unsigned_long index,
    LOC_boolean isOptional);

OS_API void
DMM_DLRLMultiRelation_delete(
    DMM_DLRLMultiRelation* _this);

OS_API DMM_Basis
DMM_DLRLMultiRelation_getBasis(
    DMM_DLRLMultiRelation* _this);

OS_API DMM_DCPSField*
DMM_DLRLMultiRelation_getIndexField(
    DMM_DLRLMultiRelation* _this);

OS_API Coll_List*
DMM_DLRLMultiRelation_getRelationTopicOwnerFields(
    DMM_DLRLMultiRelation* _this);

OS_API Coll_List*
DMM_DLRLMultiRelation_getRelationTopicTargetFields(
    DMM_DLRLMultiRelation* _this);

OS_API void
DMM_DLRLMultiRelation_setRelationTopic(
    DMM_DLRLMultiRelation* _this,
    DMM_DCPSTopic* relationTopic);

OS_API void
DMM_DLRLMultiRelation_addRelationTopicOwnerField(
    DMM_DLRLMultiRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* ownerField);

OS_API void
DMM_DLRLMultiRelation_addRelationTopicTargetField(
    DMM_DLRLMultiRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* targetField);

OS_API void
DMM_DLRLMultiRelation_setRelationTopicIndexField(
    DMM_DLRLMultiRelation* _this,
    DMM_DCPSField* indexField);

OS_API DMM_DCPSTopic*
DMM_DLRLMultiRelation_getRelationTopic(
    DMM_DLRLMultiRelation* _this);

OS_API LOC_unsigned_long
DMM_DLRLMultiRelation_us_getIndex(
    DMM_DLRLMultiRelation* _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DLRL_MULTI_RELATION_H */
