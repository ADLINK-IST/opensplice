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
#include "DMM_InheritanceTable.h"
#include "DMM_DLRLAttribute.h"

#define ENTITY_NAME "DLRL Meta Model DLRL Attribute"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

DMM_DLRLAttribute*
DMM_DLRLAttribute_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type,
    DMM_DLRLClass* owner)
{
    DMM_DLRLAttribute* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    assert(type < DMM_AttributeType_elements);
    assert(owner);

    DLRL_ALLOC(_this, DMM_DLRLAttribute, exception,  "%s '%s::%s'", allocError,
            DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    _this->classID      = DMM_DLRL_ATTRIBUTE_CLASS;
    _this->name         = NULL;
    _this->field        = NULL;
    _this->dcpsTopic    = NULL;
    _this->isImmutable  = isImmutable;
    _this->type         = type;
    _this->owner        = owner;

    DLRL_STRDUP(_this->name, name, exception, "%s name '%s::%s'", allocError,
            DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DLRLAttribute_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DLRLAttribute_delete(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */
    if (_this){
        if(_this->name){
            os_free(_this->name);
            _this->name = NULL;
        }
        /* not responsibility of dlrl attribute to delete owner and field pointer contents */
        /* So not doing anything with those pointers here. */
        /* delete the struct itself */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_long
DMM_DLRLAttribute_getClassID(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->classID;
}

LOC_boolean
DMM_DLRLAttribute_isImmutable(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->isImmutable;
}

LOC_string
DMM_DLRLAttribute_getName(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->name;
}

DMM_AttributeType
DMM_DLRLAttribute_getAttributeType(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->type;
}

DMM_DLRLClass*
DMM_DLRLAttribute_getOwner(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

void
DMM_DLRLAttribute_setOwner(
    DMM_DLRLAttribute* _this,
    DMM_DLRLClass* owner)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(owner);

    _this->owner = owner;
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSField*
DMM_DLRLAttribute_getField(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->field;
}

void
DMM_DLRLAttribute_setField(
    DMM_DLRLAttribute* _this,
    DMM_DCPSField* field)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(field);

    _this->field = field;
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSTopic*
DMM_DLRLAttribute_getTopic(
    DMM_DLRLAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->dcpsTopic;
}

void
DMM_DLRLAttribute_setTopic(
    DMM_DLRLAttribute* _this,
    DMM_DCPSTopic* dcpsTopic)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(dcpsTopic);

    _this->dcpsTopic = dcpsTopic;
    DLRL_INFO(INF_EXIT);
}
