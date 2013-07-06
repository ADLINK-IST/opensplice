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
/*  C includes */
#include <assert.h>

/*  DCPS includes */
#include "os_heap.h"

/*  DLRL includes */
#include "DLRL_Report.h"
#include "Coll_Compare.h"
#include "Coll_Iter.h"

struct Coll_Iter_s {
    Coll_Iter *_next;
    Coll_Iter *_prev;
    void      *_object;
};

Coll_Iter *
Coll_Iter_new(
    void)
{
    Coll_Iter *_this;

    DLRL_INFO(INF_ENTER);
    _this = os_malloc(sizeof(Coll_Iter));

    if (_this){
        _this->_next = NULL;
        _this->_prev = NULL;
        _this->_object = NULL;
        DLRL_INFO(INF_ENTITY, "created Iter, address = %p", _this);
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

long
Coll_Iter_delete(
    Coll_Iter *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_free(_this);
    DLRL_INFO(INF_ENTITY, "deleted Iter, address = %p", _this);
    DLRL_INFO(INF_EXIT);
    return COLL_OK;
}

Coll_Iter *
Coll_Iter_getNext(
    Coll_Iter *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->_next;
}

void
Coll_Iter_setNext(
    Coll_Iter *_this,
    Coll_Iter *next)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    _this->_next = next;
}

Coll_Iter *
Coll_Iter_getPrev(
    Coll_Iter *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* next may be null */

    DLRL_INFO(INF_EXIT);
    return _this->_prev;
}

void
Coll_Iter_setPrev(
    Coll_Iter *_this,
    Coll_Iter *prev)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* prev may be null */

    DLRL_INFO(INF_EXIT);
    _this->_prev = prev;
}

void *
Coll_Iter_getObject(
    Coll_Iter *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->_object;
}

void
Coll_Iter_setObject(
    Coll_Iter *_this,
    void *object)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* object may be null */

    DLRL_INFO(INF_EXIT);
    _this->_object = object;
}
