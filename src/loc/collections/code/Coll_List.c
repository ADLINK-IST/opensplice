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
#include "Coll_List.h"
#include "Coll_Iter.h"

Coll_List *
Coll_List_new(
    void)
{
    Coll_List *_this;

    DLRL_INFO(INF_ENTER);

    _this = os_malloc(sizeof(Coll_List));

    if (_this){
        Coll_List_init(_this);
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
Coll_List_init(
    Coll_List *_this)
{
    DLRL_INFO(INF_ENTER);

    assert(_this);

    _this->_nr_elements = 0;
    _this->_first_element = NULL;
    _this->_last_element = NULL;
    DLRL_INFO(INF_ENTITY, "created List, address = %p", _this);

    DLRL_INFO(INF_EXIT);
}

long
Coll_List_delete(
    Coll_List *_this)
{
    long retValue = COLL_ERROR_NOT_EMPTY;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    /*  only allowed if the list is empty */
    if(_this->_nr_elements == 0){
        /*  free the list header */
        os_free(_this);
        DLRL_INFO(INF_ENTITY, "deleted List, address = %p", _this);
        retValue = COLL_OK;
    }
    DLRL_INFO(INF_EXIT);
    return retValue;
}

unsigned long
Coll_List_getNrOfElements(
    const Coll_List *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->_nr_elements;
}

Coll_Iter *
Coll_List_getFirstElement(
    const Coll_List *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->_first_element;
}

Coll_Iter *
Coll_List_getLastElement(
    Coll_List *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->_last_element;
}

long
Coll_List_pushBack(
    Coll_List *_this,
    void *object)
{
    Coll_Iter *new_element;
    long retValue = COLL_ERROR_ALLOC;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* object may be null */

    new_element = Coll_Iter_new();

    if(new_element){
        Coll_Iter_setObject(new_element, object);

        if (!_this->_nr_elements){
            /*  first object to insert */
            _this->_first_element = new_element;
        } else {
            /*  insert object at the end of the list */
            Coll_Iter_setNext(_this->_last_element, new_element);
            Coll_Iter_setPrev(new_element, _this->_last_element);
        }
        /*  update list administration */
        _this->_last_element = new_element;
        _this->_nr_elements++;
        retValue = COLL_OK;
    }
    DLRL_INFO(INF_EXIT);
    return retValue;
}

void *
Coll_List_popBack(
    Coll_List *_this)
{
    Coll_Iter *element_to_remove;
    void *object = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    element_to_remove = _this->_last_element;

    if (element_to_remove){
        object = Coll_Iter_getObject(element_to_remove);

        /*  detach element from linked list */
        _this->_last_element = Coll_Iter_getPrev(element_to_remove);
        if (_this->_last_element){
          Coll_Iter_setNext(_this->_last_element, NULL);
        } else {
          _this->_first_element = NULL;
        }
        _this->_nr_elements--;

        /*  delete the element */
        Coll_Iter_delete(element_to_remove);

    }
    DLRL_INFO(INF_EXIT);
    return object;
}


void *
Coll_List_popFront(
    Coll_List *_this)
{
    Coll_Iter *element_to_remove;
    void *object = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    element_to_remove = _this->_first_element;

    if (element_to_remove){
        object = Coll_Iter_getObject(element_to_remove);

        /*  detach element from linked list */
        _this->_first_element = Coll_Iter_getNext(element_to_remove);
        if (_this->_first_element){
          Coll_Iter_setPrev(_this->_first_element, NULL);
        } else {
          _this->_last_element = NULL;
        }
        _this->_nr_elements--;

        /*  delete the element */
        Coll_Iter_delete(element_to_remove);

    }
    DLRL_INFO(INF_EXIT);
    return object;
}

void *
Coll_List_getObject(
    Coll_List *_this,
    unsigned long index)
{
    void *retValue = NULL;
    Coll_Iter *element;
    unsigned long i = 0;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);

    element = _this->_first_element;

    while (element && (i < index) ){
        element = Coll_Iter_getNext(element);
        i++;
    }
    if (element){
        retValue = Coll_Iter_getObject(element);
    }
    DLRL_INFO(INF_EXIT);
    return retValue;
}
