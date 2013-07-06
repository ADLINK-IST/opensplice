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
#ifndef DLRL_META_MODEL_DLRL_RELATION_H
#define DLRL_META_MODEL_DLRL_RELATION_H

/* dlrl metamodel includes */
#include "DMM_Types.h"
#include "DMM_DLRLClass.h"

/* collection includes */
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

struct DMM_DLRLRelation_s{
    LOC_long classID;
    LOC_boolean isComposition;
    LOC_boolean immutable;
    LOC_string name;
    LOC_string typeName;
    LOC_string associatedRelationName;
    LOC_string targetTopicName;
    LOC_boolean hasSharedKeys;/* NOT IN DESIGN */
    DMM_DLRLRelation* associatedWith;
    DMM_DLRLClass* owner;
    DMM_DLRLClass* type;
    DMM_DCPSTopic* targetTopic;
    DMM_DCPSTopic* ownerTopic;
    Coll_List targetKeyNames;
    Coll_List ownerKeys;
    Coll_List targetKeys;
    LOC_boolean isOptional;
    DMM_DCPSField* validityField;/* NOT IN DESIGN */

};

OS_API void
DMM_DLRLRelation_init(
   DMM_DLRLRelation* _this,
   DLRL_Exception* exception,
   LOC_long classID,
   LOC_boolean isComposition,
   LOC_string name,
   LOC_string typeName,
   LOC_string associatedRelationName,
   DMM_DLRLClass* owner,
   LOC_boolean isOptional);

OS_API DMM_DLRLRelation*
DMM_DLRLRelation_new(
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    DMM_DLRLClass* owner,
    LOC_boolean isOptional);

OS_API void
DMM_DLRLRelation_delete(
    DMM_DLRLRelation* _this);

OS_API LOC_long
DMM_DLRLRelation_getClassID(
    DMM_DLRLRelation* _this);

OS_API LOC_boolean
DMM_DLRLRelation_isComposition(
    DMM_DLRLRelation* _this);

OS_API LOC_string
DMM_DLRLRelation_getName(
    DMM_DLRLRelation* _this);

OS_API LOC_string
DMM_DLRLRelation_getTypeName(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_setAssociatedWith(
    DMM_DLRLRelation* _this,
    DMM_DLRLRelation* associatedWith);

OS_API DMM_DLRLRelation*
DMM_DLRLRelation_getAssociatedWith(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_setOwner(
    DMM_DLRLRelation* _this,
    DMM_DLRLClass* owner);

OS_API DMM_DLRLClass*
DMM_DLRLRelation_getOwner(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_setType(
    DMM_DLRLRelation* _this,
    DMM_DLRLClass* type);

OS_API DMM_DLRLClass*
DMM_DLRLRelation_getType(
    DMM_DLRLRelation* _this);

OS_API Coll_List*
DMM_DLRLRelation_getTargetKeyNames(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_addTargetKeyName(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    LOC_string targetKeyName);

OS_API Coll_List*
DMM_DLRLRelation_getOwnerKeys(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_addOwnerKey(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* ownerKey);

/* not in design*/
OS_API void
DMM_DLRLRelation_setValidityField(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* validityField);

OS_API Coll_List*
DMM_DLRLRelation_getTargetKeys(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_addTargetKey(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* targetKey);

OS_API void
DMM_DLRLRelation_setTargetTopic(
    DMM_DLRLRelation* _this,
    DMM_DCPSTopic* targetTopic);

OS_API DMM_DCPSTopic*
DMM_DLRLRelation_getTargetTopic(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_setOwnerTopic(
    DMM_DLRLRelation* _this,
    DMM_DCPSTopic* ownerTopic);

OS_API DMM_DCPSTopic*
DMM_DLRLRelation_getOwnerTopic(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_setTargetTopicName(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    LOC_string targetTopicName);

OS_API LOC_string
DMM_DLRLRelation_getTargetTopicName(
    DMM_DLRLRelation* _this);

OS_API LOC_string
DMM_DLRLRelation_getAssociatedRelationName(
    DMM_DLRLRelation* _this);

OS_API void
DMM_DLRLRelation_setImmutability(
    DMM_DLRLRelation* _this,
    LOC_boolean immutable);

/* NOT IN DESIGN */
OS_API LOC_boolean
DMM_DLRLRelation_hasSharedKeys(
    DMM_DLRLRelation* _this);

/* NOT IN DESIGN */
OS_API DMM_DCPSField*
DMM_DLRLRelation_getValidityField(
    DMM_DLRLRelation* _this);

/* NOT IN DESIGN */
OS_API LOC_boolean
DMM_DLRLRelation_isOptional(
    DMM_DLRLRelation* _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DLRL_RELATION_H */
