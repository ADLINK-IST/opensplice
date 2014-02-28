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
#include <math.h>
#include <errno.h>
#include <stdlib.h>

#include "c_typebase.h"
#include "c_base.h" /* for c_keep and c_free */

#include "os_report.h"
#include "os_abstract.h"
#include "os_stdlib.h"

#ifdef NDEBUG
#define purify_memset(v,b,s)
#else
#define purify_memset(v,b,s) memset(v,b,s)
#endif

c_value
c_undefinedValue()
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_UNDEFINED;
    return v;
}

c_value
c_shortValue (
    const c_short value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_SHORT;
    v.is.Short = value;
    return v;
}

c_value
c_addressValue (
    const c_address value)
{
    c_value v;
    
    memset(&v, 0, sizeof(v));
    v.kind = V_ADDRESS;
    v.is.Address = value;
    return v;
}

c_value
c_longValue (
    const c_long value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_LONG;
    v.is.Long = value;
    return v;
}

c_value
c_longlongValue (
    const c_longlong value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_LONGLONG;
    v.is.LongLong = value;
    return v;
}

c_value
c_ushortValue (
    const c_ushort value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_USHORT;
    v.is.UShort = value;
    return v;
}

c_value
c_ulongValue (
    const c_ulong value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_ULONG;
    v.is.ULong = value;
    return v;
}

c_value
c_ulonglongValue (
    const c_ulonglong value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_ULONGLONG;
    v.is.ULongLong = value;
    return v;
}

c_value
c_boolValue (
    const c_bool value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_BOOLEAN;
    v.is.Boolean = value;
    return v;
}

c_value
c_octetValue (
    const c_octet value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_OCTET;
    v.is.Octet = value;
    return v;
}

c_value
c_charValue (
    const c_char value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_CHAR;
    v.is.Char = value;
    return v;
}

c_value
c_wcharValue (
    const c_wchar value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_WCHAR;
    v.is.WChar = value;
    return v;
}

c_value
c_floatValue (
    const c_float value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_FLOAT;
    v.is.Float = value;
    return v;
}

c_value
c_doubleValue (
    const c_double value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_DOUBLE;
    v.is.Double = value;
    return v;
}

c_value
c_stringValue (
    const c_string value)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_STRING;
    v.is.String = value;
    return v;
}

c_value
c_wstringValue (
    const c_wstring value)
{
    c_value v;
 
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_WSTRING;
    v.is.WString = value;
    return v;
}

c_value
c_objectValue (
    const c_voidp object)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_OBJECT;
    v.is.Object = object;
    return v;
}

c_value
c_voidpValue (
    const c_voidp object)
{
    c_value v;
    
    purify_memset(&v, 0, sizeof(v));
    v.kind = V_VOIDP;
    v.is.Voidp = object;
    return v;
}


c_char *
c_valueImage(
    const c_value value)
{
    char buf[1024];

    switch (value.kind) {
    case V_ADDRESS:
        {
            snprintf(buf,1024,"0x"PA_ADDRFMT,(PA_ADDRCAST)value.is.Address);
        }
    break;
    case V_BOOLEAN:
        if (value.is.Boolean) {
            return os_strdup("TRUE");
        } else {
            return os_strdup("FALSE");
        }
    case V_LONGLONG:
        {
            char llstr[36];
            llstr[35] = '\0';
            snprintf(buf,1024,"%s",os_lltostr(value.is.LongLong, &llstr[35]));
        }
    break;
    case V_ULONGLONG:
        {
            char llstr[36];
            llstr[35] = '\0';
            snprintf(buf,1024,"%s",os_ulltostr(value.is.ULongLong, &llstr[35]));
        }
    break;
    case V_SHORT:     snprintf(buf,1024,"%d",value.is.Short); break;
    case V_LONG:      snprintf(buf,1024,"%d",value.is.Long); break;
    case V_OCTET:     snprintf(buf,1024,"%u",value.is.Octet); break;
    case V_USHORT:    snprintf(buf,1024,"%u",value.is.UShort); break;
    case V_ULONG:     snprintf(buf,1024,"%u",value.is.ULong); break;
    case V_CHAR:
    {
        unsigned char c = value.is.Char;

            if (c < 32 || c >= 127) {
                snprintf(buf,1024,"\\%03o",c);
            } else {
                snprintf(buf,1024,"%c",c);
            }

    }
    break;
    case V_STRING:
    {
        if (value.is.String == NULL) {
            snprintf(buf,1024,"(NULL)");
        } else {
            snprintf(buf,1024,"%s",value.is.String);
        }
    }
    break;
     case  V_FLOAT:     snprintf(buf,1024,"%0.7g",value.is.Float); break;
     case  V_DOUBLE:    snprintf(buf,1024,"%0.15g",value.is.Double); break;
     case  V_WCHAR:     snprintf(buf,1024,"(a-wchar-value)"); break;
     case  V_WSTRING:   snprintf(buf,1024,"0x"PA_ADDRFMT, (PA_ADDRCAST)value.is.WString); break;
     case  V_FIXED:     snprintf(buf,1024,"0x"PA_ADDRFMT, (PA_ADDRCAST)value.is.Fixed); break;
     case  V_OBJECT:    snprintf(buf,1024,"0x"PA_ADDRFMT, (PA_ADDRCAST)value.is.Object); break;
     case  V_VOIDP:     snprintf(buf,1024,"0x"PA_ADDRFMT, (PA_ADDRCAST)value.is.Voidp); break;
     case  V_UNDEFINED: snprintf(buf,1024,"(an-undefined-value)"); break;
     default: assert(FALSE);
    }
    return os_strdup(buf);
}

c_bool
c_imageValue(
    const char *image,
    c_value *imgValue,
    c_type imgType)
{
    c_type t;
    char *endptr;

    assert(imgType);

    t = c_typeActualType(imgType);
    switch(c_baseObjectKind(t)) {
    case M_ENUMERATION:
    {
        c_literal l = c_enumValue(c_enumeration(imgType), image);
        if (l) {
            *imgValue = l->value;
            c_free(l);
        } else {
            imgValue->kind = V_UNDEFINED;
            OS_REPORT_1(OS_API_INFO,
                         "c_typebase::c_imageValue",0,
                         "expected legal enum label instead of \"%s\".",
                         image);
        }
        break;
    }
    case M_COLLECTION:
        if (c_collectionTypeKind(t) == C_STRING) {
            if(imgValue->is.String){
                c_free(imgValue->is.String);
            }
            imgValue->is.String = c_stringNew(c_getBase(imgType), image);
            imgValue->kind = V_STRING;
        }
        break;
    case M_PRIMITIVE:
        switch (c_primitiveKind(t)) {
        case P_BOOLEAN:
            if (os_strncasecmp(image,"TRUE",5) == 0) {
                imgValue->kind = V_BOOLEAN;
                imgValue->is.Boolean = TRUE;
            } else if (os_strncasecmp(image,"FALSE",6) == 0) {
                imgValue->kind = V_BOOLEAN;
                imgValue->is.Boolean = FALSE;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_LONGLONG:
            imgValue->is.LongLong = os_strtoll (image, &endptr, 0);
            if (*endptr == '\0') {
                imgValue->kind = V_LONGLONG;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_ULONGLONG:
            imgValue->is.ULongLong = os_strtoull (image, &endptr, 0);
            if (*endptr == '\0') {
                imgValue->kind = V_ULONGLONG;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_SHORT:
            if (sscanf(image,"%hd",&imgValue->is.Short)) {
                imgValue->kind = V_SHORT;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_LONG:
            imgValue->is.Long = strtol (image, &endptr, 0);
            if (*endptr == '\0') {
                imgValue->kind = V_LONG;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_OCTET:
        {
            c_short s;

            if (sscanf(image,"%hd",&s)) {
                imgValue->kind = V_OCTET;
                imgValue->is.Octet = (c_octet)s;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        }
        case P_USHORT:
            if (sscanf(image,"%hu",&imgValue->is.UShort)) {
                imgValue->kind = V_USHORT;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_ULONG:
            imgValue->is.ULong = strtoul (image, &endptr, 0);
            if (*endptr == '\0') {
                imgValue->kind = V_ULONG;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case P_CHAR:
            if (image != NULL) {
                imgValue->kind = V_CHAR;
                imgValue->is.Char = *image;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case  P_FLOAT:
            if (sscanf(image,"%f",&imgValue->is.Float)) {
                imgValue->kind = V_FLOAT;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        case  P_DOUBLE:
            imgValue->is.Double = strtod (image, &endptr);
            if (*endptr == '\0') {
                imgValue->kind = V_DOUBLE;
            } else {
                imgValue->kind = V_UNDEFINED;
            }
            break;
        default:
            assert(0);
        }
        break;
     default:
         assert(FALSE);
         break;
    }
    return (imgValue->kind != V_UNDEFINED);
}

c_equality
c_valueCompare(
    const c_value v1,
    const c_value v2)
{
    c_long r;

#define _CASE_(l,f) case l: if (v1.is.f>v2.is.f) return C_GT; \
                            if (v1.is.f<v2.is.f) return C_LT; \
                            return C_EQ

    switch(v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
    _CASE_(V_FLOAT,Float);
    _CASE_(V_DOUBLE,Double);
    _CASE_(V_OBJECT,Object);
    _CASE_(V_VOIDP,Voidp);
     case V_STRING:
     case V_WSTRING:
     case V_FIXED:
        if (v1.is.String == v2.is.String) {
            return C_EQ;
        }
        if (v1.is.String == NULL) {
            return C_LT;
        }
        if (v2.is.String == NULL) {
            return C_GT;
        }
        r = strcmp(v1.is.String,v2.is.String);
        if (r>0) {
            return C_GT;
        }
        if (r<0) {
            return C_LT;
        }
        return C_EQ;
     case V_UNDEFINED:
     case V_COUNT:
         assert(FALSE);
     break;
    }
#undef _CASE_
    assert(FALSE);
    return C_NE;
}


c_value
c_valueStringMatch (
    const c_value patternValue,
    const c_value stringValue)
{
    c_value v;
    c_char *pattern;
    c_char *str;
    c_char *strRef = NULL;
    c_char *ptnRef = NULL;

    assert(patternValue.kind == V_STRING);
    assert(stringValue.kind == V_STRING);

    pattern = patternValue.is.String;
    str = stringValue.is.String;

    assert(pattern != NULL);

    v.kind = V_BOOLEAN;
    if (str == NULL) {
        v.is.Boolean = FALSE;
        return v;
    }
    while ((*str != 0) && (*pattern != 0)) {
        if (*pattern == '*') {
            pattern++;
            while ((*str != 0) && (*str != *pattern)) { str++; }
            if (*str != 0) {
                strRef = str+1;
                ptnRef = pattern-1;
            }
        } else if (*pattern == '?') {
            pattern++;
            str++;
        } else if (*pattern++ != *str++) {
            if (strRef == NULL) {
                v.is.Boolean = FALSE;
                return v;
            }
            str = strRef;
            pattern = ptnRef;
            strRef = NULL;
        }
    }
    if (*str == (char)0) {
        while (*pattern == '*') { pattern++; }
        if (*pattern == (char)0) {
            v.is.Boolean = TRUE;
        } else {
            v.is.Boolean = FALSE;
        }
    } else {
        v.is.Boolean = FALSE;
    }
    return v;
}


c_value
c_valueADD (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t += v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
    _CASE_(V_FLOAT,Float);
    _CASE_(V_DOUBLE,Double);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueSUB (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t -= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
    _CASE_(V_FLOAT,Float);
    _CASE_(V_DOUBLE,Double);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueLOR (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t |= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueLXOR (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t ^= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueLAND (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t &= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueMUL (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t *= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
    _CASE_(V_FLOAT,Float);
    _CASE_(V_DOUBLE,Double);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueDIV (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t /= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
    _CASE_(V_FLOAT,Float);
    _CASE_(V_DOUBLE,Double);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueMOD (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,t) case l: v1.is.t %= v2.is.t; break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_BOOLEAN,Boolean);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valuePOW (
    c_value v1,
    c_value v2)
{
#define _CASE_(l,s,t) case l: v1.is.s = \
                              (t)pow(v1.is.s, v2.is.s); break
    switch (v1.kind) {
    _CASE_(V_ADDRESS,Address,c_address);
    _CASE_(V_SHORT,Short,c_short);
    _CASE_(V_LONG,Long,c_long);
    _CASE_(V_LONGLONG,LongLong,c_longlong);
    _CASE_(V_OCTET,Octet,c_octet);
    _CASE_(V_USHORT,UShort,c_ushort);
    _CASE_(V_ULONG,ULong,c_ulong);
    _CASE_(V_ULONGLONG,ULongLong,c_ulonglong);
    _CASE_(V_CHAR,Char,c_char);
    _CASE_(V_WCHAR,WChar,c_wchar);
    _CASE_(V_FLOAT,Float,c_float);
    _CASE_(V_DOUBLE,Double,c_double);
     default: assert(FALSE);
    }
    return v1;
#undef _CASE_
}

c_value
c_valueSL (
    c_value v,
    c_value a)
{
#define _CASE_(l,t) case l: v.is.t = v.is.t << a.is.Long; break
    switch(v.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
     default: assert(FALSE);
    }
    return v;
#undef _CASE_
}

c_value
c_valueSR (
    c_value v,
    c_value a)
{
#define _CASE_(l,t) case l: v.is.t = v.is.t >> a.is.Long; break
    switch(v.kind) {
    _CASE_(V_ADDRESS,Address);
    _CASE_(V_SHORT,Short);
    _CASE_(V_LONG,Long);
    _CASE_(V_LONGLONG,LongLong);
    _CASE_(V_OCTET,Octet);
    _CASE_(V_USHORT,UShort);
    _CASE_(V_ULONG,ULong);
    _CASE_(V_ULONGLONG,ULongLong);
    _CASE_(V_CHAR,Char);
    _CASE_(V_WCHAR,WChar);
     default: assert(FALSE);
    }
    return v;
#undef _CASE_
}

c_value
c_valueCast (
    const c_value v,
    c_valueKind kind)
{
    c_value r;
    if (v.kind == kind) {
        return v;
    }
    r.kind = kind;
    switch(kind) {
    case V_CHAR: switch(v.kind) {
        case V_OCTET:     r.is.Char = (c_char)v.is.Octet; break;
        case V_STRING:    r.is.Char = *(c_char*)v.is.String; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_SHORT: switch(v.kind) {
        case V_ADDRESS:   r.is.Short = (c_short)v.is.Address; break;
        case V_CHAR:      r.is.Short = (c_short)v.is.Char; break;
        case V_OCTET:     r.is.Short = (c_short)v.is.Octet; break;
        case V_LONG:      r.is.Short = (c_short)v.is.Long; break;
        case V_LONGLONG:  r.is.Short = (c_short)v.is.LongLong; break;
        case V_USHORT:    r.is.Short = (c_short)v.is.UShort; break;
        case V_ULONG:     r.is.Short = (c_short)v.is.ULong; break;
        case V_ULONGLONG: r.is.Short = (c_short)v.is.ULongLong; break;
        case V_WCHAR:     r.is.Short = (c_short)v.is.WChar; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_LONG: switch(v.kind) {
        case V_ADDRESS:   r.is.Long = (c_long)v.is.Address; break;
        case V_OCTET:     r.is.Long = (c_long)v.is.Octet; break;
        case V_CHAR:      r.is.Long = (c_long)v.is.Char; break;
        case V_SHORT:     r.is.Long = (c_long)v.is.Short; break;
        case V_LONGLONG:  r.is.Long = (c_long)v.is.LongLong; break;
        case V_USHORT:    r.is.Long = (c_long)v.is.UShort; break;
        case V_ULONG:     r.is.Long = (c_long)v.is.ULong; break;
        case V_ULONGLONG: r.is.Long = (c_long)v.is.ULongLong; break;
        case V_WCHAR:     r.is.Long = (c_long)v.is.WChar; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_LONGLONG: switch(v.kind) {
        case V_ADDRESS:   r.is.LongLong = (c_longlong)v.is.Address; break;
        case V_OCTET:     r.is.LongLong = (c_longlong)v.is.Octet; break;
        case V_CHAR:      r.is.LongLong = (c_longlong)v.is.Char; break;
        case V_SHORT:     r.is.LongLong = (c_longlong)v.is.Short; break;
        case V_LONG:      r.is.LongLong = (c_longlong)v.is.Long; break;
        case V_USHORT:    r.is.LongLong = (c_longlong)v.is.UShort; break;
        case V_ULONG:     r.is.LongLong = (c_longlong)v.is.ULong; break;
        case V_ULONGLONG:  r.is.LongLong = (c_longlong)v.is.ULongLong; break;
        case V_WCHAR:     r.is.LongLong = (c_longlong)v.is.WChar; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_FLOAT: switch(v.kind) {
        case V_ADDRESS:   r.is.Float = (c_float)v.is.Address; break;
        case V_OCTET:     r.is.Float = (c_float)v.is.Octet; break;
        case V_CHAR:      r.is.Float = (c_float)v.is.Char; break;
        case V_WCHAR:     r.is.Float = (c_float)v.is.WChar; break;
        case V_SHORT:     r.is.Float = (c_float)v.is.Short; break;
        case V_LONG:      r.is.Float = (c_float)v.is.Long; break;
        case V_LONGLONG:  r.is.Float = (c_float)v.is.LongLong; break;
        case V_USHORT:    r.is.Float = (c_float)v.is.UShort; break;
        case V_ULONG:     r.is.Float = (c_float)v.is.ULong; break;
        case V_ULONGLONG: r.is.Float = (c_float)v.is.ULongLong; break;
        case V_DOUBLE:    r.is.Float = (c_float)v.is.Double; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_DOUBLE: switch(v.kind) {
        case V_ADDRESS:   r.is.Double = (c_double)v.is.Address; break;
        case V_OCTET:     r.is.Double = (c_double)v.is.Octet; break;
        case V_CHAR:      r.is.Double = (c_double)v.is.Char; break;
        case V_WCHAR:     r.is.Double = (c_double)v.is.WChar; break;
        case V_SHORT:     r.is.Double = (c_double)v.is.Short; break;
        case V_LONG:      r.is.Double = (c_double)v.is.Long; break;
        case V_LONGLONG:  r.is.Double = (c_double)v.is.LongLong; break;
        case V_USHORT:    r.is.Double = (c_double)v.is.UShort; break;
        case V_ULONG:     r.is.Double = (c_double)v.is.ULong; break;
        case V_ULONGLONG: r.is.Double = (c_double)v.is.ULongLong; break;
        case V_FLOAT:     r.is.Double = (c_double)v.is.Float; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_USHORT: switch(v.kind) {
        case V_ADDRESS:   r.is.UShort = (c_ushort)v.is.Address; break;
        case V_OCTET:     r.is.UShort = (c_ushort)v.is.Octet; break;
        case V_WCHAR:     r.is.UShort = (c_ushort)v.is.WChar; break;
        case V_CHAR:      r.is.UShort = (c_ushort)v.is.Char; break;
        case V_LONG:      r.is.UShort = (c_ushort)v.is.Long; break;
        case V_LONGLONG:  r.is.UShort = (c_ushort)v.is.LongLong; break;
        case V_USHORT:    r.is.UShort = (c_ushort)v.is.UShort; break;
        case V_ULONG:     r.is.UShort = (c_ushort)v.is.ULong; break;
        case V_ULONGLONG: r.is.UShort = (c_ushort)v.is.ULongLong; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_ULONG: switch(v.kind) {
        case V_ADDRESS:   r.is.ULong = (c_ulong)v.is.Address; break;
        case V_CHAR:      r.is.ULong = (c_ulong)v.is.Char; break;
        case V_OCTET:     r.is.ULong = (c_ulong)v.is.Octet; break;
        case V_SHORT:     r.is.ULong = (c_ulong)v.is.Short; break;
        case V_LONG:      r.is.ULong = (c_ulong)v.is.Long; break;
        case V_LONGLONG:  r.is.ULong = (c_ulong)v.is.LongLong; break;
        case V_USHORT:    r.is.ULong = (c_ulong)v.is.UShort; break;
        case V_ULONG:     r.is.ULong = (c_ulong)v.is.ULong; break;
        case V_ULONGLONG: r.is.ULong = (c_ulong)v.is.ULongLong; break;
        case V_WCHAR:     r.is.ULong = (c_ulong)v.is.WChar; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_ULONGLONG: switch(v.kind) {
        case V_ADDRESS:   r.is.ULongLong = (c_ulonglong)v.is.Address; break;
        case V_CHAR:      r.is.ULongLong = (c_ulonglong)v.is.Char; break;
        case V_SHORT:     r.is.ULongLong = (c_ulonglong)v.is.Short; break;
        case V_LONG:      r.is.ULongLong = (c_ulonglong)v.is.Long; break;
        case V_LONGLONG:  r.is.ULongLong = (c_ulonglong)v.is.LongLong; break;
        case V_OCTET:     r.is.ULongLong = (c_ulonglong)v.is.Octet; break;
        case V_USHORT:    r.is.ULongLong = (c_ulonglong)v.is.UShort; break;
        case V_ULONG:     r.is.ULongLong = (c_ulonglong)v.is.ULong; break;
        case V_WCHAR:     r.is.ULongLong = (c_ulonglong)v.is.WChar; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_WCHAR: switch(v.kind) {
        case V_CHAR:      r.is.WChar = (c_wchar)v.is.Char; break;
        case V_OCTET:     r.is.WChar = (c_wchar)v.is.Octet; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_OCTET: switch(v.kind) {
        case V_ADDRESS:   r.is.Octet = (c_octet)v.is.Address; break;
        case V_CHAR:      r.is.Octet = (c_octet)v.is.Char; break;
        case V_WCHAR:     r.is.Octet = (c_octet)v.is.WChar; break;
        case V_SHORT:     r.is.Octet = (c_octet)v.is.Short; break;
        case V_LONG:      r.is.Octet = (c_octet)v.is.Long; break;
        case V_LONGLONG:  r.is.Octet = (c_octet)v.is.LongLong; break;
        case V_USHORT:    r.is.Octet = (c_octet)v.is.UShort; break;
        case V_ULONG:     r.is.Octet = (c_octet)v.is.ULong; break;
        case V_ULONGLONG: r.is.Octet = (c_octet)v.is.ULongLong; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    case V_BOOLEAN: switch(v.kind) {
        case V_ADDRESS:   r.is.Boolean = (c_bool)v.is.Address; break;
        case V_OCTET:     r.is.Boolean = (c_bool)v.is.Octet; break;
        case V_CHAR:      r.is.Boolean = (c_bool)v.is.Char; break;
        case V_SHORT:     r.is.Boolean = (c_bool)v.is.Short; break;
        case V_LONG:      r.is.Boolean = (c_bool)v.is.Long; break;
        case V_USHORT:    r.is.Boolean = (c_bool)v.is.UShort; break;
        case V_WCHAR:     r.is.Boolean = (c_bool)v.is.WChar; break;
        default:          r.kind = V_UNDEFINED; break;
        }
    break;
    default:              r.kind = V_UNDEFINED; break;
    }
    return r;
}

c_valueKind
c_normalizedKind (
    c_value v1,
    c_value v2)
{
    if (v1.kind == v2.kind) {
        return v1.kind;
    }
    switch(v1.kind) {

    case V_STRING: switch(v2.kind) {
        case V_CHAR:
            if (strlen(v1.is.String) == 1) {
                return v2.kind;
            }
        break;
        default:
        break;
        }
    break;
    case V_CHAR: switch(v2.kind) {
        case V_OCTET:
            return v1.kind;
        case V_STRING:
            if (strlen(v2.is.String) == 1) {
                return v1.kind;
            }
        break;
        default:
            return v2.kind;
        }
    break;
    case V_SHORT: switch(v2.kind) {
        case V_CHAR:
        case V_OCTET:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_LONG: switch(v2.kind) {
        case V_OCTET:
        case V_CHAR:
        case V_SHORT:
        case V_USHORT:
        case V_WCHAR:
        case V_LONGLONG:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_LONGLONG: switch(v2.kind) {
        case V_OCTET:
        case V_CHAR:
        case V_SHORT:
        case V_LONG:
        case V_USHORT:
        case V_ULONG:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_FLOAT: switch(v2.kind) {
        case V_OCTET:
        case V_CHAR:
        case V_SHORT:
        case V_LONG:
        case V_LONGLONG:
        case V_USHORT:
        case V_ULONG:
        case V_ULONGLONG:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_DOUBLE: switch(v2.kind) {
        case V_OCTET:
        case V_CHAR:
        case V_SHORT:
        case V_LONG:
        case V_LONGLONG:
        case V_USHORT:
        case V_ULONG:
        case V_ULONGLONG:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_USHORT: switch(v2.kind) {
        case V_OCTET:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_ULONG: switch(v2.kind) {
        case V_OCTET:
        case V_USHORT:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_ULONGLONG: switch(v2.kind) {
        case V_OCTET:
        case V_USHORT:
        case V_ULONG:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_WCHAR: switch(v2.kind) {
        case V_CHAR:
        case V_OCTET:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_OCTET: switch(v2.kind) {
        case V_CHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    case V_BOOLEAN: switch(v2.kind) {
        case V_OCTET:
        case V_CHAR:
        case V_SHORT:
        case V_LONG:
        case V_USHORT:
        case V_WCHAR:
            return v1.kind;
        default:
            return v2.kind;
        }
    default:
    break;
    }
    return V_UNDEFINED;
}

c_char *
c_operatorImage(
    c_operator o)
{
    switch(o) {
    case O_NE: return "NE";
    case O_EQ: return "EQ";
    case O_LT: return "LT";
    case O_GT: return "GT";
    case O_LE: return "LE";
    case O_GE: return "GE";
    case O_AND: return "AND";
    case O_OR: return "OR";
    case O_LEFT: return "LEFT";
    case O_RIGHT: return "RIGHT";
    case O_LOR: return "LOR";
    case O_LXOR: return "LXOR";
    case O_LAND: return "LAND";
    case O_ADD: return "ADD";
    case O_SUB: return "SUB";
    case O_MUL: return "MUL";
    case O_DIV: return "DIV";
    case O_MOD: return "MOD";
    case O_POW: return "POW";
    default: return "<Unknown>";
    }
}

c_value
c_valueCalculate(
    const c_value v1,
    const c_value v2,
    const c_operator o)
{
    c_value v1Cast;
    c_value v2Cast;
    c_value v;
    c_valueKind kind;

    purify_memset(&v, 0, sizeof(v));
    kind = c_normalizedKind(v1,v2);
    if (v1.kind != kind) {
        v1Cast = c_valueCast(v1,kind);
    } else {
        memcpy(&v1Cast, &v1, sizeof(v1));
    }
    if (v2.kind != kind) {
        v2Cast = c_valueCast(v2,kind);
    } else {
        memcpy(&v2Cast, &v2, sizeof(v2));
    }
    
    v.kind = V_BOOLEAN;
#define _EVAL_(l,r,o) (c_valueCompare(l,r) == o)
    switch(o) {
    case O_NE:    v.is.Boolean = !_EVAL_(v1Cast,v2Cast,C_EQ); break;
    case O_EQ:    v.is.Boolean =  _EVAL_(v1Cast,v2Cast,C_EQ); break;
    case O_LT:    v.is.Boolean =  _EVAL_(v1Cast,v2Cast,C_LT); break;
    case O_GT:    v.is.Boolean =  _EVAL_(v1Cast,v2Cast,C_GT); break;
    case O_LE:    v.is.Boolean =  _EVAL_(v1Cast,v2Cast,C_LT) || _EVAL_(v1Cast,v2Cast,C_EQ); break;
    case O_GE:    v.is.Boolean =  _EVAL_(v1Cast,v2Cast,C_GT) || _EVAL_(v1Cast,v2Cast,C_EQ); break;
    case O_AND:   v.is.Boolean = (v1Cast.is.Boolean && v2Cast.is.Boolean);  break;
    case O_OR:    v.is.Boolean = (v1Cast.is.Boolean || v2Cast.is.Boolean);  break;
    case O_LEFT:  return c_valueSL  (v1Cast,v2Cast);
    case O_RIGHT: return c_valueSR  (v1Cast,v2Cast);
    case O_LOR:   return c_valueLOR (v1Cast,v2Cast);
    case O_LXOR:  return c_valueLXOR(v1Cast,v2Cast);
    case O_LAND:  return c_valueLAND(v1Cast,v2Cast);
    case O_ADD:   return c_valueADD (v1Cast,v2Cast);
    case O_SUB:   return c_valueSUB (v1Cast,v2Cast);
    case O_MUL:   return c_valueMUL (v1Cast,v2Cast);
    case O_DIV:   return c_valueDIV (v1Cast,v2Cast);
    case O_MOD:   return c_valueMOD (v1Cast,v2Cast);
    case O_POW:   return c_valuePOW (v1Cast,v2Cast);
    default:      assert(FALSE);
    }
#undef _EVAL_

    return v;
}

c_value
c_valueKeepRef(
    c_value v)
{
    if (v.kind == V_STRING) {
        c_keep(v.is.String);
    } else if (v.kind == V_OBJECT) {
        c_keep(v.is.Object);
    } /* else { } */
    return v;
}

c_value
c_valueFreeRef(
    c_value v)
{
    if (v.kind == V_STRING) {
        c_free(v.is.String);
    } else if (v.kind == V_OBJECT) {
        c_free(v.is.Object);
    } /* else { } */
    return v;
}

