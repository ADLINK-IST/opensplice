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

#ifndef U_DOMAIN_H
#define U_DOMAIN_H

#include "u_types.h"
#include "u_participant.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "vortex_os.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_domain(p) ((u_domain)(p))

#define DOMAIN_NAME "The default Domain"
#define U_DOMAIN_ID_DEFAULT 0
#define U_DOMAIN_ID_ANY 0x7fffffff
#define U_DOMAIN_ID_INVALID -1
#define U_DOMAIN_DEFAULT_TIMEOUT 1

#define U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE 36

/** \brief The class constructor (1).
 *
 * The constructor will create a new Domain object.
 * The Domain object provides access to the kernel.
 * This method will lookup or create the kernel.
 * This operation is typically used by the splice daemon.
 *
 * \param domain   the domain being created
 * \param uri      the domain configuration
 *
 * \return the created Domain object or
 *         NULL if the operation failed.
 */
OS_API u_result
u_domainNew (
          u_domain *domain,
    const os_char  *uri);

/** \brief The class constructor (2).
 *
 * The constructor will create a new Domain object.
 * The Domain object provides access to the kernel.
 * This method will try to lookup the kernel for a
 * given timeout period. In case no kernel is found
 * this operation will not create a Domain object and
 * return NULL.
 * This operation is typically used by all except the splice daemon.
 *
 * \param domain   the domain being opened
 * \param uri      the domain configuration
 * \param timeout  the duration this operation will
 *                 wait for the iavailability of the kernel.
 *
 * \return the created Domain object or
 *         NULL if the operation failed.
 */
OS_API u_result
u_domainOpen (
          u_domain *domain,
    const os_char *uri,
    const u_domainId_t id,
          os_int32 timeout); /* timeout in seconds */

/** \brief The class destructor.
 *
 * The destructor disconnects from the kernel and thereby
 * deletes all contained entities from the kernel and then
 * deletes itself.
 * This operation is typically used by all except the splice daemon.
 *
 * \param _this The Domain to operate on.
 *
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_result
u_domainClose (
    _Inout_ u_domain _this);

/** \brief The class destructor.
 *
 * The destructor disconnects from the kernel and thereby
 * deletes all contained entities from the kernel and then
 * deletes the kernel and itself.
 * This operation is typically used by the splice daemon.
 *
 * \param _this   The Domain to operate on.
 *
 * \return U_RESULT_OK on a successful operation
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 *         U_RESULT_TIMEOUT when not all threads detached within timeout period.
 *              Operation will continue freeing domain after timeout. Other
 *              failure return codes can overwrite this one.
 */
OS_API u_result
u_domainFree (
    u_domain _this);

/**
 * \brief Check if the given Participant is associated to this Domain.
 *
 * This method checks if the given Participant is contained by this Domain.
 * A Participant is contained by the Domain if the Participant is created by
 * the Domain.
 *
 * \param _this          The Domain.
 * \param participant    The Participant.
 *
 * \return               TRUE if the Participant is associated to the Domain.
 */
OS_API u_bool
u_domainContainsParticipant(
    const u_domain _this,
    const u_participant participant);

/**
 * \brief Returns the number of contained Participants.
 *
 * This method will return the number of Participants contained by this Domain.
 *
 * \param _this          The Domain.
 *
 * \return               The number of Participants.
 */
OS_API c_ulong
u_domainParticipantCount(
    const u_domain _this);

/**
 * \brief Returns a list of all associated Participants.
 *
 * This method returns a list of all Participants created by this Domain.
 * This method doesn't increase the ref count of a Participant so returned
 * Participants are not kept alive by this operation and don't need to be freed.
 *
 * \param _this          The Domain.
 * \param name           The Participant name expression, the expression can
 *                       contain wildcard symbols '*' and '?'.
 *
 * \return               A list of all contained Participants.
 */
OS_API c_iter
u_domainLookupParticipants(
    const u_domain _this,
    const os_char *name);

/**
 * \brief Execute an action operation on all contained Participants.
 *
 * This method will visit all contained Participants of this Domain and
 * execute the specified action operation on each Participant.
 * The action operation expects two parameters, the Participant and the
 * given actionArg, the actionArg parameter is passed to action operation
 * on each invocation.
 *
 * The signature of the action operation is defined in u_participant.h by the
 * following definition:
 *
 * u_bool u_participantAction(u_participant participant, void *arg);
 *
 * Note that this method will abort the walk when all participants are visited or
 * when the action operation returns FALSE.
 *
 * \param _this          The Domain.
 * \param action         The action operation.
 * \param arctionArg     The action argument that is passed to all invocations
 *                       of the action operation.
 *
 * \return               U_RESULT_OK on a succesfull walk.
 *                       U_RESULT_ALREADY_DELETED if the specified participant is deleted.
 */

typedef u_bool (*u_participantAction)(u_participant p, void *arg);

OS_API u_result
u_domainWalkParticipants(
    const u_domain _this,
    const u_participantAction action,
          void *actionArg);

OS_API u_participant
u_domainCreateParticipant (
    const u_domain _this,
    const c_char *name,
    v_qos qos,
    c_bool enable);

OS_API u_result
u_domainCreatePersistentSnapshot(
    const u_domain _this,
    const os_char *partition_expression,
    const os_char *topic_expression,
    const os_char *uri);

OS_API u_result
u_domain_load_xml_descriptor (
    const u_domain _this,
    const os_char *xml_descriptor);

OS_API os_char *
u_domain_get_xml_descriptor (
    const u_domain _this,
    const os_char *type_name);

OS_API u_domainId_t
u_domainId(
    _In_ const u_domain _this) __nonnull_all__;

OS_API const char *
u_domainName(
    _In_ const u_domain _this) __nonnull_all__;

OS_API c_type
u_domain_lookup_type(
    const u_domain _this,
    const os_char *type_name);

/** \brief Compare domainId with the domain domainURI and domainName.
 */
OS_API u_bool
u_domainCompareId(
    const u_domain _this,
    const u_domainId_t id);

OS_API os_sharedHandle
u_domainSharedMemoryHandle (
    const u_domain domain);

OS_API void *
u_domainMemoryAddress(
    const u_domain _this);

OS_API u_size
u_domainMemorySize(
    const u_domain _this);

OS_API u_result
u_domainEnableStatistics(
    const u_domain _this,
    const os_char *categoryName);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
