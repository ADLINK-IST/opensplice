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
/**
 * @file api/dcps/saj/include/saj_qosUtils.h
 * @brief This file contains functions to synchronize java and user layer qos objects.
 * These functions have been placed in a seperate file for convenience.
 * Copyout represents the translation of java objects to user layer objects while
 * Copyout stands for the translation of user layer objects to java objects.
 */

#ifndef SAJ_QOSUTILS_H
#define SAJ_QOSUTILS_H

#include "saj_utilities.h"
#include "v_kernel.h"

/**
 * @brief Copies the java DomainParticipantQos to a user layer DomainParticipantQos
 * object.
 * @param env The JNI environment.
 * @param src The java DomainParticipantQos.
 * @param dst The user layer DomainParticipantQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_domainParticipantQosCopyIn(
    JNIEnv *env,
    const jobject src,
    u_participantQos dst);

/**
 * @brief Copies the user layer DomainParticipantQos to a java DomainParticipantQos
 * object.
 * @param env The JNI environment.
 * @param src The user layer DomainParticipantQos.
 * @param dst The java DomainParticipantQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_domainParticipantQosCopyOut(
    JNIEnv *env,
    u_participantQos src,
    jobject *dst);

/**
 * @brief Copies the java TopicQos to a user layer
 * TopicQos object.
 * @param env The JNI environment.
 * @param src The user layer TopicQos.
 * @param dst The java TopicQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_topicQosCopyIn(
    JNIEnv *env,
    const jobject src,
    u_topicQos dst);

/**
 * @brief Copies the user layer TopicQos to a java
 * TopicQos object.
 * @param env The JNI environment.
 * @param src The user layer TopicQos.
 * @param dst The java TopicQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_topicQosCopyOut(
    JNIEnv *env,
    u_topicQos src,
    jobject *dst);

/**
 * @brief Copies the java SubscriberQos to a user layer SubscriberQos
 * object.
 * @param env The JNI environment.
 * @param src The java SubscriberQos.
 * @param dst The user layer SubscriberQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_subscriberQosCopyIn(
    JNIEnv *env,
    const jobject src,
    u_subscriberQos dst);

/**
 * @brief Copies the user layer SubscriberQos to a java
 * SubscriberQos object.
 * @param env The JNI environment.
 * @param src The user layer SubscriberQos.
 * @param dst The java SubscriberQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_subscriberQosCopyOut(
    JNIEnv *env,
    u_subscriberQos src,
    jobject *dst);

/**
 * @brief Copies the java PublisherQos to a user layer
 * PublisherQos object.
 * @param env The JNI environment.
 * @param src The java PublisherQos.
 * @param dst The user layer PublisherQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_publisherQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_publisherQos dst);

/**
 * @brief Copies the user layer PublisherQos to a java
 * PublisherQos object.
 * @param env The JNI environment.
 * @param src The user layer PublisherQos.
 * @param dst The java PublisherQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_publisherQosCopyOut(
    JNIEnv *env,
    u_publisherQos src,
    jobject *dst);

/**
 * @brief Copies the java DataReaderQos to a user layer
 * DataReaderQos object.
 * @param env The JNI environment.
 * @param src The java DataReaderQos.
 * @param dst The user layer DataReaderQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_dataReaderQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_readerQos dst);

/**
 * @brief Copies the user layer DataReaderQos to a java
 * DataReaderQos object.
 * @param env The JNI environment.
 * @param src The user layer DataReaderQos.
 * @param dst The java DataReaderQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_dataReaderQosCopyOut(
    JNIEnv *env,
    u_readerQos src,
    jobject *dst);

/**
 * @brief Copies the java DataReaderViewQos to a user layer
 * DataReaderViewQos object.
 * @param env The JNI environment.
 * @param src The java DataReaderViewQos.
 * @param dst The user layer DataReaderViewQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_dataReaderViewQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_dataViewQos dst);

/**
 * @brief Copies the user layer DataReaderViewQos to a java
 * DataReaderViewQos object.
 * @param env The JNI environment.
 * @param src The user layer DataReaderViewQos.
 * @param dst The java DataReaderViewQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_dataReaderViewQosCopyOut(
    JNIEnv *env,
    u_dataViewQos src,
    jobject *dst);

/**
 * @brief Copies the java DataWriterQos to a user layer
 * DataWriterQos object.
 * @param env The JNI environment.
 * @param src The java DataWriterQos.
 * @param dst The user layer DataWriterQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_dataWriterQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_writerQos dst);

/**
 * @brief Copies the user layer DataWriterQos to a java
 * DataWriterQos object.
 * @param env The JNI environment.
 * @param src The user layer DataWriterQos.
 * @param dst The java DataWriterQos.
 * @return SAJ_RETCODE_INTERNAL_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_dataWriterQosCopyOut(
    JNIEnv *env,
    u_writerQos src,
    jobject *dst);

saj_returnCode
saj_shareQosPolicyCopyOut(
    JNIEnv *env,
    struct v_sharePolicy *src,
    jobject *dst);

saj_returnCode
saj_latencyBudgetQosPolicyCopyOut(
    JNIEnv *env,
    struct v_latencyPolicy *src,
    jobject *dst);

saj_returnCode
saj_livelinessQosPolicyCopyOut(
    JNIEnv *env,
    struct v_livelinessPolicy *src,
    jobject *dst);

saj_returnCode
saj_readerLifespanQosPolicyCopyOut(
    JNIEnv *env,
    struct v_readerLifespanPolicy *src,
    jobject *dst);

saj_returnCode
saj_userDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userDataPolicy *src,
    jobject *dst);

saj_returnCode
saj_builtinUserDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinUserDataPolicy *src,
    jobject *dst);

saj_returnCode
saj_entityFactoryQosPolicyCopyOut(
    JNIEnv *env,
    struct v_entityFactoryPolicy *src,
    jobject *dst);

saj_returnCode
saj_deadlineQosPolicyCopyOut(
    JNIEnv *env,
    struct v_deadlinePolicy *src,
    jobject *dst);

saj_returnCode
saj_durabilityQosPolicyCopyOut(
    JNIEnv *env,
    struct v_durabilityPolicy *src,
    jobject *dst);

saj_returnCode
saj_durabilityServiceQosPolicyCopyOut(
    JNIEnv *env,
    struct v_durabilityServicePolicy *src,
    jobject *dst);

saj_returnCode
saj_groupDataQosPolicyCopyOut(
    JNIEnv *env,
    v_groupDataPolicyI *src,
    jobject *dst);

saj_returnCode
saj_builtinGroupDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinGroupDataPolicy *src,
    jobject *dst);

saj_returnCode
saj_historyQosPolicyCopyOut(
    JNIEnv *env,
    struct v_historyPolicy *src,
    jobject *dst);

saj_returnCode
saj_lifespanQosPolicyCopyOut(
    JNIEnv *env,
    struct v_lifespanPolicy *src,
    jobject *dst);

saj_returnCode
saj_ownershipQosPolicyCopyOut(
    JNIEnv *env,
    struct v_ownershipPolicy *src,
    jobject *dst);

saj_returnCode
saj_ownershipStrengthQosPolicyCopyOut(
    JNIEnv *env,
    struct v_strengthPolicy *src,
    jobject *dst);

saj_returnCode
saj_builtinPartitionQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinPartitionPolicy *src,
    jobject *dst);

saj_returnCode
saj_partitionQosPolicyCopyOut(
    JNIEnv *env,
    v_partitionPolicyI *src,
    jobject *dst);

saj_returnCode
saj_presentationQosPolicyCopyOut(
    JNIEnv *env,
    struct v_presentationPolicy *src,
    jobject *dst);

saj_returnCode
saj_readerDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    struct v_readerLifecyclePolicy *src,
    jobject *dst);

saj_returnCode
saj_subscriptionKeysQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userKeyPolicy *src,
    jobject *dst);

saj_returnCode
saj_subscriptionKeysQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userKeyPolicy *src,
    jobject *dst);

saj_returnCode
saj_viewKeysQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userKeyPolicy *src,
    jobject *dst);

saj_returnCode
saj_reliabilityQosPolicyCopyOut(
    JNIEnv *env,
    struct v_reliabilityPolicy *src,
    jobject *dst);

saj_returnCode
saj_resourceLimitsQosPolicyCopyOut(
    JNIEnv *env,
    struct v_resourcePolicy *src,
    jobject *dst);

saj_returnCode
saj_timeBasedFilterQosPolicyCopyOut(
    JNIEnv *env,
    struct v_pacingPolicy *src,
    jobject *dst);

saj_returnCode
saj_topicDataQosPolicyCopyOut(
    JNIEnv *env,
    v_topicDataPolicyI *src,
    jobject *dst);

saj_returnCode
saj_builtinTopicDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinTopicDataPolicy *src,
    jobject *dst);

saj_returnCode
saj_transportPriorityQosPolicyCopyOut(
    JNIEnv *env,
    struct v_transportPolicy *src,
    jobject *dst);

saj_returnCode
saj_writerDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    struct v_writerLifecyclePolicy *src,
    jobject *dst);

saj_returnCode
saj_destinationOrderQosPolicyCopyOut(
    JNIEnv *env,
    struct v_orderbyPolicy *src,
    jobject *dst);

saj_returnCode
saj_shareQosPolicyICopyOut(
    JNIEnv *env,
    v_sharePolicyI *src,
    jobject *dst);

saj_returnCode
saj_latencyBudgetQosPolicyICopyOut(
    JNIEnv *env,
    v_latencyPolicyI *src,
    jobject *dst);

saj_returnCode
saj_livelinessQosPolicyICopyOut(
    JNIEnv *env,
    v_livelinessPolicyI *src,
    jobject *dst);

saj_returnCode
saj_readerLifespanQosPolicyICopyOut(
    JNIEnv *env,
    v_readerLifespanPolicyI *src,
    jobject *dst);

saj_returnCode
saj_userDataQosPolicyICopyOut(
    JNIEnv *env,
    v_userDataPolicyI *src,
    jobject *dst);

saj_returnCode
saj_entityFactoryQosPolicyICopyOut(
    JNIEnv *env,
    v_entityFactoryPolicyI *src,
    jobject *dst);

saj_returnCode
saj_deadlineQosPolicyICopyOut(
    JNIEnv *env,
    v_deadlinePolicyI *src,
    jobject *dst);

saj_returnCode
saj_durabilityQosPolicyICopyOut(
    JNIEnv *env,
    v_durabilityPolicyI *src,
    jobject *dst);

saj_returnCode
saj_durabilityServiceQosPolicyICopyOut(
    JNIEnv *env,
    v_durabilityServicePolicyI *src,
    jobject *dst);

saj_returnCode
saj_groupDataQosPolicyICopyOut(
    JNIEnv *env,
    v_groupDataPolicyI *src,
    jobject *dst);

saj_returnCode
saj_historyQosPolicyICopyOut(
    JNIEnv *env,
    v_historyPolicyI *src,
    jobject *dst);

saj_returnCode
saj_lifespanQosPolicyICopyOut(
    JNIEnv *env,
    v_lifespanPolicyI *src,
    jobject *dst);

saj_returnCode
saj_ownershipQosPolicyICopyOut(
    JNIEnv *env,
    v_ownershipPolicyI *src,
    jobject *dst);

saj_returnCode
saj_ownershipStrengthQosPolicyICopyOut(
    JNIEnv *env,
    v_strengthPolicyI *src,
    jobject *dst);

saj_returnCode
saj_partitionQosPolicyICopyOut(
    JNIEnv *env,
    v_partitionPolicyI *src,
    jobject *dst);

saj_returnCode
saj_presentationQosPolicyICopyOut(
    JNIEnv *env,
    v_presentationPolicyI *src,
    jobject *dst);

saj_returnCode
saj_readerDataLifecycleQosPolicyICopyOut(
    JNIEnv *env,
    v_readerLifecyclePolicyI *src,
    jobject *dst);

saj_returnCode
saj_subscriptionKeysQosPolicyICopyOut(
    JNIEnv *env,
    v_userKeyPolicyI *src,
    jobject *dst);

saj_returnCode
saj_subscriptionKeysQosPolicyICopyOut(
    JNIEnv *env,
    v_userKeyPolicyI *src,
    jobject *dst);

saj_returnCode
saj_viewKeysQosPolicyICopyOut(
    JNIEnv *env,
    v_userKeyPolicyI *src,
    jobject *dst);

saj_returnCode
saj_reliabilityQosPolicyICopyOut(
    JNIEnv *env,
    v_reliabilityPolicyI *src,
    jobject *dst);

saj_returnCode
saj_resourceLimitsQosPolicyICopyOut(
    JNIEnv *env,
    v_resourcePolicyI *src,
    jobject *dst);

saj_returnCode
saj_timeBasedFilterQosPolicyICopyOut(
    JNIEnv *env,
    v_pacingPolicyI *src,
    jobject *dst);

saj_returnCode
saj_topicDataQosPolicyICopyOut(
    JNIEnv *env,
    v_topicDataPolicyI *src,
    jobject *dst);

saj_returnCode
saj_transportPriorityQosPolicyICopyOut(
    JNIEnv *env,
    v_transportPolicyI *src,
    jobject *dst);

saj_returnCode
saj_writerDataLifecycleQosPolicyICopyOut(
    JNIEnv *env,
    v_writerLifecyclePolicyI *src,
    jobject *dst);

saj_returnCode
saj_destinationOrderQosPolicyICopyOut(
    JNIEnv *env,
    v_orderbyPolicyI *src,
    jobject *dst);

#endif /* SAJ_QOSUTILS_H */
