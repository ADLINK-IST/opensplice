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

#include "os_report.h"

#include "u_user.h"
#include "u__cfAttribute.h"
#include "u__cfValue.h"
#include "u__entity.h"

#include "v_cfNode.h"

#define U_CFATTRIBUTE_SIZE (sizeof(C_STRUCT(u_cfAttribute)))

u_cfAttribute
u_cfAttributeNew(
    u_participant participant,
    v_cfAttribute kAttribute)
{
    u_cfAttribute _this = NULL;

    if ((participant == NULL) || (kAttribute == NULL)) {
        OS_REPORT(OS_ERROR, "u_cfAttributeNew", 0, "Illegal parameter");
    } else {
        _this = u_cfAttribute(os_malloc(U_CFATTRIBUTE_SIZE));
        u_cfNodeInit(u_cfNode(_this),participant,v_cfNode(kAttribute));
    }
    return _this;
}

void
u_cfAttributeFree(
    u_cfAttribute _this)
{
    if (_this != NULL) {
        u_cfNodeDeinit(u_cfNode(_this));
        memset(_this, 0, (size_t)sizeof(U_CFATTRIBUTE_SIZE));
        os_free(_this);
    }
}

c_bool
u_cfAttributeStringValue(
    u_cfAttribute attr,
    c_char **str)
{
    u_result r;
    c_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((attr != NULL) && (str != NULL)) {
        r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_STRING, &resultValue);

            if (result == TRUE) {
                *str = resultValue.is.String;
            }
            u_cfNodeRelease(u_cfNode(attr));
        }
    }
    return result;
}

c_bool
u_cfAttributeBoolValue(
    u_cfAttribute attr,
    c_bool *b)
{
    u_result r;
    c_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((attr != NULL) && (b != NULL)) {
        r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

            if (result == TRUE) {
                *b = resultValue.is.Boolean;
            }
            u_cfNodeRelease(u_cfNode(attr));
        }
    }
    return result;
}

c_bool
u_cfAttributeLongValue(
    u_cfAttribute attr,
    c_long *lv)
{
    u_result r;
    c_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((attr != NULL) && (lv != NULL)) {
        r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_LONG, &resultValue);

            if (result == TRUE) {
                *lv = resultValue.is.Long;
            }
            u_cfNodeRelease(u_cfNode(attr));
        }
    }
    return result;
}

c_bool
u_cfAttributeULongValue(
    u_cfAttribute attr,
    c_ulong *ul)
{
    u_result r;
    c_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((attr != NULL) && (ul != NULL)) {
        r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_ULONG, &resultValue);

            if (result == TRUE) {
                *ul = resultValue.is.ULong;
            }
            u_cfNodeRelease(u_cfNode(attr));
        }
    }
    return result;
}

c_bool
u_cfAttributeSizeValue(
    u_cfAttribute attr,
    c_size *size)
{
    u_result r;
    c_bool result;
    v_cfAttribute kAttr;
    c_value value;

    result = FALSE;
    if ((attr != NULL) && (size != NULL)) {
        r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            if(value.kind == V_STRING){
                result = u_cfDataSizeValueFromString(value.is.String, size);
            }
            else
            {
                OS_REPORT(OS_ERROR, "u_cfAttributeSizeValue", 0, "Value is not a string");
                assert(value.kind == V_STRING);
            }
            u_cfNodeRelease(u_cfNode(attr));
        }
    }
    return result;
}

c_bool
u_cfAttributeFloatValue(
    u_cfAttribute attr,
    c_float *f)
{
    u_result r;
    c_bool result;
    v_cfAttribute kAttr;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((attr != NULL) && (f != NULL)) {
        r = u_cfNodeReadClaim(u_cfNode(attr), (v_cfNode*)(&kAttr));
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_FLOAT, &resultValue);

            if (result == TRUE) {
                *f = resultValue.is.Float;
            }
            u_cfNodeRelease(u_cfNode(attr));
        }
    }
    return result;
}
