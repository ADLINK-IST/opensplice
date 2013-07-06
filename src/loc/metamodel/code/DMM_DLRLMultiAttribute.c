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

/*  DLRL */
#include "DLRL_Util.h"
#include "DLRL_Report.h"
#include "DMM_InheritanceTable.h"
#include "DMM_DLRLMultiAttribute.h"

#define ENTITY_NAME "DLRL Meta Model DLRL MultiAttribute"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

DMM_DLRLMultiAttribute*
DMM_DLRLMultiAttribute_new(
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type,
    DMM_Basis basis,
    DMM_DLRLClass* owner)
{
    DMM_DLRLMultiAttribute* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(name);
    assert(type < DMM_AttributeType_elements);
    assert(basis < DMM_Basis_elements);
    assert(owner);

    DLRL_ALLOC(_this, DMM_DLRLMultiAttribute, exception, "%s '%s::%s'", allocError,
            DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    _this->base.classID     =   DMM_DLRL_MULTI_ATTRIBUTE_CLASS;
    _this->base.isImmutable =   isImmutable;
    _this->base.type        =   type;
    _this->base.owner       =   owner;
    _this->base.name        =   NULL;
    _this->base.field       =   NULL;
    _this->indexField       =   NULL;
    _this->basis            =   basis;

    DLRL_STRDUP(_this->base.name, name, exception, "%s name '%s::%s'", allocError,
            DLRL_VALID_NAME(DMM_DLRLClass_getName(owner)), DLRL_VALID_NAME(name));

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        DMM_DLRLMultiAttribute_delete(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DMM_DLRLMultiAttribute_delete(
    DMM_DLRLMultiAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if (_this){
        if(_this->base.name){
            os_free(_this->base.name);
            _this->base.name = NULL;
        }
        /* not responsibility of dlrl multi attribute to delete index field pointer content */
        /* So not doing anything with that pointer here. */
        /* delete the struct itself */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }
    DLRL_INFO(INF_EXIT);
}

DMM_Basis
DMM_DLRLMultiAttribute_getBasis(
    DMM_DLRLMultiAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->basis;
}

void
DMM_DLRLMultiAttribute_setIndexField(
    DMM_DLRLMultiAttribute* _this,
    DMM_DCPSField* indexField)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(indexField);

    _this->indexField = indexField;
    DLRL_INFO(INF_EXIT);
}

DMM_DCPSField*
DMM_DLRLMultiAttribute_getIndexField(
    DMM_DLRLMultiAttribute* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->indexField;
}
