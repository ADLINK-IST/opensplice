 
#include "v_avgValue.h"

void
v_avgValueInit(
    v_avgValue *_this)
{
    assert(_this != NULL);
    v_avgValueReset(_this);
}

void
v_avgValueReset(
    v_avgValue *_this)
{
    assert(_this != NULL);
    _this->count = 0;
    _this->value = 0.0;
}

c_float
v_avgValueGetValue(
    v_avgValue *_this)
{
    assert(_this != NULL);
    return _this->value;
}

void
v_avgValueSetValue(
    v_avgValue *_this, 
    c_ulong value)
{
    c_float delta;

    _this->count++;
    delta = ((c_float)value - _this->value) / _this->count;
    _this->value += delta;
}

