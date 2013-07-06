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

/**@file api/dcps/jni/include/jni_subscriber.h
 * 
 * @brief The jni_subscriber object maps to a DCPS Subscriber object. 
 * 
 * It is a placeholder for datareaders.
 */

#ifndef JNI_SUBSCRIBER_H
#define JNI_SUBSCRIBER_H

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

/**@brief The subscriber object.
 * 
 * Maps to DCPS Subscriber.
 */
C_STRUCT(jni_subscriber){
    C_EXTENDS(jni_entity);
    u_subscriber usubscriber;   /*<The user subscriber.*/
    c_iter readers;             /*<Set of readers.*/
    jni_participant participant;/*<The participant it lives in.*/
};

#define jni_subscriber(s) ((jni_subscriber)(s))

/**@brief Creates a new subscriber. This function should only be called by the
 * jni_participant object (jni_createSubscriber).
 * 
 * @param p The participant to create the subscriber in.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created subscriber. If failed, NULL is returned.
 */
OS_API jni_subscriber
jni_subscriberNew(
    jni_participant p,
    v_subscriberQos qos);

/**@brief Deletes the supplied subscriber. This function sould only be called
 * by the jni_participant object (jni_deleteDubscriber).
 * 
 * @param s The subscriber to delete.
 */
OS_API jni_result
jni_subscriberFree(
    jni_subscriber s);

/**@brief Deletes all entities within the supplied subscriber. 
 * 
 * Before doing this it first recursively calls the delete contained entities operation of each
 * contained entity.
 * 
 * @param sub The subscriber to delete all contained entities of.
 * 
 * @return JNI_RESULT_OK if succeeded, anty other jni_result otherwise.
 */
OS_API jni_result
jni_deleteSubscriberEntities(
    jni_subscriber sub);

/**@brief Creates a new reader within the supplied subscriber.
 * 
 * @param sub The subscriber to create the reader in.
 * @param top The topic description where the reader must read instances of.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created reader.
 */
OS_API jni_reader
jni_createReader(
    jni_subscriber sub,
    jni_topicDescription top,
    v_readerQos qos);
 
/**@brief Deletes the supplied reader from the supplied subscriber.
 * 
 * All contained entities of the reader must be deleted prior to calling this 
 * operation. If not, JNI_RESULT_PRE_CONDITION_NO_MET is returned. If the 
 * supplied subscriber does not contain the supplied reader, 
 * JNI_PRECONDITION_NOT_MET is also returned.
 * 
 * @param sub The subscriber that contains the reader to delete.
 * @param reader The reader to delete.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deleteReader(
    jni_subscriber sub,
    jni_reader reader);


/**@brief Attaches the supplied Subscriber to the partitions to match the supplied
 * expression.
 * 
 * @param sub The subscriber where to attach the partitions to.
 * @param partitionExpr The partition expression.
 */
OS_API jni_result
jni_subscriberSubscribe(
    jni_subscriber sub,
    const c_char* partitionExpr);

/**@brief Looks up a DataReader, that is associated with the
 * supplied Topic, in the supplied Subscriber.
 * 
 * If multiple readers exist that are associated with the supplied topic, one
 * of them is returned. It is not specified which one.
 * 
 * @param sub The subscriber to look in.
 * @param topicName The name of the topic that must be read by the reader.
 * @result The reader that reads data of the supplied topic, or NULL if it cannot
 * be found.
 */
OS_API jni_reader
jni_subscriberLookupReader(
    jni_subscriber sub, 
    const c_char* topicName);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* JNI_SUBSCRIBER_H */
