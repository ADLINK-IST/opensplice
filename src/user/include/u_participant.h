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
#ifndef U_PARTICIPANT_H
#define U_PARTICIPANT_H

/** \file u_participant.h
 *  \brief The participant class defines a specific kernel connection.
 *
 * Processes can connect to an arbitrary number of splice systems (kernels).
 * The participant class specifies objects that implement a connection to a
 * kernel. A user can create a participant for a specified kernel. The
 * participant can only be created if the user is initialized (see u_user.h).
 * All participants are automatically managed by the user object so that
 * resources claimed by participants are automatically freed at process
 * termination.
 */

/** Supported methods
 * u_participant u_participantNew    (u_kernel k, const c_char *name, v_qos qos);
 * u_result      u_participantFree   (u_participant p);
 * u_result      u_participantInit   (u_participant p);
 * u_result      u_participantDeinit (u_participant p);
 * u_kernel      u_participantKernel (u_participant p);
 * u_result      u_participantAssertLiveliness (u_participant p);
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_cfElement.h"
#include "v_statistics.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
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
#define u_participant(p) ((u_participant)(p))

/** \brief The class constructor.
 *
 * The constructor will create a kernel participant and proxy for the specified
 * kernel. The participant is also added to the process its user object so its
 * lifecycle will be managed by the user object.
 *
 * \param k the kernel that must be specified.
 * \param name an optional name that can be used to identify a participant.
 * \param qos an optional parameter to specify participant specific quality of
 *        service settings.
 * \return the created participant proxy on a succesful operation or NULL
 *         if the operation failed.
 */
OS_API u_participant
u_participantNew(
    const c_char *uri,
    c_long        timeout,
    const c_char *name,
    v_qos         qos,
    c_bool        enable);

/** \brief The class destructor.
 *
 * The destructor notifies the kernel to destruct the kernel participant
 * associated to this proxy. Also all kernel object owned by the deleted kernel
 * participant are deleted.
 *
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_result
u_participantFree(
    u_participant _this);

/** \brief Disables the specified participant.
 *
 * When a participant is disabled, all its entities are also disabled.
 *
 * \param _this The participant to operate on
 * \return U_RESULT_OK the participant is disabled.<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_result
u_participantDisable(
    u_participant _this);


OS_API u_cfElement u_participantGetConfiguration(u_participant participant);

/**
 * \brief Renews the lease of the participant.
 *
 * This method renews the lease of the participant by adding the lease period to the current time.
 *
 * \param p The participant proxy to operate on.
 */
OS_API u_result u_participantRenewLease(u_participant p, v_duration leasePeriod);

/**
 * \brief asks the system to find the topic specified by a given name.
 *
 * This method will ask the system to look for the topic specified by the given name.
 * If the topic is unknown on the local node a request is sent to the network, the topic
 * information will be sent to the requesting node if another node has this information.
 * This method will return a list of all topics found within the specified timeout period. 
 *
 * \param p The participant proxy to operate on.
 * \param name The topic name expression that may contain wildcard characters (*,?).
 * \param timeout The timeout period for the find operation.
 */
OS_API c_iter u_participantFindTopic(u_participant p, const c_char *name, v_duration timeout);

/**
 * \brief returns the built-in subscriber.
 *
 * This method returns the built-in subscriber, which holds built-in datareader's for every
 * built-in topic. These datareader's can be obtained using the method 
 * <code>u_subscriberLookupDatareader</code>.
 *
 * \param p The participant proxy to operate on.
 * \return the built-in subscriber.
 */
OS_API u_subscriber u_participantGetBuiltinSubscriber(u_participant p);

OS_API u_result u_participantAssertLiveliness(u_participant p);

OS_API u_result     u_participantDeleteHistoricalData (u_participant p,
                                                       const c_char* partitionExpr,
                                                       const c_char* topicExpr);

/** \brief Retrieves the kernel associated to the given participants.
 *
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_kernel u_participantKernel(u_participant _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
