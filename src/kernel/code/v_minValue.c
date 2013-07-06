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
 
#include "v_minValue.h"
#include "v_timedValue.h"

void
v_minValueInit(
    v_minValue *_this)
{
    assert(_this != NULL);
    v_minValueReset(_this);
}

void
v_minValueReset(
    v_minValue *_this)
{
    assert(_this != NULL);
    v_timedValueReset(_this);
    v_timedValueSetValue(_this,0x7FFFFFFF);
}

c_time
v_minValueGetTime(
    v_minValue *_this)
{
    assert(_this != NULL);
    return v_timedValueGetTime(_this);
}

c_ulong
v_minValueGetValue(
    v_minValue *_this)
{
    assert(_this != NULL);
    return v_timedValueGetValue(_this);
}

void
v_minValueSetValue(
    v_minValue *_this, 
    c_ulong value)
{
    if (value < v_timedValueGetValue(_this)) {
        v_timedValueSetValue(_this,value);
    }
}

