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

/*  dlrl includes */
#include "DLRL_Util.h"
#include "DLRL_Report.h"
#include "DMM_DCPSTopic.h"
#include "u_user.h"
#include "DMM_DCPSField.h"

#include "u_user.h"

#define ENTITY_NAME "DLRL Meta Model DCPS Field"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

DMM_DCPSField*
DMM_DCPSField_new(
    DLRL_Exception* exception,
    LOC_string name,
    DMM_KeyType fieldType,
    DMM_AttributeType type,
    DMM_DCPSTopic* owner)
{
    DMM_DCPSField* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    assert(fieldType < DMM_KeyType_elements);
    assert(type < DMM_AttributeType_elements);
    assert(owner);

    DLRL_ALLOC(_this, DMM_DCPSField, exception,  "%s '%s::%s'", allocError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(owner)), DLRL_VALID_NAME(name));

    _this->name = NULL;
    _this->fieldType = fieldType;
    _this->type = type;
    _this->owner = owner;
    _this->databaseField = NULL;
    _this->udIndex = -1;

    DLRL_STRDUP(_this->name, name, exception, "%s name '%s::%s'", allocError,
            DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(owner)), DLRL_VALID_NAME(name));

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DCPSField_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DCPSField_delete(
    DMM_DCPSField* _this)
{

    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */
    if (_this){
        if(_this->name){
            os_free(_this->name);
            _this->name = NULL;
        }
        if(_this->databaseField)
        {
            /* memory leak detected */
            DLRL_REPORT(
                REPORT_WARNING,
                "DLRL meta data 'DCPSField' is being deleted, but the link to the database "
                "has not been severed. This can result in a potential memory leak in "
                "shared memory.");
        }
        /* not responsibility of dcps field to delete owner pointer content */
        /* So not doing anything with that pointer here. */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_string
DMM_DCPSField_getName(
    DMM_DCPSField* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->name;
}

DMM_KeyType
DMM_DCPSField_getFieldType(
    DMM_DCPSField* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->fieldType;
}

DMM_AttributeType
DMM_DCPSField_getAttributeType(
    DMM_DCPSField* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->type;
}

DMM_DCPSTopic*
DMM_DCPSField_getOwner(
    DMM_DCPSField* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

void
DMM_DCPSField_setDatabaseField(
    DMM_DCPSField* _this,
    c_field databaseField)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /*databaseField may be null*/

    u_userFree(c_object(_this->databaseField));
    _this->databaseField = (c_field)u_userKeep(databaseField);
    DLRL_INFO(INF_EXIT);
}

c_field
DMM_DCPSField_getDatabaseField(
    DMM_DCPSField* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->databaseField;
}

void
DMM_DCPSField_setUserDefinedIndex(
    DMM_DCPSField* _this,
    LOC_long udIndex)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    _this->udIndex = udIndex;

    DLRL_INFO(INF_EXIT);
}

LOC_long
DMM_DCPSField_getUserDefinedIndex(
    DMM_DCPSField* _this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->udIndex;
}
