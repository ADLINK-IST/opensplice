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
/* C includes */
#include <assert.h>

/* OS abstraction layer includes */
#include "os_heap.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_DCPSUtility.h"
#include "DK_ObjectHolder.h"

#define ENTITY_NAME "DLRL Kernel ObjectHolder"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_ObjectHolder_us_delete(
    DK_ObjectHolder* _this);


DK_ObjectHolder*
DK_ObjectHolder_newResolved(
    DLRL_Exception* exception,
    DK_Entity* owner,
    DK_ObjectAdmin* target,
    u_instanceHandle elementHandle,
    LOC_unsigned_long index)
{
    DK_ObjectHolder* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(owner);
    assert((u_instanceHandleIsNil(elementHandle)) || (!u_instanceHandleIsNil(elementHandle) &&
        ((DK_Entity_getClassID(owner) == DK_CLASS_SET_ADMIN) || (DK_Entity_getClassID(owner) == DK_CLASS_MAP_ADMIN))));
    /* target, values or element handle may all be nil */

    DLRL_ALLOC(_this, DK_ObjectHolder, exception, allocError);

    _this->owner = DK_Entity_ts_duplicate(owner);
    _this->elementHandle = elementHandle;
    _this->index = index;

    if(target)
    {
        assert(target->alive);
        _this->ref.target = (DK_ObjectAdmin*)DK_Entity_ts_duplicate((DK_Entity*)target);
    } else
    {
        _this->ref.target = NULL;
    }
    _this->arraySize = 0;
    _this->resolved = TRUE;
    _this->userData = NULL;
    _this->noWritersCount = 0;
    _this->disposedCount = 0;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return _this;
}


DK_ObjectHolder*
DK_ObjectHolder_newUnresolved(
    DLRL_Exception* exception,
    DK_Entity* owner,
    void* values,
    LOC_unsigned_long arraySize,
    u_instanceHandle elementHandle,
    LOC_unsigned_long index)
{
    DK_ObjectHolder* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(owner);
    assert((u_instanceHandleIsNil(elementHandle) && (DK_Entity_getClassID(owner) == DK_CLASS_OBJECT_ADMIN)) || (!u_instanceHandleIsNil(elementHandle) &&
        ((DK_Entity_getClassID(owner) == DK_CLASS_SET_ADMIN) || (DK_Entity_getClassID(owner) == DK_CLASS_MAP_ADMIN))));
    /* values or element handle may all be nil */

    DLRL_ALLOC(_this, DK_ObjectHolder, exception, allocError);

    _this->owner = DK_Entity_ts_duplicate(owner);
    _this->elementHandle = elementHandle;
    _this->ref.values = values;
    _this->arraySize = arraySize;
    _this->resolved = FALSE;
    _this->userData = NULL;
    _this->index = index;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_ObjectHolder_us_destroy(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DK_ObjectHolder_us_delete(_this);
    os_free(_this);

    DLRL_INFO(INF_EXIT);
}

LOC_long
DK_ObjectHolder_us_getNoWritersCount(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->noWritersCount;
}

void
DK_ObjectHolder_us_setNoWritersCount(
    DK_ObjectHolder* _this,
    LOC_long noWritersCount)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->noWritersCount = noWritersCount;

    DLRL_INFO(INF_EXIT);
}

LOC_long
DK_ObjectHolder_us_getDisposedCount(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->disposedCount;
}

void
DK_ObjectHolder_us_setDisposedCount(
    DK_ObjectHolder* _this,
    LOC_long disposedCount)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->disposedCount = disposedCount;

    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN - name changed */
void
DK_ObjectHolder_us_setHandle(
    DK_ObjectHolder* _this,
    u_instanceHandle elementHandle)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* element handle may be nil */
    _this->elementHandle = elementHandle;

    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHolder_us_setHandleNil(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* element handle may be nil */
    _this->elementHandle = DK_DCPSUtility_ts_getNilHandle();

    DLRL_INFO(INF_EXIT);
}

/* no duplicate done */
DK_ObjectAdmin*
DK_ObjectHolder_us_getTarget(
    DK_ObjectHolder* _this)
{
    DK_ObjectAdmin* target = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->resolved)
    {
        target = _this->ref.target;
    }

    DLRL_INFO(INF_EXIT);
    return target;
}

LOC_unsigned_long
DK_ObjectHolder_us_getIndex(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->index;
}

/* takes care of release and duplicate, may pass along a null value */
void
DK_ObjectHolder_us_setTarget(
    DK_ObjectHolder* _this,
    DK_ObjectAdmin* target)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* target may be null */

    if(_this->resolved)
    {
        if(_this->ref.target)
        {
            DK_Entity_ts_release((DK_Entity*)_this->ref.target);
            _this->ref.target = NULL;
        }/* else nothing to release */
    } else
    {
        DK_DCPSUtility_us_destroyValueArray(_this->ref.values, _this->arraySize);
        _this->ref.values = NULL;
    }

    if(target)
    {
        assert(target->alive);
        _this->ref.target = (DK_ObjectAdmin*)DK_Entity_ts_duplicate((DK_Entity*)target);
    }
    _this->resolved = TRUE;
    _this->arraySize = 0;

    DLRL_INFO(INF_EXIT);
}

/* may return null */
void*
DK_ObjectHolder_us_getValues(
    DK_ObjectHolder* _this)
{
    void* values = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(!(_this->resolved))
    {
        values = _this->ref.values;
    }

    DLRL_INFO(INF_EXIT);
    return values;
}

void
DK_ObjectHolder_us_setValues(
    DK_ObjectHolder*_this,
    void* values,
    LOC_unsigned_long arraySize)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* values may be null */

    if(_this->resolved)
    {
        if(_this->ref.target)
        {
            DK_Entity_ts_release((DK_Entity*)_this->ref.target);
        }
    } else
    {
        DK_DCPSUtility_us_destroyValueArray(_this->ref.values, _this->arraySize);
    }

    _this->ref.values = values;
    _this->arraySize = arraySize;
    _this->resolved = FALSE;

    DLRL_INFO(INF_EXIT);
}

LOC_unsigned_long
DK_ObjectHolder_us_getValuesSize(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->arraySize;
}

/* no duplicate done */
DK_Entity*
DK_ObjectHolder_us_getOwner(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

/* may return null */
u_instanceHandle
DK_ObjectHolder_us_getHandle(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->elementHandle;
}

LOC_boolean
DK_ObjectHolder_us_isResolved(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->resolved;
}

/* no duplicate / copy made, may return null. */
void*
DK_ObjectHolder_us_getUserData(
    DK_ObjectHolder* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->userData;
}

void
DK_ObjectHolder_us_setUserData(
    DK_ObjectHolder* _this,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* user data may be null */

    _this->userData = userData;
    DLRL_INFO(INF_EXIT);

}

void
DK_ObjectHolder_us_delete(
    DK_ObjectHolder* _this)
{
    void* oldUserData = NULL;
    DLRL_Exception exception;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_Exception_init(&exception);

    if(_this->owner)
    {
        DK_Entity_ts_release(_this->owner);
        _this->owner = NULL;
    }
    if(_this->resolved)
    {
        if(_this->ref.target)
        {

            DK_Entity_ts_release((DK_Entity*)_this->ref.target);
            _this->ref.target = NULL;
        }/* else nothing to do */
    } else
    {
        DK_DCPSUtility_us_destroyValueArray(_this->ref.values, _this->arraySize);
        _this->ref.values = NULL;
        _this->arraySize = 0;
    }
    if(!u_instanceHandleIsNil(_this->elementHandle))
    {
        /* report exception */
        oldUserData = DK_DCPSUtility_us_setUserDataBasedOnHandle(&exception, _this->elementHandle, NULL);
        if(exception.exceptionID != DLRL_NO_EXCEPTION)
        {
            DLRL_REPORT(REPORT_ERROR, "Failed to reset user data of an element handle. Exception: %s :: %s",
            DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
        }
        assert(oldUserData == _this || !oldUserData);
        _this->elementHandle = DK_DCPSUtility_ts_getNilHandle();
    }
    assert(!_this->userData);/* must have been reset before the delete was called. */

    DLRL_INFO(INF_EXIT);
}
