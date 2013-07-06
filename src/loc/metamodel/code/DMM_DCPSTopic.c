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
/* c includes */
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* os abstraction layer includes */
#include "os_heap.h"
#include "os_stdlib.h"

/* DLRL includes */
#include "DLRL_Report.h"
#include "Coll_Defs.h"
#include "DMM_DCPSField.h"
#include "DMM_DCPSTopic.h"
#include "DLRL_Util.h"

#define ENTITY_NAME "DLRL Meta Model DCPS Topic"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;
static LOC_string addError = "Unable to add";

static LOC_boolean DMM_DCPSTopic_canAddKeyField(DMM_DCPSTopic* _this, DMM_DCPSField* keyField);

static LOC_boolean DMM_DCPSTopic_canAddForeignKeyField(DMM_DCPSTopic* _this, DMM_DCPSField* foreignKeyField);

DMM_DCPSTopic*
DMM_DCPSTopic_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName)
{
    DMM_DCPSTopic* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    /* typeName may be null */

    DLRL_ALLOC(_this, DMM_DCPSTopic, exception,  "%s '%s'", allocError, DLRL_VALID_NAME(name));

     _this->name = NULL;
    _this->typeName = NULL;
    _this->size = 0;/* will be determined at registration time.   */
    Coll_List_init(&(_this->fields));
    Coll_List_init(&(_this->keyFields));
    Coll_List_init(&(_this->foreignKeyFields));
    Coll_List_init(&(_this->validityFields));

    DLRL_STRDUP(_this->name, name, exception, "%s '%s' name",
        allocError, DLRL_VALID_NAME(name));

    DLRL_STRDUP(_this->typeName, typeName, exception, "%s '%s' type name",
        allocError , DLRL_VALID_NAME(name));

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DCPSTopic_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DCPSTopic_delete(
    DMM_DCPSTopic* _this)
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
            _this->name = NULL;
        }
        while(Coll_List_getNrOfElements(&(_this->keyFields)) > 0){
            Coll_List_popBack(&(_this->keyFields));
            /* dont delete the field, key fields is a subset of regular fields. the fields will be deleted in */
            /* the second next while loop. Thus ignore return value from the pop back. */
        }
        while(Coll_List_getNrOfElements(&(_this->foreignKeyFields)) > 0){
            Coll_List_popBack(&(_this->foreignKeyFields));
            /* dont delete the field, foreignKeyFields is a subset of regular fields. the fields will be deleted in */
            /* the next while loop. Thus ignore return value from the pop back. */
        }
        while(Coll_List_getNrOfElements(&(_this->validityFields)) > 0){
            Coll_List_popBack(&(_this->validityFields));
            /* dont delete the field, validityFields is a subset of regular fields. the fields will be deleted in */
            /* the next while loop. Thus ignore return value from the pop back. */
        }
        while(Coll_List_getNrOfElements(&(_this->fields)) > 0){
            DMM_DCPSField* aField = (DMM_DCPSField*)Coll_List_popBack(&(_this->fields));
            DMM_DCPSField_delete(aField);
        }
        /* delete the struct itself */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_char*
DMM_DCPSTopic_getTopicName(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->name;
}

LOC_char*
DMM_DCPSTopic_getTopicTypeName(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->typeName;
}

void
DMM_DCPSTopic_setSize(
    DMM_DCPSTopic* _this,
    LOC_long size)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(size >=0);

    _this->size = size;
    DLRL_INFO(INF_EXIT);
}

LOC_long
DMM_DCPSTopic_getSize(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->size;
}

Coll_List*
DMM_DCPSTopic_getFields(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->fields);
}

/* only altered code. */
void
DMM_DCPSTopic_addField(
    DMM_DCPSTopic* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* field)
{
    LOC_long returnCode;
    DMM_KeyType type;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(field);

    type = DMM_DCPSField_getFieldType(field);
    assert(type < DMM_KeyType_elements);
    if ((type == DMM_KEYTYPE_KEY) || (type == DMM_KEYTYPE_SHARED_KEY)){
        if(DMM_DCPSTopic_canAddKeyField(_this, field)){
            returnCode = Coll_List_pushBack(&(_this->keyFields), (void*)field);
            if(returnCode != COLL_OK){
                DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                    "%s key field '%s' to %s '%s'", addError,
                    DLRL_VALID_NAME(DMM_DCPSField_getName(field)),
                    ENTITY_NAME, DLRL_VALID_NAME(_this->name));
            }
            assert(Coll_List_getNrOfElements(&(_this->keyFields)) > 0);
            DMM_DCPSField_setUserDefinedIndex(field, (LOC_long)(Coll_List_getNrOfElements(&(_this->keyFields)) -1));
        } else {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to add key field '%s' to %s '%s'. A key field with this name already exists!",
                DLRL_VALID_NAME(DMM_DCPSField_getName(field)), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
        }
    } else if(type == DMM_KEYTYPE_FOREIGN_KEY){
        if(DMM_DCPSTopic_canAddForeignKeyField(_this, field)){
            returnCode = Coll_List_pushBack(&(_this->foreignKeyFields), (void*)field);
            if(returnCode != COLL_OK){
                DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                    "%s foreign key field '%s' to %s '%s'", addError,
                    DLRL_VALID_NAME(DMM_DCPSField_getName(field)),
                    ENTITY_NAME, DLRL_VALID_NAME(_this->name));
            }
            assert(Coll_List_getNrOfElements(&(_this->foreignKeyFields)) > 0);
            DMM_DCPSField_setUserDefinedIndex(field, (LOC_long)(Coll_List_getNrOfElements(&(_this->foreignKeyFields)) -1));
        } else {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to add foreign key field '%s' to %s '%s'. A foreign key field with this name already exists!",
                DLRL_VALID_NAME(DMM_DCPSField_getName(field)), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
        }
    }
    if(NULL != DMM_DCPSTopic_findFieldByName(_this, DMM_DCPSField_getName(field))){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s field '%s' to %s '%s'. A field with that name already exists!", addError,
            DLRL_VALID_NAME(DMM_DCPSField_getName(field)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    returnCode = Coll_List_pushBack(&(_this->fields), (void*)field);
    if (returnCode != COLL_OK) {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s field '%s' to %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSField_getName(field)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DMM_DCPSTopic_addValidityField(
    DMM_DCPSTopic* _this,
    DLRL_Exception* exception,
    DMM_DCPSField* validityField)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(validityField);
    assert(DMM_DCPSField_getFieldType(validityField) == DMM_KEYTYPE_NORMAL);

    returnCode = Coll_List_pushBack(&(_this->validityFields), (void*)validityField);
    if (returnCode != COLL_OK) {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "%s (validity) field '%s' to %s '%s'", addError,
            DLRL_VALID_NAME(DMM_DCPSField_getName(validityField)),
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    DMM_DCPSField_setUserDefinedIndex(validityField, (LOC_long)(Coll_List_getNrOfElements(&(_this->validityFields)) -1));

    DLRL_Exception_EXIT(exception);

    DLRL_INFO(INF_EXIT);
}

Coll_List*
DMM_DCPSTopic_getValidityFields(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->validityFields);
}

Coll_List*
DMM_DCPSTopic_getForeignKeyFields(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->foreignKeyFields);
}

LOC_boolean
DMM_DCPSTopic_canAddKeyField(
    DMM_DCPSTopic* _this,
    DMM_DCPSField* keyField)
{
    LOC_boolean returnValue = TRUE;
    Coll_Iter* iterator = NULL;
    DMM_DCPSField* aKeyField = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(keyField);

    iterator = Coll_List_getFirstElement(&(_this->keyFields));
    while(iterator && returnValue){
        aKeyField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DCPSField_getName(keyField), DMM_DCPSField_getName(aKeyField))){
            returnValue = FALSE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_INFO(INF_EXIT);
    return returnValue;
}

LOC_boolean
DMM_DCPSTopic_canAddForeignKeyField(
    DMM_DCPSTopic* _this,
    DMM_DCPSField* foreignKeyField)
{
    LOC_boolean returnValue = TRUE;
    Coll_Iter* iterator = NULL;
    DMM_DCPSField* aForeignKeyField = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(foreignKeyField);

    iterator = Coll_List_getFirstElement(&(_this->foreignKeyFields));
    while(iterator && returnValue){
        aForeignKeyField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DMM_DCPSField_getName(foreignKeyField), DMM_DCPSField_getName(aForeignKeyField))){
            returnValue = FALSE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_INFO(INF_EXIT);
    return returnValue;
}

Coll_List*
DMM_DCPSTopic_getKeyFields(
    DMM_DCPSTopic* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->keyFields);
}

DMM_DCPSField*
DMM_DCPSTopic_findFieldByName(
    DMM_DCPSTopic* _this,
    LOC_string fieldName)
{
    LOC_boolean found = FALSE;
    DMM_DCPSField* aField = NULL;
    DMM_DCPSField* tempField = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(fieldName);

    iterator = Coll_List_getFirstElement(&(_this->fields));
    while(iterator && !found){
        tempField = (DMM_DCPSField*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(fieldName, DMM_DCPSField_getName(tempField))){
            found = TRUE;
            aField = tempField;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_INFO(INF_EXIT);
    return aField;
}
