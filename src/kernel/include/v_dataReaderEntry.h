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

typedef enum {
    V_DATAREADER_INSERTED,
    V_DATAREADER_OUTDATED,
    V_DATAREADER_NOT_OWNER,
    V_DATAREADER_MAX_SAMPLES,
    V_DATAREADER_MAX_INSTANCES,
    V_DATAREADER_INSTANCE_FULL,
    V_DATAREADER_SAMPLE_LOST,
    V_DATAREADER_DUPLICATE_SAMPLE,
    V_DATAREADER_OUT_OF_MEMORY,
    V_DATAREADER_INTERNAL_ERROR,
    V_DATAREADER_UNDETERMINED,
    V_DATAREADER_FILTERED_OUT,
    V_DATAREADER_COUNT
} v_dataReaderResult;

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
    c_array filterInstance,
    c_array filterData);

OS_API void
v_dataReaderEntryFree(
    v_dataReaderEntry _this);

OS_API v_writeResult
v_dataReaderEntryWrite(
    v_dataReaderEntry _this,
    v_message o,
    v_instance *instance);

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

OS_API void
v_dataReaderEntryAbortTransaction(
    v_dataReaderEntry _this,
    v_gid writerGID);

OS_API v_writeResult
v_dataReaderEntryDisposeAll (
    v_dataReaderEntry _this,
    v_message disposeMsg);

OS_API v_dataReaderResult
v_dataReaderEntryApplyUnregisterMessageToInstanceList (
    v_dataReaderEntry _this,
    v_message unregisterMsg,
    c_iter instanceList);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
