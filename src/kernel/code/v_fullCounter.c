/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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


os_timeW
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

os_timeW
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

