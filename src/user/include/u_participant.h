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
 *  u_participantFree();
 *
 *  u_participantDomain();
 *  u_participantAssertLiveliness();
 *  u_participantDisable();
 *  u_participantGetConfiguration();
 *  u_participantFindTopic();
 *  u_participantGetBuiltinSubscriber();
 *  u_participantDeleteHistoricalData ();
 *
 *  u_participantCreatePublisher();
 *  u_participantContainsPublisher();
 *  u_participantLookupPublishers();
 *  u_participantPublisherCount();
 *  u_participantWalkPublishers();
 *
 *  u_participantCreateSubscriber();
 *  u_participantContainsSubscriber();
 *  u_participantLookupSubscribers();
 *  u_participantSubscriberCount();
 *  u_participantWalkSubscribers();
 *
 *  u_participantCreateTopic();
 *  u_participantContainsTopic();
 *  u_participantLookupTopics();
 *  u_participantTopicCount();
 *  u_participantWalkTopics();
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_topic.h"
#include "u_publisher.h"
#include "u_subscriber.h"
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
#define u_participant(p) \
        ((u_participant)u_entityCheckType(u_entity(p), U_PARTICIPANT))

typedef c_bool
(*u_participantAction)(
    v_participant p,
    c_voidp arg);

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
 *
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
 *
 * \return U_RESULT_OK the participant is disabled.<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_result
u_participantDisable(
    u_participant _this);

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
    u_participant _this);

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
    u_participant _this,
    const c_char *name,
    v_duration timeout);

/**
 * \brief returns the built-in subscriber.
 *
 * This method returns the built-in subscriber, which holds built-in datareader's for every
 * built-in topic. These datareader's can be obtained using the method
 * <code>u_subscriberLookupDatareader</code>.
 *
 * \param _this         The participant proxy to operate on.
 *
 * \return              The built-in subscriber.
 */
OS_API u_subscriber
u_participantGetBuiltinSubscriber(
    u_participant _this);

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
    u_participant _this);

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
    u_participant _this,
    const c_char* partitionExpr,
    const c_char* topicExpr);

/** \brief Retrieves the Domain associated to the given Participant.
 *
 * \param _this The participant to operate on.
 *
 * \return U_RESULT_OK on a succesful operation or
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 *         U_RESULT_ALREADY_DELETED if the specified participant is deleted.
 */
OS_API u_domain
u_participantDomain(
    u_participant _this);

/**
 * \brief Check if the given Publisher is associated to this Participant.
 *
 * This method checks if the given Publisher is contained by this Participant.
 * A Publisher is contained by the Participant if the Publisher is created by
 * the Participant.
 *
 * \param _this          The Participant.
 * \param publisher      The Publisher.
 *
 * \return               TRUE if the publisher is associated to0 the Participant.
 */
OS_API c_bool
u_participantContainsPublisher(
    u_participant _this,
    u_publisher publisher);

/**
 * \brief Returns a list of all associated Publishers.
 *
 * This method returns a list of all Publishers created by this Participant.
 * This method doesn't increase the ref count of a Publisher so returned
 * Publishers are not kept alive by this operation and don't need to be freed.
 *
 * \param _this          The Participant.
 *
 * \return               A list of all contained Publishers.
 */
OS_API c_iter
u_participantLookupPublishers(
    u_participant _this);

/**
 * \brief Returns the number of contained Publishers.
 *
 * This method will return the number of Publishers contained by this Participant.
 *
 * \param _this          The Participant.
 *
 * \return               The number of Publishers.
 */
OS_API c_long
u_participantPublisherCount(
    u_participant _this);

/**
 * \brief Execute an action operation on all contained Publishers.
 *
 * This method will visit all contained Publishers of this Participant and
 * execute the specified action operation on each Publisher.
 * The action operation expects two parameters, the Publisher and the
 * given actionArg, the actionArg parameter is passed to action operation
 * on each invocation.
 *
 * The signature of the action operation is defined in u_publisher.h by the
 * following definition:
 *
 * c_bool u_publisherAction(u_publisher publisher, c_voidp arg);
 *
 * Note that this method will abort the walk when all publishers are visited or
 * when the action operation returns FALSE.
 *
 * \param _this          The Participant.
 * \param action         The action operation.
 * \param arctionArg     The action argument that is passed to all invocations
 *                       of the action operation.
 *
 * \return               U_RESULT_OK on a succesfull walk.
 *                       U_RESULT_ALREADY_DELETED if the specified participant is deleted.
 */
OS_API u_result
u_participantWalkPublishers(
    u_participant _this,
    u_publisherAction action,
    c_voidp actionArg);

/**
 * \brief Check if the given Subscriber is associated to this Participant.
 *
 * This method checks if the given Subscriber is contained by this Participant.
 * A Subscriber is contained by the Participant if the Subscriber is created by
 * the Participant.
 *
 * \param _this          The Participant.
 * \param subscriber     The Subscriber.
 *
 * \return               TRUE if the subscriber is associated to the Participant.
 */
OS_API c_bool
u_participantContainsSubscriber(
    u_participant _this,
    u_subscriber subscriber);

/**
 * \brief Returns a list of all associated Subscribers.
 *
 * This method returns a list of all Subscribers created by this Participant.
 * This method doesn't increase the ref count of a Subscriber so returned
 * Subscribers are not kept alive by this operation and don't need to be freed.
 *
 * \param _this          The Participant.
 *
 * \return               A list of all contained Subscribers.
 */
OS_API c_iter
u_participantLookupSubscribers(
    u_participant _this);

/**
 * \brief Returns the number of contained Subscribers.
 *
 * This method will return the number of Subscribers contained by this Participant.
 *
 * \param _this          The Participant.
 *
 * \return               The number of Subscribers.
 */
OS_API c_long
u_participantSubscriberCount(
    u_participant _this);

/**
 * \brief Execute an action operation on all contained Subscribers.
 *
 * This method will visit all contained Subscribers of this Participant and
 * execute the specified action operation on each Subscriber.
 * The action operation expects two parameters, the Subscriber and the
 * given actionArg, the actionArg parameter is passed to action operation
 * on each invocation.
 *
 * The signature of the action operation is defined in u_subscriber.h by the
 * following definition:
 *
 * c_bool u_subscriberAction(u_subscriber subscriber, c_voidp arg);
 *
 * Note that this method will abort the walk when all Subscribers are visited or
 * when the action operation returns FALSE.
 *
 * \param _this          The Participant.
 * \param action         The action operation.
 * \param arctionArg     The action argument that is passed to all invocations
 *                       of the action operation.
 *
 * \return               U_RESULT_OK on a succesfull walk.
 */
OS_API u_result
u_participantWalkSubscribers(
    u_participant _this,
    u_subscriberAction action,
    c_voidp actionArg);

/**
 * \brief Check if the given Topic is associated to this Participant.
 *
 * This method checks if the given Topic is contained by this Participant.
 * A Topic is contained by the Participant if the Topic is created by
 * the Participant.
 *
 * \param _this          The Participant.
 * \param topic          The Topic.
 *
 * \return               TRUE if the Topic is associated to the Participant.
 */
OS_API c_bool
u_participantContainsTopic(
    u_participant _this,
    u_topic topic);

/**
 * \brief returns all topics matching the topic name expression.
 *
 * This method provides the list of all local created topics which are
 * associated to this participant by creation and match the value of
 * topic_name. Wildcard characters are not (yet) supported, however
 * not specifying a name (NULL) implies '*' meaning all topics.
 * Multiple Topics for one name is supported.
 *
 * \param _this          The Participant.
 * \param topic_name     The name of the topic.
 *
 * \return               the list of matching topics
 */
OS_API c_iter
u_participantLookupTopics(
    u_participant _this,
    const c_char *topic_name);

/**
 * \brief returns the number of topics associated to this participant.
 *
 * This method provides the number of all local created topics which are
 * associated to this participant by creation.
 *
 * \param _this          The Participant.
 *
 * \return               the number of topics.
 */
OS_API c_long
u_participantTopicCount(
    u_participant _this);

/**
 * \brief Execute an action operation on all contained Topic.
 *
 * This method will visit all contained Topics of this Participant and
 * execute the specified action operation on each Topic.
 * The action operation expects two parameters, the Topic and the
 * given actionArg, the actionArg parameter is passed to action operation
 * on each invocation.
 *
 * The signature of the action operation is defined in u_topic.h by the
 * following definition:
 *
 * c_bool u_topicAction(u_topic topic, c_voidp arg);
 *
 * Note that this method will abort the walk when all Topic are visited or
 * when the action operation returns FALSE.
 *
 * \param _this          The Participant.
 * \param action         The action operation.
 * \param arctionArg     The action argument that is passed to all invocations
 *                       of the action operation.
 *
 * \return               U_RESULT_OK on a succesfull walk.
 */
OS_API u_result
u_participantWalkTopics(
    u_participant _this,
    u_topicAction action,
    c_voidp actionArg);

/**
 * \brief Delete all contained Entities.
 *
 * This method will delete all Entities contained by this Participant.
 * Entities contained by the Participant are all Topics, Publishers and
 * Subscribers created by this Participant.
 *
 * \param _this          The Participant.
 *
 * \return               U_RESULT_OK on successful operation.
 */
OS_API u_result
u_participantDeleteContainedEntities(
    u_participant _this);

/**
 * \brief Verifies if the given Subscriber is the builtin Subscriber.
 *
 * This method will verify if the given Subscriber is the buitin Subscriber
 * associated to this Participant.
 *
 * \param _this          The Participant.
 * \param subscriber     The Subscriber.
 *
 * \return               TRUE if the Subscriber is the builtin Subscriber.
 */
OS_API c_bool
u_participantIsBuiltinSubscriber(
    u_participant _this,
    u_subscriber subscriber);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
