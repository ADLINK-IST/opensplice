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
#ifndef U_DATAREADER_H
#define U_DATAREADER_H

#include "u_types.h"
#include "u_reader.h"
#include "v_readerQos.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_dataReader(o) ((u_dataReader)(o))

typedef void (*u_copyIn)(c_type type, void *data, void *to);

OS_API u_dataReader
u_dataReaderNew(
    u_subscriber _scope,
    const c_char *name,
    q_expr OQLexpr,
    c_value params[],
    v_readerQos qos,
    c_bool enable);

OS_API u_result
u_dataReaderFree(
    u_dataReader _this);

OS_API u_result
u_dataReaderRead(
    u_dataReader _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_dataReaderTake(
    u_dataReader _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API void *
u_dataReaderReadList(
    u_dataReader _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg);

OS_API void *
u_dataReaderTakeList(
    u_dataReader _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg);

OS_API u_result
u_dataReaderReadInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_dataReaderTakeInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_dataReaderReadNextInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_dataReaderTakeNextInstance(
    u_dataReader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_dataReaderWaitForHistoricalData(
    u_dataReader _this,
    c_time timeout);

OS_API u_result
u_dataReaderWaitForHistoricalDataWithCondition(
    u_dataReader _this,
    c_char* filter,
    c_char* params[],
    c_ulong paramsLength,
    c_time min_source_time,
    c_time max_source_time,
    struct v_resourcePolicy* resourceLimits,
    c_time timeout);

OS_API u_result
u_dataReaderLookupInstance(
    u_dataReader _this,
    c_voidp keyTemplate,
    u_copyIn copyIn,
    u_instanceHandle *handle);

#if 1

/* Deprecated */

OS_API c_bool
u_dataReaderDefaultCopy(
    v_collection c,
    c_object o,
    c_voidp actionArg);
#endif

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
