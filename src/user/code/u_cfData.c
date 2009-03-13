
#include "os_report.h"

#include "u_user.h"
#include "u__cfData.h"
#include "u__cfValue.h"

#include "v_cfNode.h"

#define U_CFDATA_SIZE (sizeof(C_STRUCT(u_cfData)))

u_result
u_cfDataClaim(
    u_cfData data,
    v_cfData *kData)
{
    u_result r = U_RESULT_OK;

    if ((data == NULL) || (kData == NULL)) {
        OS_REPORT(OS_ERROR, "u_cfDataClaim", 0, "Illegal parameter");
        r = U_RESULT_ILL_PARAM;
    } else {
        *kData = v_cfData(u_cfNodeClaim(u_cfNode(data)));
        if (*kData == NULL) {
            r = U_RESULT_INTERNAL_ERROR;
        }
    }
    return r;
}

u_result
u_cfDataRelease(
    u_cfData data)
{
    u_result r = U_RESULT_OK;

    if (data == NULL) {
        OS_REPORT(OS_ERROR, "u_cfDataClaim", 0, "Illegal parameter");
        r = U_RESULT_ILL_PARAM;
    } else {
        u_cfNodeRelease(u_cfNode(data));
    }
    return r;
}

u_cfData
u_cfDataNew(
    u_participant participant,
    v_cfData kData)
{
    u_cfData data;

    if ((participant == NULL) || (kData == NULL)) {
        OS_REPORT(OS_ERROR, "u_cfDataNew", 0, "Illegal parameter");
        data = NULL;
    } else {
        data = u_cfData(os_malloc(U_CFDATA_SIZE));
        u_cfNodeInit(u_cfNode(data),participant,v_cfNode(kData));
    }
    return data;
}

void
u_cfDataFree(
    u_cfData _this)
{
    if (_this != NULL) {
        u_cfNodeDeinit(u_cfNode(_this));
        memset(_this, 0, (size_t)sizeof(U_CFDATA_SIZE));
        os_free(_this);
    }
}

c_bool
u_cfDataStringValue(
    u_cfData data,
    c_char **str)
{
    u_result r;
    c_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((data != NULL) && (str != NULL)) {
        r = u_cfDataClaim(data, &kData);
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_STRING, &resultValue);

            if (result == TRUE) {
                *str = resultValue.is.String;
            }
            u_cfDataRelease(data);
        }
    }
    return result;
}

c_bool
u_cfDataBoolValue(
    u_cfData data,
    c_bool *b)
{
    u_result r;
    c_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((data != NULL) && (b != NULL)) {
        r = u_cfDataClaim(data, &kData);
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

            if (result == TRUE) {
                *b = resultValue.is.Boolean;
            }
            u_cfDataRelease(data);
        }
    }
    return result;
}

c_bool
u_cfDataLongValue(
    u_cfData data,
    c_long *lv)
{
    u_result r;
    c_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((data != NULL) && (lv != NULL)) {
        r = u_cfDataClaim(data, &kData);
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_LONG, &resultValue);

            if (result == TRUE) {
                *lv = resultValue.is.Long;
            }
            u_cfDataRelease(data);
        }
    }
    return result;
}

c_bool
u_cfDataULongValue(
    u_cfData data,
    c_ulong *ul)
{
    u_result r;
    c_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((data != NULL) && (ul != NULL)) {
        r = u_cfDataClaim(data, &kData);
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_ULONG, &resultValue);

            if (result == TRUE) {
                *ul = resultValue.is.ULong;
            }
            u_cfDataRelease(data);
        }
    }
    return result;
}

c_bool
u_cfDataFloatValue(
    u_cfData data,
    c_float *f)
{
    u_result r;
    c_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    result = FALSE;
    if ((data != NULL) && (f != NULL)) {
        r = u_cfDataClaim(data, &kData);
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_FLOAT, &resultValue);

            if (result == TRUE) {
                *f = resultValue.is.Float;
            }
            u_cfDataRelease(data);
        }
    }
    return result;
}
