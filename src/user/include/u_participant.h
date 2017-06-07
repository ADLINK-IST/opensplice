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
#ifndef U_PARTICIPANT_H
#define U_PARTICIPANT_H

/** \file u_participant.h
 *  \brief The participant class defines a specific kernel connection.
 *
 * Processes can connect to an arbitrary number of nodal domain instances (kernels).
 * An instances of the u_participant class implements a connection to a
 * kernel. A user can create a participant for a specified kernel. The
 * participant can only be created if the user is initialized (see u_user.h).
 * All participants are automatically managed by the user object so that
 * resources claimed by participants are automatically freed at process
 * termination.
 */

/** Supported methods:
 *
 *  u_participantNew();
 *
 *  u_participantDomain();
 *  u_participantAssertLiveliness();
 *  u_participantGetConfiguration();
 *  u_participantFindTopic();
 *  u_participantDeleteHistoricalData ();
 *
 */

#include "u_cfElement.h"
#include "u_types.h"
#include "u_object.h"
#include "u_topic.h"
#include "u_publisher.h"
#include "u_subscriber.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The u_participant cast method.
 *
 * In comparison with kernel classes this cast method will not perform
 * runtime type checking due to the lack of type information.
 */
#define u_participant(p) \
        ((u_participant)u_objectCheckType(u_object(p), U_PARTICIPANT))

/** \brief The class constructor.
 *
 * The constructor will create a kernel participant and proxy for the specified
 * kernel. The participant is also added to the process its user object so its
 * lifecycle will be managed by the user object.
 *
 * \param uri      the domain configuration
 * \param timeout  the maximum time this operation will try to connect to the service.
 * \param name     an optional name that can be used to identify a participant.
 * \param qos      an optional parameter to specify participant specific quality of
 *                 service settings.
 * \param enable   provides option to create the participant in a dormant state (false)
 *                 in this state the participant is constructed but has no
 *                 connection to the domain yet.
 *
 * \return the created participant proxy on a succesful operation or NULL
 *         if the operation failed.
 */
OS_API u_participant
u_participantNew(
    const os_char *uri,
    const u_domainId_t id,
          os_uint32 timeout,
    const os_char *name,
    const u_participantQos qos,
          u_bool enable);

OS_API u_result
u_participantGetQos (
    const u_participant _this,
          u_participantQos *qos);

OS_API u_result
u_participantSetQos (
    const u_participant _this,
    const u_participantQos qos);

/**
 * \brief Returns the nodal Domain configuration.
 *
 * This method will return a copy of the nodal Domain configuration.
 *
 * \param _this          The Participant.
 *
 * \return               A copy of the nodal Domain configuration.
 */
OS_API u_cfElement
u_participantGetConfiguration(
    const u_participant _this);

/**
 * \brief asks the system to find the topic specified by a given name.
 *
 * This method will search the nodal domain for the topic(s) identified
 * by the specified name. The search is aborted and no topics are returned
 * when the specified timeout period has elapsed.
 * All instances returned are newly created user layer objects, even if
 * it hase been found before or is locally created.
 *
 * \todo solve SI912
 *
 * \param _this         The participant proxy to operate on.
 * \param name          The topic name expression that may contain wildcard characters (*,?).
 * \param timeout       The timeout period for the find operation.
 *
 * \return              The list of found Topics.
 */
OS_API c_iter
u_participantFindTopic(
    const u_participant _this,
    const os_char *name,
    const os_duration timeout);

/**
 * \brief Asserts the liveliness of the Participant
 *
 * This method notifies the domain that the participant is alive.
 *
 * \param _this          The Participant.
 *
 * \return U_RESULT_OK if the liveliness is asserted.
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_result
u_participantAssertLiveliness(
    const u_participant _this);

/**
 * \brief Delete historical data from the nodal domain.
 *
 * This method deletes all local non volatile data that matches the partition
 * and topic expression.
 *
 * \param _this          The Participant.
 * \param partitionExpr  The partition expression.
 * \param topicExpr      The topic expression
 *
 * \return U_RESULT_OK on a successful operation.
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_result
u_participantDeleteHistoricalData (
    const u_participant _this,
    const os_char *partitionExpr,
    const os_char *topicExpr);

/** \brief Retrieves the Domain associated to the given Participant.
 *
 * \param _this The participant to operate on.
 *
 * \return The associated Domain.
 */
OS_API u_domain
u_participantDomain(
    const u_participant _this);

/** \brief Retrieves the Id of the Domain associated to the given Participant.
 *
 * \param _this The participant to operate on.
 *
 * \return The associated Domain Id.
 */
OS_API u_domainId_t
u_participantGetDomainId(
    const u_participant _this);

/** \brief See u_domainFederationSpecificPartitionName. */
OS_API u_result
u_participantFederationSpecificPartitionName (
    u_participant _this,
    c_char *buf,
    os_size_t bufsize);

/** \brief Register a TypeRepresentation with the given Participant.
 *
 * \param _this The participant to operate on.
 * \param tr    TypeRepresentation to register.
 *
 * \return U_RESULT_OK on a successful operation.
 *         U_RESULT_* on failure.
 */
OS_API u_result
u_participantRegisterTypeRepresentation (
    u_participant _this,
    const u_typeRepresentation tr);

/**
 * \brief Locks the given participant and performs the action function to create a copy cache in the locked situation.
 *
 * This method locks the given participant and performs the action function to create a copy cache
 * in the locked situation.
 *
 * \param _this          The Participant.
 * \param action         The action function to create the copy cache.
 * \param arg            The pointer to the arguments needed by the action function.
 * \return               Void pointer to the created copy cache
 */
void*
u_domainParticipant_create_copy_cache(
    u_participant _this,
    void* (action)(void* arg),
    void* arg
    );

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
