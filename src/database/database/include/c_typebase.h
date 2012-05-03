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
#ifndef C_TYPEBASE_H
#define C_TYPEBASE_H

#include "os_defs.h"
#include <limits.h>

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
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

#define C_ALIGNMENT(t) \
        ((c_ulong)(&((struct {c_char d;t a;}*)(void *)0)->a))

#define C_MAXALIGNMENT \
        (C_ALIGNMENT(c_value))

#define C_ALIGNSIZE(size,alignment) \
        ((((size-1)/C_MAXALIGNMENT)+1)*C_MAXALIGNMENT)

#define C_MAXALIGNSIZE(size) \
        C_ALIGNSIZE(size,C_MAXALIGNMENT)

#define C_DISPLACE(ptr,offset) \
        ((c_object)(C_ADDRESS(ptr)+C_ADDRESS(offset)))

#define C_REFGET(ptr,offset) \
        (c_object(*((c_object *)C_DISPLACE((ptr),(offset)))))

#define C_REFSET(ptr,offset,o) \
        {c_object*r=(c_object *)C_DISPLACE((ptr),(offset));*r=o;}

C_CLASS(c_base);

/** \todo to be moved to abstraction layer */
#if !defined FALSE || (FALSE != 0)
#undef FALSE
#define FALSE              (0)
#endif
#if !defined TRUE || (TRUE != (!FALSE))
#undef TRUE
#define TRUE               (!FALSE)
#endif

#define OS_ALIGNMENT       (8)

#undef NULL
#define NULL (0)

typedef os_address          c_address;
typedef c_address           c_size;
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
#define C_MIN_CHAR             '\0'
#define C_MAX_CHAR             '\0xff'
#define C_MIN_OCTET            0                                /* 0x0                                  */
#define C_MAX_OCTET            255                              /* 0xFF                                 */
#define C_MIN_SHORT(sa)       -32768##sa                        /* 0x8000     Two's Complement          */
#define C_MAX_SHORT(sa)        32767##sa                        /* 0x7FFF                               */
#define C_MIN_USHORT(sa)       0U##sa                           /* 0x0                                  */
#define C_MAX_USHORT(sa)       65535U##sa                       /* 0xFFFF                               */
#define C_MIN_LONG(sa)        -2147483648##sa                   /* 0x80000000 Two's Complement          */
#define C_MAX_LONG(sa)         2147483647U##sa                  /* 0x7FFFFFFF                           */
#define C_MIN_ULONG(sa)        0U##sa                           /* 0x0                                  */
#define C_MAX_ULONG(sa)        4294967295U##sa                  /* 0xFFFFFFFF                           */
#define C_MIN_LONGLONG(sa)    -9223372036854775808##sa          /* 0x8000000000000000 Two's Complement  */
#define C_MAX_LONGLONG(sa)     9223372036854775807##sa          /* 0x7FFFFFFFFFFFFFFF                   */
#define C_MIN_ULONGLONG(sa)    0U##sa                           /* 0x0                                  */
#define C_MAX_ULONGLONG(sa)    18446744073709551615U##sa        /* 0xFFFFFFFFFFFFFFFF                   */
#define C_MIN_FLOAT            1.1754944909521339405E-38F
#define C_MAX_FLOAT            3.4028234663852885981E+38F
#define C_MIN_DOUBLE           1.1125369292536011856E-308
#define C_MAX_DOUBLE           1.7976931348623157081E+308
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

OS_API c_char *   c_valueImage       (const c_value v);
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
