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
 
#include "v_timedValue.h"

void
v_timedValueInit(
    v_timedValue *_this)
{
    assert(_this != NULL);
    v_timedValueReset(_this);
}

void
v_timedValueReset(
    v_timedValue *_this)
{
    assert(_this != NULL);
    _this->value = 0;
    _this->lastUpdate = C_TIME_ZERO;
}

c_ulong
v_timedValueGetValue(
    v_timedValue *_this)
{
    assert(_this != NULL);
    return _this->value;
}

c_time
v_timedValueGetTime(
    v_timedValue *_this)
{
    assert(_this != NULL);
    return _this->lastUpdate;
}

void
v_timedValueSetValue(
    v_timedValue *_this,
    c_ulong value)
{
    assert(_this != NULL);
    _this->value = value;
    _this->lastUpdate = v_timeGet();
}

void
v_timedValueValuePlus(
    v_timedValue *_this)
{
    assert(_this != NULL);
    _this->value++;
}

void
v_timedValueValueMin(
    v_timedValue * _this)
{
    assert(_this != NULL);
    _this->value--;
}


