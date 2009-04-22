#include "os.h"
#include "Coll_Set.h"
#include "Coll_Iter.h"

Coll_Set *
Coll_Set_new(
    int (* const isLess)(void *left, void *right),
    int sorted)
{
    Coll_Set *_this = NULL;

    assert(isLess);

    _this = os_malloc(sizeof(Coll_Set));

    if (_this){
        Coll_Set_init(_this, isLess, sorted);
    }
    return _this;
}

void
Coll_Set_init(
    Coll_Set *_this,
    int (* const isLess)(void *left, void *right),
    int sorted)
{
    assert(_this);
    assert(isLess);

    _this->_nr_elements = 0;
    _this->_isLess = isLess;
    _this->_is_sorted = sorted;
    _this->_first_element = NULL;
    _this->_last_element = NULL;
}

long
Coll_Set_delete(
    Coll_Set *_this)
{
    long retValue = COLL_OK;

assert(_this);

    /*  only allowed if the set is empty */
    if(_this->_nr_elements){
        retValue = COLL_ERROR_NOT_EMPTY;
    } else {
        /*  free the set header */
        os_free(_this);
    }
    return retValue;
}

long
Coll_Set_getNrOfElements(
    const Coll_Set *_this)
{
    assert(_this);
    return _this->_nr_elements;
}

Coll_Iter *
Coll_Set_getFirstElement(
    const Coll_Set *_this)
{
    assert(_this);
    return _this->_first_element;
}

Coll_Iter *
Coll_Set_getLastElement(
    Coll_Set *_this)
{
    assert(_this);
    return _this->_last_element;
}

long
Coll_Set_add(
    Coll_Set *_this,
    void *object)
{
    long retValue;
    Coll_Iter *new_element;

    assert(_this);
    /* object may be null */

    if (!_this->_is_sorted && !Coll_Set_find(_this, object)){
        /*  set is unsorted and does not contain the to be added object => the object can be added as a unique object */
        retValue = Coll_Set_addUniqueObject(_this, object);
    } else {
        /*  set is sorted, add the object at the right place */
        new_element = Coll_Iter_new();
        if (!new_element){
            retValue = COLL_ERROR_ALLOC;
        } else {
            Coll_Iter_setObject(new_element, object);

            if ( (_this->_nr_elements == 0) || (_this->_isLess(object, Coll_Iter_getObject(_this->_first_element) ) ) ){
                /*  first object to insert */
                Coll_Iter_setNext(new_element, _this->_first_element);
                if (_this->_first_element){
                    Coll_Iter_setPrev(_this->_first_element, new_element);
                }
                _this->_first_element = new_element;

                /*  update set administration */
                _this->_nr_elements++;
            } else {
                int ready = 0;

                /*  insert object at the right place */
                Coll_Iter *iter = _this->_first_element;

                while (!ready) {
                    int isLess = _this->_isLess(object, Coll_Iter_getObject(iter));

                    if ( !(isLess || _this->_isLess(Coll_Iter_getObject(iter), object)) ) {
                        /*  equal => element already exists in set => delete iterator object again */
                        Coll_Iter_delete(new_element);
                        ready = 1;
                    } else if (isLess) {
                        /*  smaller => insert the new element here */
                        Coll_Iter_setNext(new_element, iter);
                        Coll_Iter_setPrev(new_element, Coll_Iter_getPrev(iter));
                        Coll_Iter_setNext(Coll_Iter_getPrev(iter), new_element);
                        Coll_Iter_setPrev(iter, new_element);

                        /*  update set administration */
                       _this->_nr_elements++;

                        ready = 1;
                    } else if (Coll_Iter_getNext(iter) == NULL){
                        /*  append new element to the end of the set */
                        Coll_Iter_setPrev(new_element, iter);
                        Coll_Iter_setNext(iter, new_element);

                        /*  update set administration */
                        _this->_last_element = new_element;
                        _this->_nr_elements++;

                        ready = 1;
                    } else {
                        /*  do nothing */
                    }
                    iter = Coll_Iter_getNext(iter);
                }
            }
            retValue = COLL_OK;
        }
    }
    return retValue;
}

long
Coll_Set_addUniqueObject(
    Coll_Set *_this,
    void *object)
{
    long retValue = COLL_OK;
    Coll_Iter *new_element = NULL;

    assert(_this);
    /* object may be null */

    if (_this->_is_sorted){
        retValue = COLL_ERROR_PRECONDITION_NOT_MET;
    } else {
        new_element = Coll_Iter_new();
        if(!new_element){
            retValue = COLL_ERROR_ALLOC;
        }
    }

    if(retValue == COLL_OK){
        Coll_Iter_setObject(new_element, object);

        if (_this->_nr_elements == 0){
            /*  first object to insert */

            /*  Following actions already handled by Coll_Iter constructor */
            /*  Coll_Iter_setPrev(new_element, NULL); */
            /*  Coll_Iter_setNext(new_element, NULL); */
            _this->_first_element = new_element;
            _this->_last_element = new_element;
        } else {
            /*  simply append the new object to the end of the list */

            /*  Following action already handled by Coll_Iter constructor */
            /*  Coll_Iter_setNext(new_element, NULL); */
            Coll_Iter_setPrev(new_element, _this->_last_element);
            Coll_Iter_setNext(_this->_last_element, new_element);
            _this->_last_element = new_element;
        }
        /*  update set administration */
        _this->_nr_elements++;
    }
    return retValue;
}

/* dont change the way this operation removes elements.  */
/* dependencies are made onto this, which is abit spagetti-like */
/* but sometimes you are just into italian food... */
void*
Coll_Set_remove(
    Coll_Set *_this,
    void *object)
{
    void* returnVal = NULL;
    Coll_Iter *iter;
    Coll_Iter *prev;
    Coll_Iter *next;

    assert(_this);
    /* object may be null */

    iter = Coll_Set_find(_this, object);
    if (iter){
        returnVal = Coll_Iter_getObject(iter);
        prev = Coll_Iter_getPrev(iter);
        next = Coll_Iter_getNext(iter);

        /*  delink the found element from the set */
        if (prev){
            Coll_Iter_setNext(prev, next);
        }
        if (next){
            Coll_Iter_setPrev(next, prev);
        }

        /*  update set administration */
        if (iter == _this->_first_element){
            _this->_first_element = next;
        }
        if (iter == _this->_last_element){
            _this->_last_element = prev;
        }
        _this->_nr_elements--;

        /*  Free element */
        Coll_Iter_delete(iter);
    }
    return returnVal;
}

int
Coll_Set_contains(
    Coll_Set *_this,
    void *object)
{
    assert(_this);
    /* object may be null */
    return (Coll_Set_find(_this, object) != NULL);
}

Coll_Iter *
Coll_Set_find(
    Coll_Set *_this,
    void *object)
{
    Coll_Iter *iter;
    int ready ;

    assert(_this);
    /* object may be null */

    iter = _this->_first_element;

    ready = (iter) ? 0 : 1;

    while (!ready){
        int isLess = _this->_isLess(object, Coll_Iter_getObject(iter));

        if ( !(isLess || _this->_isLess(Coll_Iter_getObject(iter), object)) ){
            /*  equal => ready */
            ready = 1;
        } else if (isLess && _this->_is_sorted){
            /*  smaller => object does not exist */
            iter = NULL;
            ready = 1;
        } else {
            /*  larger => continue search */
            iter = Coll_Iter_getNext(iter);
            if (!iter){
                ready = 1;
            }
        }
    }
    return iter;
}
