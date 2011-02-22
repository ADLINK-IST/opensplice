/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "os_report.h"

#include "u_user.h"
#include "u__cfData.h"
#include "u__cfValue.h"
#include "u__entity.h"
#include "v_cfNode.h"
#include <math.h>

#define U_CFDATA_SIZE (sizeof(C_STRUCT(u_cfData)))

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
        r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_STRING, &resultValue);

            if (result == TRUE) {
                *str = resultValue.is.String;
            }
            u_cfNodeRelease(u_cfNode(data));
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
        r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

            if (result == TRUE) {
                *b = resultValue.is.Boolean;
            }
            u_cfNodeRelease(u_cfNode(data));
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
        r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_LONG, &resultValue);

            if (result == TRUE) {
                *lv = resultValue.is.Long;
            }
            u_cfNodeRelease(u_cfNode(data));
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
        r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_ULONG, &resultValue);

            if (result == TRUE) {
                *ul = resultValue.is.ULong;
            }
            u_cfNodeRelease(u_cfNode(data));
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
        r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_FLOAT, &resultValue);

            if (result == TRUE) {
                *f = resultValue.is.Float;
            }
            u_cfNodeRelease(u_cfNode(data));
        }
    }
    return result;
}

c_bool
u_cfDataSizeValue(
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
        r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (r == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            result = u_cfValueScan(value, V_SIZE, &resultValue);

            if (result == TRUE) {
                *ul = resultValue.is.ULong;
            }
            u_cfNodeRelease(u_cfNode(data));
        }
    }
    return result;
}

c_bool
u_cfDataSizeValueFromString(
    c_char *str,
    c_ulong *ul)
{
    c_bool result;
    c_char *temp;
    c_char chp;
    c_ulong base,res,retval = 0;
    result = TRUE;

    assert(str);

    temp = os_malloc(strlen(str) +1);
    if (temp != NULL) {
        strcpy (temp,str);
        retval = sscanf(temp, "%u%c",&res, &chp);
        if (retval == 1) {
            base =1;
        } else if (retval == 2) {
            switch(chp) {
            case 'K':
                base = 1024;
                break;
            case 'M':
                base = pow(1024,2);
                break;
            case 'G':
                base = pow(1024,3);
                break;
            default:
                OS_REPORT_1(OS_ERROR, "u_cfDataSizeValueFromString", 0, "Invalid size specifier (%c)", chp);
                base =1;
                break;
            }
        } else {
            OS_REPORT_1(OS_ERROR, "u_cfDataSizeValueFromString", 0, "Invalid size value (%s)", str);
            res = 0;
            base = 1;
        }
        /* boundary checking */
        if (res > C_MAX_ULONG(L)/base) {
            *ul = C_MAX_ULONG(L);
            OS_REPORT_2(OS_WARNING, "u_cfDataSizeValueFromString", 0, "Configuration parameter value (%s) exceeds maximum size ulong, value changed to %lu",str,C_MAX_ULONG(L));
        } else {
            *ul = res*base;
        }
        os_free(temp);
    } else {
        result =0;
        OS_REPORT_1(OS_ERROR, "u_cfDataSizeValueFromString", 0, "Malloc failed for configuration parameter value (%s)", str);
    }
    return result;
}
