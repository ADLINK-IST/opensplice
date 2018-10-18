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
    const u_participant participant,
    v_cfData kData)
{
    u_cfData data;

    assert(participant != NULL);
    assert(kData != NULL);

    data = u_cfData(os_malloc(U_CFDATA_SIZE));
    if (data)
    {
        u_cfNodeInit(u_cfNode(data),participant,v_cfNode(kData));
    }

    return data;
}

void
u_cfDataFree(
    u_cfData _this)
{
    assert (_this);
    u_cfNodeDeinit(u_cfNode(_this));
    memset(_this, 0, sizeof(U_CFDATA_SIZE));
    os_free(_this);
}

u_bool
u_cfDataStringValue(
    const u_cfData data,
    os_char **str)
{
    u_result r;
    u_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    assert(data != NULL);
    assert(str != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
    if (r == U_RESULT_OK) {
        value = v_cfDataValue(kData);
        result = u_cfValueScan(value, V_STRING, &resultValue);

        if (result == TRUE) {
            *str = resultValue.is.String;
        }
        u_cfNodeRelease(u_cfNode(data));
    }
    return result;
}

u_bool
u_cfDataBoolValue(
    const u_cfData data,
    u_bool *b)
{
    u_result r;
    u_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    assert(data != NULL);
    assert(b != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
    if (r == U_RESULT_OK) {
        value = v_cfDataValue(kData);
        result = u_cfValueScan(value, V_BOOLEAN, &resultValue);

        if (result == TRUE) {
            *b = resultValue.is.Boolean;
        }
        u_cfNodeRelease(u_cfNode(data));
    }
    return result;
}

u_bool
u_cfDataLongValue(
    const u_cfData data,
    os_int32 *lv)
{
    u_result r;
    u_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    assert(data != NULL);
    assert(lv != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
    if (r == U_RESULT_OK) {
        value = v_cfDataValue(kData);
        result = u_cfValueScan(value, V_LONG, &resultValue);

        if (result == TRUE) {
            *lv = resultValue.is.Long;
        }
        u_cfNodeRelease(u_cfNode(data));
    }
    return result;
}

u_bool
u_cfDataULongValue(
    const u_cfData data,
    os_uint32 *ul)
{
    u_result r;
    u_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    assert(data != NULL);
    assert(ul != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
    if (r == U_RESULT_OK) {
        value = v_cfDataValue(kData);
        result = u_cfValueScan(value, V_ULONG, &resultValue);

        if (result == TRUE) {
            *ul = resultValue.is.ULong;
        }
        u_cfNodeRelease(u_cfNode(data));
    }
    return result;
}

u_bool
u_cfDataFloatValue(
    const u_cfData data,
    os_float *f)
{
    u_result r;
    u_bool result;
    v_cfData kData;
    c_value value;
    c_value resultValue;

    assert(data != NULL);
    assert(f != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(u_cfNode(data), (v_cfNode*)(&kData));
    if (r == U_RESULT_OK) {
        value = v_cfDataValue(kData);
        result = u_cfValueScan(value, V_FLOAT, &resultValue);

        if (result == TRUE) {
            *f = resultValue.is.Float;
        }
        u_cfNodeRelease(u_cfNode(data));
    }
    return result;
}

struct unit {
  const char *name;
  os_uint64 multiplier;
};

/* comparison is case-sensitive */
static const struct unit unittab_bytes[] = {
  { "B",   1 },
  { "b",   1 },
  { "KiB", 1024 },
  { "kB",  1024 },
  { "K",   1024 },
  { "k",   1024 },
  { "MiB", 1048576 /* 1024*1024 */ },
  { "MB",  1048576 /* 1024*1024 */ },
  { "M",   1048576 /* 1024*1024 */ },
  { "m",   1048576 /* 1024*1024 */ },
  { "GiB", 1073741824 /* 1024*1024*1024 */ },
  { "GB",  1073741824 /* 1024*1024*1024 */ },
  { "G",   1073741824 /* 1024*1024*1024 */ },
  { "g",   1073741824 /* 1024*1024*1024 */ },
  { "TiB", 1099511627776 /* 1024*1024*1024*1024 */ },
  { "TB",  1099511627776 /* 1024*1024*1024*1024 */ },
  { "T",   1099511627776 /* 1024*1024*1024*1024 */ },
  { "t",   1099511627776 /* 1024*1024*1024*1024 */ },
  { NULL, 0 }
};

static os_uint64
lookup_multiplier(
    const struct unit *unittab,
    const char *str,
    int unit_pos,
    int value_is_zero,
    os_uint64 def_mult,
    int err_on_unrecognised)
{
    int i = 0;
    char srchstr[4]; /* strlen(longestUnit) + 1 */
    assert((unit_pos >= 0) && ((os_size_t)unit_pos <= strlen(str)));

    while(unittab[i].name) {
        assert(strlen(unittab[i].name) < sizeof(srchstr));
        i++;
    }

    /* skip whitespace */
    while (str[unit_pos] == ' ') {
        unit_pos++;
    }

    if (str[unit_pos] == 0) {
        /* No unit supplied in value str */
        if (value_is_zero) {
            return 1;
        } else if (def_mult == 0) {
            OS_REPORT(OS_WARNING, "lookup_multiplier", 0,
                "Unit is mandatory (in value string '%s')", str);
            return 0;
        } else {
            return def_mult;
        }
    } else {
        /* Lookup unit multiplier */
        strncpy(srchstr, str + unit_pos, sizeof(srchstr));
        srchstr[sizeof(srchstr) - 1] = '\0';
        srchstr[strcspn(srchstr, " ")] = '\0';
        for (i = 0; unittab[i].name != NULL; i++) {
            if (strcmp(unittab[i].name, srchstr) == 0) {
                return unittab[i].multiplier;
            }
        }

        if (err_on_unrecognised) {
            OS_REPORT(OS_ERROR, "lookup_multiplier", 0,
                "Unsupported unit-name '%s' (in value string '%s')", str + unit_pos, str);
        }
        return 0;
    }
}

static u_bool
parseUnitString(
    const char *str,
    os_uint64 *value)
{
    c_bool result;
    int pos;
    double v_dbl;
    os_uint64 v_int;
    os_uint64 mult;

    /* empty string */
    if (*str == 0) {
        *value = 0; /* silence static code analyzers */
        result = FALSE;
    /* convert integer + optional unit */
    } else if ((sscanf(str, "%"PA_SCNu64"%n", &v_int, &pos) == 1) &&
               ((mult = lookup_multiplier(unittab_bytes, str, pos, (v_int == 0), 1, 0)) != 0)) {
        assert(mult > 0);
        if (v_int < C_MAX_ULONGLONG/(mult)) {
            *value = v_int * mult;
        } else {
            *value = C_MAX_ULONGLONG;
        }
        result = TRUE;
    /* convert float + optional unit (round, not truncate, to integer) */
    } else if ((sscanf(str, "%lf%n", &v_dbl, &pos) == 1) &&
               ((mult = lookup_multiplier(unittab_bytes, str, pos, (v_dbl == 0), 1, 1)) != 0)) {
        double dmult = (double)mult;
        assert(dmult > 0);
        if (v_dbl < 0) {
            OS_REPORT(OS_ERROR, "parseUnitString", 0,
                "Signed values not supported (in value string '%s')", str);
            result = FALSE;
        } else {
            *value = (os_uint64)(v_dbl * dmult + 0.5);
            result = TRUE;
        }
    } else {
        OS_REPORT(OS_ERROR, "parseUnitString", 0,
            "Invalid value string '%s'", str);
        result = FALSE;
    }
    return result;
}

static u_bool
parseCfNodeUnitValue(
    const u_cfNode node,
    os_uint64 *value)
{
    u_result r;
    v_cfData kData;
    c_value cval;
    c_bool result;

    assert(node != NULL);
    assert(value != NULL);

    result = FALSE;
    r = u_cfNodeReadClaim(node, (v_cfNode*)&kData);
    if (r == U_RESULT_OK) {
        cval = v_cfDataValue(kData);
        if (cval.kind == V_STRING) {
            result = parseUnitString(cval.is.String, value);
        } else {
            OS_REPORT(OS_ERROR, "parseNodeUnitValue", U_RESULT_ILL_PARAM,
                "Data is not a string");
            assert(cval.kind == V_STRING);
        }
        u_cfNodeRelease(node);
    }
    return result;
}

u_bool
u_cfDataSizeValue(
    const u_cfData data,
    u_size *size)
{
    u_bool result;
    os_uint64 holder;
    result = parseCfNodeUnitValue(u_cfNode(data), &holder);
    if (result) {
        if (holder > OS_MAX_INTEGER(u_size)) {
            OS_REPORT(OS_WARNING, "u_cfDataSizeValue", 0,
                "Value '%"PA_PRIu64"' out of range in node %s: truncated",
                holder, u_cfNodeName(u_cfNode(data)));
            holder = OS_MAX_INTEGER(u_size);
        }
        *size = (u_size)holder;
    }
    return result;
}

u_bool
u_cfDataUInt64Value(
    const u_cfData data,
    os_uint64 *size)
{
    return parseCfNodeUnitValue(u_cfNode(data), size);
}

u_bool
u_cfDataSizeValueFromString(
    const os_char *str,
    u_size *size)
{
    u_bool result;
    os_uint64 holder = 0;
    result = parseUnitString(str, &holder);
    if (result) {
        if (holder > OS_MAX_INTEGER(u_size)) {
            holder = OS_MAX_INTEGER(u_size);
            OS_REPORT(OS_WARNING, "u_cfDataSizeValueFromString", 0,
                "Value out of range in string '%s': truncated to %"PA_PRIu64, str, holder);
        }
    }
    *size = (u_size)holder;

    return result;
}

u_bool
u_cfDataUInt64ValueFromString(
    const os_char *str,
    os_uint64 *size)
{
    return parseUnitString(str, size);
}
