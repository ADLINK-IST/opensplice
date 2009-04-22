#include "os.h"
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

    _this = os_malloc(sizeof(Coll_Iter));

    if (_this){
        _this->_next = NULL;
        _this->_prev = NULL;
        _this->_object = NULL;
    }
    return _this;
}

long
Coll_Iter_delete(
    Coll_Iter *_this)
{
    assert(_this);

    os_free(_this);

    return COLL_OK;
}

Coll_Iter *
Coll_Iter_getNext(
    Coll_Iter *_this)
{
    assert(_this);

    return _this->_next;
}

void
Coll_Iter_setNext(
    Coll_Iter *_this,
    Coll_Iter *next)
{
    assert(_this);

    _this->_next = next;
}

Coll_Iter *
Coll_Iter_getPrev(
    Coll_Iter *_this)
{
    assert(_this);
    /* next may be null */
    return _this->_prev;
}

void
Coll_Iter_setPrev(
    Coll_Iter *_this,
    Coll_Iter *prev)
{
    assert(_this);
    /* prev may be null */
    _this->_prev = prev;
}

void *
Coll_Iter_getObject(
    Coll_Iter *_this)
{
    assert(_this);
    return _this->_object;
}

void
Coll_Iter_setObject(
    Coll_Iter *_this,
    void *object)
{
    assert(_this);
    /* object may be null */
    _this->_object = object;
}
