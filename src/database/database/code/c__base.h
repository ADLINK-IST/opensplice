/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

#include "c_base.h"
#include "c_mmbase.h"
#include "c__extent.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define c__getBase(o) c_getBase(o)
#define c__getType(o) c_getType(o)

#define _TYPE_CACHE_(_type) \
        static c_type _##_type##_t = NULL; \
        c_type \
        _type##_t (c_base _this) \
        { \
            if (!_##_type##_t) { \
                _##_type##_t = c_resolve(_this,#_type); \
            } \
            return c_keep(_##_type##_t); \
        }

/** @fn c_getMetaType (c_base base, c_metaKind kind)
    @brief Lookup the database meta data description of the specified meta type kind.
*/
OS_API c_type   c_getMetaType (c_base base, c_metaKind kind);
/** @fn c_baseMM (c_base base)
    @brief Lookup the memory management object of the given database.
*/
OS_API c_mm     c_baseMM      (c_base base);
OS_API c_long   c_typeSize    (c_type type, c_voidp data);

OS_API void     c__free(c_object object);

OS_API c_long   c_memsize(c_type type);
OS_API c_object c_mem2object(c_voidp mem, c_type type);
OS_API c_voidp  c_object2mem(c_object o);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
