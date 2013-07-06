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

/**@file api/dcps/jni/include/jni_participant.h
 * 
 * @brief jni_participant is the representation of a DCPS DomainParticipant.
 * 
 * It represents a participation in a DCPS Domain. It is the placeholder
 * for publishers, subscribers and topics.
 */
 
#ifndef JNI_PARTICIPANT_H
#define JNI_PARTICIPANT_H

#include "jni_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSJNI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**@brief The participant object.
 * Maps to DCPS DomainParticipant.
 */
C_STRUCT(jni_participant){
    C_EXTENDS(jni_entity);
    u_participant uparticipant; /*!The user participant.*/
    c_iter subscribers;         /*!Set of subscribers.*/
    c_iter publishers;          /*!Set of publishers.*/
    c_iter topics;              /*!Set of topics.*/
    c_iter partitions;          /*!Set of partitions.*/
    long domainId;              /*!The id of the Domain it participates in.*/
};

#define jni_participant(p) ((jni_participant)(p))

/**@brief Creates a new participant. 
 * 
 * This function should only be called by the jni_participantFactory.
 * 
 * @param kernel The kernel that is associated with the DCPS Domain.
 * The participant will be created within this kernel.
 * @param name The name of the participant.
 * @param domainId The id of the DCPS Domain.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created participant.
 */
OS_API jni_participant
jni_participantNew(
    const c_char* uri,
    const char* name,
    long domainId,
    v_qos qos);

/**@brief Destroys the supplied participant. 
 * 
 * All contained entities must have been deleted before this is possible. If this is not the 
 * case JNI_RESULT_PRECONDITION_NOT_MET will be returned. This function
 * should only be called by the jni_participantFactory.
 * 
 * @param p The participant to destroy.
 * 
 * @return JNI_RESULT_OK if succeeded, any other result otherwise.
 */
OS_API jni_result
jni_participantFree(
    jni_participant p);

/**@brief Creates a new publisher within the supplied participant. 
 * 
 * The created publisher is stored in this participant.
 * 
 * @param p The participant to create the publisher in.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created publisher.
 */
OS_API jni_publisher
jni_createPublisher(
    jni_participant p, 
    v_publisherQos qos);

/**@brief Deletes the supplied publisher and removes it from the supplied participant.
 * 
 * If the supplied participant does not contain the supplied publisher,
 * JNI_PRECONDITION_NOT_MET is returned. The entities within the supplied 
 * publisher must have been deleted before calling this function. If this is 
 * not the case, JNI_RESULT_PRECONDITION_NOT_MET is returned.
 * 
 * @param p The participant that contains the publisher.
 * @param pub The publisher to delete.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deletePublisher(
    jni_participant p, 
    jni_publisher pub);

/**@brief Creates a new subscriber within the supplied participant.
 * 
 * The created subscriber is stored in this participant.
 * 
 * @param p The participant to create the subscriber in.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created subscriber.
 */
OS_API jni_subscriber
jni_createSubscriber(
    jni_participant p, 
    v_subscriberQos qos);

/**@brief Deletes the supplied subscriber and removes it from the supplied participant.
 * 
 * If the supplied participant does not contain the supplied subscriber,
 * JNI_PRECONDITION_NOT_MET is returned. The entities within the supplied 
 * publisher must have been deleted before calling this function. If this is 
 * not the case, JNI_RESULT_PRECONDITION_NOT_MET is returned.
 * 
 * @param p The participant that contains the subscriber.
 * @param sub The subscriber to delete.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deleteSubscriber(
    jni_participant p, 
    jni_subscriber sub);

/**@brief Creates a new topic within the supplied participant. 
 * The created topic is stored in this participant.
 * At this time the topic must already exist.
 * The topic cannot be created otherwise.
 * 
 * @param p The participant to create the topic in.
 * @param name The name of the topic.
 * @param typeName The name of the type of the topic.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created topic.
 */
OS_API jni_topic
jni_createTopic(
    jni_participant p,
    const char* name,
    const char* typeName,
    v_topicQos qos);

/**@brief Looks up the topic with the supplied name within the supplied
 * participant.
 * 
 * @param p The participant that must contain the topic.
 * @param name The name of the topic to find.
 * 
 * @return The topic with the supplied name. If the topic could not be
 * found, NULL is returned.
 */
OS_API jni_topic
jni_lookupTopic(
    jni_participant p,
    const char* name);

/**@brief Removes the supplied topic from the supplied participant.
 * If the supplied participant does not contain the supplied topic,
 * JNI_PRECONDITION_NOT_MET is returned.
 * 
 * @param p The participant that contains the topic.
 * @param top The topic to remove from the participant..
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deleteTopic(
    jni_participant p,
    jni_topic top);
                                  
/**@brief Removes all entities within the supplied participant. 
 * 
 * That means; topics, publishers and subscribers will be deleted. This funtion first
 * recursively calls the delete entities functions of the entities it contains.
 * After a succesfull result, the participant can be deleted safely.
 * 
 * @param p The participant to delete all entities of.
 * @return JNI_RESULT_OK if succeeded, any other result otherwise.
 */      
OS_API jni_result
jni_deleteParticipantEntities(
    jni_participant p);

/**@brief Adds a partition to the supplied participant.
 * 
 * In the following cases no partition is created:
 * - name = ""  -> No valid partition.
 * - name = "*" -> All partitions, these are the default and already work.
 * - If the name already exists it will not be created again and JNI_RESULT_OK
 * is returned.
 * 
 * @param p The participant to add the partition to.
 * @param partitionName The name of the partition to add to the participant.
 * @result JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_participantAddPartition(
    jni_participant p,
    const c_char* partitionName);

#undef OS_API
   
#if defined (__cplusplus)
}
#endif

#endif /* JNI_PARTICIPANT_H */
