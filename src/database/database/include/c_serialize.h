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
#ifndef C_SERIALIZE_H_
#define C_SERIALIZE_H_

#include "c_metabase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef c_voidp c_serializeActionArg;
typedef void  (* c_serializeGetBufferAction) (
                 c_octet **buffer, c_ulong *length, c_serializeActionArg arg);

OS_API void
c_serialize(
    c_object object,
    c_octet **buffer,
    c_ulong *length,
    c_serializeGetBufferAction action,
    c_serializeActionArg arg);

OS_API c_object
c_deserialize(
    c_type type,
    c_octet **buffer,
    c_ulong *length,
    c_serializeGetBufferAction action,
    c_serializeActionArg arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
