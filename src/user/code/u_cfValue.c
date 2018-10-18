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
#include "vortex_os.h"
#include "u__cfValue.h"
#include "u_cfData.h"
#include "os_stdlib.h"

#define CF_SPACES  " \t\n"

#define CF_F_PREF     "%"
#define CF_F_BYTE     ""
#define CF_F_SHORT    "h"
#define CF_F_LONG     ""
#define CF_F_LONGLONG "ll"
#define CF_F_CHAR     "c"
#define CF_F_UCHAR    "u"
#define CF_F_INT      "d"
#define CF_F_UINT     "u"
#define CF_F_FLOAT    "f"

#define CF_FORMAT_CHAR         CF_F_PREF CF_F_BYTE     CF_F_CHAR
#define CF_FORMAT_OCTET        CF_F_PREF "hh"          CF_F_UCHAR
#define CF_FORMAT_BOOLEAN      CF_F_PREF CF_F_BYTE     CF_F_CHAR
#define CF_FORMAT_SHORT        CF_F_PREF CF_F_SHORT    CF_F_INT
#define CF_FORMAT_USHORT       CF_F_PREF CF_F_SHORT    CF_F_UINT
#define CF_FORMAT_WCHAR        CF_F_PREF CF_F_LONG     CF_F_CHAR
#define CF_FORMAT_LONG         CF_F_PREF CF_F_LONG     CF_F_INT
#define CF_FORMAT_ULONG        CF_F_PREF CF_F_LONG     CF_F_UINT
#define CF_FORMAT_LONGLONG     CF_F_PREF PA_PRId64
#define CF_FORMAT_ULONGLONG    CF_F_PREF PA_PRIu64
#define CF_FORMAT_FLOAT        CF_F_PREF CF_F_LONG     CF_F_FLOAT
#define CF_FORMAT_DOUBLE       CF_F_PREF CF_F_LONGLONG CF_F_FLOAT

#define CF_FORMAT(x) CF_FORMAT_##x

/* Special routine for scanning booleans */
/* scanf does not support this           */
static u_bool
u_cfValueScanBoolean(
    c_char *dataPtr,
    u_bool *resultPtr)
{
    u_bool result = FALSE;
    size_t l;

    l = strspn(dataPtr, CF_SPACES);
    if (l <= strlen(dataPtr)) {
        if (os_strncasecmp(&dataPtr[l], "TRUE", 4) == 0) {
            *resultPtr = TRUE;
            result = TRUE;
        } else {
            if (os_strncasecmp(&dataPtr[l], "FALSE", 5) == 0) {
                *resultPtr = FALSE;
                result = TRUE;
            }
        }
    }

    return result;
}

#define __CASE__(kind,type)                                  \
    case V_##kind:                                           \
    {                                                        \
        type dest;                                           \
        i = sscanf(value.is.String, CF_FORMAT(kind), &dest); \
        if (i>0) {                                           \
            *valuePtr = type##Value(dest);                   \
            result = TRUE;                                   \
        }                                                    \
    }                                                        \
    break

u_bool
u_cfValueScan(
    const c_value value,
    c_valueKind valueKind,
    c_value *valuePtr)
{
    int i;
    u_bool result = FALSE;

    if (value.kind == V_STRING) {
        switch (valueKind) {
        __CASE__(CHAR,c_char);
        __CASE__(OCTET,c_octet);
        __CASE__(SHORT,c_short);
        __CASE__(USHORT,c_ushort);
        __CASE__(LONG,c_long);
        __CASE__(ULONG,c_ulong);
        __CASE__(LONGLONG,c_longlong);
        __CASE__(ULONGLONG,c_ulonglong);
        __CASE__(FLOAT,c_float);
        case V_BOOLEAN:
        {
            u_bool dest;
            result = u_cfValueScanBoolean(value.is.String, &dest);
            if (result) {
                *valuePtr = c_boolValue(dest);
            }
        }
        break;
        case V_STRING:
        {
             c_char *str;
             str = os_strdup(value.is.String);
             *valuePtr = c_stringValue(str);
             result = TRUE;
        }
        break;
        case V_DOUBLE:
        default:
            result = FALSE;
            (void)result;
            assert(0); /* Catch unhandled case */
        break;
        }
    } else {
        result = FALSE;
    }

    return result;
}

#undef __CASE__
