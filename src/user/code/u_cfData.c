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
#include "os_abstract.h"

#include "u_user.h"
#include "u__cfData.h"
#include "u__cfValue.h"
#include "u__entity.h"
#include "v_cfNode.h"
#include "c_stringSupport.h"

#include <ctype.h>

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
        c_size *size)
{
    u_result user_result;
    c_bool result;
    v_cfData kData;
    c_value value;

    result = FALSE;
    if ((data != NULL) && (size != NULL)) {
        user_result = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
        if (user_result == U_RESULT_OK) {
            value = v_cfDataValue(kData);
            if(value.kind == V_STRING){
                result = u_cfDataSizeValueFromString(value.is.String,size);
            }
            else
            {
                OS_REPORT(OS_ERROR, "u_cfDataSizeValue", 0, "Value is not a string");
                assert(value.kind == V_STRING);
            }

            u_cfNodeRelease(u_cfNode(data));
        }
    }
    return result;
}

c_bool
u_cfDataSizeValueFromString(
    c_char *str,
    c_size *size)
{
    c_bool result = TRUE;
    c_char *temp, *startChar, *endChar;
    c_char tempChar;

    if(str){

        temp = c_trimString(str);

        if(temp)
        {
            startChar = temp;

            /* get the numeric part */
            endChar = startChar;
            while(*endChar != '\0' && isdigit(*endChar))
            {
                endChar++;
            }

            if(startChar == endChar)
            {
                result = FALSE;
            }
            else {
                /* endChar now points to the character in the string after the last digit */
                tempChar = *endChar;
                *endChar = '\0'; /* make it a 0-terminated string */

                sscanf(startChar, PA_SIZEFMT, size);

                *endChar = tempChar;

                /*
                 * If endChar is now pointing at '\0', then the size does not contain
                 * a user-friendly size character. Else continue and interpret the user
                 * friendly size character.
                 */
                if(*endChar != '\0'){
                    endChar++;
                    if(*endChar != '\0')
                    {
                        /*
                         * If endChar is '\0' now, then this means that after the
                         * user friendly size character comes more, which is not
                         * allowed.
                         */
                        result = FALSE;
                    }
                    else
                    {
                        /* now get the user-friendly part, if available */
                        switch(tempChar) { /* tmpChar contains the user-friendly character */
                            case 'K':
                            case 'k':
                                if(*size > C_MAX_SIZE/(1<<10)) /* boundary checking */
                                {
                                    *size = C_MAX_SIZE;
                                    OS_REPORT_2(OS_WARNING,
                                                "u_cfDataSizeValueFromString",
                                                0,
                                                "Configuration parameter value (%s) exceeds maximum size, value changed to " PA_SIZEFMT,
                                                temp,
                                                C_MAX_SIZE);
                                } else {
                                    *size <<= 10; /* multiply by 1024 */
                                }
                                break;
                            case 'M':
                            case 'm':
                                if(*size > C_MAX_SIZE/(1<<20)) /* boundary checking */
                                {
                                    *size = C_MAX_SIZE;
                                    OS_REPORT_2(OS_WARNING,
                                                "u_cfDataSizeValueFromString",
                                                0,
                                                "Configuration parameter value (%s) exceeds maximum size, value changed to " PA_SIZEFMT,
                                                temp,
                                                C_MAX_SIZE);
                                } else {
                                    *size <<= 20; /* multiply by 1048576 */
                                }
                                break;
                            case 'G':
                            case 'g':
                                if(*size > C_MAX_SIZE/(1<<30)) /* boundary checking */
                                {
                                    *size = C_MAX_SIZE;
                                    OS_REPORT_2(OS_WARNING,
                                                "u_cfDataSizeValueFromString",
                                                0,
                                                "Configuration parameter value (%s) exceeds maximum size, value changed to " PA_SIZEFMT,
                                                temp,
                                                C_MAX_SIZE);
                                } else {
                                    *size <<= 30; /* multiply by 1073741824 */
                                }
                                break;
                            default:
                                result = FALSE;
                                break;
                        }
                    }
                }
            }

            if(!result)
            {
                *size = 0;
                OS_REPORT_1(OS_ERROR, "u_cfDataSizeValueFromString", 0, "Invalid size value (\"%s\")", temp);
            }

            os_free(temp);
        }
        else
        {
            OS_REPORT_1(OS_ERROR, "u_cfDataSizeValueFromString", 0, "String trimming failed for configuration parameter value (%s)", str);
            result = FALSE;
        }
    }
    else {
        OS_REPORT(OS_ERROR, "u_cfDataSizeValueFromString", 0, "Illegal parameter given (NULL pointer) to u_cfDataSizeValueFromString");
        result = FALSE;
    }

    return result;
}
