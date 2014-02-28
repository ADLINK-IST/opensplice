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
#ifndef GAPI_TYPESUPPORT_H
#define GAPI_TYPESUPPORT_H

#include "gapi_common.h"
#include "gapi_object.h"

#include "c_base.h"
#include "c_typebase.h"
#include "c_metabase.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define _TypeSupport(o) ((_TypeSupport)(o))

#define gapi_typeSupportClaim(h,r) \
        (_TypeSupport(gapi_objectClaim(h,OBJECT_KIND_TYPESUPPORT,r)))

#define gapi_typeSupportClaimNB(h,r) \
        (_TypeSupport(gapi_objectClaimNB(h,OBJECT_KIND_TYPESUPPORT,r)))

#define _TypeSupportAlloc() \
        (_TypeSupport(_ObjectAlloc(OBJECT_KIND_TYPESUPPORT, \
                                    C_SIZEOF(_TypeSupport), \
                                    _TypeSupport_free)))

C_STRUCT(_TypeSupport) {
    C_EXTENDS(_Object);
    gapi_unsigned_long    refCount;
    gapi_string           type_name;
    gapi_string           type_keys;
    gapi_string           type_def;
    gapi_typeSupportLoad  type_load;
    gapi_unsigned_long    alloc_size;
    gapi_topicAllocBuffer alloc_buffer;
    gapi_copyIn           copy_in;
    gapi_copyOut          copy_out;
    gapi_copyCache        copy_cache;
    gapi_readerCopy       reader_copy;
    gapi_writerCopy       writer_copy;
    c_metaObject          typeSpec;
    gapi_boolean          useTypeinfo;
};

OS_API _TypeSupport
_TypeSupportNew (
    const gapi_char *type_name,
    const gapi_char *type_keys,
    const gapi_char *type_def,
    gapi_typeSupportLoad type_load,
    gapi_copyIn copy_in,
    gapi_copyOut copy_out,
    gapi_unsigned_long alloc_size,
    gapi_topicAllocBuffer alloc_buffer,
    gapi_readerCopy reader_copy,
    gapi_writerCopy writer_copy);

OS_API gapi_returnCode_t
gapi_typeSupport_register_type (
    gapi_typeSupport _this,
    gapi_domainParticipant dp,
    gapi_string name);

OS_API void
_TypeSupportDup (
    _TypeSupport _this);


OS_API gapi_typeSupportLoad
_TypeSupportTypeLoad (
    _TypeSupport _this);

OS_API gapi_char *
_TypeSupportTypeName (
    _TypeSupport _this);

OS_API gapi_char *
_TypeSupportTypeKeys (
    _TypeSupport _this);

OS_API gapi_readerCopy
_TypeSupportGetReaderCopy (
    _TypeSupport _this);

OS_API gapi_writerCopy
_TypeSupportGetWriterCopy (
    _TypeSupport _this);

OS_API gapi_copyIn
_TypeSupportCopyIn (
    _TypeSupport _this);

OS_API gapi_copyOut
_TypeSupportCopyOut (
    _TypeSupport _this);

OS_API gapi_copyCache
_TypeSupportCopyCache (
    _TypeSupport _this);

OS_API gapi_unsigned_long
_TypeSupportTopicAllocSize (
    _TypeSupport _this);

OS_API gapi_topicAllocBuffer
_TypeSupportTopicAllocBuffer (
    _TypeSupport _this);

OS_API gapi_boolean
_TypeSupportEquals (
    _TypeSupport t1,
    _TypeSupport t2);

OS_API gapi_returnCode_t
_TypeSupportGenericCopyInit (
    _TypeSupport       _this,
    _DomainParticipant participant);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
