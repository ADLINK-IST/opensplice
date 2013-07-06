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
/**
 * @file api/dcps/saj/include/saj_qosUtils.h
 * @brief This file contains functions to synchronize java and gapi qos objects.
 * These functions have been placed in a seperate file for convenience.
 * Copyout represents the translation of java objects to gapi objects while
 * Copyout stands for the translation of gapi objects to java objects.
 */

#ifndef SAJ_QOSUTILS_H
#define SAJ_QOSUTILS_H

#include "saj_utilities.h"

/**
 * @brief Transforms the java UserDataQosPolicy object into a
 * gapi UserDataQosPolicy object.
 * @param env The JNI environment.
 * @param src Java UserDataQosPolicy object.
 * @param dst Pointer to a gapi gapi_userDataQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_UserDataQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_userDataQosPolicy *dst);

/**
 * @brief Transforms the gapi UserDataQosPolicy object into a
 * Java UserDataQosPolicy object.
 * @param env The JNI environment.
 * @param src pointer to a gapi gapi_userDataQosPolicy object.
 * @param dst Java userDataQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_UserDataQosPolicyCopyOut(
    JNIEnv *env,  gapi_userDataQosPolicy *src, jobject *dst);

/**
 * @brief Transforms the java EntityFactoryQosPolicy object into a
 * gapi EntityFactoryQosPolicy object.
 * @param env The JNI environment.
 * @param src Java EntityFactoryQosPolicy object.
 * @param dst Pointer to a gapi gapi_entityFactoryQosPolicy object.
 */
saj_returnCode saj_EntityFactoryQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_entityFactoryQosPolicy *dst);

/**
 * @brief Transforms the gapi EntityFactoryQosPolicy object into a
 * java EntityFactoryQosPolicy object.
 * @param env The JNI environment.
 * @param src Gapi gapi_entityFactoryQosPolicy object.
 * @param dst Java EntityFactoryQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_EntityFactoryQosPolicyCopyOut(
    JNIEnv *env, gapi_entityFactoryQosPolicy *src, jobject *dst);

/**
 * @brief Transforms the java PresentationQosPolicy object into a
 * gapi PresentationQosPolicy object.
 * @param env The JNI environment.
 * @param src The java PresentationQosPolicy object.
 * @param dst The gapi PresentationQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_PresentationQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_presentationQosPolicy *dst);

/**
 * @brief Copies the java PartitionQosPolicy to a gapi PartitionQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java PartitionQosPolicy.
 * @param dst The gapi PartitionQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_PartitionQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_partitionQosPolicy *dst);

/**
 * @brief Copies the java GroupDataQosPolicy to a gapi GroupDataQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java GroupDataQosPolicy.
 * @param dst The gapi GroupDataQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_GroupDataQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_groupDataQosPolicy *dst);

/**
 * @brief Copies the java TopicDataQosPolicy to a gapi TopicDataQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java TopicDataQosPolicy.
 * @param dst The gapi TopicDataQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_TopicDataQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_topicDataQosPolicy *dst);

/**
 * @brief Copies the java DurabilityQosPolicy to a gapi DurabilityQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java DurabilityQosPolicy.
 * @param dst The gapi DurabilityQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DurabilityQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_durabilityQosPolicy *dst);

/**
 * @brief Copies the java DurabilityServiceQosPolicy to a gapi DurabilityServiceQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java DurabilityServiceQosPolicy.
 * @param dst The gapi DurabilityServiceQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DurabilityServiceQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_durabilityServiceQosPolicy *dst);

/**
 * @brief Copies the java DeadlineQosPolicy to a gapi DeadlineQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java DeadlineQosPolicy.
 * @param dst The gapi DeadlineQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DeadlineQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_deadlineQosPolicy *dst);

/**
 * @brief Transforms the gapi DeadlineQosPolicy object into a
 * java DeadlineQosPolicy object.
 * @param env The JNI environment.
 * @param src Gapi _DDS_DeadlineQosPolicy object.
 * @param dst Java DeadlineQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DeadlineQosPolicyCopyOut(
    JNIEnv *env, gapi_deadlineQosPolicy *src, jobject *dst);

/**
 * @brief Copies the java LatencyBudgetQosPolicy to a gapi LatencyBudgetQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java LatencyBudgetQosPolicy.
 * @param dst The gapi LatencyBudgetQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_LatencyBudgetQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_latencyBudgetQosPolicy *dst);

/**
 * @brief Transforms the gapi LatencyBudgetQosPolicy object into a
 * java LatencyBudgetQosPolicy object.
 * @param env The JNI environment.
 * @param src Gapi LatencyBudgetQosPolicy object.
 * @param dst Java LatencyBudgetQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_LatencyBudgetQosPolicyCopyOut(
    JNIEnv *env, gapi_latencyBudgetQosPolicy *src, jobject *dst);

/**
 * @brief Copies the java LivelinessQosPolicy to a gapi LivelinessQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java LivelinessQosPolicy.
 * @param dst The gapi LivelinessQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_LivelinessQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_livelinessQosPolicy *dst);

/**
 * @brief Transforms the gapi LivelinessQosPolicy object into a
 * java LivelinessQosPolicy object.
 * @param env The JNI environment.
 * @param src Gapi LivelinessQosPolicy object.
 * @param dst Java LivelinessQosPolicy object.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_LivelinessQosPolicyCopyOut(
    JNIEnv *env, gapi_livelinessQosPolicy *src,  jobject *dst);

/**
 * @brief Copies the java ReliabilityQosPolicy to a gapi ReliabilityQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java ReliabilityQosPolicy.
 * @param dst The gapi ReliabilityQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_ReliabilityQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_reliabilityQosPolicy *dst);

/**
 * @brief Copies the java DestinationOrderQosPolicy to a gapi DestinationOrderQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java DestinationOrderQosPolicy.
 * @param dst The gapi DestinationOrderQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DestinationOrderQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_destinationOrderQosPolicy *dst);

/**
 * @brief Copies the java HistoryQosPolicy to a gapi HistoryQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java HistoryQosPolicy.
 * @param dst The gapi HistoryQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_HistoryQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_historyQosPolicy *dst);

/**
 * @brief Copies the java ResourceLimitsQosPolicy to a gapi
 * ResourceLimitsQosPolicy object.
 * @param env The JNI environment.
 * @param src The java ResourceLimitsQosPolicy.
 * @param dst The gapi ResourceLimitsQosPolicy.
 */
saj_returnCode saj_ResourceLimitsQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_resourceLimitsQosPolicy *dst);

/**
 * @brief Copies the java TransportPriorityQosPolicy to a gapi
 * TransportPriorityQosPolicy  object.
 * @param env The JNI environment.
 * @param src The java TransportPriorityQosPolicy.
 * @param dst The gapi TransportPriorityQosPolicy.
 */
saj_returnCode saj_TransportPriorityQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_transportPriorityQosPolicy *dst);

/**
 * @brief Copies the java LifespanQosPolicy to a gapi LifespanQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java LifespanQosPolicy.
 * @param dst The gapi LifespanQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_LifespanQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_lifespanQosPolicy *dst);

/**
 * @brief Copies the java OwnershipQosPolicy to a gapi OwnershipQosPolicy
 * object.
 * @param env The JNI environment.
 * @param src The java OwnershipQosPolicy.
 * @param dst The gapi OwnershipQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_OwnershipQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_ownershipQosPolicy *dst);


saj_returnCode saj_DomainParticipantFactoryQosCopyIn(
    JNIEnv *env, const jobject src, gapi_domainParticipantFactoryQos *dst);

saj_returnCode saj_DomainParticipantFactoryQosCopyOut(
    JNIEnv *env, gapi_domainParticipantFactoryQos *src, jobject *dst);

/**
 * @brief Copies the java DomainParticipantQos to a gapi DomainParticipantQos
 * object.
 * @param env The JNI environment.
 * @param src The java DomainParticipantQos.
 * @param dst The gapi DomainParticipantQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DomainParticipantQosCopyIn(
    JNIEnv *env, const jobject src, gapi_domainParticipantQos *dst);

/**
 * @brief Copies the gapi DomainParticipantQos to a java DomainParticipantQos
 * object.
 * @param env The JNI environment.
 * @param src The gapi DomainParticipantQos.
 * @param dst The java DomainParticipantQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_DomainParticipantQosCopyOut(
    JNIEnv *env, gapi_domainParticipantQos *src, jobject *dst);

/**
 * @brief Copies the java OwnershipStrengthQosPolicy to a gapi
 * OwnershipStrengthQosPolicy  object.
 * @param env The JNI environment.
 * @param src The java OwnershipStrengthQosPolicy.
 * @param dst The gapi OwnershipStrengthQosPolicy.
 */
saj_returnCode saj_OwnershipStrengthQosPolicyCopyIn(
    JNIEnv *env, jobject src, gapi_ownershipStrengthQosPolicy *dst);

/**
 * @brief Copies the java ReaderDataLifecycleQosPolicy to a gapi
 * ReaderDataLifecycleQosPolicy object.
 * @param env The JNI environment.
 * @param src The java ReaderDataLifecycleQosPolicy.
 * @param dst The gapi ReaderDataLifecycleQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_ReaderDataLifecycleQosPolicyCopyIn(
    JNIEnv *env, const jobject src, gapi_readerDataLifecycleQosPolicy *dst);

/**
 * @brief Copies the java TimeBasedFilterQosPolicy to a gapi
 * TimeBasedFilterQosPolicy object.
 * @param env The JNI environment.
 * @param src The java TimeBasedFilterQosPolicy.
 * @param dst The gapi TimeBasedFilterQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_TimeBasedFilterQosPolicyCopyIn(
    JNIEnv *env, const jobject src, gapi_timeBasedFilterQosPolicy *dst);

/**
 * @brief Copies the java ShareQosPolicy to a gapi
 * ShareQosPolicy object.
 * TODO: Implement ShareQosPolicy-support for Java.
 * @param env The JNI environment.
 * @param src The java ShareQosPolicy.
 * @param dst The gapi ShareQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ShareQosPolicyCopyIn(
    JNIEnv *env, const jobject src, gapi_shareQosPolicy *dst);

/**
 * @brief Copies the java ShareQosPolicy to a gapi
 * ShareQosPolicy object.
 * TODO: Implement ShareQosPolicy-support for Java.
 * @param env The JNI environment.
 * @param src The java ShareQosPolicy.
 * @param dst The gapi ShareQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ShareQosPolicyCopyOut(
    JNIEnv *env, gapi_shareQosPolicy *src,  jobject *dst);

/**
 * @brief Copies the java SubscriberQos to a gapi SubscriberQos
 * object.
 * @param env The JNI environment.
 * @param src The java SubscriberQos.
 * @param dst The gapi SubscriberQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_SubscriberQosCopyIn(
    JNIEnv *env, const jobject src, gapi_subscriberQos *dst);

/**
 * @brief Copies the gapi DestinationOrderQosPolicy to a java
 * DestinationOrderQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi DestinationOrderQosPolicy.
 * @param dst The java DestinationOrderQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DestinationOrderQosPolicyCopyOut(
    JNIEnv *env, gapi_destinationOrderQosPolicy *src, jobject *dst);

/**
 * @brief Copies the gapi DurabilityQosPolicy to a java
 * DurabilityQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi DurabilityQosPolicy.
 * @param dst The java DurabilityQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DurabilityQosPolicyCopyOut(
    JNIEnv                      *env,
    gapi_durabilityQosPolicy    *src,
    jobject                     *dst);

/**
 * @brief Copies the gapi DurabilityServiceQosPolicy to a java
 * DurabilityServiceQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi DurabilityServiceQosPolicy.
 * @param dst The java DurabilityServiceQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DurabilityServiceQosPolicyCopyOut(
    JNIEnv                          *env,
    gapi_durabilityServiceQosPolicy *src,
    jobject                         *dst);

/**
 * @brief Copies the gapi GroupDataQosPolicy to a java
 * GroupDataQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi GroupDataQosPolicy.
 * @param dst The java GroupDataQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_GroupDataQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_groupDataQosPolicy *src,
    jobject                 *dst);

/**
 * @brief Copies the gapi HistoryQosPolicy to a java
 * HistoryQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi HistoryQosPolicy.
 * @param dst The java HistoryQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_HistoryQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_historyQosPolicy   *src,
    jobject                 *dst);

/**
 * @brief Copies the gapi LifespanQosPolicy to a java
 * LifespanQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi LifespanQosPolicy.
 * @param dst The java LifespanQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_LifespanQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_lifespanQosPolicy  *src,
    jobject                 *dst);

/**
 * @brief Copies the gapi OwnershipQosPolicy to a java
 * OwnershipQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi OwnershipQosPolicy.
 * @param dst The java OwnershipQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_OwnershipQosPolicyCopyOut(
    JNIEnv *env,
    gapi_ownershipQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi OwnershipStrengthQosPolicy to a java
 * OwnershipStrengthQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi OwnershipStrengthQosPolicy.
 * @param dst The java OwnershipStrengthQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_OwnershipStrengthQosPolicyCopyOut(
    JNIEnv *env,
    gapi_ownershipStrengthQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi PartitionQosPolicy to a java
 * PartitionQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi PartitionQosPolicy.
 * @param dst The java PartitionQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_PartitionQosPolicyCopyOut(
    JNIEnv *env,
    gapi_partitionQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi PresentationQosPolicy to a java
 * PresentationQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi PresentationQosPolicy.
 * @param dst The java PresentationQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_PresentationQosPolicyCopyOut(
    JNIEnv  *env,
    gapi_presentationQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi ReaderDataLifecycleQosPolicy to a java
 * ReaderDataLifecycleQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi ReaderDataLifecycleQosPolicy.
 * @param dst The java ReaderDataLifecycleQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ReaderDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    gapi_readerDataLifecycleQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi ReaderLifespanQosPolicy to a java
 * ReaderLifespanQosPolicy object.
 * TODO: Implement ReaderLifespanQosPolicy-support for Java.
 * @param env The JNI environment.
 * @param src The gapi ReaderLifespanQosPolicy.
 * @param dst The java ReaderLifespanQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ReaderLifespanQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_readerLifespanQosPolicy *dst);

/**
 * @brief Copies the gapi ReliabilityQosPolicy to a java
 * ReliabilityQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi ReliabilityQosPolicy.
 * @param dst The java ReliabilityQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ReliabilityQosPolicyCopyOut(
    JNIEnv *env,
    gapi_reliabilityQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi ResourceLimitsQosPolicy to a java
 * ResourceLimitsQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi ResourceLimitsQosPolicy.
 * @param dst The java ResourceLimitsQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ResourceLimitsQosPolicyCopyOut(
    JNIEnv *env,
    gapi_resourceLimitsQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi TimeBasedFilterQosPolicy to a java
 * TimeBasedFilterQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi TimeBasedFilterQosPolicy.
 * @param dst The java TimeBasedFilterQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TimeBasedFilterQosPolicyCopyOut(
    JNIEnv *env,
    gapi_timeBasedFilterQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the java TopicQos to a gapi
 * TopicQos object.
 * @param env The JNI environment.
 * @param src The gapi TopicQos.
 * @param dst The java TopicQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TopicQosCopyIn(
    JNIEnv          *env,
    const jobject   src,
    gapi_topicQos   *dst);

/**
 * @brief Copies the gapi TopicDataQosPolicy to a java
 * TopicDataQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi TopicDataQosPolicy.
 * @param dst The java TopicDataQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TopicDataQosPolicyCopyOut(
    JNIEnv *env,
    gapi_topicDataQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi TransportPriorityQosPolicy to a java
 * TransportPriorityQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi TransportPriorityQosPolicy.
 * @param dst The java TransportPriorityQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TransportPriorityQosPolicyCopyOut(
    JNIEnv *env,
    gapi_transportPriorityQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi WriterDataLifecycleQosPolicy to a java
 * WriterDataLifecycleQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi WriterDataLifecycleQosPolicy.
 * @param dst The java WriterDataLifecycleQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_WriterDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    gapi_writerDataLifecycleQosPolicy *src,
    jobject *dst);

/**
 * @brief Copies the gapi TopicQos to a java
 * TopicQos object.
 * @param env The JNI environment.
 * @param src The gapi TopicQos.
 * @param dst The java TopicQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TopicQosCopyOut(
    JNIEnv          *env,
    gapi_topicQos   *src,
    jobject         *dst);

/**
 * @brief Copies the gapi SubscriberQos to a java
 * SubscriberQos object.
 * @param env The JNI environment.
 * @param src The gapi SubscriberQos.
 * @param dst The java SubscriberQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_SubscriberQosCopyOut(
    JNIEnv *env,
    gapi_subscriberQos *src,
    jobject *dst);

/**
 * @brief Copies the gapi PublisherQos to a java
 * PublisherQos object.
 * @param env The JNI environment.
 * @param src The gapi PublisherQos.
 * @param dst The java PublisherQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_PublisherQosCopyOut(
    JNIEnv *env,
    gapi_publisherQos *src,
    jobject *dst);

/**
 * @brief Copies the java PublisherQos to a gapi
 * PublisherQos object.
 * @param env The JNI environment.
 * @param src The java PublisherQos.
 * @param dst The gapi PublisherQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_PublisherQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_publisherQos *dst);

/**
 * @brief Copies the java DataReaderQos to a gapi
 * DataReaderQos object.
 * @param env The JNI environment.
 * @param src The java DataReaderQos.
 * @param dst The gapi DataReaderQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DataReaderQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_dataReaderQos *dst);

/**
 * @brief Copies the gapi DataReaderQos to a java
 * DataReaderQos object.
 * @param env The JNI environment.
 * @param src The gapi DataReaderQos.
 * @param dst The java DataReaderQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DataReaderQosCopyOut(
    JNIEnv *env,
    gapi_dataReaderQos *src,
    jobject *dst);

/**
 * @brief Copies the java DataReaderViewQos to a gapi
 * DataReaderViewQos object.
 * @param env The JNI environment.
 * @param src The java DataReaderViewQos.
 * @param dst The gapi DataReaderViewQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DataReaderViewQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_dataReaderViewQos *dst);

/**
 * @brief Copies the gapi DataReaderViewQos to a java
 * DataReaderViewQos object.
 * @param env The JNI environment.
 * @param src The gapi DataReaderViewQos.
 * @param dst The java DataReaderViewQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DataReaderViewQosCopyOut(
    JNIEnv *env,
    gapi_dataReaderViewQos *src,
    jobject *dst);

/**
 * @brief Copies the java DataWriterQos to a gapi
 * DataWriterQos object.
 * @param env The JNI environment.
 * @param src The java DataWriterQos.
 * @param dst The gapi DataWriterQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DataWriterQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_dataWriterQos *dst);

/**
 * @brief Copies the gapi DataWriterQos to a java
 * DataWriterQos object.
 * @param env The JNI environment.
 * @param src The gapi DataWriterQos.
 * @param dst The java DataWriterQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_DataWriterQosCopyOut(
    JNIEnv *env,
    gapi_dataWriterQos *src,
    jobject *dst);

/**
 * @brief Copies the java TopicQos to a gapi
 * TopicQos object.
 * @param env The JNI environment.
 * @param src The java TopicQos.
 * @param dst The gapi TopicQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TopicQosCopyIn(
    JNIEnv          *env,
    const jobject   src,
    gapi_topicQos   *dst);

/**
 * @brief Copies the gapi TopicQos to a java
 * TopicQos object.
 * @param env The JNI environment.
 * @param src The gapi TopicQos.
 * @param dst The java TopicQos.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_TopicQosCopyOut(
    JNIEnv          *env,
    gapi_topicQos   *src,
    jobject         *dst);

/**
 * @brief Copies the gapi SubscriptionKeyQosPolicy to a java
 * SubscriptionKeyQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi SubscriptionKeyQosPolicy.
 * @param dst The java SubscriptionKeyQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_SubscriptionKeyQosPolicyCopyOut(
    JNIEnv                          *env,
    gapi_subscriptionKeyQosPolicy   *src,
    jobject                         *dst);

/**
 * @brief Copies the java SubscriptionKeyQosPolicy to a gapi
 * SubscriptionKeyQosPolicy object.
 * @param env The JNI environment.
 * @param src The java SubscriptionKeyQosPolicy.
 * @param dst The gapi SubscriptionKeyQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_SubscriptionKeyQosPolicyCopyIn(
    JNIEnv                          *env,
    const jobject                   src,
    gapi_subscriptionKeyQosPolicy   *dst);

/**
 * @brief Copies the gapi ViewKeyQosPolicy to a java
 * ViewKeyQosPolicy object.
 * @param env The JNI environment.
 * @param src The gapi ViewKeyQosPolicy.
 * @param dst The java ViewKeyQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ViewKeyQosPolicyCopyOut(
    JNIEnv                          *env,
    gapi_viewKeyQosPolicy   *src,
    jobject                         *dst);

/**
 * @brief Copies the java ViewKeyQosPolicy to a gapi
 * ViewKeyQosPolicy object.
 * @param env The JNI environment.
 * @param src The java ViewKeyQosPolicy.
 * @param dst The gapi ViewKeyQosPolicy.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode
saj_ViewKeyQosPolicyCopyIn(
    JNIEnv                          *env,
    const jobject                   src,
    gapi_viewKeyQosPolicy   *dst);

#endif /* SAJ_QOSUTILS_H */
