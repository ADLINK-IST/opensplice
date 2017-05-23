/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef c_bool (*v_groupEntryAction)(v_entry e, c_voidp arg);
typedef c_bool (*v_groupWriterAction)(v_writer w, c_voidp arg);
typedef c_bool (*groupInstanceDisposeFunc)(v_groupInstance instance, c_voidp arg);
typedef c_bool (*dataReaderInstanceDisposeFunc)(v_dataReaderInstance instance, c_voidp arg);


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
        ((v_group(_this))->name != NULL?(v_group(_this))->name:"NULL")

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

#define v_groupwriterAdministration(o) (C_CAST(o,v_groupwriterAdministration))


typedef c_equality (*v_matchIdentityAction)(v_gid id1, v_gid id2);

typedef enum {
    V_GROUP_FLUSH_REGISTRATION,     /* The object flushed is a v_registration */
    V_GROUP_FLUSH_UNREGISTRATION,   /* The object flushed is a v_registration */
    V_GROUP_FLUSH_MESSAGE,          /* The object flushed is a v_message */
    V_GROUP_FLUSH_TRANSACTION       /* The object flushed is a v_transaction */
} v_groupFlushType;

struct v_groupFlushData {
    c_object object;
    v_groupInstance instance;
    v_groupFlushType flushType;
};

typedef c_bool (*v_groupFlushCallback)
               (c_object obj, v_groupInstance instance,
                       v_groupFlushType flushType, c_voidp arg);

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

OS_API c_bool
v_groupSetRoutingEnabled(
    v_group group,
    c_bool routingEnabled);

/* The following hash defines implement a basic form of
 * destination identification for resend messages.
 * The resendScope is a set of these bits specifying the
 * resend scope.
 */

#define V_RESEND_NONE        (0u)
#define V_RESEND_TOPIC       (1u)
#define V_RESEND_VARIANT     (2u)
#define V_RESEND_REMOTE      (4u)
#define V_RESEND_DURABLE     (8u)
#define V_RESEND_ALL        (15u)

#define v_resendScopeSet(scope,mask)    ((scope)|=(mask))
#define v_resendScopeClear(scope,mask)  ((scope)&=(v_resendScope)(~mask))
#define v_resendScopeTest(scope,mask)   (((scope)&(mask))==(mask))
#define v_resendScopeTestNot(scope,mask)(((scope)&(~mask))==(scope))

OS_API os_ssize_t
v_resendScopeImage (
    os_char *str,
    os_size_t len,
    v_resendScope scope);

OS_API v_writeResult
v_groupWrite (
    v_group _this, v_message o,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    v_resendScope *resendScope);

OS_API v_writeResult
v_groupResend (
    v_group _this, v_message o,
    v_groupInstance *instancePtr,
    v_resendScope *resendScope,
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
v_groupWriteCheckSampleLost(
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    v_resendScope *resendScope);

/**
 * \brief Disposes all instances in the group and the registered DataReaders.
 *
 * This is a convenience function that builds on v_groupDisposeAllMatchingInstances
 */
OS_API v_writeResult
v_groupDisposeAll (
    v_group group,
    os_timeW t,
    c_ulong flags);


/**
 * \brief Disposes all instances in the group and the registered DataReaders
 *        for which the provided callback functions match.
 *
 * \param group
 *      The group for which the instances must be disposed.
 * \param t
 *      The source timestamp of the dispose message.
 *      The dispose operation only affects message that were written
 *      prior to the timestamp.
 * \param flags
 *      The additional flags to set for the dispose messages.
 * \param condition
 *      Callback function to filter to which groupInstances and
 *      dataReaderInstances this disposeAll-functionality matches.
 *      When NULL all instances are assumed to match.
 * \param arg
 *      Optional parameter of the callback function
 *
 * \remark The 'flags' parameter is introduced to implement the REPLACE
 * merge policy.
 */
OS_API v_writeResult
v_groupDisposeAllMatchingInstances (
    v_group group,
    os_timeW t,
    c_ulong flags,
    c_bool (*condition)(c_object instance, c_voidp arg),
    c_voidp arg);

/**
 * \brief Disposes all instances in the group and the registered DataReaders
 *        for which the provided callback functions match. The difference
 *        with v_groupDisposeAllMatchingInstances is that this one slaves the
 *        readerInstances to the groupInstances, where the former walks over
 *        groupInstances and readerInstances separately.
 *
 * \param group
 *      The group for which the instances must be disposed.
 * \param t
 *      The source timestamp of the dispose message.
 *      The dispose operation only affects message that were written
 *      prior to the timestamp.
 * \param condition
 *      Callback function to filter to which groupInstances and
 *      dataReaderInstances this disposeAll-functionality matches.
 *      When NULL all instances are assumed to match.
 * \param arg
 *      Optional parameter of the callback function
 *
 * \remark The 'flags' parameter is introduced to implement the REPLACE
 * merge policy.
 */
OS_API v_writeResult
v_groupSweepMarkedInstances(
        v_group group,
        os_timeW timestamp);

OS_API v_writeResult
v_groupDeleteHistoricalData (
    v_group _this,
    os_timeE t);

OS_API v_writeResult
v_groupResend (
    v_group _this,
    v_message o,
    v_groupInstance *instancePtr,
    v_resendScope *resendScope,
    v_networkId writingNetworkId);

OS_API c_bool
v_groupNwAttachedGet (
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
v_groupFlushAction (
    v_group _this,
    v_groupFlushCallback action,
    c_voidp arg);

OS_API c_bool
v_groupWalkEntries (
    v_group _this,
    v_groupEntryAction action,
    c_voidp arg);

OS_API v_result
v_groupGetHistoricalData (
    v_group _this,
    v_entry e,
    c_bool openTransactions);

OS_API void
v_groupStreamHistoricalData(
    v_group g,
    v_groupStream stream);

OS_API v_result
v_groupGetHistoricalDataWithCondition(
    v_group g,
    v_entry entry,
    v_historicalDataRequest request);

OS_API void
v_groupFlushActionWithCondition(
    v_group  g,
    v_historicalDataRequest request,
    v_groupFlushCallback action,
    c_voidp arg);

OS_API void
v_groupUpdatePurgeList(
    v_group group);

OS_API v_groupInstance
v_groupLookupInstance(
    v_group group,
    c_value keyValue[]);

OS_API v_groupInstance
v_groupLookupInstanceAndRegistration(
    v_group group,
    c_value keyValue[],
    v_gid gidTemplate,
    v_matchIdentityAction predicate,
    v_registration *registration);

OS_API v_message
v_groupCreateUntypedInvalidMessage(
    v_kernel kernel,
    v_message typedMsg);

OS_API void
v_groupRemoveAwareness (
    v_group _this,
    const c_char* serviceName);

OS_API void
v_groupCheckForSampleLost(
    v_group group,
    v_message msg);

/**
 * \brief Set the specified flags in the instanceStates
 * of all dataReader instances associated with the specified group.
 *
 * \param group The group for which all instanceStates of all DataReaders must set the flags.
 * \param flags The flags to set for all instanceStates of the DataReaders.
 *
 * \remark The function is thread-safe. During the execution of the function access
 * to the group is locked.
 */
OS_API void
v_groupMarkGroupInstanceStates (
    v_group group,
    c_ulong flags);


OS_API void
v_groupUnmarkGroupInstanceStates (
    v_group group,
    c_ulong flags);

/**
 * \brief Reset the specified flags in the instanceStates
 * of all dataReader instances associated with the specified group.
 *
 * \param group The group for which all instanceStates of all DataReaders must reset the flags.
 * \param flags The flags to reset for all instanceStates of the DataReaders.
 *
 * \remark The function is thread-safe. During the execution of the function access
 * to the group is locked.
 */
OS_API void
v_groupUnmarkReaderInstanceStates (
    v_group group,
    c_ulong flags);

/**
 * \brief Sets the onRequest flag of the group
 *
 * When this flag is set then when a group transaction becomes
 * complete and is flushed the transaction messages are not
 * inserted in the group.
 * Note that this flag is used in case of On_Request alignment
 * where durability inserts the samples directly in the readers
 * and not in the group but transaction message have to be
 * handled by the group. In that case when the transaction
 * becomes complete this flag ensures that the transaction
 * is not flushed to the group and only to the datareaders.
 *
 * \param group The group
 * \param value The value to set the onRequest flag
 *
 */
OS_API void
v_groupSetOnRequest(
    v_group _this,
    c_bool value);

OS_API void
v_groupDisconnectWriter(
    v_group g,
    struct v_publicationInfo *oInfo /* key only */,
    os_timeW timestamp,
    c_bool isLocal,
    c_bool isImplicit);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
