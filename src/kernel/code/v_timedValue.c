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
    _this->lastUpdate = OS_TIMEW_ZERO;
}

c_ulong
v_timedValueGetValue(
    v_timedValue *_this)
{
    assert(_this != NULL);
    return _this->value;
}

os_timeW
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
    _this->lastUpdate = os_timeWGet();
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


