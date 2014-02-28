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
/** @file c__base.h
    @brief The database protected interface.
*/

#ifndef C__BASE_H
#define C__BASE_H

#include "ut_avl.h"
#include "c_base.h"
#include "c_mmbase.h"
#include "c_module.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define c__getBase(o) c_getBase(o)
#define c__getType(o) c_getType(o)

#define _TYPE_CACHE_(_type) \
        c_type \
        _type##_t (c_base _this) \
        { \
            if (!_this->baseCache.typeCache._type##_t) { \
               _this->baseCache.typeCache._type##_t = c_resolve(_this,#_type); \
            } \
            return c_keep(_this->baseCache.typeCache._type##_t); \
        }

C_STRUCT(c_queryCache) {
    c_type c_qConst_t;
    c_type c_qType_t;
    c_type c_qVar_t;
    c_type c_qField_t;
    c_type c_qFunc_t;
    c_type c_qPred_t;
    c_type c_qKey_t;
    c_type c_qRange_t;
    c_type c_qExpr_t;
};

C_STRUCT(c_fieldCache) {
    c_type c_field_t;
    c_collectionType c_fieldPath_t;
    c_collectionType c_fieldRefs_t;
};

C_STRUCT(c_typeCache) {
    c_type c_object_t;
    c_type c_voidp_t;
    c_type c_bool_t;
    c_type c_address_t;
    c_type c_octet_t;
    c_type c_char_t;
    c_type c_short_t;
    c_type c_long_t;
    c_type c_longlong_t;
    c_type c_uchar_t;
    c_type c_ushort_t;
    c_type c_ulong_t;
    c_type c_ulonglong_t;
    c_type c_float_t;
    c_type c_double_t;
    c_type c_string_t;
    c_type c_wchar_t;
    c_type c_wstring_t;
    c_type c_array_t;
    c_type c_type_t;
    c_type c_valueKind_t;
    c_type c_member_t;
    c_type c_literal_t;
    c_type c_constant_t;
    c_type c_unionCase_t;
    c_type c_property_t;
};

C_STRUCT(c_baseCache) {
    C_STRUCT(c_queryCache) queryCache;
    C_STRUCT(c_fieldCache) fieldCache;
    C_STRUCT(c_typeCache) typeCache;
};

C_STRUCT(c_base) {
    C_EXTENDS(c_module);
    c_mm      mm;
    c_bool    maintainObjectCount;
    c_long    confidence;
    ut_avlTree_t bindings;
    c_mutex   bindLock;
    c_mutex   serLock; /* currently only used for defining enums from sd_serializerXMLTypeinfo.c */
    c_type    metaType[M_COUNT];
    c_type    string_type;
    c_string  emptyString;
    c_wstring  emptyWstring;
    C_STRUCT(c_baseCache) baseCache;

#ifndef NDEBUG
#ifdef OBJECT_WALK
    c_object  firstObject;
    c_object  lastObject;
#endif
#endif
};

/** @fn c_getMetaType (c_base base, c_metaKind kind)
    @brief Lookup the database meta data description of the specified meta type kind.
*/
OS_API c_type   c_getMetaType (c_base base, c_metaKind kind);
/** @fn c_baseMM (c_base base)
    @brief Lookup the memory management object of the given database.
*/
OS_API void     c__free(c_object object);

OS_API c_long   c_memsize(c_type type);
OS_API c_object c_mem2object(c_voidp mem, c_type type);
OS_API c_voidp  c_object2mem(c_object o);
#ifndef NDEBUG
OS_API void     c__assertValidDatabaseObject(c_voidp o);
#define c_assertValidDatabaseObject(o) c__assertValidDatabaseObject(o)
#else
#define c_assertValidDatabaseObject(o)
#endif

OS_API c_mutexAttr c_baseGetMutexAttr(c_base base);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
