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
/*  c includes */
#include <string.h>
#include <stdio.h>
#include <assert.h>

/*  os abstraction layer includes */
#include "os_heap.h"
#include "os_stdlib.h"

/*  DLRL includes */
#include "DLRL_Util.h"
#include "DLRL_Report.h"
#include "Coll_Defs.h"
#include "DMM_InheritanceTable.h"
#include "DMM_DLRLRelation.h"
#include "DMM_DCPSField.h"
#include "DMM_DCPSTopic.h"

#define ENTITY_NAME "DLRL Meta Model DLRL Relation"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;
static LOC_string addError = "Unable to add";

void
DMM_DLRLRelation_init(
   DMM_DLRLRelation* _this,
   DLRL_Exception* exception,
   LOC_long classID,
   LOC_boolean isComposition,
   LOC_string name,
   LOC_string typeName,
   LOC_string associatedRelationName,
   DMM_DLRLClass* owner,
   LOC_boolean isOptional)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* Set everything to default values */
    _this->classID          = classID;
    _this->type             = NULL;
    _this->associatedWith   = NULL;
    _this->associatedRelationName = NULL;
    _this->targetTopicName  = NULL;
    _this->name             = NULL;
    _this->typeName         = NULL;
    _this->targetTopicName  = NULL;
    _this->targetTopic      = NULL;
    _this->ownerTopic       = NULL;
    _this->hasSharedKeys    = FALSE;
    _this->owner            = owner;
    _this->isComposition    = isComposition;
    _this->isOptional       = isOptional;
    _this->immutable        = FALSE;
    _this->validityField    = NULL;
    Coll_List_init(&(_this->targetKeyNames));
    Coll_List_init(&(_this->ownerKeys));
    Coll_List_init(&(_this->targetKeys));

    DLRL_STRDUP(_this->name, name, exception, "%s '%s::%s' name", allocError,
        DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    DLRL_STRDUP(_this->typeName, typeName, exception, "%s '%s::%s' typeName '%s'", allocError,
        DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name),  DLRL_VALID_NAME(typeName));

    if(associatedRelationName){
        DLRL_STRDUP(_this->associatedRelationName, associatedRelationName, exception,
            "%s '%s::%s' associatedRelationName '%s'", allocError, DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)),
            DLRL_VALID_NAME(name),  DLRL_VALID_NAME(associatedRelationName));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DMM_DLRLRelation*
DMM_DLRLRelation_new(
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    DMM_DLRLClass* owner,
    LOC_boolean isOptional)
{
    DMM_DLRLRelation* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    assert(typeName);
    /* associatedRelationName may be null */
    assert(owner);

    DLRL_ALLOC(_this, DMM_DLRLRelation, exception, "%s '%s::%s'", allocError,
        DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    DMM_DLRLRelation_init(_this, exception, DMM_DLRL_RELATION_CLASS,
        isComposition, name, typeName, associatedRelationName,owner, isOptional);

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);
    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DLRLRelation_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DLRLRelation_delete(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if (_this){
        if(_this->name){
            os_free(_this->name);
            _this->name = NULL;
        }
        if(_this->typeName){
            os_free(_this->typeName);
            _this->typeName = NULL;
        }
        if(_this->targetTopicName){
            os_free(_this->targetTopicName);
            _this->targetTopicName = NULL;
        }
        if(_this->associatedRelationName){
            os_free(_this->associatedRelationName);
            _this->associatedRelationName = NULL;
        }
        while(Coll_List_getNrOfElements(&(_this->ownerKeys)) > 0){
            Coll_List_popBack(&(_this->ownerKeys));
            /* not deleting anything, this class does not have that responsibility. */
        }
        while(Coll_List_getNrOfElements(&(_this->targetKeys)) > 0){
            Coll_List_popBack(&(_this->targetKeys));
            /* not deleting anything, this class does not have that responsibility. */
        }
        if(_this->validityField){
            _this->validityField = NULL;
            /* dont delete, this class does not have that responsibility */
        }
        while(Coll_List_getNrOfElements(&(_this->targetKeyNames)) > 0){
            LOC_string aTargetKeyName = (LOC_string)Coll_List_popBack(&(_this->targetKeyNames));
            os_free(aTargetKeyName);
        }
        /* not responsibility of dlrl relation to delete type, associatedWith and owner pointer contents */
        /* So not doing anything with those pointers here. */
        /* delete the struct itself */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DMM_DLRLRelation_isOptional(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->isOptional;
}

LOC_long
DMM_DLRLRelation_getClassID(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->classID;
}

LOC_boolean
DMM_DLRLRelation_isComposition(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->isComposition;
}

LOC_string
DMM_DLRLRelation_getName(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->name;
}

LOC_string
DMM_DLRLRelation_getTypeName(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->typeName;
}

void
DMM_DLRLRelation_setAssociatedWith(
    DMM_DLRLRelation* _this,
    DMM_DLRLRelation* associatedWith)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(associatedWith);

    _this->associatedWith = associatedWith;
    DLRL_INFO(INF_EXIT);
}

DMM_DLRLRelation*
DMM_DLRLRelation_getAssociatedWith(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->associatedWith;
}

void
DMM_DLRLRelation_setOwner(
    DMM_DLRLRelation* _this,
    DMM_DLRLClass* owner)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(owner);

    _this->owner = owner;
    DLRL_INFO(INF_EXIT);
}

DMM_DLRLClass*
DMM_DLRLRelation_getOwner(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

void
DMM_DLRLRelation_setType(
    DMM_DLRLRelation* _this,
    DMM_DLRLClass* type)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(type);

    _this->type = type;
    DLRL_INFO(INF_EXIT);
}

DMM_DLRLClass*
DMM_DLRLRelation_getType(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->type;
}

Coll_List*
DMM_DLRLRelation_getTargetKeyNames(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->targetKeyNames);
}

void
DMM_DLRLRelation_addTargetKeyName(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    LOC_string targetKeyName)
{
    LOC_string name = NULL;
    LOC_long errorCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetKeyName);

    DLRL_STRDUP(name, targetKeyName, exception, "%s targetKeyName '%s' for %s '%s::%s'",
        allocError, DLRL_VALID_NAME(targetKeyName), ENTITY_NAME,
        DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->owner)), DLRL_VALID_NAME(_this->name));

    errorCode = Coll_List_pushBack(&(_this->targetKeyNames), name);
    if(errorCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s targetKeyName '%s' to %s '%s::%s'",
            addError, DLRL_VALID_NAME(targetKeyName), ENTITY_NAME,
            DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->owner)),
            DLRL_VALID_NAME(_this->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

Coll_List*
DMM_DLRLRelation_getOwnerKeys(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->ownerKeys);
}

void
DMM_DLRLRelation_setImmutability(
    DMM_DLRLRelation* _this,
    LOC_boolean immutable)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->immutable = immutable;

    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLRelation_setValidityField(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* validityField)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(validityField);
    assert(_this->ownerTopic);

    _this->validityField = validityField;
    DMM_DCPSTopic_addValidityField(_this->ownerTopic, exception, validityField);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLRelation_addOwnerKey(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* ownerKey)
{
    LOC_long errorCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(ownerKey);

    errorCode = Coll_List_pushBack(&(_this->ownerKeys), ownerKey);
    if(errorCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s ownerKey '%s' to %s '%s::%s'",
            addError, DLRL_VALID_NAME(DMM_DCPSField_getName(ownerKey)),
            ENTITY_NAME, DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->owner)),
            DLRL_VALID_NAME(_this->name));
    }
    if((!_this->hasSharedKeys) && (DMM_DCPSField_getFieldType(ownerKey) == DMM_KEYTYPE_SHARED_KEY)){
        _this->hasSharedKeys = TRUE;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DMM_DLRLRelation_hasSharedKeys(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->hasSharedKeys;
}

Coll_List*
DMM_DLRLRelation_getTargetKeys(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->targetKeys);
}

void
DMM_DLRLRelation_addTargetKey(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* targetKey)
{
    LOC_long errorCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetKey);

    errorCode = Coll_List_pushBack(&(_this->targetKeys), targetKey);
    if(errorCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s targetKey '%s' to %s '%s::%s'",
            addError, DLRL_VALID_NAME(DMM_DCPSField_getName(targetKey)),
            ENTITY_NAME, DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->owner)),
            DLRL_VALID_NAME(_this->name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLRelation_setTargetTopic(
    DMM_DLRLRelation* _this,
    DMM_DCPSTopic* targetTopic)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(targetTopic);

    _this->targetTopic = targetTopic;
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLRelation_getTargetTopic(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->targetTopic;
}

void
DMM_DLRLRelation_setOwnerTopic(
    DMM_DLRLRelation* _this,
    DMM_DCPSTopic* ownerTopic)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(ownerTopic);

    _this->ownerTopic = ownerTopic;
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLRelation_getOwnerTopic(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ownerTopic;
}

void
DMM_DLRLRelation_setTargetTopicName(
    DMM_DLRLRelation* _this,
    DLRL_Exception* exception,
    LOC_string targetTopicName)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetTopicName);

    DLRL_STRDUP(_this->targetTopicName, targetTopicName, exception, "%s targetTopicName '%s' for %s '%s::%s'",
            allocError, DLRL_VALID_NAME(targetTopicName), ENTITY_NAME,
            DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->owner)), DLRL_VALID_NAME(_this->name));

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_string
DMM_DLRLRelation_getTargetTopicName(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->targetTopicName;
}

LOC_string
DMM_DLRLRelation_getAssociatedRelationName(
    DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->associatedRelationName;
}

 DMM_DCPSField*
 DMM_DLRLRelation_getValidityField(
     DMM_DLRLRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->validityField;
}
