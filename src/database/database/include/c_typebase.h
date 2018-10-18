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
#ifndef C_TYPEBASE_H
#define C_TYPEBASE_H

#include "os_defs.h"
#include <limits.h>
#include <stddef.h>

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define C_STRUCT(name)  struct name##_s
#define C_EXTENDS(type) C_STRUCT(type) _parent
#define C_CLASS(name)   typedef C_STRUCT(name) *name

#define C_SIZEOF(name)  sizeof(C_STRUCT(name))
#define C_ADDRESS(ptr)  ((c_address)(ptr))

#define C_ALIGNSIZE(size,alignment) \
        (((((size)-1)/(alignment))+1)*(alignment))

#define C_MAXALIGNSIZE(size) \
        C_ALIGNSIZE(size,C_MAXALIGNMENT)

#define C_DISPLACE(ptr,offset) \
        ((c_object)(C_ADDRESS(ptr)+C_ADDRESS(offset)))

#define C_REFGET(ptr,offset) \
        (c_object(*((c_object *)C_DISPLACE((ptr),(offset)))))

#define C_REFSET(ptr,offset,o) \
        {c_object*r=(c_object *)C_DISPLACE((ptr),(offset));*r=o;}

C_CLASS(c_base);
C_CLASS(c_type);

/* VERSION is the deprecated form of OSPL_VERSION
 * This is only defined when building OpenSplice itself */
#if defined (VERSION) || defined (OSPL_VERSION)
/* TODO: Check this definition of NULL. See OSPL-2272 */
#undef NULL
#define NULL (0)
#endif

typedef os_address          c_address;
typedef os_size_t           c_size;
typedef void               *c_object;
typedef void               *c_voidp;
typedef os_uchar            c_octet;
typedef os_short            c_short;
typedef os_int32            c_long;
typedef os_ushort           c_ushort;
typedef os_uint32           c_ulong;
typedef os_char             c_char;
typedef os_short            c_wchar;
typedef os_float            c_float;
typedef os_double           c_double;
typedef os_uchar            c_bool;
typedef os_int64            c_longlong;
typedef os_uint64           c_ulonglong;
typedef c_char             *c_string;
typedef c_wchar            *c_wstring;
typedef c_object           *c_array;
typedef c_object           *c_sequence;

/* min and max definitions
 * =======================
 * These values are deliberately written as decimals, because of the following
 * reasons:
 *  1) (short)0x8000 == -32768, but (int)0x8000 == 32768, so we cannot guarantee
 *     the correct value if written hexadecimal.
 *  2) we cannot assume each platform uses the Two's Complement notation.
 */
#define C_MAX_SCHAR            INT8_MAX                         /* 0x7f                                 */
#define C_MIN_SCHAR            INT8_MIN                         /* 0x80                                 */
#define C_MAX_UCHAR            UINT8_MAX                        /* 0xff                                 */
#define C_MIN_UCHAR            0                                /* 0x0                                  */
#define C_MAX_OCTET            UINT8_MAX                        /* 0xFF                                 */
#define C_MIN_OCTET            0                                /* 0x0                                  */
#define C_MAX_SHORT            INT16_MAX                        /* 0x7FFF                               */
#define C_MIN_SHORT            INT16_MIN                        /* 0x8000     Two's Complement          */
#define C_MAX_USHORT           UINT16_MAX                       /* 0xFFFF                               */
#define C_MIN_USHORT           0                                /* 0x0                                  */
#define C_MAX_LONG             INT32_MAX                        /* 0x7FFFFFFF                           */
#define C_MIN_LONG             INT32_MIN                        /* 0x80000000 Two's Complement          */
#define C_MAX_ULONG            UINT32_MAX                       /* 0xFFFFFFFF                           */
#define C_MIN_ULONG            0U                               /* 0x0                                  */
#define C_MAX_LONGLONG         INT64_MAX                        /* 0x7FFFFFFFFFFFFFFF                   */
#define C_MIN_LONGLONG         INT64_MIN                        /* 0x8000000000000000 Two's Complement  */
#define C_MIN_ULONGLONG        0ULL                             /* 0x0                                  */
#define C_MAX_ULONGLONG        UINT64_MAX                       /* 0xFFFFFFFFFFFFFFFF                   */
#define C_MIN_FLOAT            1.1754943508222875E-38
#define C_MAX_FLOAT            3.4028234663852886E+38
#define C_MIN_DOUBLE           2.2250738585072014E-308
#define C_MAX_DOUBLE           1.7976931348623157E+308
#define C_MIN_SIZE             0
#define C_MAX_SIZE             OS_MAX_INTEGER(c_size)

#ifdef NATIVESEQ

typedef struct c_sequence {
    c_ulong maximum;
    c_ulong length;
    c_bool  release;
    c_voidp buffer;
} c_sequence;

#endif

typedef enum c_fixType {
    C_PREFIX,
    C_INFIX,
    C_POSTFIX
} c_fixType;

typedef enum c_equality {
    C_PL = -4,    /* Partial less (structure)    */
    C_EL = -3,    /* Less or Equal (set)         */
    C_LE = -2,    /* Less or Equal               */
    C_LT = -1,    /* Less                        */
    C_EQ = 0,     /* Equal                       */
    C_GT = 1,     /* Greater                     */
    C_GE = 2,     /* Greater or Equal            */
    C_EG = 3,     /* Greater or Equal (set)      */
    C_PG = 4,     /* Partial greater (structure) */
    C_PE = 10,    /* Partial Equal               */
    C_NE = 20,    /* Not equal                   */
    C_ER = 99     /* Error: equality undefined   */
} c_equality;

typedef enum c_operator {
    O_AND,  O_OR,
    O_LE,   O_LT,    O_EQ,  O_GT,   O_GE,  O_NE,
    O_ADD,  O_SUB,   O_MUL, O_DIV,  O_MOD, O_POW,
    O_LEFT, O_RIGHT, O_LOR, O_LXOR, O_LAND
} c_operator;

typedef enum c_valueKind {
    V_UNDEFINED,
    V_ADDRESS, V_BOOLEAN, V_OCTET,
    V_SHORT,   V_LONG,   V_LONGLONG,
    V_USHORT,  V_ULONG,  V_ULONGLONG,
    V_FLOAT,   V_DOUBLE,
    V_CHAR,    V_STRING,
    V_WCHAR,   V_WSTRING,
    V_FIXED,   V_OBJECT,
    V_VOIDP,
    V_COUNT
} c_valueKind;

typedef enum c_memoryThreshold_e
{
    C_MEMTHRESHOLD_OK,
    C_MEMTHRESHOLD_APP_REACHED,
    C_MEMTHRESHOLD_SERV_REACHED
} c_memoryThreshold;

typedef struct c_value {
    c_valueKind kind;
    union {
        c_address    Address;
        c_short      Short;
        c_long       Long;
        c_longlong   LongLong;
        c_octet      Octet;
        c_ushort     UShort;
        c_ulong      ULong;
        c_ulonglong  ULongLong;
        c_char       Char;
        c_wchar      WChar;
        c_float      Float;
        c_double     Double;
        c_string     String;
        c_wstring    WString;
        c_string     Fixed;
        c_bool       Boolean;
        c_voidp      Object;
        c_voidp      Voidp;
    } is;
} c_value;

#define c_value(v) ((c_value)(v))

#define C_ALIGNMENT_TYPE(t) struct c_alignment_type_##t { char c; t x; }
C_ALIGNMENT_TYPE (c_address);
C_ALIGNMENT_TYPE (c_size);
C_ALIGNMENT_TYPE (c_voidp);
C_ALIGNMENT_TYPE (c_octet);
C_ALIGNMENT_TYPE (c_short);
C_ALIGNMENT_TYPE (c_ushort);
C_ALIGNMENT_TYPE (c_long);
C_ALIGNMENT_TYPE (c_ulong);
C_ALIGNMENT_TYPE (c_char);
C_ALIGNMENT_TYPE (c_wchar);
C_ALIGNMENT_TYPE (c_float);
C_ALIGNMENT_TYPE (c_double);
C_ALIGNMENT_TYPE (c_bool);
C_ALIGNMENT_TYPE (c_longlong);
C_ALIGNMENT_TYPE (c_ulonglong);
C_ALIGNMENT_TYPE (c_string);
C_ALIGNMENT_TYPE (c_wstring);
C_ALIGNMENT_TYPE (c_array);
C_ALIGNMENT_TYPE (c_sequence);
C_ALIGNMENT_TYPE (pa_uint32_t);
C_ALIGNMENT_TYPE (pa_uintptr_t);
C_ALIGNMENT_TYPE (pa_voidp_t);
C_ALIGNMENT_TYPE (c_value);

#define C_ALIGNMENT(t) ((c_ulong) offsetof (struct c_alignment_type_##t, x))
#define C_MAXALIGNMENT C_ALIGNMENT(c_value)

#define C_ALIGNMENT_C_STRUCT_TYPE(t) struct c_alignment_type_struct_##t { char c; C_STRUCT(t) x; }
#define C_ALIGNMENT_C_STRUCT(t) ((c_ulong) offsetof (struct c_alignment_type_struct_##t, x))

OS_API c_value c_undefinedValue (void);
OS_API c_value c_shortValue     (const c_short     value);
OS_API c_value c_addressValue   (const c_address value);
OS_API c_value c_longValue      (const c_long      value);
OS_API c_value c_longlongValue  (const c_longlong  value);
OS_API c_value c_ushortValue    (const c_ushort    value);
OS_API c_value c_ulongValue     (const c_ulong     value);
OS_API c_value c_ulonglongValue (const c_ulonglong value);
OS_API c_value c_boolValue      (const c_bool      value);
OS_API c_value c_octetValue     (const c_octet     value);
OS_API c_value c_charValue      (const c_char      value);
OS_API c_value c_wcharValue     (const c_wchar     value);
OS_API c_value c_floatValue     (const c_float     value);
OS_API c_value c_doubleValue    (const c_double    value);
OS_API c_value c_stringValue    (const c_string    value);
OS_API c_value c_wstringValue   (const c_wstring   value);
OS_API c_value c_objectValue    (const c_voidp     object);
OS_API c_value c_voidpValue     (const c_voidp     object);

OS_API c_value c_shortMinValue      (void);
OS_API c_value c_longMinValue       (void);
OS_API c_value c_longlongMinValue   (void);
OS_API c_value c_ushortMinValue     (void);
OS_API c_value c_ulongMinValue      (void);
OS_API c_value c_ulonglongMinValue  (void);
OS_API c_value c_boolMinValue       (void);
OS_API c_value c_octetMinValue      (void);
OS_API c_value c_charMinValue       (void);
OS_API c_value c_floatMinValue      (void);
OS_API c_value c_doubleMinValue     (void);

OS_API c_value c_shortMaxValue      (void);
OS_API c_value c_longMaxValue       (void);
OS_API c_value c_longlongMaxValue   (void);
OS_API c_value c_ushortMaxValue     (void);
OS_API c_value c_ulongMaxValue      (void);
OS_API c_value c_ulonglongMaxValue  (void);
OS_API c_value c_boolMaxValue       (void);
OS_API c_value c_octetMaxValue      (void);
OS_API c_value c_charMaxValue       (void);
OS_API c_value c_floatMaxValue      (void);
OS_API c_value c_doubleMaxValue     (void);

OS_API c_char *   c_valueImage       (const c_value v);
OS_API c_bool     c_imageValue       (const char *image, c_value *imgValue, c_type imgType);
OS_API c_equality c_valueCompare     (const c_value v1, const c_value v2);
OS_API c_value    c_valueStringMatch (const c_value patternValue, const c_value stringValue);
OS_API c_value    c_valueCalculate   (const c_value v1, const c_value v2, const c_operator o);
OS_API c_value    c_valueCast        (const c_value v,  const c_valueKind kind);
OS_API c_value    c_valueKeepRef     (c_value v);
OS_API c_value    c_valueFreeRef     (c_value v);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_TYPEBASE_H */
