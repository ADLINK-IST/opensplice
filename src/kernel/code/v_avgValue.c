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

