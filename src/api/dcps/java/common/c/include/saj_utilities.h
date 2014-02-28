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
 * @file api/dcps/saj/include
 * @brief Utility functions for use the saj JNI implementation.
 */

#ifndef SAJ_UTILITIES_H
#define SAJ_UTILITIES_H

#include "gapi.h"
#include <jni.h>
#include <stdio.h>
#include "os_abstract.h"
#include "saj_listener.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAJ
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* In order to force 'kind' compilers that only warn about 0-size arrays, the
 * macro makes the expression result negative when the constraint is not
 * satisfied, forcing a compiler error on the negative array-size. */
#define SAJ_COMPILE_CONSTRAINT_SIZE_EQ(type1, type2) \
{\
    struct type1##_eq_##type2 { \
        char require_sizeof_##type1##_eq_sizeof_##type2 [(sizeof(type1) == sizeof(type2)) ? 1 : -1]; \
        char non_empty_dummy_last_member[1]; \
    }; \
}

#ifndef OSPL_SAJ_NO_LOOP_UNROLLING
/* Implements an 8-fold loop-unroll using a derivative of Duff's engine. */
#define SAJ_LOOP_UNROLL(loopLen, srcBaseType, src, dstBaseType, dst)            \
do {                                                                            \
    register srcBaseType * copySrc = (src);                                     \
    register dstBaseType * copyDst = (dst);                                     \
    dstBaseType const * const dstEnd = (copyDst) + (loopLen);                   \
    assert(loopLen);                                                            \
    /* The number of case statements must be a power of 2 for the */            \
    /* switch-case modulo operation to be correct. */                           \
    switch((loopLen) & ( 8 - 1)) {                                              \
        /* Intentionally missing breaks for cases. Fallthrough is intended. */  \
        case 0: do {   *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 7:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 6:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 5:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 4:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 3:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 2:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
        case 1:        *(copyDst++) = (dstBaseType)*(copySrc++);                \
                   } while(copyDst < dstEnd);                                   \
    }                                                                           \
} while(FALSE)
#endif /* OSPL_SAJ_NO_LOOP_UNROLLING */

#define SET_CACHED(var, value) jniCache.var = value
#define GET_CACHED(var) jniCache.var

/* Defines the package of the java implementation classes */
#define PACKAGENAME "org/opensplice/dds/dcps/"

typedef gapi_long saj_returnCode;

typedef struct sajParDemContext_s * sajParDemContext;

enum saj_returnCode {SAJ_RETCODE_ERROR, SAJ_RETCODE_OK};

/**
 * Represents the SAJ userdata object of a GAPI entity object.
 * It contains:
 * - saj_object              --> A reference to the Java object associated with
 *                               the GAPI object it is in.
 * - listenerData            --> A reference to the data in the listener object
 *                               that is attached to the saj_object in this
 *                               userData.
 * - statusConditionUserData --> Userdata of the GAPI statusCondition
 *                               object that belongs to the saj_object's GAPI
 *                               object.
 */
C_CLASS(saj_userData);

C_STRUCT(saj_userData){
    jobject saj_object;
    saj_listenerData listenerData;
    saj_userData statusConditionUserData;
};

#define saj_userData(d) ((saj_userData)(d))

typedef struct jni_cache_t {
    /* cache the class reference to a GapiSuperClass object */
    jclass gapiSuperClass_class;

    /* field id of the gapiPeer field */
    jfieldID sajSuperClassGapiPeer_fid;

    /* cache the class reference to a FooDataReader object */
    jclass dataReaderImpl_class;

    /* field-/method-id's of the parallelDemarshallingContext field of the DR */
    jfieldID dataReaderImplClassParallelDemarshallingContext_fid;
    jmethodID dataReaderImplClassStartWorkers_mid;
    jmethodID dataReaderImplClassJoinWorkers_mid;
    jfieldID dataReaderImplClassCDRCopy_fid;
    jmethodID dataReaderImplClassCDRDeserializeByteBuffer_mid;

    /* field-id's of the PropertyInterface related fields */
    jfieldID property_name_fid;
    jfieldID property_value_fid;
    jfieldID propertyHolder_value_fid;

    /* Caching the field id's of the QosPolicy attributes */
    jfieldID deadlineQosPolicy_period_fid;
    jfieldID destinationOrderQosPolicy_kind_fid;
    jfieldID durabilityQosPolicy_kind_fid;

    jfieldID durabilityServiceQosPolicy_historyKind_fid;
    jfieldID durabilityServiceQosPolicy_historyDepth_fid;
    jfieldID durabilityServiceQosPolicy_maxSamples_fid;
    jfieldID durabilityServiceQosPolicy_maxInstances_fid;
    jfieldID durabilityServiceQosPolicy_maxSamplesPerInstance_fid;
    jfieldID durabilityServiceQosPolicy_serviceCleanupDelay_fid;

    jfieldID entityFactoryQosPolicy_autoenableCreatedEntities_fid;
    jfieldID groupDataQosPolicy_value_fid;
    jfieldID historyQosPolicy_kind_fid;
    jfieldID historyQosPolicy_depth_fid;
    jfieldID latencyBudgetQosPolicy_duration_fid;
    jfieldID lifespanQosPolicy_duration_fid;
    jfieldID livelinessQosPolicy_leaseDuration_fid;
    jfieldID livelinessQosPolicy_kind_fid;
    jfieldID ownershipQosPolicy_kind_fid;
    jfieldID ownershipStrengthQosPolicy_value_fid;
    jfieldID partitionQosPolicy_name_fid;
    jfieldID presentationQosPolicy_accessScope_fid;
    jfieldID presentationQosPolicy_coherentAccess_fid;
    jfieldID presentationQosPolicy_orderedAccess_fid;
    jfieldID invalidSampleVisibilityQosPolicy_kind_fid;
    jfieldID readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay_fid;
    jfieldID readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay_fid;
    jfieldID readerDataLifecycleQosPolicy_enable_invalid_samples_fid;
    jfieldID readerDataLifecycleQosPolicy_invalid_sample_visibility_fid;
    jfieldID reliabilityQosPolicy_kind_fid;
    jfieldID reliabilityQosPolicy_maxBlockingTime_fid;
    jfieldID reliabilityQosPolicy_synchronous_fid;
    jfieldID resourceLimitsQosPolicy_maxSamples_fid;
    jfieldID resourceLimitsQosPolicy_maxInstances_fid;
    jfieldID resourceLimitsQosPolicy_maxSamplesPerInstance_fid;
    jfieldID timeBasedFilterQosPolicy_minimumSeparation_fid;
    jfieldID topicDataQosPolicy_value_fid;
    jfieldID transportPriorityQosPolicy_value_fid;
    jfieldID userDataQosPolicy_value_fid;
    jfieldID writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances_fid;
    jfieldID writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay_fid;
    jfieldID writerDataLifecycleQosPolicy_autounregisterInstanceDelay_fid;
    jfieldID schedulingClassQosPolicy_kind_fid;
    jfieldID schedulingPriorityQosPolicy_kind_fid;
    jfieldID schedulingQosPolicy_schedulingClass_fid;
    jfieldID schedulingQosPolicy_schedulingPriorityKind_fid;
    jfieldID schedulingQosPolicy_schedulingPriority_fid;

    jfieldID shareQosPolicy_enable_fid;
    jfieldID shareQosPolicy_name_fid;

    jfieldID readerLifespanQosPolicy_useLifespan_fid;
    jfieldID readerLifespanQosPolicy_duration_fid;

    jfieldID subscriptionKeyQosPolicy_useKeyList_fid;
    jfieldID subscriptionKeyQosPolicy_keyList_fid;

    jfieldID viewKeyQosPolicy_useKeyList_fid;
    jfieldID viewKeyQosPolicy_keyList_fid;

    /* caching the field id's for the DomainParticipantFactoryQos */
    jfieldID domainParticipantFactoryQos_entityFactory_fid;

    /* caching the field id's for the DomainParticipantQos */
    jfieldID domainParticipantQos_userData_fid;
    jfieldID domainParticipantQos_entityFactory_fid;
    jfieldID domainParticipantQos_watchdogScheduling_fid;
    jfieldID domainParticipantQos_listenerScheduling_fid;

    /* caching the field id's for the DataReaderQos */
    jfieldID dataReaderQos_durability_fid;
    jfieldID dataReaderQos_deadline_fid;
    jfieldID dataReaderQos_latencyBudget_fid;
    jfieldID dataReaderQos_liveliness_fid;
    jfieldID dataReaderQos_reliability_fid;
    jfieldID dataReaderQos_destinationOrder_fid;
    jfieldID dataReaderQos_history_fid;
    jfieldID dataReaderQos_resourceLimits_fid;
    jfieldID dataReaderQos_userData_fid;
    jfieldID dataReaderQos_ownership_fid;
    jfieldID dataReaderQos_timeBasedFilter_fid;
    jfieldID dataReaderQos_readerDataLifecycle_fid;
    jfieldID dataReaderQos_share_fid;
    jfieldID dataReaderQos_readerLifespan_fid;
    jfieldID dataReaderQos_subscriptionKeys_fid;

    /* caching the field id's for the DataReaderViewQos */
    jfieldID dataReaderViewQos_viewKeys_fid;

    /* caching the field id's for the DataWriterQos */
    jfieldID dataWriterQos_durability_fid;
    jfieldID dataWriterQos_deadline_fid;
    jfieldID dataWriterQos_latencyBudget_fid;
    jfieldID dataWriterQos_liveliness_fid;
    jfieldID dataWriterQos_reliability_fid;
    jfieldID dataWriterQos_destinationOrder_fid;
    jfieldID dataWriterQos_history_fid;
    jfieldID dataWriterQos_resourceLimits_fid;
    jfieldID dataWriterQos_transportPriority_fid;
    jfieldID dataWriterQos_lifespan_fid;
    jfieldID dataWriterQos_userData_fid;
    jfieldID dataWriterQos_ownership_fid;
    jfieldID dataWriterQos_ownershipStrength_fid;
    jfieldID dataWriterQos_writerDataLifecycle_fid;

    /* caching the field id's for the TopicQos */
    jfieldID topicQos_topicData_fid;
    jfieldID topicQos_durability_fid;
    jfieldID topicQos_durabilityService_fid;
    jfieldID topicQos_deadline_fid;
    jfieldID topicQos_latencyBudget_fid;
    jfieldID topicQos_liveliness_fid;
    jfieldID topicQos_reliability_fid;
    jfieldID topicQos_destinationOrder_fid;
    jfieldID topicQos_history_fid;
    jfieldID topicQos_resourceLimits_fid;
    jfieldID topicQos_transportPriority_fid;
    jfieldID topicQos_lifespan_fid;
    jfieldID topicQos_ownership_fid;

    /* caching the field id's for the PublisherQos */
    jfieldID publisherQos_presentation_fid;
    jfieldID publisherQos_partition_fid;
    jfieldID publisherQos_groupData_fid;
    jfieldID publisherQos_entityFactory_fid;

    /* caching the field id's for the SubscriberQos */
    jfieldID subscriberQos_presentation_fid;
    jfieldID subscriberQos_partition_fid;
    jfieldID subscriberQos_groupData_fid;
    jfieldID subscriberQos_entityFactory_fid;
    jfieldID subscriberQos_share_fid;

     /* caching the field id's for the SubscriptionBuiltinTopicData */
    jfieldID subscriptionBuiltinTopicData_key_fid;
    jfieldID subscriptionBuiltinTopicData_participantKey_fid;
    jfieldID subscriptionBuiltinTopicData_topicName_fid;
    jfieldID subscriptionBuiltinTopicData_typeName_fid;
    jfieldID subscriptionBuiltinTopicData_durability_fid;
    jfieldID subscriptionBuiltinTopicData_latencyBudget_fid;
    jfieldID subscriptionBuiltinTopicData_liveliness_fid;
    jfieldID subscriptionBuiltinTopicData_reliability_fid;
    jfieldID subscriptionBuiltinTopicData_ownership_fid;
    jfieldID subscriptionBuiltinTopicData_destinationOrder_fid;
    jfieldID subscriptionBuiltinTopicData_userData_fid;
    jfieldID subscriptionBuiltinTopicData_timeBasedFilter_fid;
    jfieldID subscriptionBuiltinTopicData_deadline_fid;
    jfieldID subscriptionBuiltinTopicData_presentation_fid;
    jfieldID subscriptionBuiltinTopicData_partition_fid;
    jfieldID subscriptionBuiltinTopicData_topicData_fid;
    jfieldID subscriptionBuiltinTopicData_groupData_fid;

    /* caching the field id's for the PublicationBuiltinTopicData */
    jfieldID publicationBuiltinTopicData_key_fid;
    jfieldID publicationBuiltinTopicData_participantKey_fid;
    jfieldID publicationBuiltinTopicData_topicName_fid;
    jfieldID publicationBuiltinTopicData_typeName_fid;
    jfieldID publicationBuiltinTopicData_durability_fid;
    jfieldID publicationBuiltinTopicData_latencyBudget_fid;
    jfieldID publicationBuiltinTopicData_liveliness_fid;
    jfieldID publicationBuiltinTopicData_lifespan_fid;
    jfieldID publicationBuiltinTopicData_reliability_fid;
    jfieldID publicationBuiltinTopicData_ownership_fid;
    jfieldID publicationBuiltinTopicData_ownershipStrength_fid;
    jfieldID publicationBuiltinTopicData_destinationOrder_fid;
    jfieldID publicationBuiltinTopicData_userData_fid;
    jfieldID publicationBuiltinTopicData_deadline_fid;
    jfieldID publicationBuiltinTopicData_presentation_fid;
    jfieldID publicationBuiltinTopicData_partition_fid;
    jfieldID publicationBuiltinTopicData_topicData_fid;
    jfieldID publicationBuiltinTopicData_groupData_fid;

    /* caching the field id of the attribute 'value' for the Qos Holder classes */
    jfieldID domainParticipantFactoryQosHolder_value_fid;
    jfieldID conditionSeqHolder_value_fid;
    jfieldID dataReaderQosHolder_value_fid;
    jfieldID namedDataReaderQosHolder_value_fid;
    jfieldID dataReaderViewQosHolder_value_fid;
    jfieldID dataReaderSeqHolder_value_fid;
    jfieldID dataWriterQosHolder_value_fid;
    jfieldID namedDataWriterQosHolder_value_fid;
    jfieldID domainParticipantQosHolder_value_fid;
    jfieldID namedDomainParticipantQosHolder_value_fid;
    jfieldID instanceHandleSeqHolder_value_fid;
    jfieldID publicationBuiltinTopicDataHolder_value_fid;
    jfieldID publisherQosHolder_value_fid;
    jfieldID namedPublisherQosHolder_value_fid;
    jfieldID subscriberQosHolder_value_fid;
    jfieldID namedSubscriberQosHolder_value_fid;
    jfieldID subscriptionBuiltinTopicDataHolder_value_fid;
    jfieldID topicQosHolder_value_fid;
    jfieldID namedTopicQosHolder_value_fid;
    jfieldID sampleInfoHolder_value_fid;
    jfieldID sampleInfoSeqHolder_value_fid;
    jfieldID time_tHolder_value_fid;
    jfieldID stringHolder_value_fid;
    jfieldID errorCodeHolder_value_fid;

    /* caching the class ID, constructor ID and field id's for the sampleInfo class */
    jclass    sampleInfo_class;
    jmethodID sampleInfo_constructor_mid;
    jfieldID  sampleInfo_sample_state_fid;
    jfieldID  sampleInfo_view_state_fid;
    jfieldID  sampleInfo_instance_state_fid;
    jfieldID  sampleInfo_valid_data_fid;
    jfieldID  sampleInfo_source_timestamp_fid;
    jfieldID  sampleInfo_instance_handle_fid;
    jfieldID  sampleInfo_publication_handle_fid;
    jfieldID  sampleInfo_disposed_generation_count_fid;
    jfieldID  sampleInfo_no_writers_generation_count_fid;
    jfieldID  sampleInfo_sample_rank_fid;
    jfieldID  sampleInfo_generation_rank_fid;
    jfieldID  sampleInfo_absolute_generation_rank_fid;
    jfieldID  sampleInfo_reception_timestamp_fid;

    /* caching the field id's of the Duration_t class */
    jclass    duration_t_class;
    jmethodID duration_t_constructor_mid;
    jfieldID  duration_t_sec_fid;
    jfieldID  duration_t_nanosec_fid;

    /* caching the field id's of the Time_t class */
    jclass    time_t_class;
    jmethodID time_t_constructor_mid;
    jfieldID  time_t_sec_fid;
    jfieldID  time_t_nanosec_fid;

    /* cache the class reference to status objects */
    jclass    qosPolicyCount_class;
    jmethodID qosPolicyCount_constructor_mid;
    jclass    inconsistentTopicStatus_class;
    jmethodID inconsistentTopicStatus_constructor_mid;
    jclass    allDataDisposedTopicStatus_class;
    jmethodID allDataDisposedTopicStatus_constructor_mid;
    jclass    livelinessLostStatus_class;
    jmethodID livelinessLostStatus_constructor_mid;
    jclass    offeredDeadlineMissedStatus_class;
    jmethodID offeredDeadlineMissedStatus_constructor_mid;
    jclass    offeredIncompatibleQosStatus_class;
    jmethodID offeredIncompatibleQosStatus_constructor_mid;
    jclass    publicationMatchStatus_class;
    jmethodID publicationMatchStatus_constructor_mid;
    jclass    sampleRejectedStatusKind_class;
    jmethodID sampleRejectedStatusKind_fromInt_mid;
    jclass    sampleRejectedStatus_class;
    jmethodID sampleRejectedStatus_constructor_mid;
    jclass    livelinessChangedStatus_class;
    jmethodID livelinessChangedStatus_constructor_mid;
    jclass    requestedDeadlineMissedStatus_class;
    jmethodID requestedDeadlineMissedStatus_constructor_mid;
    jclass    requestedIncompatibleQosStatus_class;
    jmethodID requestedIncompatibleQosStatus_constructor_mid;
    jclass    subscriptionMatchStatus_class;
    jmethodID subscriptionMatchStatus_constructor_mid;
    jclass    sampleLostStatus_class;
    jmethodID sampleLostStatus_constructor_mid;

    /* caching the field id of the attribute 'value' for the Status Holder classes */
    jfieldID inconsistentTopicStatusHolder_value_fid;
    jfieldID allDataDisposedTopicStatusHolder_value_fid;
    jfieldID livelinessLostStatusHolder_value_fid;
    jfieldID offeredDeadlineMissedStatusHolder_value_fid;
    jfieldID offeredIncompatibleQosStatusHolder_value_fid;
    jfieldID publicationMatchedStatusHolder_value_fid;

    jfieldID livelinessChangedStatusHolder_value_fid;
    jfieldID requestedDeadlineMissedStatusHolder_value_fid;
    jfieldID requestedIncompatibleQosStatusHolder_value_fid;
    jfieldID subscriptionMatchedStatusHolder_value_fid;
    jfieldID sampleRejectedStatusHolder_value_fid;
    jfieldID sampleLostStatusHolder_value_fid;

    /*Caching Typesupport*/
    jclass    typeSupport_class;
    jfieldID  typeSupportDataReader_fid;
    jfieldID  typeSupportDataReaderView_fid;
    jfieldID  typeSupportDataWriter_fid;
    jfieldID  typeSupportConstructorSignature_fid;

    /*Listener cache*/
    /*topic listener*/
    jclass    listener_topic_class;
    jmethodID listener_onInconsistentTopic_mid;
    /*ext topic listener*/
    jclass    listener_extTopic_class;
    jmethodID listener_onAllDataDisposed_mid;
    /*datareader listener*/
    jclass    listener_datareader_class;
    jmethodID listener_onRequestedDeadlineMissed_mid;
    jmethodID listener_onRequestedIncompatibleQos_mid;
    jmethodID listener_onSampleRejected_mid;
    jmethodID listener_onLivelinessChanged_mid;
    jmethodID listener_onDataAvailable_mid;
    jmethodID listener_onSubscriptionMatch_mid;
    jmethodID listener_onSampleLost_mid;
    /*dataWriter listener*/
    jclass    listener_datawriter_class;
    jmethodID listener_onOfferedDeadlineMissed_mid;
    jmethodID listener_onOfferedIncompatibleQos_mid;
    jmethodID listener_onLivelinessLost_mid;
    jmethodID listener_onPublicationMatch_mid;
    /*subscriber listener*/
    jclass    listener_subscriber_class;
    jmethodID listener_onDataOnReaders_mid;

    /*utilities cache*/
    jclass    utilities_class;
    jmethodID utilities_throwException_mid;

    /*#define constant cache*/
    jobject PARTICIPANT_QOS_DEFAULT;
    jobject TOPIC_QOS_DEFAULT;
    jobject PUBLISHER_QOS_DEFAULT;
    jobject SUBSCRIBER_QOS_DEFAULT;
    jobject DATAWRITER_QOS_DEFAULT;
    jobject DATAREADER_QOS_DEFAULT;
    jobject DATAREADERVIEW_QOS_DEFAULT;
    jobject DATAWRITER_QOS_USE_TOPIC_QOS;
    jobject DATAREADER_QOS_USE_TOPIC_QOS;

    jfieldID stringSeqHolder_stringSeq_fid;
} jni_cache;

extern jni_cache jniCache;

void
saj_exceptionCheck (
    JNIEnv *env);

/**
 * @brief Initialize the SAJ library.
 * @param env The JNI environment.
 */
saj_returnCode
saj_InitializeSAJ(
    JNIEnv *env);

/**
 * @brief Obtains the cached emptyString object. Use this operation instead of allocating separate empty strings.
 * @return the pre-allocated empty string object.
 */
jobject saj_getEmptyStringRef(JNIEnv *);

/**
 * @brief Reads the address of a gapi object from a java object.
 * @param env JNI environment.
 * @param java_object The java object from which the address will be read.
 * @return the adress of the gapi object.
 */
OS_API PA_ADDRCAST
saj_read_gapi_address(
    JNIEnv *env,
    jobject java_object);

/**
 * @brief Writes the address of a gapi object to a java object.
 * @param env The JNI environment.
 * @param java_object The Java object to which the adress will be written.
 * @param address The adress of the gapi object.
 */
void
saj_write_gapi_address (
    JNIEnv *env,
    jobject java_object,
    PA_ADDRCAST address);

/**
 * @brief Reads the address of a java object from a gapi object.
 * @param gapi_obj The gapi object from which the address of the java object will be
 * read.
 * @return The java object that is associated with the gapi object.
 */
jobject
saj_read_java_address(
    gapi_object gapi_obj);

/**
 * @brief Reads the address of a java Listener object from a gapi object.
 * @param gapi_obj The gapi object from which the address of the java object
 * will be read.
 * @return The java listener object that is associated with the gapi entity.
 */
jobject
saj_read_java_listener_address(
    void *listenerData);

/**
 * @brief Reads the address of a java StatusCondition object from a gapi object.
 * @param gapi_obj The gapi object from which the address of the java object
 * will be read.
 * @return The java statusCondition object that is associated with the gapi entity.
 */
jobject
saj_read_java_statusCondition_address(
    gapi_object gapi_obj);

/**
 * @brief Writes the address of a java object to a gapi object.
 * This function reads the address from the java object and stores it in the
 * gapi object.
 * @param env JNI environment.
 * @param gapi_obj The gapi object where the java object will be stored in.
 * @param java_object The java object who's address will be stored in the gapi.
 */
void
saj_write_java_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    jobject java_object);

/**
 * @brief Does the same as the saj_write_java_address, but creates a
 *        weak global reference for the Java object.
 * @param env JNI environment.
 * @param gapi_obj The gapi object where the java object will be stored in.
 * @param java_object The java object who's address will be stored in the gapi.
 */
void
saj_write_weak_java_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    jobject java_object);

/**
 * @brief Writes the address of the java listener object to the user_data
 * of the gapi object. If there already is a listener, it is replaced.
 *
 * @param env The JNI environment.
 * @param gapi_obj The gapi object to store the listener in.
 * @param listenerData The listener data that contains a reference to the
 *                     java vm, the java listener and the callback routines.
 */
void
saj_write_java_listener_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    saj_listenerData listenerData);

/**
 * @brief Writes the address of the java StatusCondition object to the user_data
 * of the gapi object. If there already is a statusCondition, it is replaced.
 *
 * @param env The JNI environment.
 * @param gapi_obj The gapi object to store the listener in.
 * @param condition The gapi statuscondition, which userdata is also stored in
 *                  the userdata of gapi_obj.
 * @param java_object The Java StatusCondition object to store in the gapi object.
 */
void
saj_write_java_statusCondition_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    gapi_statusCondition condition,
    jobject java_object);

/**
 * @brief Callback routine for delete_contained_entities functions.
 *
 * This function deletes the global java references in a gapi object after it
 * has been deleted.
 *
 * @param entityUserData The user data in the gapi object.
 * @param userData The user data provided to the delete_contained_entities
 *                 function. This must be the JNI environment.
 */
void
saj_destroy_user_data_callback(
    void* entityUserData,
    void* userData);

/**
 * @brief Deletes global references in the data and completely frees the
 * memory used by the userdata.
 * @param env The JNI environment.
 * @param ud The user data to free.
 */
void
saj_destroy_user_data(
    JNIEnv *env,
    saj_userData ud);

/**
 * @brief Deletes WEAK global references in the data and completely frees the
 * memory used by the userdata.
 * @param env The JNI environment.
 * @param ud The user data to free.
 */
void
saj_destroy_weak_user_data(
    JNIEnv *env,
    saj_userData ud);

/**
 * @brief Creates a new java object, stores the address of the equivalent
 * gapi object in it and stores the java pbject as a GLOBAL reference in the
 * gapi object.
 * @param env The JNI environment.
 * @param classname The fully qualified classname in which the "." seperator
 * has been replaced with a "/".
 * @param gapi_obj_address The memory address of the gapi object.
 * @param java_object A pointer to the newly created java object. May not be NULL.
 * @return Return code.
 */
saj_returnCode
saj_construct_java_object(
    JNIEnv *env,
    const char *classname,
    PA_ADDRCAST gapi_obj_address,
    jobject *java_object);

/**
 * @brief Same as saj_construct_java_object, except it does not call the
 *        default constructor but the one with the supplied signature and
 *        with the typeSupport arg as argument.
 * @param env The JNI environment.
 * @param classname The fully qualified classname in which the "." seperator
 * has been replaced with a "/".
 * @param gapi_obj_address The memory address of the gapi object.
 * @param java_object A pointer to the newly created java object. May not be NULL.
 * @param constructorSignature The signature of the constructor to call.
 * @param typeSupport the typeSupport arg to pass on to the constructor.
 * @return Return code.
 */
saj_returnCode
saj_construct_typed_java_object(
    JNIEnv *env,
    const char *classname,
    PA_ADDRCAST gapi_obj_address,
    jobject *new_java_object,
    const char *constructorSignature,
    jobject typeSupport);

/**
 * @brief Stores the address of the gapi object in the Java object and stores
 * Jhe java object as a WEAK GLOBAL reference in the gapi object.
 *
 * @param env The JNI environment.
 * @param gapi_obj_address The memory address of the gapi object.
 * @param java_object A pointer to the newly created java object. May not be NULL.
 * @return Return code.
 */
saj_returnCode
saj_register_weak_java_object(
    JNIEnv *env,
    PA_ADDRCAST gapi_obj_address,
    jobject new_java_object);

/**
 * @brief Creates a new java object.
 * @param env The JNI environment.
 * @param classname The fully qualified classname in which the "." seperator
 * has been replaced with a "/".
 * @param java_object A pointer to the newly created java object. May not be NULL.
 * @return Return code.
 */
saj_returnCode
saj_create_new_java_object(
    JNIEnv *env,
    const char *classname,
    jobject *new_java_object);


saj_returnCode
saj_create_new_typed_java_object(
    JNIEnv *env,
    const char *classname,
    jobject *new_java_object,
    const char* signature,
    jobject typeSupport);

/**
 * @brief Copies the duration defined in the java object to a gapi
 * gapi_duration_t struct.
 * @param env The JNI environment.
 * @param in The java Duration object.
 * @param out The gapi gapi_duration_t struct.
 */
saj_returnCode
saj_durationCopyIn(
    JNIEnv *env,
    jobject in,
    gapi_duration_t *out);

/**
 * @brief Copies the duration defined in the gapi_duration_t object to a java
 * Duration_t object.
 * @param env The JNI environment.
 * @param src The gapi Duration_t object.
 * @param dst The java Duration_t object.
 * @return Return code.
 */
saj_returnCode
saj_durationCopyOut(
    JNIEnv *env,
    gapi_duration_t *src,
    jobject *dst
);

/**
 * @brief Copies the time defined in the java object to a gapi
 * gapi_time_t struct.
 * @param env The JNI environment.
 * @param in The java Time object.
 * @param out The gapi gapi_time_t struct.
 */
saj_returnCode
saj_timeCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_time_t *dst);

/**
 * @brief Copies the time defined in the gapi_time_t object to a java
 * Time_t object.
 * @param env The JNI environment.
 * @param src The gapi Time_t object.
 * @param dst The java Time_t object.
 * @return Return code.
 */
saj_returnCode
saj_timeCopyOut(
    JNIEnv *env,
    gapi_time_t *src,
    jobject *dst
);

/**
 * @brief Copies the values of a java String array to a gapi_stringSeq struct.
 * First call _DDS_sequence_malloc to allocate a new sequence.
 * If the _release flag is set to FALSE the programmer should free the allocated
 * memory by calling gapi_sequence_free().
 * The _release flag is set to FALSE by default.
 * @param env JNI environment.
 * @param stringArray Java String array.
 * @param out The gapi_stringSeq struct containing the strings.
 * @return Return code.
 */
saj_returnCode
saj_stringSequenceCopyIn(
    JNIEnv *env,
    jobjectArray stringArray,
    gapi_stringSeq *out);

/**
 * @brief Copies the values of a java byte array to a gapi_sequence_octet struct.
 * First call gapi_sequence_octet__alloc to allocate a new sequence.
 * If the _release flag is set to FALSE the programmer should free the allocated
 * memory by calling gapi_sequence_free().
 * The _release flag is set to FALSE by default.
 * @param env JNI environment.
 * @param octetArray Java byte array.
 * @param out The gapi_sequence_octet struct.
 * @return Return code.
 */
saj_returnCode
saj_octetSequenceCopyIn(
    JNIEnv *env,
    jbyteArray jArray,
    gapi_octetSeq *in);

/**
 * @brief Copies the values of a gapi_sequence_octet struct to a java byte array.
 * @param env JNI environment.
 * @param src The gapi_sequence_octet struct.
 * @param dst Java byte array.
 * @return Return code.
 */
saj_returnCode
saj_octetSequenceCopyOut(
    JNIEnv *env,
    gapi_octetSeq *src,
    jbyteArray *dst);


saj_returnCode
saj_instanceHandleSequenceCopyOut(
    JNIEnv* env,
    gapi_instanceHandleSeq *src,
    jintArray *dst);

saj_returnCode
saj_subscriptionBuiltinTopicDataCopyOut(
    JNIEnv* env,
    gapi_subscriptionBuiltinTopicData *src,
    jobject *dst);

saj_returnCode
saj_publicationBuiltinTopicDataCopyOut(
    JNIEnv* env,
    gapi_publicationBuiltinTopicData *src,
    jobject *dst);

/**
 * @brief Copies the value of a java enumerated type to an integer.
 * @parem env JNI environment.
 * @param src Java enumerated type.
 * @param dst Enumeration.
 * @return Return code.
 */
saj_returnCode
saj_EnumCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_unsigned_long *dst);

/**
 * @brief Copies the value of an integer to a java enumerated type.
 * @parem env JNI environment.
 * @param classname The name of xxxKind object that should be initialized.
 * @parem src Enumeration.
 * @param dst Java enumerated type.
 * @return Return code.
 */
saj_returnCode
saj_EnumCopyOut(
    JNIEnv              *env,
    const char          *classname,
    gapi_unsigned_long  src,
    jobject             *dst);

/**
 * @brief Copies the values of a gapi_stringSeq struct to a java String array.
 * @param env JNI environment.
 * @param src The gapi_stringSeq struct containing the strings.
 * @param dst Pointer to a java String array.
 * @return Return code.
 */
saj_returnCode
saj_stringSequenceCopyOut(
    JNIEnv *env,
    gapi_stringSeq src,
    jobjectArray *dst);

/**
 * @brief Copies the values of a gapi_sampleInfo struct to a java SampleInfo.
 * @param env JNI environment.
 * @param src The gapi_smapleInfo struct containing the sample info.
 * @param dst Pointer to a SampleInfo object.
 * @return Return code.
 */
saj_returnCode
saj_sampleInfoCopyOut(
    JNIEnv *env,
    gapi_sampleInfo *src,
    jobject *dst);

/**
 * @brief Copies the values of a gapi_sampleInfo struct to a java SampleInfoHolder.
 * @param env JNI environment.
 * @param src The gapi_smapleInfo struct containing the sample info.
 * @param dst Pointer to a SampleInfoHolder object.
 * @return Return code.
 */
saj_returnCode
saj_sampleInfoHolderCopyOut(
    JNIEnv *env,
    gapi_sampleInfo *src,
    jobject *dst);

/**
 * @brief Copies references of java objects to a typed object array.
 * It is assumed that the gapi and the java object already exist and that
 * this function is called by a function which looks up an existing sequence.
 * @param env JNI environment.
 * @param src Input parameter for the gapi sequence.
 * @param dst Output parameter for the java sequence.
 * @return Return code.
 */
saj_returnCode
saj_LookupExistingConditionSeq(
    JNIEnv *env,
    gapi_conditionSeq *src,
    jobjectArray *dst);

saj_returnCode
saj_LookupExistingDataReaderSeq(
    JNIEnv *env,
    gapi_dataReaderSeq *src,
    jobjectArray *dst);

saj_returnCode
saj_LookupTypeSupportDataReader(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result);

saj_returnCode
saj_LookupTypeSupportDataReaderView(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result);

saj_returnCode
saj_LookupTypeSupportDataWriter(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result);

saj_returnCode
saj_LookupTypeSupportConstructorSignature(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result);

saj_returnCode
saj_write_parallelDemarshallingContext_address(
    JNIEnv *env,
    jobject java_object,
    sajParDemContext address);

saj_returnCode
saj_read_parallelDemarshallingContext_address(
    JNIEnv *env,
    jobject java_object,
    sajParDemContext *address);

saj_returnCode
saj_write_CDRCopy_value(
    JNIEnv *env,
    jobject java_object,
    long value);

c_bool
saj_setThreadEnv(
    JNIEnv *env);

JNIEnv *
saj_getThreadEnv();

void
saj_delThreadEnv(
    c_bool value);

#undef OS_API

#endif /* SAJ_UTILITIES_H */
