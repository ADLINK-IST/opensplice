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
#ifndef V_KERNEL_H
#define V_KERNEL_H

/** \file kernel/include/v_kernel.h
*  \brief This file defines the interface
*
*/

#include "kernelModuleI.h"
#include "os_process.h"

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
#define C_REPORT(s) printf(s)

/**
* \brief The <code>c_object</code> cast method.
*
* This method casts an object to a <code>c_object</code> object.
*/
#define v_object(o)  ((v_object)(o))
/**
* \brief The <code>v_kernel</code> cast method.
*
* This method casts an object to a <code>v_kernel</code> object.
* Before the cast is performed, if compiled with the NDEBUG flag not set,
* the type of the object is checked to be <code>v_kernel</code> or
* one of its subclasses.
*/
#define v_kernel(o)  (C_CAST(o,v_kernel))
/**
* \brief The <code>v_context</code> cast method.
*
* This method casts an object to a <code>v_context</code> object.
* Before the cast is performed, if compiled with the NDEBUG flag not set,
* the type of the object is checked to be <code>v_context</code> or
* one of its subclasses.
*/
#define v_context(o) (C_CAST(o,v_context))

#define v_objectKernel(o) (v_kernel(v_object(o)->kernel))
#define v_objectKind(o)   (v_object(o)->kind)

_Ret_z_
const char *
v_objectKindImage(
    _In_ v_object _this);

#define v_kernelType(k,kind) (v_kernel(k)->type[kind])

#define V_MASK_READ_SAMPLE          (0x0001U << 0)
#define V_MASK_NOT_READ_SAMPLE      (0x0001U << 1)
#define V_MASK_NEW_VIEW             (0x0001U << 2)
#define V_MASK_NOT_NEW_VIEW         (0x0001U << 3)
#define V_MASK_ALIVE_INSTANCE       (0x0001U << 4)
#define V_MASK_DISPOSED_INSTANCE    (0x0001U << 5)
#define V_MASK_NOWRITERS_INSTANCE   (0x0001U << 6)

#define V_MASK_ANY_SAMPLE           (V_MASK_READ_SAMPLE | \
                                     V_MASK_NOT_READ_SAMPLE)
#define V_MASK_ANY_VIEW             (V_MASK_NEW_VIEW | \
                                     V_MASK_NOT_NEW_VIEW)
#define V_MASK_ANY_INSTANCE         (V_MASK_DISPOSED_INSTANCE | \
                                     V_MASK_NOWRITERS_INSTANCE | \
                                     V_MASK_ALIVE_INSTANCE)

#define V_MASK_ANY                  (V_MASK_ANY_SAMPLE | \
                                     V_MASK_ANY_VIEW | \
                                     V_MASK_ANY_INSTANCE)

#define v_sampleMaskSet(state,mask)   ((state)|=(mask))
#define v_sampleMaskClear(state,mask) ((state)&=(~mask))
#define v_sampleMaskTest(state,mask)  (((state)&(mask))==(mask))

typedef os_uint32 v_sampleMask;

typedef enum {
    V_RESULT_UNDEFINED = OS_RETCODE_ID_V_RESULT,
    V_RESULT_OK,
    V_RESULT_INTERRUPTED,
    V_RESULT_NOT_ENABLED,
    V_RESULT_OUT_OF_MEMORY,
    V_RESULT_INTERNAL_ERROR,
    V_RESULT_ILL_PARAM,
    V_RESULT_CLASS_MISMATCH,
    V_RESULT_DETACHING,
    V_RESULT_TIMEOUT,
    V_RESULT_OUT_OF_RESOURCES,
    V_RESULT_INCONSISTENT_QOS,
    V_RESULT_IMMUTABLE_POLICY,
    V_RESULT_PRECONDITION_NOT_MET,
    V_RESULT_ALREADY_DELETED,
    V_RESULT_HANDLE_EXPIRED,
    V_RESULT_NO_DATA,
    V_RESULT_UNSUPPORTED
} v_result;

OS_API c_bool
v_isEnabledStatistics (
    v_kernel _this,
    const char *categoryName);

_Check_return_
_Ret_maybenull_
_Success_(return != NULL)
OS_API v_kernel
v_kernelNew (
    _In_ c_base base,
    _In_z_ const c_char *name,
    _In_ v_kernelQos qos,
    _Outptr_ v_processInfo* procInfo);

OS_API void
v_kernelEnable(
    _Inout_ v_kernel kernel);

OS_API v_kernel
v_kernelAttach (
    c_base b,
    const c_char *name,
    os_duration timeout,
    v_processInfo *procInfo /* Don't free this reference */);

OS_API c_ulong
v_kernelMyProtectCount(
    v_kernel k);

OS_API v_result
v_kernelDetach (
    v_kernel _this,
    os_procId procId);

OS_API c_ulong
v_kernelUserCount (
    v_kernel _this);

OS_API c_ulong
v_kernelNetworkCount (
    v_kernel _this);

OS_API void
v_kernelSetNetworkCount (
    v_kernel _this,
    c_ulong count);

OS_API v_result
v_kernelWaitForDurabilityAvailability(
    v_kernel _this,
    os_duration timeout);

OS_API v_partition
v_addPartition (
    v_kernel _this,
    v_partition p);

OS_API v_partition
v_removePartition (
    v_kernel _this,
    v_partition p);

OS_API c_iter
v_resolvePartitions (
    v_kernel _this,
    const c_char *name);

OS_API v_topic
v__addTopic (
    v_kernel _this,
    v_topic t);

OS_API v_topic
v_removeTopic (
    v_kernel _this,
    v_topic t);

OS_API v_topic
v_lookupTopic (
    v_kernel _this,
    const char *name);

OS_API c_iter
v_resolveTopics (
    v_kernel _this,
    const c_char *expr);

OS_API v_participant
v_addParticipant (
    v_kernel _this,
    v_participant p);

OS_API v_participant
v_removeParticipant (
    v_kernel _this,
    v_participant p);

OS_API c_iter
v_resolveParticipants (
    v_kernel _this,
    const c_char *name);

_Check_return_
_Ret_notnull_
_Pre_satisfies_(kind >= K_KERNEL && kind < K_TYPECOUNT)
OS_API v_object
v_objectNew (
    _In_ v_kernel _this,
    _In_ v_kind kind);

_Check_return_
_Ret_maybenull_
OS_API v_object
v_objectNew_s (
    _In_ v_kernel _this,
    _In_ v_kind kind);

OS_API v_object
v_new (
    v_kernel _this,
    c_type type);

OS_API v_serviceManager
v_getServiceManager (
    v_kernel _this);

OS_API v_configuration
v_getConfiguration (
    v_kernel _this);

OS_API v_result
v_kernelConfigure (
    v_kernel _this,
    v_configuration config);

OS_API void
v_enableStatistics (
    v_kernel _this,
    const char *categoryName);

OS_API void
v_disableStatistics (
    v_kernel _this,
    const char *categoryName);

OS_API c_bool
v_kernelCheckHandleServer (
    v_kernel _this,
    c_address serverId);

OS_API void
v_writeBuiltinTopic (
    v_kernel _this,
    enum v_infoId id,
    v_message msg);
void
v_writeDisposeBuiltinTopic (
    v_kernel _this,
    enum v_infoId id,
    v_message msg);
void
v_unregisterBuiltinTopic (
    v_kernel _this,
    enum v_infoId id,
    v_message msg);

OS_API v_result
v_kernelCreatePersistentSnapshot(
    v_kernel _this,
    const c_char * partition_expression,
    const c_char * topic_expression,
    const c_char * uri);

typedef v_result (*v_domainReadAction)(const v_group group, const v_groupInstance instance, const v_message msg, const void *actionArg);

OS_API v_result
v_kernelRead(
    const v_kernel _this,
    const os_char *partition,
    const os_char *topic,
    const os_char *query,
    const v_domainReadAction action,
    const void *actionArg);

typedef v_result (*v_domainGroupReadAction)(const v_group group, const void *actionArg);

OS_API v_result
v_kernelGroupRead(
    const v_kernel _this,
    const os_char *partition,
    const os_char *topic,
    const v_domainGroupReadAction action,
    const void *actionArg);

OS_API v_accessMode
v_kernelPartitionAccessMode(
    v_kernel _this,
    v_partitionPolicyI partition);

OS_API v_result
v_kernel_load_xml_descriptor (
    v_kernel _this,
    const os_char *xml_descriptor);

OS_API os_char *
v_kernel_get_xml_descriptor (
    v_kernel _this,
    const os_char *type_name);

OS_API c_type
v_kernel_lookup_type(
    v_kernel _this,
    const os_char *type_name);

_Ret_z_
OS_API const os_char *
v_resultImage (
    _In_ v_result result);

OS_API const os_char *
v_writeResultImage(
    const v_writeResult result);

OS_API os_int
v_resultToReturnCode(
    v_result result);

OS_API os_int
v_writeResultToReturnCode(
    v_writeResult result);

OS_API const os_char *
v_dataReaderResultImage(
    const v_dataReaderResult result);

OS_API os_int
v_dataReaderResultToReturnCode(
    v_dataReaderResult result);

OS_API c_bool
v_sampleMaskPass(
    v_sampleMask mask,
    const c_object sample);

OS_API c_iter
v_kernelGetAttachedProcesses(
    v_kernel _this);

OS_API c_iter
v_resolveServiceByServiceType(
   v_kernel kernel,
   v_serviceType type);

/**
 * Returns the process-info record for the process identified by pid. The
 * returned memory should be considered read-only for all processes except the
 * owner. In case the owner isn't running anymore, the memory may be interpreted.
 *
 * @param _this the kernel to retrieve the processInfo from
 * @param pid the processId to retrieve the processInfo for
 * @return the processInfo for pid if it exists. The reference must be freed.
 */
_Ret_maybenull_
_Must_inspect_result_
OS_API v_processInfo
v_kernelGetProcessInfo(
    _Inout_ v_kernel _this,
    _In_ os_procId pid);

/**
 * Returns the process-info record for the current process. This record always
 * exists if the thread gained access to the kernel in a normal way.
 *
 * @param _this the kernel to retrieve the processInfo from
 * @return the processInfo for the current process. The returned reference must
 * be freed.
 */
_Ret_notnull_
_Must_inspect_result_
OS_API v_processInfo
v_kernelGetOwnProcessInfoWeakRef(
    _Inout_ v_kernel _this);

#define V_KERNEL_THREAD_FLAGS_GET           (0U)
#define V_KERNEL_THREAD_FLAG_SERVICETHREAD  (1U << 31)
#define V_KERNEL_THREAD_FLAG_WAITING        (1U << 30)
#define V_KERNEL_THREAD_FLAG_DOMAINID       (0xFF)

OS_API os_uint32
v_kernelThreadFlags (
    _In_ os_uint32 mask,
    _In_ os_uint32 value);

OS_API c_bool
v_kernelThreadInProtectedArea();

OS_API os_uint32
v_kernelThreadProtectCount (
    _In_ os_uint32 serial);


/**
 * This call is to be used in protected context to indicate that
 * although of being in a protected context the data is not modified
 * and that it is safe to remove the corresponding resources in case
 * of an application crash where otherwise a crash while being in
 * protected context would cause the service to shutdown.
 *
 * The call increases the wait-count of the process info to indicate
 * that the caller is not modifying SHM resources and that when the
 * caller crashes during this operation the SHM monitor is able to
 * safely remove the shared resources of the crashed application.
 * Every invocation of the call must be matched by a call to
 * v_kernelProtectStrictReadOnlyExit. Further the call may
 * only be invoked when the SHM is protected by the use of
 * v_kernelProtect.
 * Note that this call may only be used when the caller is reading
 * and not modifying SHM resources.
 *
 * !!! Use this function very carefully.
 */
OS_API void
v_kernelProtectStrictReadOnlyEnter(void);

/**
 * This call must be invoked to indicate that the caller when still
 * in protected context has finished with a read-only operation.
 *
 * !!! Use this function very carefully.
 */
OS_API void
v_kernelProtectStrictReadOnlyExit(void);

/**
 * This call must always be invoked before accessing the kernel from the user-
 * layer and signals that SHM may be modified by the thread that invokes this call.
 * Every successful invocation of this call must be matched by a call to
 * v_kernelUnprotect().
 *
 * @param info The process information in which the protect-count of this process
 *             is administered.
 * @param block Pointer to flag indicating whether threads are allowed to wake from
 *              a waiting state (e.g., condition variable).
 * @param blockmask Bitmask that is used to mask irrelevant bits in block
 * @param deadlock Pointer to a mutex that must be locked in case threads aren't
 *                 allowed to wake from a waiting state.
 * @param usrData Optional pointer to userdata return by the matching v_kernelUnprotect.
 * @return V_RESULT_OK on success. The first invocation of a thread to this call
 * may return V_RESULT_OUT_OF_RESOURCES if no thread specific memory could be
 * allocated to store the process information. */
_Check_return_
OS_API v_result
v_kernelProtect(
    _Inout_ v_processInfo info,
    _In_ pa_uint32_t *block,
    _In_ os_uint32 blockmask,
    _Inout_ os_mutex *deadlock,
    _In_opt_ void *usrData);

/**
 * This call must be invoked to signal that the calling thread is not accessing
 * the kernel anymore (accessing SHM).
 *
 * @pre v_kernelProtect() was invoked and returned V_RESULT_OK
 */
_Ret_maybenull_
OS_API void *
v_kernelUnprotect(void);

OS_API void
v_kernelUnprotectFinalize(
    void * usrData);

OS_API os_int32
v_kernelThreadInfoGetDomainId(void);

/**
 * This call returns a boolean, TRUE if a durability-service or client-durability is configured.
 * Note that this is not sufficient to determine if a certain partition/topic is managed by durability, since
 * a service can be enabled, but configured to ignore certain data.
 */
OS_API c_bool
v_kernelGetDurabilitySupport(
    _In_ _Const_ v_kernel kernel);

OS_API c_bool
v_kernelHasDurabilityService(
    _In_ _Const_ v_kernel kernel);

/**
 * This call acquires the kernel group transaction access lock.
 * Taking the kernel group transaction access lock ensures that the group transaction
 * information can be read without being modified by a writer.
 */
OS_API void
v_kernelGroupTransactionBeginAccess(
    _Inout_ v_kernel _this);

/**
 * This call releases the kernel group transaction access lock.
 */
OS_API void
v_kernelGroupTransactionEndAccess(
    _Inout_ v_kernel _this);


OS_API c_bool
v_kernelGetAlignedState(
    v_kernel _this);

OS_API void
v_kernelSetAlignedState(
    v_kernel _this,
    c_bool aligned);

OS_API void
v_kernelTransactionsPurge(
    v_kernel _this);

/* The operation WalkPublications will visit all discovered publication messages and
 * invoke the given publication action function on each publication message.
 * The given argument arg is passed as context information to the invoked action function.
 * The walk will stop when all messages are processed or when the action function returns FALSE.
 */
typedef os_boolean (*v_publication_action)(const v_message publication, void *arg);

OS_API v_result
v_kernelWalkPublications(
    v_kernel _this,
    v_publication_action action,
    void *arg);

/* The operation WalkSubscriptions will visit all discovered subscription messages and
 * invoke the given subscription action function on each subscription message.
 * The given argument arg is passed as context information to the invoked action function.
 * The walk will stop when all messages are processed or when the action function returns FALSE.
 */
typedef os_boolean (*v_subscription_action)(const v_message subscription, void *arg);

OS_API v_result
v_kernelWalkSubscriptions(
    v_kernel _this,
    v_subscription_action action,
    void *arg);

#define V_ISOLATE_NONE (0)
#define V_ISOLATE_DEAF (1)
#define V_ISOLATE_MUTE (2)
#define V_ISOLATE_ALL (V_ISOLATE_DEAF | V_ISOLATE_MUTE)

OS_API v_result
v_kernelSetIsolate(
    const v_kernel _this,
    const os_uint32 isolate);

OS_API v_result
v_kernelGetIsolate(
    const v_kernel _this,
    os_uint32 *isolate);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
