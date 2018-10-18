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
#ifndef V_DATAREADERENTRY_H
#define V_DATAREADERENTRY_H

/** \file kernel/include/v_dataReaderEntry.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_entry.h"

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

/**
 * \brief The <code>v_dataReaderEntry</code> cast method.
 *
 * This method casts an object to a <code>v_dataReaderEntry</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataReaderEntry</code> or
 * one of its subclasses.
 */
#define v_dataReaderEntry(e) (C_CAST(e,v_dataReaderEntry))

#define v_dataReaderEntryTopic(_this) \
        v_topic(v_dataReaderEntry(_this)->topic)

OS_API v_dataReaderEntry
v_dataReaderEntryNew(
    v_dataReader dataReader,
    v_topic topic,
    q_expr _where,
    const c_value *params[],
    os_uint32 nrOfParams);

OS_API void
v_dataReaderEntryFree(
    v_dataReaderEntry _this);

OS_API void
v_dataReaderEntrySetTransactionAdmin(
    _Inout_ v_dataReaderEntry _this,
    _In_opt_ v_transactionGroupAdmin admin);

OS_API v_writeResult
v_dataReaderEntryWrite(
    v_dataReaderEntry _this,
    v_message o,
    v_instance *instance,
    v_messageContext context);

OS_API v_writeResult
v_dataReaderEntryWriteEOT(
    v_dataReaderEntry _this,
    v_message message);

OS_API void
v_dataReaderEntryAddIncompatibleWriter(
    v_dataReaderEntry _this,
    v_gid *writerGID);

OS_API void
v_dataReaderEntryRemoveIncompatibleWriter(
    v_dataReaderEntry _this,
    v_gid *writerGID);

OS_API void
v_dataReaderEntryUpdatePurgeLists(
    v_dataReaderEntry _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
