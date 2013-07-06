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
#include "os.h"
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
#define CF_F_INT      "d"
#define CF_F_UINT     "u"
#define CF_F_FLOAT    "f"

#define CF_FORMAT_CHAR         CF_F_PREF CF_F_BYTE     CF_F_CHAR

#define CF_FORMAT_BOOLEAN      CF_F_PREF CF_F_BYTE     CF_F_CHAR
#define CF_FORMAT_SHORT        CF_F_PREF CF_F_SHORT    CF_F_INT
#define CF_FORMAT_USHORT       CF_F_PREF CF_F_SHORT    CF_F_UINT
#define CF_FORMAT_WCHAR        CF_F_PREF CF_F_LONG     CF_F_CHAR
#define CF_FORMAT_LONG         CF_F_PREF CF_F_LONG     CF_F_INT
#define CF_FORMAT_ULONG        CF_F_PREF CF_F_LONG     CF_F_UINT
#define CF_FORMAT_LONGLONG     CF_F_PREF CF_F_LONGLONG CF_F_INT
#define CF_FORMAT_ULONGLONG    CF_F_PREF CF_F_LONGLONG CF_F_UINT
#define CF_FORMAT_FLOAT        CF_F_PREF CF_F_LONG     CF_F_FLOAT
#define CF_FORMAT_DOUBLE       CF_F_PREF CF_F_LONGLONG CF_F_FLOAT

#define CF_FORMAT(x) CF_FORMAT_##x

/**************************************************************
 * Private functions
 **************************************************************/
/* Special routine for scanning booleans */
/* scanf does not support this           */
static c_bool
u_cfValueScanBoolean(
    c_char *dataPtr,
    c_bool *resultPtr)
{
    c_bool result = FALSE;
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

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/

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

c_bool
u_cfValueScan(
    c_value value,
    c_valueKind valueKind,
    c_value *valuePtr)
{
    int i;
    c_bool result = FALSE;

    if (value.kind == V_STRING) {
        switch (valueKind) {
        __CASE__(CHAR,c_char);
        __CASE__(SHORT,c_short);
        __CASE__(USHORT,c_ushort);
        __CASE__(LONG,c_long);
        __CASE__(ULONG,c_ulong);
        __CASE__(LONGLONG,c_longlong);
        __CASE__(ULONGLONG,c_ulonglong);
        __CASE__(FLOAT,c_float);
        case V_BOOLEAN:
        {
            c_bool dest;
            result = u_cfValueScanBoolean(value.is.String, &dest);
            if (result) {
                *valuePtr = c_boolValue(dest);
            }
        }
        break;
        case V_STRING:
        {
             c_char *str;
             c_ulong length;

             length = (c_ulong)strlen(value.is.String);
             str = os_malloc(length + 1U);
             os_strncpy(str, value.is.String, length);
             str[length] = 0;
             *valuePtr = c_stringValue(str);
             result = TRUE;
        }
        break;
        case V_OCTET:
        case V_DOUBLE:
        default:
            result = FALSE;
            assert(0); /* Catch unhandled case */
        break;
        }
    } else {
        result = FALSE;
    }

    return result;
}

#undef __CASE__
