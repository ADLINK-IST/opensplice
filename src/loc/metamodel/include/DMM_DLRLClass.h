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
#ifndef DLRL_META_MODEL_DLRL_CLASS_H
#define DLRL_META_MODEL_DLRL_CLASS_H

/* DLRL meta model includes */
#include "DMM_Types.h"
#include "DMM_Mapping.h"

/* Collection includes */
#include "Coll_Set.h"
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

struct DMM_DLRLClass_s {
    LOC_string name;
    LOC_string parentName;/* may be NULL */
    DMM_Mapping mapping;
    DMM_DCPSTopic* mainTopic;
    DMM_DCPSTopic* extensionTopic;
    Coll_Set placeTopics;
    Coll_Set multiPlaceTopics;
    Coll_List singleRelations;
    Coll_List multiRelations;
    Coll_Set attributes;
    Coll_Set topics;
    void* userData;
    LOC_unsigned_long nrOfMandatoryRelations;
};

OS_API DMM_DLRLClass*
DMM_DLRLClass_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string parentName,
    DMM_Mapping mapping,
    void* userData);

OS_API void
DMM_DLRLClass_delete(
    DMM_DLRLClass* _this);

OS_API LOC_string
DMM_DLRLClass_getName(
    DMM_DLRLClass* _this);

OS_API LOC_string
DMM_DLRLClass_getParentName(
    DMM_DLRLClass* _this);

OS_API DMM_Mapping
DMM_DLRLClass_getMapping(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_setMainTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* mainTopic);

OS_API DMM_DCPSTopic*
DMM_DLRLClass_getMainTopic(
    DMM_DLRLClass* _this);

OS_API Coll_Set*
DMM_DLRLClass_getTopics(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_setExtensionTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* extensionTopic);

OS_API DMM_DCPSTopic*
DMM_DLRLClass_getExtensionTopic(
    DMM_DLRLClass* _this);

OS_API Coll_Set*
DMM_DLRLClass_getPlaceTopics(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_addPlaceTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* placeTopic);

OS_API Coll_Set*
DMM_DLRLClass_getMultiPlaceTopics(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_addMultiPlaceTopic(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DCPSTopic* multiPlaceTopic);

OS_API Coll_List*
DMM_DLRLClass_getMultiRelations(
    DMM_DLRLClass* _this);

OS_API Coll_List*
DMM_DLRLClass_getRelations(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_addRelation(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DLRLRelation* relation);

OS_API Coll_Set*
DMM_DLRLClass_getAttributes(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_addAttribute(
    DMM_DLRLClass* _this,
    DLRL_Exception* exception,
    DMM_DLRLAttribute* attribute);

OS_API DMM_DCPSTopic*
DMM_DLRLClass_findTopicByName(
    DMM_DLRLClass* _this,
    LOC_string topicName);

OS_API DMM_DCPSTopic*
DMM_DLRLClass_findMultiPlaceTopicByName(
    DMM_DLRLClass* _this,
    LOC_string topicName);

OS_API DMM_DLRLAttribute*
DMM_DLRLClass_findAttributeByName(
    DMM_DLRLClass* _this,
    LOC_string attributeName);

OS_API DMM_DLRLRelation*
DMM_DLRLClass_findRelationByName(
    DMM_DLRLClass* _this,
    const char* relationName);

OS_API DMM_DLRLMultiRelation*
DMM_DLRLClass_findMultiRelationByName(
    DMM_DLRLClass* _this,
    LOC_string relationName);

OS_API DMM_DLRLRelation*
DMM_DLRLClass_getSingleRelation(
    DMM_DLRLClass* _this,
    LOC_unsigned_long relationIndex);

OS_API void*
DMM_DLRLClass_getUserData(
    DMM_DLRLClass* _this);

OS_API LOC_unsigned_long
DMM_DLRLClass_getNrOfMandatorySingleRelations(
    DMM_DLRLClass* _this);

OS_API void
DMM_DLRLClass_calculateNrOfMandatorySingleRelations(
    DMM_DLRLClass* _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_META_MODEL_DLRL_CLASS_H */
