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
/**@file api/cm/xml/include/cmx_participant.h
 * @brief Represents a participant in Splice. It offers facilities to 
 * create/free participants and resolve participants/topics/domains in the 
 * system.
 */
#ifndef CMX_PARTICIPANT_H
#define CMX_PARTICIPANT_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * @brief Creates a new participant. It is creates by creating its user layer 
 * counterpart and serializing it into XML format.
 * 
 * This function constructs a new user layer participant and creates an XML
 * representation of it. The XML participant format is as described in 
 * cmx_entity.h. In contradiction with the usual XML entity, the user entity
 * that is associated with the XML entity is owner of the kernel entity. That
 * means that the kernel entity will be removed when the participant is freed.
 * 
 * @param uri The URI of the kernel, where the participant must be created in.
 * @param timeout The maximum amount of time the function must keep trying to
 * create a participant when creating wasn't a success.
 * @param name The name that the participant must have.
 * @param qos The qos for the participant. If NULL is supplied, the default is
 *            taken.
 * @return The participant, which is the XML representation of the user
 *         participant.
 */
OS_API c_char*         cmx_participantNew          (const c_char* uri, 
                                                    c_long timeout,
                                                    const c_char* name,
                                                    const c_char* qos);

/**
 * @brief Resolves all participants in the same kernel the supplied participant
 * is participating in.
 * 
 * The result of this function is a list of participants in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 *
 * @param The participant where to resolve the kernel from.
 * @return A list of all participants that are participating in the same kernel
 * as the supplied participant, including the supplied participant itself.
 */
OS_API c_char*         cmx_participantAllParticipants  (const c_char* participant);

/**
 * @brief Resolves all topics in the same kernel the supplied participant
 * is participating in.
 * 
 * The result of this function is a list of topics in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 *
 * @param The participant where to resolve the kernel from.
 * @return A list of all topics that are available in the same kernel
 * as the supplied participant.
 */
OS_API c_char*         cmx_participantAllTopics    (const c_char* participant);

/**
 * @brief Resolves all domains in the same kernel the supplied participant
 * is participating in.
 * 
 * The result of this function is a list of domains in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 *
 * @param The participant where to resolve the kernel from.
 * @return A list of all domains that are available in the same kernel
 * as the supplied participant.
 */
OS_API c_char*         cmx_participantAllDomains   (const c_char* participant);

/**
 * @brief Registers the supplied type in the database.
 * 
 * Subsequent registration of the same type has no effect.
 * 
 * @param participant The participant to use for registering the type.
 * @param type The XML representation of the type to register in the database.
 * @return The result of the registration. If 
 *         succeeded @verbatim<result>OK</result>@endverbatim is returned, 
 *         @verbatim<result>...</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_participantRegisterType (const c_char* participant,
                                                    const c_char* type);

/**
 * @brief Resolves a list of topics that match the supplied expression.
 * 
 * The topicName may contain wilcards '*' and '?'. The result of this function 
 * is a list of domains in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 *
 * @param participant The participant to use when resolving the topics.
 * @param topicName The name of the topics, that may contain wildcards.
 * @return A list of topics that match the supplied expression.
 */
OS_API c_char*         cmx_participantFindTopic    (const c_char* participant, 
                                             const c_char* topicName);

/**
 * @brief Allows the automatic deletion of the participant when the SPLICE-DDS
 * service terminates.
 * 
 * @param participant The participant that needs to registered for automatic 
 *                    deletion.
 * @param enable If TRUE, the autodetach is enabled. If FALSE, the autodetach
 *               is disabled.
 * @param The result of the autodetach action. If 
 *         succeeded @verbatim<result>OK</result>@endverbatim is returned, 
 *         @verbatim<result>...</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_participantAutoDetach   (const c_char* participant,
                                                    c_bool enable);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_PARTICIPANT_H */
