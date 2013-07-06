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
#include "v_fullCounter.h"
#include "v_minValue.h"
#include "v_maxValue.h"
#include "v_avgValue.h"

void
v_fullCounterInit(
    v_fullCounter *_this)
{
    assert(_this != NULL);
    v_fullCounterReset(_this);
}

void
v_fullCounterReset(
    v_fullCounter *_this)
{
    assert(_this != NULL);
    _this->value = 0;
    v_minValueReset(&_this->min);
    v_maxValueReset(&_this->max);
    v_avgValueReset(&_this->avg);
}

c_ulong
v_fullCounterGetValue(
    v_fullCounter *_this)
{
    assert(_this != NULL);
    return _this->value;
}

void
v_fullCounterSetValue(
    v_fullCounter *_this, 
    c_ulong value)
{
    assert(_this != NULL);

    _this->value = value;
    v_minValueSetValue(&_this->min,value);
    v_maxValueSetValue(&_this->max,value);
    v_avgValueSetValue(&_this->avg,value);
}

void
v_fullCounterValueInc( 
    v_fullCounter *_this)
{
    v_fullCounterSetValue(_this, _this->value+1);
}
void
v_fullCounterValueDec(
    v_fullCounter *_this)
{
    v_fullCounterSetValue(_this, _this->value-1);
}


c_time
v_fullCounterGetMaxTime(
    v_fullCounter *_this)
{
    assert(_this != NULL);
    return v_maxValueGetTime(&_this->max);
}

c_ulong
v_fullCounterGetMax(
    v_fullCounter *_this)
{
    assert(_this != NULL);
    return v_maxValueGetValue(&_this->max);
}

c_time
v_fullCounterGetMinTime(
    v_fullCounter *_this)
{
    return v_minValueGetTime(&_this->min);
}

c_ulong
v_fullCounterGetMin(
    v_fullCounter *_this)
{
    return v_minValueGetValue(&_this->min);
}

c_float
v_fullCounterGetAvg(
    v_fullCounter *_this)
{
    return v_avgValueGetValue(&_this->avg);
}

