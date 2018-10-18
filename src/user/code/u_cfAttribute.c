/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "os_report.h"

#include "u_user.h"
#include "u__cfAttribute.h"
#include "u__cfValue.h"
#include "u__entity.h"

#include "v_cfNode.h"

#define U_CFATTRIBUTE_SIZE (sizeof(C_STRUCT(u_cfAttribute)))

u_cfAttribute
u_cfAttributeNew(
    const u_participant participant,
    v_cfAttribute kAttribute)
{
    u_cfAttribute _this;

    assert(participant != NULL);

    _this = u_cfAttribute(os_malloc(U_CFATTRIBUTE_SIZE));
    if (_this)
    {
        u_cfNodeInit(u_cfNode(_this),participant,v_cfNode(kAttribute));
    }

    return _this;
}

void
u_cfAttributeFree(
    u_cfAttribute _this)
{
    assert(_this != NULL);
    u_cfNodeDeinit(u_cfNode(_this));
    memset(_this, 0, sizeof(U_CFATTRIBUTE_SIZE));
    os_free(_this);
}

u_bool
u_cfAttributeStringValue(
    const u_cfAttribute attr,
    os_char **str)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    assert(attr != NULL);
    assert(str != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        result = u_cfValueScan(value, V_STRING, &resultValue);

        if (result == TRUE) {
            *str = resultValue.is.String;
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}

u_bool
u_cfAttributeBoolValue(
    const u_cfAttribute attr,
    u_bool *b)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    assert(attr != NULL);
    assert(b != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

        if (result == TRUE) {
            *b = resultValue.is.Boolean;
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}

u_bool
u_cfAttributeLongValue(
    const u_cfAttribute attr,
    os_int32 *lv)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    assert(attr != NULL);
    assert(lv != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        result = u_cfValueScan(value, V_LONG, &resultValue);

        if (result == TRUE) {
            *lv = resultValue.is.Long;
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}

u_bool
u_cfAttributeULongValue(
    const u_cfAttribute attr,
    os_uint32 *ul)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    assert(attr != NULL);
    assert(ul != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        result = u_cfValueScan(value, V_ULONG, &resultValue);

        if (result == TRUE) {
            *ul = resultValue.is.ULong;
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}

u_bool
u_cfAttributeOctetValue(
    const u_cfAttribute attr,
    os_uchar *uc)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    assert(attr != NULL);
    assert(uc != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        result = u_cfValueScan(value, V_OCTET, &resultValue);

        if (result == TRUE) {
            *uc = resultValue.is.Octet;
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}

u_bool
u_cfAttributeSizeValue(
    const u_cfAttribute attr,
    u_size *size)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;

    assert(attr != NULL);
    assert(size != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        if(value.kind == V_STRING){
           result = u_cfDataSizeValueFromString(value.is.String, size);
        } else {
            OS_REPORT(OS_ERROR, "u_cfAttributeSizeValue", r, "Value is not a string");
            assert(value.kind == V_STRING);
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}

u_bool
u_cfAttributeFloatValue(
    const u_cfAttribute attr,
    os_float *f)
{
    u_result r;
    u_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    assert(attr != NULL);
    assert(f != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
    if (r == U_RESULT_OK) {
        value = v_cfAttributeValue(kAttr);
        result = u_cfValueScan(value, V_FLOAT, &resultValue);

        if (result == TRUE) {
            *f = resultValue.is.Float;
        }
        u_cfNodeRelease(u_cfNode(attr));
    }
    return result;
}
