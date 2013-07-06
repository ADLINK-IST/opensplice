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

/*  DLRL  */
#include "DLRL_Util.h"
#include "DLRL_Report.h"
#include "DMM_InheritanceTable.h"
#include "DMM_DLRLMultiRelation.h"

#define ENTITY_NAME "DLRL Meta Model DLRL MultiRelation"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;
static LOC_string addError = "Unable to add";
DMM_DLRLMultiRelation*
DMM_DLRLMultiRelation_new(
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    DMM_Basis basis,
    DMM_DLRLClass* owner,
    LOC_unsigned_long index,
    LOC_boolean isOptional)
{
    DMM_DLRLMultiRelation* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    assert(typeName);
    /* associatedRelationName may be null */
    assert(owner);
    assert(basis < DMM_Basis_elements);

    DLRL_ALLOC(_this, DMM_DLRLMultiRelation, exception, "%s '%s::%s'", allocError,
        DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    /* initialise the internal DMM_DLRLRelation struct */
    DMM_DLRLRelation_init(&(_this->base), exception, DMM_DLRL_MULTI_RELATION_CLASS,
        isComposition, name, typeName, associatedRelationName,owner, isOptional);

    /* initialise variables specific to DMM_DLRLMultiRelation struct */
    _this->basis = basis;
    _this->relationTopicIndexField = NULL;
    _this->relationTopic = NULL;
    _this->index = index;
    Coll_List_init(&(_this->relationTopicOwnerFields));
    Coll_List_init(&(_this->relationTopicTargetFields));

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DLRLMultiRelation_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DLRLMultiRelation_delete(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if (_this){
        if(_this->base.name){
            os_free(_this->base.name);
            _this->base.name = NULL;
        }
        if(_this->base.typeName){
            os_free(_this->base.typeName);
            _this->base.typeName = NULL;
        }
        if(_this->base.targetTopicName){
            os_free(_this->base.targetTopicName);
            _this->base.targetTopicName = NULL;
        }
        if(_this->base.associatedRelationName){
            os_free(_this->base.associatedRelationName);
            _this->base.associatedRelationName = NULL;
        }
        while(Coll_List_getNrOfElements(&(_this->base.ownerKeys)) > 0){
            Coll_List_popBack(&(_this->base.ownerKeys));
            /* not deleting anything, this class does not have that responsibility. */
        }
        while(Coll_List_getNrOfElements(&(_this->relationTopicOwnerFields)) > 0){
            Coll_List_popBack(&(_this->relationTopicOwnerFields));
            /* not deleting anything, this class does not have that responsibility. */
        }
        while(Coll_List_getNrOfElements(&(_this->relationTopicTargetFields)) > 0){
            Coll_List_popBack(&(_this->relationTopicTargetFields));
            /* not deleting anything, this class does not have that responsibility. */
        }
        while(Coll_List_getNrOfElements(&(_this->base.targetKeys)) > 0){
            Coll_List_popBack(&(_this->base.targetKeys));
            /* not deleting anything, this class does not have that responsibility. */
        }
        if(_this->base.validityField){
            _this->base.validityField = NULL;
            /* dont delete, this class does not have that responsibility */
        }
        while(Coll_List_getNrOfElements(&(_this->base.targetKeyNames)) > 0){
            LOC_string aTargetKeyName = (LOC_string)Coll_List_popBack(&(_this->base.targetKeyNames));
            os_free(aTargetKeyName);
        }
         /* not responsibility of dlrl relation to delete type, associatedWith, owner  and relationTopicIndexField */
         /* pointer contents. So not doing anything with those pointers here. */
        /* delete the struct itself */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

DMM_Basis
DMM_DLRLMultiRelation_getBasis(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->basis;
}

DMM_DCPSField*
DMM_DLRLMultiRelation_getIndexField(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->relationTopicIndexField;
}

Coll_List*
DMM_DLRLMultiRelation_getRelationTopicOwnerFields(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->relationTopicOwnerFields);
}

Coll_List*
DMM_DLRLMultiRelation_getRelationTopicTargetFields(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->relationTopicTargetFields);
}

void
DMM_DLRLMultiRelation_setRelationTopic(
    DMM_DLRLMultiRelation* _this,
    DMM_DCPSTopic* relationTopic)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(relationTopic);

    _this->relationTopic = relationTopic;

    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLMultiRelation_addRelationTopicOwnerField(
    DMM_DLRLMultiRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* ownerField)
{
    LOC_long errorCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(ownerField);

    errorCode = Coll_List_pushBack(&(_this->relationTopicOwnerFields), ownerField);
    if(errorCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s ownerField '%s' to %s '%s::%s'",
            addError, DLRL_VALID_NAME(DMM_DCPSField_getName(ownerField)),
            ENTITY_NAME, DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->base.owner)),
            DLRL_VALID_NAME(_this->base.name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLMultiRelation_addRelationTopicTargetField(
    DMM_DLRLMultiRelation* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* targetField)
{
    LOC_long errorCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(targetField);

    errorCode = Coll_List_pushBack(&(_this->relationTopicTargetFields), targetField);
    if(errorCode != COLL_OK){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s targetField '%s' to %s '%s::%s'",
            addError, DLRL_VALID_NAME(DMM_DCPSField_getName(targetField)),
            ENTITY_NAME, DLRL_VALID_NAME(DMM_DLRLClass_getName(_this->base.owner)),
            DLRL_VALID_NAME(_this->base.name));
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DLRLMultiRelation_setRelationTopicIndexField(
    DMM_DLRLMultiRelation* _this,
    DMM_DCPSField* indexField)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(indexField);

    _this->relationTopicIndexField = indexField;

    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLMultiRelation_getRelationTopic(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->relationTopic;
}

LOC_unsigned_long
DMM_DLRLMultiRelation_us_getIndex(
    DMM_DLRLMultiRelation* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->index;
}
