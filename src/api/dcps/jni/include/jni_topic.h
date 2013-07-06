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

/**@file api/dcps/jni/include/jni_topic.h
 * 
 * @brief The jni_topic object maps to a DCPS Topic.
 */
 
#ifndef JNI_TOPIC_H
#define JNI_TOPIC_H

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

/**@brief The topic description object.
 * 
 * Maps to DCPS TopicDescription.
 */
C_STRUCT(jni_topicDescription){
    C_EXTENDS(jni_entity);
    char* name;                 /*!The name of the topic.*/
    char* typeName;             /*!The name of the type of the topic.*/
    char* keyList;              /*!The list of keys of the topic.*/
    jni_participant participant;/*!The participant it lives in.*/
};

/**@brief The topic object.
 * 
 * Maps to DCPS Topic.
 */
OS_API C_STRUCT(jni_topic){
    C_EXTENDS(jni_topicDescription);
    u_topic utopic;                 /*!The user topic.*/
};

#define jni_topic(t) ((jni_topic)(t))
#define jni_topicDescription(td) ((jni_topicDescription)(td))

/**@brief Creates a new topic. 
 * 
 * The topic must already be known in the Domain prior to
 * calling this function. This function should only be called by the 
 * jni_participant object (jni_createTopic).
 * 
 * @param p The participant to create the topic in.
 * @param name The name of the topic.
 * @param typeName The name of the type of the topic.
 * @param keyList The keylist of the topic.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created topic. If the operation failed, NULL is returned.
 */
OS_API jni_topic
jni_topicNew(
    jni_participant p,
    const char* name,
    const char* typeName,
    const char* keyList,
    v_topicQos qos);

/**@brief Removes the supplied topic from the participant. 
 * 
 * This function should only be called be the jni_participant 
 * object (jni_deleteTopic).
 * 
 * @param topic The topic to remove.
 */
OS_API jni_result
jni_topicFree(
    jni_topic topic);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* JNI_TOPIC_H */
