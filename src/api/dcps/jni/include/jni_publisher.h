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

/**@file api/dcps/jni/include/jni_publisher.h
 * 
 * @brief The jni_publisher object represents a DCPS Publisher. 
 * 
 * It acts as placeholder for jni_writer objects.
 */

#ifndef JNI_PUBLISHER_H
#define JNI_PUBLISHER_H

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

/**@brief The publisher object. Maps to DCPS Publisher.
 */
C_STRUCT(jni_publisher){
    C_EXTENDS(jni_entity);
    u_publisher upublisher;     /*!The user publisher.*/
    c_iter writers;             /*!Set of writers.*/
    jni_participant participant;/*!The participant it lives in.*/
};

#define jni_publisher(p) ((jni_publisher)(p))

/**@brief Creates a new publisher. 
 * 
 * This function should only be called by
 * the jni_participant (jni_createPublisher).
 * 
 * @param p The participant to create the publisher in.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created publisher.
 */
OS_API jni_publisher
jni_publisherNew(
    jni_participant p, 
    v_publisherQos qos);

/**@brief Deletes a publisher. 
 * 
 * This function should only be called by the
 * jni_participant (jni_deletePublisher). Before calling this function
 * all contained entities of the publisher must have been deleted. If this
 * is not the case, JNI_RESULT_PRECONDITION_NO_MET is returned.
 * 
 * @param p The publisher to delete. 
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_publisherFree(
    jni_publisher p);

/**@brief Deletes all contained entities of the supplied publisher. 
 * 
 * After a succesfull execution of the operation it is safe to delete the publisher.
 * 
 * @param pub The publisher to delete the entities of.
 */
OS_API jni_result
jni_deletePublisherEntities(
    jni_publisher pub);

/**@brief Creates a writer that writes instances of the supplied topic into
 * the supplied publisher.
 * 
 * @param pub The publisher to create the writer in.
 * @param top The topic where the writer must write instances of.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created writer. If the operation did not succeed, NULL is
 * returned.
 */
OS_API jni_writer
jni_createWriter(
    jni_publisher pub,
    jni_topic top,
    v_writerQos qos);

/**@brief Deletes the supplied writer from the supplied publisher. 
 * 
 * If the supplied publisher does not contain the supplied writer, 
 * JNI_RESULT_PRECONDITION_NOT_MET is returned.
 * 
 * @param pub The publisher that contains the writer.
 * @param wri The writer to delete.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deleteWriter(
    jni_publisher pub,
    jni_writer wri);

/**@brief Attaches publisher to the partitions associated with the supplied expression.
 * 
 * @param pub The publisher where the partitions will be attached to.
 * @param partitionExpr The expression which contains the query for partitions.
 * @return JNI_RESULT_OK if succeeded, any other result otherwise.
 */
OS_API jni_result
jni_publisherPublish(
    jni_publisher pub,
    const c_char* partitionExpr);

/**@brief Looks up the DataWriter associated with the supplied topic within
 * the supplied Publisher.
 * 
 * If multiple DataWriter objects exist that are associated with the supplied
 * topic, one of them is returned. It is not specified, which one.
 * 
 * @param pub The Publisher to find the DataWriter in.
 * @param topicName The name of the Topic that the DataWriter must be associated
 * with.
 * @return A DataWriter that is associated with the supplied Topic, or NULL if 
 * it could not be found.
 */
OS_API jni_writer
jni_publisherLookupWriter(
    jni_publisher pub,
    const c_char* topicName);

#undef OS_API
   
#if defined (__cplusplus)
}
#endif

#endif /* JNI_PUBLISHER__H */
