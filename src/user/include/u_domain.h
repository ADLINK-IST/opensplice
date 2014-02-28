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

#ifndef U_DOMAIN_H
#define U_DOMAIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_participant.h"
#include "os_if.h"
#include "os.h"
#include "cf_node.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_domain(p) ((u_domain)(p))

#define DOMAIN_NAME "The default Domain"
#define DEF_DOMAIN_ID 0
#define MAX_DOMAIN_ID 0x7fffffff

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
    const c_char *uri);

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
    const c_char *uri,
    c_long timeout); /* timeout in seconds */


/** \brief get uri from domain id.
 *
 * This method retrieves the uri that belongs to a given domain id.
 *
 * \param id      the domain id
 * \param timeout  the duration this operation will
 *                 wait for the availability of the kernel.
 *
 * \return the uri that belongs to the given id.
 */
OS_API const c_char *
u_domainOpenWithId (
    const os_int32 uri,
    c_long timeout); /* timeout in seconds */

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
    u_domain _this);

/** \brief The class destructor.
 *
 * The destructor disconnects from the kernel and thereby
 * deletes all contained entities from the kernel and then
 * deletes the kernel and itself.
 * This operation is typically used by the splice daemon.
 *
 * \param _this The Domain to operate on.
 *
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
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
OS_API c_bool
u_domainContainsParticipant(
    u_domain _this,
    u_participant participant);

/**
 * \brief Returns the number of contained Participants.
 *
 * This method will return the number of Participants contained by this Domain.
 *
 * \param _this          The Domain.
 *
 * \return               The number of Participants.
 */
OS_API c_long
u_domainParticipantCount(
    u_domain _this);

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
    u_domain _this,
    const c_char *name);

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
 * c_bool u_participantAction(u_participant participant, c_voidp arg);
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
OS_API u_result
u_domainWalkParticipants(
    u_domain _this,
    u_participantAction action,
    c_voidp actionArg);

OS_API u_participant
u_domainCreateParticipant (
    u_domain _this,
    const c_char *name,
    v_qos qos,
    c_bool enable);

OS_API u_result
u_domainCreatePersistentSnapshot(
    u_domain _this,
    const c_char *partition_expression,
    const c_char *topic_expression,
    const c_char *uri);

/** \brief Compare domainId with the domain domainURI and domainName.
 */
OS_API c_bool
u_domainCompareDomainId(
    u_domain _this,
    const c_char * arg);

OS_API v_kernel
u_domainSource (
    u_domain _this);

OS_API os_sharedHandle
u_domainSharedMemoryHandle (
    u_domain domain);

OS_API c_voidp
u_domainMemoryAddress(
   u_domain _this);

OS_API c_size
u_domainMemorySize(
   u_domain _this);

/** \brief Fills buf with the domain-specific built-in partition name.
    Bufsize must be no greater than the number of bytes available in
    buf and must be at least
    U_DOMAIN_FEDERATIONSPECIFICPARTITIONNAME_MINBUFSIZE, or ILL_PARAM
    will be returned. */
OS_API u_result
u_domainFederationSpecificPartitionName (
    u_domain _this,
    c_char *buf,
    os_size_t bufsize);

/* Note: This function is part of a temporary workaround used by R&R.
 * The function copies heap-configuration (cf_node from parser) to shm-configuration (v_cfNode).
 * This is needed so R&R can use a single operation to process configuration nodes, either from the config file (URI)
 * or a string (R&R config command).
 * Ideally in the future this will be replaced by memory-agnostic XML processing (so that it doesn't make a difference
 * if the configuration is on heap or in shared memory)
 */
OS_API u_result
u_domainCopyConfiguration(
    cf_node  cfgNode,
    u_participant p,
    u_cfElement *elem);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
