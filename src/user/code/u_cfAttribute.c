
#include "os_report.h"

#include "u_user.h"
#include "u__cfAttribute.h"
#include "u__cfValue.h"

#include "v_cfNode.h"

#define U_CFATTRIBUTE_SIZE (sizeof(C_STRUCT(u_cfAttribute)))

u_result
u_cfAttributeClaim(
    u_cfAttribute _this,
    v_cfAttribute *kAttr)
{
    u_result r = U_RESULT_OK;

    if ((_this == NULL) || (kAttr == NULL)) {
        r = U_RESULT_ILL_PARAM;
    } else {
        *kAttr = v_cfAttribute(u_cfNodeClaim(u_cfNode(_this)));
        if (*kAttr == NULL) {
            r = U_RESULT_INTERNAL_ERROR;
        }
    }
    return r;
}

u_result
u_cfAttributeRelease(
    u_cfAttribute _this)
{
    u_result r = U_RESULT_OK;

    if (_this == NULL) {
        r = U_RESULT_ILL_PARAM;
    } else {
        u_cfNodeRelease(u_cfNode(_this));
    }
    return r;
}

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
        r = u_cfAttributeClaim(attr, &kAttr);
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_STRING, &resultValue);

            if (result == TRUE) {
                *str = resultValue.is.String;
            }
            u_cfAttributeRelease(attr);
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
        r = u_cfAttributeClaim(attr, &kAttr);
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

            if (result == TRUE) {
                *b = resultValue.is.Boolean;
            }
            u_cfAttributeRelease(attr);
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
        r = u_cfAttributeClaim(attr, &kAttr);
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_LONG, &resultValue);

            if (result == TRUE) {
                *lv = resultValue.is.Long;
            }
            u_cfAttributeRelease(attr);
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
        r = u_cfAttributeClaim(attr, &kAttr);
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_ULONG, &resultValue);

            if (result == TRUE) {
                *ul = resultValue.is.ULong;
            }
            u_cfAttributeRelease(attr);
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
        r = u_cfAttributeClaim(attr, &kAttr);
        if (r == U_RESULT_OK) {
            value = v_cfAttributeValue(kAttr);
            result = u_cfValueScan(value, V_FLOAT, &resultValue);

            if (result == TRUE) {
                *f = resultValue.is.Float;
            }
            u_cfAttributeRelease(attr);
        }
    }
    return result;
}
