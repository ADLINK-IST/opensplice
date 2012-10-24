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
#ifndef V_GROUP_H
#define V_GROUP_H

/** \file kernel/include/v_group.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_entry.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef c_bool (*v_groupEntryAction)(v_entry e, c_voidp arg);
typedef c_bool (*v_groupWriterAction)(v_writer w, c_voidp arg);

/**
 * \brief The <code>v_group</code> cast method.
 *
 * This method casts an object to a <code>v_group</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_group</code> or
 * one of its subclasses.
 */
#define v_group(o) (C_CAST(o,v_group))

/**
 * \brief The <code>v_groupSample</code> cast method.
 *
 * This method casts an object to a <code>v_groupSample</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_groupSample</code> or
 * one of its subclasses.
 */
#define v_groupSample(o) (C_CAST(o,v_groupSample))

/**
 * \brief The <code>v_groupAction</code> cast method.
 *
 * This method casts an object to a <code>v_groupAction</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_groupAction</code> or
 * one of its subclasses.
 */
#define v_groupAction(o) (C_CAST(o,v_groupAction))

#define v_groupSampleTemplate(o) ((v_groupSampleTemplate)(o))

#define v_groupName(_this) \
        (v_entityName(v_group(_this)))

#define v_groupDataDescription(_this)\
        v_data(v_group(_this)->dataDescription)

#define v_groupTopic(_this)\
        v_topic(v_group(_this)->topic)

#define v_groupPartition(_this)\
        v_partition(v_group(_this)->partition)

#define v_groupSampleMessage(_this) \
        (v_groupSampleTemplate(_this)->message)

#define v_groupSampleSetMessage(_this,_message) \
        (v_groupSampleTemplate(_this)->message = c_keep(_message))

#define v_groupPartitionAccessMode(_this)\
        (v_group(_this)->partitionAccessMode)

OS_API void
v_groupFree (
    v_group _this);

OS_API void
v_groupAddEntry (
    v_group _this,
    v_entry e);

OS_API void
v_groupRemoveEntry (
    v_group _this,
    v_entry e);

OS_API v_entry
v_groupLookupEntry (
    v_group _this,
    v_reader r);

OS_API c_bool
v_groupAddStream (
    v_group _this,
    v_groupStream stream);

OS_API c_bool
v_groupRemoveStream (
    v_group _this,
    v_groupStream stream);

OS_API c_long
v_groupSampleCount (
    v_group _this);

OS_API v_writeResult
v_groupWrite (
    v_group _this, v_message o,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId);

OS_API v_writeResult
v_groupWriteNoStream (
    v_group _this,
    v_message message,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId);

OS_API v_writeResult
v_groupWriteNoStreamWithEntry (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    v_entry entry);

OS_API v_writeResult
v_groupDeleteHistoricalData (
    v_group _this,
    c_time t);

OS_API c_bool
v_groupNwAttachedGet (
    v_group _this );

OS_API c_time
v_groupCreationTimeGet (
    v_group _this );

OS_API c_bool
v_groupCompleteGet (
    v_group _this );

OS_API void
v_groupCompleteSet (
    v_group _this,
    c_bool complete);

OS_API void
v_groupNotifyAwareness (
    v_group _this,
    const c_char* serviceName,
    c_bool interested);

OS_API v_groupAttachState
v_groupServiceGetAttachState (
    v_group _this,
    const c_char* serviceName);

OS_API void
v_groupFlush (
    v_group _this);

OS_API void
v_groupFlushAction (
    v_group _this,
    c_action action,
    c_voidp arg);

OS_API c_bool
v_groupWalkEntries (
    v_group _this,
    v_groupEntryAction action,
    c_voidp arg);

OS_API c_bool
v_groupWaitForComplete (
    v_group _this,
    c_time waitTime);

OS_API v_groupSample
v_groupSampleNew (
    v_group _this,
    v_message message);

OS_API void
v_groupGetHistoricalData (
    v_group _this,
    v_entry e);

OS_API void
v_groupStreamHistoricalData(
    v_group g,
    v_groupStream stream);

OS_API c_bool
v_groupGetHistoricalDataWithCondition(
    v_group g,
    v_entry entry,
    v_historicalDataRequest request);

OS_API void
v_groupFlushActionWithCondition(
    v_group  g,
    v_historicalDataRequest request,
    c_action action,
    c_voidp arg);

OS_API void
v_groupUpdatePurgeList(
    v_group group);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
