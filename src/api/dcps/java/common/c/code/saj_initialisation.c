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

#include "saj_utilities.h"
#include "saj_qosUtils.h"

/**
 * @brief Initializes the DeadlineQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDeadlineQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the DestinationOrderQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDestinationOrderQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the DurabilityQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDurabilityQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the DurabilityServiceQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDurabilityServiceQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the EntityFactoryQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeEntityFactoryQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the ShareQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeShareQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the ReaderLifespanQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeReaderLifespanQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the SubscriptionKeyQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSubscriptionKeyQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the ViewKeyQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeViewKeyQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the GroupDataQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeGroupDataQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the HistoryQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeHistoryQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the LatencyBudgetQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeLatencyBudgetQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the LifespanQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeLifespanQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the LivelinessQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeLivelinessQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the OwnershipQosPolicy by caching the field id's of
 * its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeOwnershipQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the OwnershipStrengthQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeOwnershipStrengthQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the PartitionQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializePartitionQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the PresentationQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializePresentationQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the InvalidSampleVisibilityQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeInvalidSampleVisibilityQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the ReaderDataLifecycleQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeReaderDataLifecycleQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the ReliabilityQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeReliabilityQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the ResourceLimitsQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeResourceLimitsQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the TimeBasedFilterQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTimeBasedFilterQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the TopicDataQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTopicDataQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the TransportPriorityQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTransportPriorityQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the UserDataQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeUserDataQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the WriterDataLifecycleQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeWriterDataLifecycleQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the SchedulingClassQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSchedulingClassQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the SchedulingPriorityQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSchedulingPriorityQosPolicy(JNIEnv *env);

/**
 * @brief Initializes the SchedulingQosPolicy by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSchedulingQosPolicy(JNIEnv *env);


saj_returnCode saj_InitializeDomainParticipantFactoryQos(JNIEnv *env);

/**
 * @brief Initializes the DomainParticipantQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDomainParticipantQos(JNIEnv *env);

/**
 * @brief Initializes the DataReaderQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataReaderQos(JNIEnv *env);

/**
 * @brief Initializes the DomainReaderViewQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataReaderViewQos(JNIEnv *env);

/**
 * @brief Initializes the DataWriterQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataWriterQos(JNIEnv *env);

/**
 * @brief Initializes the TopicQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTopicQos(JNIEnv *env);

/**
 * @brief Initializes the PublisherQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializePublisherQos(JNIEnv *env);

/**
 * @brief Initializes the SubscriberQos by caching the field id's
 * of its attributes.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSubscriberQos(JNIEnv *env);

/**
 * @brief Initializes the DataReaderQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataReaderQosHolder(JNIEnv *env);

/**
 * @brief Initializes the NamedDataReaderQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeNamedDataReaderQosHolder(JNIEnv *env);

/**
 * @brief Initializes the DataReaderViewQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataReaderViewQosHolder(JNIEnv *env);

/**
 * @brief Initializes the InstanceHandleSeqHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeInstanceHandleSeqHolder(JNIEnv *env);

/**
 * @brief Initializes the PublicationBuiltinTopicDataHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializePublicationBuiltinTopicDataHolder(JNIEnv *env);

saj_returnCode saj_InitializePublicationBuiltinTopicData(JNIEnv *env);

saj_returnCode saj_InitializeStatusHolders(JNIEnv *env);


/**
 * @brief Initializes the DataWriterQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataWriterQosHolder(JNIEnv *env);

/**
 * @brief Initializes the NamedDataWriterQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeNamedDataWriterQosHolder(JNIEnv *env);

/**
 * @brief Initializes the SubscriptionBuiltinTopicDataHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSubscriptionBuiltinTopicDataHolder(JNIEnv *env);

saj_returnCode saj_InitializeSubscriptionBuiltinTopicData(JNIEnv *env);

saj_returnCode saj_InitializeDomainParticipantFactoryQosHolder(JNIEnv *env);

/**
 * @brief Initializes the DomainParticipantQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDomainParticipantQosHolder(JNIEnv *env);

/**
 * @brief Initializes the NamedDomainParticipantQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeNamedDomainParticipantQosHolder(JNIEnv *env);

/**
 * @brief Initializes the PublisherQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializePublisherQosHolder(JNIEnv *env);

/**
 * @brief Initializes the NamedPublisherQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeNamedPublisherQosHolder(JNIEnv *env);

/**
 * @brief Initializes the SubscriberQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSubscriberQosHolder(JNIEnv *env);

/**
 * @brief Initializes the NamedSubscriberQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeNamedSubscriberQosHolder(JNIEnv *env);

/**
 * @brief Initializes the TopicQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTopicQosHolder(JNIEnv *env);

/**
 * @brief Initializes the NamedTopicQosHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeNamedTopicQosHolder(JNIEnv *env);

/**
 * @brief Initializes the DataReaderSeqHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDataReaderSeqHolder(JNIEnv *env);

/**
 * @brief Initializes the ConditionSeqHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeConditionSeqHolder(JNIEnv *env);

/**
 * @brief Initializes the SampleInfo by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSampleInfo(JNIEnv *env);

/**
 * @brief Initializes the SampleInfoHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSampleInfoHolder(JNIEnv *env);

/**
 * @brief Initializes the SampleInfoSeqHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeSampleInfoSeqHolder(JNIEnv *env);

/**
 * @brief Initializes the Time_tHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTime_tHolder(JNIEnv *env);

/**
 * @brief Initializes the StringHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeStringHolder(JNIEnv *env);

/**
 * @brief Initializes the ErrorCodeHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeErrorCodeHolder(JNIEnv *env);

/**
 * @brief Initializes the Duration_t by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeDuration_t(JNIEnv *env);

/**
 * @brief Initializes the Time_t by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeTime_t(JNIEnv *env);

saj_returnCode saj_InitializeInconsistentTopicStatus(JNIEnv *env);

saj_returnCode saj_InitializeGetAllDataDisposedTopicStatus(JNIEnv *env);

saj_returnCode saj_InitializeLivelinessLostStatus(JNIEnv *env);

saj_returnCode saj_InitializeOfferedDeadlineMissedStatus(JNIEnv *env);

saj_returnCode saj_InitializeOfferedIncompatibleQosStatus(JNIEnv *env);

saj_returnCode saj_InitializePublicationMatchStatus(JNIEnv *env);

saj_returnCode saj_InitializeQosPolicyCount(JNIEnv *env);

saj_returnCode saj_InitializeSampleRejectedStatus(JNIEnv *env);

saj_returnCode saj_InitializeLivelinessChangedStatus(JNIEnv *env);

saj_returnCode saj_InitializeRequestedDeadlineMissedStatus(JNIEnv *env);

saj_returnCode saj_InitializeRequestedIncompatibleQosStatus(JNIEnv *env);

saj_returnCode saj_InitializeSubscriptionMatchStatus(JNIEnv *env);

saj_returnCode saj_InitializeSampleLostStatus(JNIEnv *env);

saj_returnCode saj_InitializeTypeSupport(JNIEnv *env);

saj_returnCode saj_InitializeDataReaderListener(JNIEnv* env);

saj_returnCode saj_InitializeTopicListener(JNIEnv* env);

saj_returnCode saj_InitializeExtTopicListener(JNIEnv* env);

saj_returnCode saj_InitializeDataWriterListener(JNIEnv* env);

saj_returnCode saj_InitializeSubscriberListener(JNIEnv* env);

saj_returnCode saj_InitializeUtilities(JNIEnv* env);

saj_returnCode saj_InitializeConstantObjects(JNIEnv* env);

saj_returnCode saj_InitializeDataReaderImpl(JNIEnv *env);

/**
 * @brief Initializes the Property by caching the
 * field id of the attributes name and value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeProperty(JNIEnv *env);

/**
 * @brief Initializes the PropertyHolder by caching the
 * field id of the attribute value.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializePropertyHolder(JNIEnv *env);

/**
 * Optimization code for the management of empty strings.
 * By globally allocating one empty string object at the start, we can avoid allocating
 * separate empty string objects for each empty string encountered during the copyOut.
 * This may save precious resources and allocation time when handling lots of empty strings.
 */

static jobject saj_emptyString = NULL;

/**
 * @brief Initializes the emptyString by caching a GlobalRef to an empty string object.
 * @param env The JNI environment.
 * @return SAJ_RETCODE_ERROR in case the VM has thrown a error.
 */
saj_returnCode saj_InitializeEmptyStringRef(JNIEnv *env)
{
    saj_returnCode result = SAJ_RETCODE_OK;
    jobject localRef = (*env)->NewStringUTF (env, "");
    saj_emptyString = (*env)->NewGlobalRef (env, localRef);
    if ((*env)->ExceptionCheck (env))
    {
        result = SAJ_RETCODE_ERROR;
    }
    return result;
}

/**
 * @brief Obtains the cached emptyString object. Use this operation instead of allocating separate empty strings.
 * @return the pre-allocated empty string object.
 */
jobject saj_getEmptyStringRef(JNIEnv *env)
{
    return (*env)->NewLocalRef(env, saj_emptyString);
}

saj_returnCode
saj_InitializeDataReaderImpl(
    JNIEnv *env)
{
    jclass cls, grCls;

    cls = (*env)->FindClass(env, "org/opensplice/dds/dcps/DataReaderImpl");
    grCls = (*env)->NewGlobalRef (env, cls);

    SET_CACHED(dataReaderImpl_class, grCls);
    if (GET_CACHED(dataReaderImpl_class) != NULL){
        SET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid,
            (*env)->GetFieldID(env, cls, "parallelDemarshallingContext", "J"));

        SET_CACHED(dataReaderImplClassStartWorkers_mid,
            (*env)->GetMethodID (env, cls, "startWorkers", "(I)I"));

        SET_CACHED(dataReaderImplClassJoinWorkers_mid,
                    (*env)->GetMethodID (env, cls, "joinWorkers", "()V"));

        SET_CACHED(dataReaderImplClassCDRCopy_fid,
            (*env)->GetFieldID(env, cls, "CDRCopy", "J"));

        SET_CACHED(dataReaderImplClassCDRDeserializeByteBuffer_mid,
            (*env)->GetMethodID (env, cls, "CDRDeserializeByteBuffer", "(Ljava/nio/ByteBuffer;)Ljava/lang/Object;"));
    } else {
        SET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid, NULL);
        SET_CACHED(dataReaderImplClassStartWorkers_mid, NULL);
        SET_CACHED(dataReaderImplClassJoinWorkers_mid, NULL);
        SET_CACHED(dataReaderImplClassCDRCopy_fid, NULL);
        SET_CACHED(dataReaderImplClassCDRDeserializeByteBuffer_mid, NULL);
    }

    (*env)->DeleteLocalRef(env, cls);

    if(GET_CACHED(dataReaderImpl_class)
       && GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid)
       && GET_CACHED(dataReaderImplClassStartWorkers_mid)
       && GET_CACHED(dataReaderImplClassJoinWorkers_mid)
       && GET_CACHED(dataReaderImplClassCDRCopy_fid)
       && GET_CACHED(dataReaderImplClassCDRDeserializeByteBuffer_mid)){
        return SAJ_RETCODE_OK;
    } else {
        return SAJ_RETCODE_ERROR;
    }
}

saj_returnCode
saj_InitializeProperty(
    JNIEnv *env)
{
    jclass propertyClass = (*env)->FindClass(env, "DDS/Property");

    if(propertyClass){
        SET_CACHED(property_name_fid ,(*env)->GetFieldID(
            env,
            propertyClass,
            "name",
            "Ljava/lang/String;"
        ));

        SET_CACHED(property_value_fid ,(*env)->GetFieldID(
            env,
            propertyClass,
            "value",
            "Ljava/lang/String;"
        ));

        (*env)->DeleteLocalRef(env, propertyClass);

        if(GET_CACHED(property_name_fid) && GET_CACHED(property_value_fid)){
            return SAJ_RETCODE_OK;
        }
    }
    return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_InitializePropertyHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PropertyHolder");
    SET_CACHED(propertyHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/Property;"
        )
    );

    if (GET_CACHED(propertyHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSAJ(JNIEnv *env)
{
    if (saj_InitializeDeadlineQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDestinationOrderQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDurabilityQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDurabilityServiceQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeEntityFactoryQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeGroupDataQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeShareQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeReaderLifespanQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSubscriptionKeyQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeViewKeyQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeHistoryQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeLatencyBudgetQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeLifespanQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeLivelinessQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeOwnershipQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeOwnershipStrengthQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePartitionQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePresentationQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeInvalidSampleVisibilityQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* vM has thrown an exception */
    }
    if (saj_InitializeReaderDataLifecycleQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeReliabilityQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeResourceLimitsQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTimeBasedFilterQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTopicDataQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTransportPriorityQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeUserDataQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeWriterDataLifecycleQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSchedulingClassQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSchedulingPriorityQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSchedulingQosPolicy(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDomainParticipantFactoryQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDomainParticipantQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataReaderQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataReaderViewQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataWriterQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTopicQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePublisherQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSubscriberQos(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataReaderQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeNamedDataReaderQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataReaderViewQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeInstanceHandleSeqHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePublicationBuiltinTopicDataHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePublicationBuiltinTopicData(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataWriterQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeNamedDataWriterQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSubscriptionBuiltinTopicDataHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSubscriptionBuiltinTopicData(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDomainParticipantFactoryQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDomainParticipantQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeNamedDomainParticipantQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePublisherQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeNamedPublisherQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSubscriberQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeNamedSubscriberQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTopicQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeNamedTopicQosHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDataReaderSeqHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeConditionSeqHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSampleInfo(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSampleInfoHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeSampleInfoSeqHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTime_tHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeStringHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeErrorCodeHolder(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeDuration_t(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeTime_t(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeInconsistentTopicStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeGetAllDataDisposedTopicStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeLivelinessLostStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializeOfferedDeadlineMissedStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeOfferedIncompatibleQosStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializePublicationMatchStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeQosPolicyCount(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeSampleRejectedStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeLivelinessChangedStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeRequestedDeadlineMissedStatus(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeRequestedIncompatibleQosStatus(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeSubscriptionMatchStatus(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeSampleLostStatus(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeTypeSupport(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if(saj_InitializeTopicListener(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if(saj_InitializeExtTopicListener(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if(saj_InitializeDataReaderListener(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if(saj_InitializeDataWriterListener(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if(saj_InitializeSubscriberListener(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if (saj_InitializeStatusHolders(env) != SAJ_RETCODE_OK)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if(saj_InitializeUtilities(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if(saj_InitializeConstantObjects(env) != SAJ_RETCODE_OK){
        return SAJ_RETCODE_ERROR;
    }
    if (saj_InitializeEmptyStringRef(env)  != SAJ_RETCODE_OK) {
        return SAJ_RETCODE_ERROR;
    }
    if (saj_InitializeDataReaderImpl(env) != SAJ_RETCODE_OK) {
        return SAJ_RETCODE_ERROR;
    }
    if (saj_InitializeProperty(env) != SAJ_RETCODE_OK) {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    if (saj_InitializePropertyHolder(env) != SAJ_RETCODE_OK) {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDeadlineQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DeadlineQosPolicy");
    SET_CACHED(deadlineQosPolicy_period_fid ,(*env)->GetFieldID(
        env,
        tempClass,
        "period",
        "LDDS/Duration_t;"
    ));

    if (GET_CACHED(deadlineQosPolicy_period_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDestinationOrderQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DestinationOrderQosPolicy");
    SET_CACHED(destinationOrderQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/DestinationOrderQosPolicyKind;"
        )
    );

    if (GET_CACHED(destinationOrderQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDurabilityQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DurabilityQosPolicy");
    SET_CACHED(durabilityQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/DurabilityQosPolicyKind;"
        )
    );

    if (GET_CACHED(durabilityQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDurabilityServiceQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DurabilityServiceQosPolicy");

    SET_CACHED(durabilityServiceQosPolicy_historyKind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "history_kind",
            "LDDS/HistoryQosPolicyKind;"
        )
    );

    SET_CACHED(durabilityServiceQosPolicy_historyDepth_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "history_depth",
            "I"
        )
    );

    SET_CACHED(durabilityServiceQosPolicy_maxSamples_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_samples",
            "I"
        )
    );

    SET_CACHED(durabilityServiceQosPolicy_maxInstances_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_instances",
            "I"
        )
    );

    SET_CACHED(durabilityServiceQosPolicy_maxSamplesPerInstance_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_samples_per_instance",
            "I"
        )
    );

    SET_CACHED(durabilityServiceQosPolicy_serviceCleanupDelay_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "service_cleanup_delay",
            "LDDS/Duration_t;"
        )
    );

    if (GET_CACHED(durabilityServiceQosPolicy_historyKind_fid) == NULL  ||
        GET_CACHED(durabilityServiceQosPolicy_historyDepth_fid) == NULL ||
        GET_CACHED(durabilityServiceQosPolicy_maxSamples_fid) == NULL ||
        GET_CACHED(durabilityServiceQosPolicy_maxInstances_fid) == NULL ||
        GET_CACHED(durabilityServiceQosPolicy_maxSamplesPerInstance_fid) == NULL ||
        GET_CACHED(durabilityServiceQosPolicy_serviceCleanupDelay_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeEntityFactoryQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/EntityFactoryQosPolicy");
    SET_CACHED(entityFactoryQosPolicy_autoenableCreatedEntities_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "autoenable_created_entities",
            "Z" /* = boolean */
        )
    );

    if (GET_CACHED(entityFactoryQosPolicy_autoenableCreatedEntities_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeGroupDataQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/GroupDataQosPolicy");
    SET_CACHED(groupDataQosPolicy_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "[B" /* = byte array = char sequence */
        )
    );

    if (GET_CACHED(groupDataQosPolicy_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeShareQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ShareQosPolicy");
    SET_CACHED(shareQosPolicy_enable_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "enable",
            "Z" /* = boolean */
        )
    );

    if (GET_CACHED(shareQosPolicy_enable_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(shareQosPolicy_name_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "name",
            "Ljava/lang/String;" /* = string object */
        )
    );

    if (GET_CACHED(shareQosPolicy_name_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeReaderLifespanQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ReaderLifespanQosPolicy");
    SET_CACHED(readerLifespanQosPolicy_useLifespan_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "use_lifespan",
            "Z" /* = boolean */
        )
    );

    if (GET_CACHED(readerLifespanQosPolicy_useLifespan_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(readerLifespanQosPolicy_duration_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "duration",
            "LDDS/Duration_t;" /* = duration object */
        )
    );

    if (GET_CACHED(readerLifespanQosPolicy_duration_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeInvalidSampleVisibilityQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/InvalidSampleVisibilityQosPolicy");
    SET_CACHED(invalidSampleVisibilityQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/InvalidSampleVisibilityQosPolicyKind;"
        )
    );

    if (GET_CACHED(invalidSampleVisibilityQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSubscriptionKeyQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SubscriptionKeyQosPolicy");
    SET_CACHED(subscriptionKeyQosPolicy_useKeyList_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "use_key_list",
            "Z" /* = boolean */
        )
    );

    if (GET_CACHED(subscriptionKeyQosPolicy_useKeyList_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(subscriptionKeyQosPolicy_keyList_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "key_list",
            "[Ljava/lang/String;" /* = string array */
        )
    );

    if (GET_CACHED(subscriptionKeyQosPolicy_keyList_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeViewKeyQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ViewKeyQosPolicy");
    SET_CACHED(viewKeyQosPolicy_useKeyList_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "use_key_list",
            "Z" /* = boolean */
        )
    );

    if (GET_CACHED(viewKeyQosPolicy_useKeyList_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(viewKeyQosPolicy_keyList_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "key_list",
            "[Ljava/lang/String;" /* = string array */
        )
    );

    if (GET_CACHED(viewKeyQosPolicy_keyList_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeHistoryQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/HistoryQosPolicy");
    SET_CACHED(historyQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/HistoryQosPolicyKind;"
        )
    );

    SET_CACHED(historyQosPolicy_depth_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "depth",
            "I" /* = int */
        )
    );

    if (GET_CACHED(historyQosPolicy_kind_fid) == NULL ||
        GET_CACHED(historyQosPolicy_depth_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeLatencyBudgetQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/LatencyBudgetQosPolicy");
    SET_CACHED(latencyBudgetQosPolicy_duration_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "duration",
            "LDDS/Duration_t;"
        )
    );

    if (GET_CACHED(latencyBudgetQosPolicy_duration_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeLifespanQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/LifespanQosPolicy");
    SET_CACHED(lifespanQosPolicy_duration_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "duration",
            "LDDS/Duration_t;"
        )
    );

    if (GET_CACHED(lifespanQosPolicy_duration_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeLivelinessQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/LivelinessQosPolicy");
    SET_CACHED(livelinessQosPolicy_leaseDuration_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "lease_duration",
            "LDDS/Duration_t;"
        )
    );

    SET_CACHED(livelinessQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/LivelinessQosPolicyKind;"
        )
    );

    if (GET_CACHED(livelinessQosPolicy_leaseDuration_fid) == NULL ||
        GET_CACHED(livelinessQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeOwnershipQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/OwnershipQosPolicy");
    SET_CACHED(ownershipQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/OwnershipQosPolicyKind;"
        )
    );

    if (GET_CACHED(ownershipQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeOwnershipStrengthQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/OwnershipStrengthQosPolicy");
    SET_CACHED(ownershipStrengthQosPolicy_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "I"
        )
    );

    if (GET_CACHED(ownershipStrengthQosPolicy_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializePartitionQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PartitionQosPolicy");
    SET_CACHED(partitionQosPolicy_name_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "name",
            "[Ljava/lang/String;" /* string array */
        )
    );

    if (GET_CACHED(partitionQosPolicy_name_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializePresentationQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PresentationQosPolicy");
    SET_CACHED(presentationQosPolicy_accessScope_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "access_scope",
            "LDDS/PresentationQosPolicyAccessScopeKind;"
        )
    );

    SET_CACHED(presentationQosPolicy_coherentAccess_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "coherent_access",
            "Z" /* boolean */
        )
    );

    SET_CACHED(presentationQosPolicy_orderedAccess_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ordered_access",
            "Z" /* boolean */
        )
    );

    if (GET_CACHED(presentationQosPolicy_accessScope_fid) == NULL ||
        GET_CACHED(presentationQosPolicy_coherentAccess_fid) == NULL ||
        GET_CACHED(presentationQosPolicy_orderedAccess_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeReaderDataLifecycleQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ReaderDataLifecycleQosPolicy");
    SET_CACHED(readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "autopurge_nowriter_samples_delay",
            "LDDS/Duration_t;"
        )
    );

    if (GET_CACHED(
        readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "autopurge_disposed_samples_delay",
            "LDDS/Duration_t;"
        )
    );

    if (GET_CACHED(
        readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(readerDataLifecycleQosPolicy_enable_invalid_samples_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "enable_invalid_samples",
            "Z" /* boolean */
        )
    );

    if (GET_CACHED(
        readerDataLifecycleQosPolicy_enable_invalid_samples_fid) ==
            NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(readerDataLifecycleQosPolicy_invalid_sample_visibility_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "invalid_sample_visibility",
            "LDDS/InvalidSampleVisibilityQosPolicy;"
        )
    );

    if (GET_CACHED(
        readerDataLifecycleQosPolicy_invalid_sample_visibility_fid) ==
            NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeReliabilityQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ReliabilityQosPolicy");
    SET_CACHED(reliabilityQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/ReliabilityQosPolicyKind;"
        )
    );

    SET_CACHED(reliabilityQosPolicy_maxBlockingTime_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_blocking_time",
            "LDDS/Duration_t;"
        )
    );

    SET_CACHED(reliabilityQosPolicy_synchronous_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "synchronous",
            "Z" /* boolean */
        )
    );

    if (GET_CACHED(reliabilityQosPolicy_kind_fid) == NULL ||
        GET_CACHED(reliabilityQosPolicy_maxBlockingTime_fid) == NULL ||
        GET_CACHED(reliabilityQosPolicy_synchronous_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeResourceLimitsQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ResourceLimitsQosPolicy");
    SET_CACHED(resourceLimitsQosPolicy_maxSamples_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_samples",
            "I" /* int */
        )
    );

    SET_CACHED(resourceLimitsQosPolicy_maxInstances_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_instances",
            "I" /* int */
        )
    );

    SET_CACHED(resourceLimitsQosPolicy_maxSamplesPerInstance_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "max_samples_per_instance",
            "I" /* int */
        )
    );

    if (GET_CACHED(resourceLimitsQosPolicy_maxSamples_fid) == NULL ||
        GET_CACHED(resourceLimitsQosPolicy_maxInstances_fid) == NULL ||
        GET_CACHED(resourceLimitsQosPolicy_maxSamplesPerInstance_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeTimeBasedFilterQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/TimeBasedFilterQosPolicy");
    SET_CACHED(timeBasedFilterQosPolicy_minimumSeparation_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "minimum_separation",
            "LDDS/Duration_t;"
        )
    );

    if (GET_CACHED(timeBasedFilterQosPolicy_minimumSeparation_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeTopicDataQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/TopicDataQosPolicy");
    SET_CACHED(topicDataQosPolicy_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "[B" /* byte array */
        )
    );

    if (GET_CACHED(topicDataQosPolicy_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeTransportPriorityQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/TransportPriorityQosPolicy");
    SET_CACHED(transportPriorityQosPolicy_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "I" /* int */
        )
    );

    if (GET_CACHED(transportPriorityQosPolicy_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeUserDataQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/UserDataQosPolicy");
    SET_CACHED(userDataQosPolicy_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "[B" /* byte array */
        )
    );

    if (GET_CACHED(userDataQosPolicy_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeWriterDataLifecycleQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/WriterDataLifecycleQosPolicy");
    SET_CACHED(writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "autodispose_unregistered_instances",
            "Z" /* boolean */
        )
    );

    if (GET_CACHED(
        writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances_fid) ==
            NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "autopurge_suspended_samples_delay",
            "LDDS/Duration_t;" /* duration */
        )
    );

    if (GET_CACHED(
            writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay_fid) ==
            NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    SET_CACHED(writerDataLifecycleQosPolicy_autounregisterInstanceDelay_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "autounregister_instance_delay",
            "LDDS/Duration_t;" /* duration */
        )
    );

    if (GET_CACHED(
            writerDataLifecycleQosPolicy_autounregisterInstanceDelay_fid) ==
            NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSchedulingClassQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SchedulingClassQosPolicy");
    SET_CACHED(schedulingClassQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/SchedulingClassQosPolicyKind;"
        )
    );

    if (GET_CACHED(schedulingClassQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSchedulingPriorityQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SchedulingPriorityQosPolicy");
    SET_CACHED(schedulingPriorityQosPolicy_kind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "kind",
            "LDDS/SchedulingPriorityQosPolicyKind;"
        )
    );

    if (GET_CACHED(schedulingPriorityQosPolicy_kind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSchedulingQosPolicy(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SchedulingQosPolicy");
    SET_CACHED(schedulingQosPolicy_schedulingClass_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "scheduling_class",
            "LDDS/SchedulingClassQosPolicy;"
        )
    );

    SET_CACHED(schedulingQosPolicy_schedulingPriorityKind_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "scheduling_priority_kind",
            "LDDS/SchedulingPriorityQosPolicy;"
        )
    );

    SET_CACHED(schedulingQosPolicy_schedulingPriority_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "scheduling_priority",
            "I" /* int */
        )
    );

    if (GET_CACHED(schedulingQosPolicy_schedulingClass_fid) == NULL ||
        GET_CACHED(schedulingQosPolicy_schedulingPriorityKind_fid) == NULL ||
        GET_CACHED(schedulingQosPolicy_schedulingPriorityKind_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDomainParticipantFactoryQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DomainParticipantFactoryQos");

    SET_CACHED(domainParticipantFactoryQos_entityFactory_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "entity_factory",
            "LDDS/EntityFactoryQosPolicy;"
        )
    );
    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}


saj_returnCode saj_InitializeDomainParticipantQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DomainParticipantQos");
    SET_CACHED(domainParticipantQos_userData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "user_data",
            "LDDS/UserDataQosPolicy;"
        )
    );

    SET_CACHED(domainParticipantQos_entityFactory_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "entity_factory",
            "LDDS/EntityFactoryQosPolicy;"
        )
    );

    SET_CACHED(domainParticipantQos_watchdogScheduling_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "watchdog_scheduling",
            "LDDS/SchedulingQosPolicy;"
        )
    );

    SET_CACHED(domainParticipantQos_listenerScheduling_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "listener_scheduling",
            "LDDS/SchedulingQosPolicy;"
        )
    );

    if (GET_CACHED(domainParticipantQos_userData_fid) == NULL ||
        GET_CACHED(domainParticipantQos_entityFactory_fid) == NULL ||
        GET_CACHED(domainParticipantQos_watchdogScheduling_fid) == NULL ||
        GET_CACHED(domainParticipantQos_listenerScheduling_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDataReaderQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataReaderQos");
    SET_CACHED(dataReaderQos_durability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "durability",
            "LDDS/DurabilityQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_deadline_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "deadline",
            "LDDS/DeadlineQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_latencyBudget_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "latency_budget",
            "LDDS/LatencyBudgetQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_liveliness_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "liveliness",
            "LDDS/LivelinessQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_reliability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reliability",
            "LDDS/ReliabilityQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_destinationOrder_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "destination_order",
            "LDDS/DestinationOrderQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_history_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "history",
            "LDDS/HistoryQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_resourceLimits_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "resource_limits",
            "LDDS/ResourceLimitsQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_userData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "user_data",
            "LDDS/UserDataQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_ownership_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ownership",
            "LDDS/OwnershipQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_timeBasedFilter_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "time_based_filter",
            "LDDS/TimeBasedFilterQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_readerDataLifecycle_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reader_data_lifecycle",
            "LDDS/ReaderDataLifecycleQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_share_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "share",
            "LDDS/ShareQosPolicy;"
        )
    );

    SET_CACHED(dataReaderQos_readerLifespan_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reader_lifespan",
            "LDDS/ReaderLifespanQosPolicy;"
        )
    );
    SET_CACHED(dataReaderQos_subscriptionKeys_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "subscription_keys",
            "LDDS/SubscriptionKeyQosPolicy;"
        )
    );

    if (GET_CACHED(dataReaderQos_durability_fid) == NULL        ||
        GET_CACHED(dataReaderQos_latencyBudget_fid) == NULL     ||
        GET_CACHED(dataReaderQos_liveliness_fid) == NULL        ||
        GET_CACHED(dataReaderQos_reliability_fid) == NULL       ||
        GET_CACHED(dataReaderQos_destinationOrder_fid) == NULL  ||
        GET_CACHED(dataReaderQos_history_fid) == NULL           ||
        GET_CACHED(dataReaderQos_resourceLimits_fid) == NULL    ||
        GET_CACHED(dataReaderQos_userData_fid) == NULL          ||
        GET_CACHED(dataReaderQos_ownership_fid) == NULL         ||
        GET_CACHED(dataReaderQos_timeBasedFilter_fid) == NULL   ||
        GET_CACHED(dataReaderQos_readerDataLifecycle_fid) == NULL ||
        GET_CACHED(dataReaderQos_share_fid) == NULL             ||
        GET_CACHED(dataReaderQos_readerLifespan_fid) == NULL    ||
        GET_CACHED(dataReaderQos_subscriptionKeys_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDataReaderViewQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataReaderViewQos");

    SET_CACHED(dataReaderViewQos_viewKeys_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "view_keys",
            "LDDS/ViewKeyQosPolicy;"
        )
    );

    if (GET_CACHED(dataReaderViewQos_viewKeys_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDataWriterQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataWriterQos");
    SET_CACHED(dataWriterQos_durability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "durability",
            "LDDS/DurabilityQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_deadline_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "deadline",
            "LDDS/DeadlineQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_latencyBudget_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "latency_budget",
            "LDDS/LatencyBudgetQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_liveliness_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "liveliness",
            "LDDS/LivelinessQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_reliability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reliability",
            "LDDS/ReliabilityQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_destinationOrder_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "destination_order",
            "LDDS/DestinationOrderQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_history_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "history",
            "LDDS/HistoryQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_resourceLimits_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "resource_limits",
            "LDDS/ResourceLimitsQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_transportPriority_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "transport_priority",
            "LDDS/TransportPriorityQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_lifespan_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "lifespan",
            "LDDS/LifespanQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_userData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "user_data",
            "LDDS/UserDataQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_ownership_fid,
            (*env)->GetFieldID(
                env,
                tempClass,
                "ownership",
                "LDDS/OwnershipQosPolicy;"
            )
        );

    SET_CACHED(dataWriterQos_ownershipStrength_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ownership_strength",
            "LDDS/OwnershipStrengthQosPolicy;"
        )
    );

    SET_CACHED(dataWriterQos_writerDataLifecycle_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "writer_data_lifecycle",
            "LDDS/WriterDataLifecycleQosPolicy;"
        )
    );

    if (GET_CACHED(dataWriterQos_durability_fid) == NULL        ||
        GET_CACHED(dataWriterQos_deadline_fid) == NULL          ||
        GET_CACHED(dataWriterQos_latencyBudget_fid) == NULL     ||
        GET_CACHED(dataWriterQos_liveliness_fid) == NULL        ||
        GET_CACHED(dataWriterQos_reliability_fid) == NULL       ||
        GET_CACHED(dataWriterQos_destinationOrder_fid) == NULL  ||
        GET_CACHED(dataWriterQos_history_fid) == NULL           ||
        GET_CACHED(dataWriterQos_resourceLimits_fid) == NULL    ||
        GET_CACHED(dataWriterQos_transportPriority_fid) == NULL ||
        GET_CACHED(dataWriterQos_lifespan_fid) == NULL          ||
        GET_CACHED(dataWriterQos_userData_fid) == NULL          ||
        GET_CACHED(dataWriterQos_ownership_fid) == NULL         ||
        GET_CACHED(dataWriterQos_ownershipStrength_fid) == NULL ||
        GET_CACHED(dataWriterQos_writerDataLifecycle_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeTopicQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/TopicQos");
    SET_CACHED(topicQos_topicData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "topic_data",
            "LDDS/TopicDataQosPolicy;"
        )
    );

    SET_CACHED(topicQos_durability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "durability",
            "LDDS/DurabilityQosPolicy;"
        )
    );

    SET_CACHED(topicQos_durabilityService_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "durability_service",
            "LDDS/DurabilityServiceQosPolicy;"
        )
    );

    SET_CACHED(topicQos_deadline_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "deadline",
            "LDDS/DeadlineQosPolicy;"
        )
    );

    SET_CACHED(topicQos_latencyBudget_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "latency_budget",
            "LDDS/LatencyBudgetQosPolicy;"
        )
    );

    SET_CACHED(topicQos_liveliness_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "liveliness",
            "LDDS/LivelinessQosPolicy;"
        )
    );

    SET_CACHED(topicQos_reliability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reliability",
            "LDDS/ReliabilityQosPolicy;"
        )
    );

    SET_CACHED(topicQos_destinationOrder_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "destination_order",
            "LDDS/DestinationOrderQosPolicy;"
        )
    );

    SET_CACHED(topicQos_history_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "history",
            "LDDS/HistoryQosPolicy;"
        )
    );

    SET_CACHED(topicQos_resourceLimits_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "resource_limits",
            "LDDS/ResourceLimitsQosPolicy;"
        )
    );

    SET_CACHED(topicQos_transportPriority_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "transport_priority",
            "LDDS/TransportPriorityQosPolicy;"
        )
    );

    SET_CACHED(topicQos_lifespan_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "lifespan",
            "LDDS/LifespanQosPolicy;"
        )
    );

    SET_CACHED(topicQos_ownership_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ownership",
            "LDDS/OwnershipQosPolicy;"
        )
    );

    if (GET_CACHED(topicQos_topicData_fid) == NULL          ||
        GET_CACHED(topicQos_durability_fid) == NULL         ||
        GET_CACHED(topicQos_durabilityService_fid) == NULL  ||
        GET_CACHED(topicQos_deadline_fid) == NULL           ||
        GET_CACHED(topicQos_latencyBudget_fid) == NULL      ||
        GET_CACHED(topicQos_liveliness_fid) == NULL         ||
        GET_CACHED(topicQos_reliability_fid) == NULL        ||
        GET_CACHED(topicQos_destinationOrder_fid) == NULL   ||
        GET_CACHED(topicQos_history_fid) == NULL            ||
        GET_CACHED(topicQos_resourceLimits_fid) == NULL     ||
        GET_CACHED(topicQos_transportPriority_fid) == NULL  ||
        GET_CACHED(topicQos_lifespan_fid) == NULL           ||
        GET_CACHED(topicQos_ownership_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializePublisherQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PublisherQos");
    SET_CACHED(publisherQos_presentation_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "presentation",
            "LDDS/PresentationQosPolicy;"
        )
    );

    SET_CACHED(publisherQos_partition_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "partition",
            "LDDS/PartitionQosPolicy;"
        )
    );

    SET_CACHED(publisherQos_groupData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "group_data",
            "LDDS/GroupDataQosPolicy;"
        )
    );

    SET_CACHED(publisherQos_entityFactory_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "entity_factory",
            "LDDS/EntityFactoryQosPolicy;"
        )
    );

    if (GET_CACHED(publisherQos_presentation_fid) == NULL   ||
        GET_CACHED(publisherQos_partition_fid) == NULL      ||
        GET_CACHED(publisherQos_groupData_fid) == NULL      ||
        GET_CACHED(publisherQos_entityFactory_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSubscriberQos(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SubscriberQos");
    SET_CACHED(subscriberQos_presentation_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "presentation",
            "LDDS/PresentationQosPolicy;"
        )
    );

    SET_CACHED(subscriberQos_partition_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "partition",
            "LDDS/PartitionQosPolicy;"
        )
    );

    SET_CACHED(subscriberQos_groupData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "group_data",
            "LDDS/GroupDataQosPolicy;"
        )
    );

    SET_CACHED(subscriberQos_entityFactory_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "entity_factory",
            "LDDS/EntityFactoryQosPolicy;"
        )
    );

    SET_CACHED(subscriberQos_share_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "share",
            "LDDS/ShareQosPolicy;"
        )
    );

    if (GET_CACHED(subscriberQos_presentation_fid) == NULL  ||
        GET_CACHED(subscriberQos_partition_fid) == NULL     ||
        GET_CACHED(subscriberQos_groupData_fid) == NULL     ||
        GET_CACHED(subscriberQos_entityFactory_fid) == NULL ||
        GET_CACHED(subscriberQos_share_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDataReaderQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataReaderQosHolder");
    SET_CACHED(dataReaderQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/DataReaderQos;"
        )
    );

    if (GET_CACHED(dataReaderQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeNamedDataReaderQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/NamedDataReaderQosHolder");
    SET_CACHED(namedDataReaderQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/NamedDataReaderQos;"
        )
    );

    if (GET_CACHED(namedDataReaderQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDataReaderViewQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataReaderViewQosHolder");
    SET_CACHED(dataReaderViewQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/DataReaderViewQos;"
        )
    );

    if (GET_CACHED(dataReaderViewQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeInstanceHandleSeqHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/InstanceHandleSeqHolder");
    SET_CACHED(instanceHandleSeqHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "[J"
        )
    );

    if (GET_CACHED(instanceHandleSeqHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializePublicationBuiltinTopicDataHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PublicationBuiltinTopicDataHolder");
    SET_CACHED(publicationBuiltinTopicDataHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/PublicationBuiltinTopicData;"
        )
    );

    if (GET_CACHED(publicationBuiltinTopicDataHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializePublicationBuiltinTopicData(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PublicationBuiltinTopicData");
    SET_CACHED(publicationBuiltinTopicData_key_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "key",
            "[I"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_participantKey_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "participant_key",
            "[I"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_topicName_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "topic_name",
            "Ljava/lang/String;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_typeName_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "type_name",
            "Ljava/lang/String;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_durability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "durability",
            "LDDS/DurabilityQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_latencyBudget_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "latency_budget",
            "LDDS/LatencyBudgetQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_liveliness_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "liveliness",
            "LDDS/LivelinessQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_reliability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reliability",
            "LDDS/ReliabilityQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_ownership_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ownership",
            "LDDS/OwnershipQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_ownershipStrength_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ownership_strength",
            "LDDS/OwnershipStrengthQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_lifespan_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "lifespan",
            "LDDS/LifespanQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_destinationOrder_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "destination_order",
            "LDDS/DestinationOrderQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_userData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "user_data",
            "LDDS/UserDataQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_deadline_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "deadline",
            "LDDS/DeadlineQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_presentation_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "presentation",
            "LDDS/PresentationQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_partition_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "partition",
            "LDDS/PartitionQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_topicData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "topic_data",
            "LDDS/TopicDataQosPolicy;"
        )
    );
    SET_CACHED(publicationBuiltinTopicData_groupData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "group_data",
            "LDDS/GroupDataQosPolicy;"
        )
    );

    if (GET_CACHED(publicationBuiltinTopicData_key_fid) == NULL               ||
        GET_CACHED(publicationBuiltinTopicData_participantKey_fid) == NULL    ||
        GET_CACHED(publicationBuiltinTopicData_topicName_fid) == NULL         ||
        GET_CACHED(publicationBuiltinTopicData_typeName_fid) == NULL          ||
        GET_CACHED(publicationBuiltinTopicData_durability_fid) == NULL        ||
        GET_CACHED(publicationBuiltinTopicData_latencyBudget_fid) == NULL     ||
        GET_CACHED(publicationBuiltinTopicData_liveliness_fid) == NULL        ||
        GET_CACHED(publicationBuiltinTopicData_reliability_fid) == NULL       ||
        GET_CACHED(publicationBuiltinTopicData_ownership_fid) == NULL         ||
        GET_CACHED(publicationBuiltinTopicData_ownershipStrength_fid) == NULL ||
        GET_CACHED(publicationBuiltinTopicData_lifespan_fid) == NULL          ||
        GET_CACHED(publicationBuiltinTopicData_destinationOrder_fid) == NULL  ||
        GET_CACHED(publicationBuiltinTopicData_userData_fid) == NULL          ||
        GET_CACHED(publicationBuiltinTopicData_deadline_fid) == NULL          ||
        GET_CACHED(publicationBuiltinTopicData_presentation_fid) == NULL      ||
        GET_CACHED(publicationBuiltinTopicData_partition_fid) == NULL         ||
        GET_CACHED(publicationBuiltinTopicData_topicData_fid) == NULL         ||
        GET_CACHED(publicationBuiltinTopicData_groupData_fid) == NULL )
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;

}

saj_returnCode saj_InitializeDataWriterQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataWriterQosHolder");
    SET_CACHED(dataWriterQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/DataWriterQos;"
        )
    );

    if (GET_CACHED(dataWriterQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeNamedDataWriterQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/NamedDataWriterQosHolder");
    SET_CACHED(namedDataWriterQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/NamedDataWriterQos;"
        )
    );

    if (GET_CACHED(namedDataWriterQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSubscriptionBuiltinTopicDataHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SubscriptionBuiltinTopicDataHolder");
    SET_CACHED(subscriptionBuiltinTopicDataHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/SubscriptionBuiltinTopicData;"
        )
    );

    if (GET_CACHED(subscriptionBuiltinTopicDataHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSubscriptionBuiltinTopicData(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SubscriptionBuiltinTopicData");
    SET_CACHED(subscriptionBuiltinTopicData_key_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "key",
            "[I"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_participantKey_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "participant_key",
            "[I"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_topicName_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "topic_name",
            "Ljava/lang/String;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_typeName_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "type_name",
            "Ljava/lang/String;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_durability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "durability",
            "LDDS/DurabilityQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_latencyBudget_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "latency_budget",
            "LDDS/LatencyBudgetQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_liveliness_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "liveliness",
            "LDDS/LivelinessQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_reliability_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "reliability",
            "LDDS/ReliabilityQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_ownership_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "ownership",
            "LDDS/OwnershipQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_destinationOrder_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "destination_order",
            "LDDS/DestinationOrderQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_userData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "user_data",
            "LDDS/UserDataQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_timeBasedFilter_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "time_based_filter",
            "LDDS/TimeBasedFilterQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_deadline_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "deadline",
            "LDDS/DeadlineQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_presentation_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "presentation",
            "LDDS/PresentationQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_partition_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "partition",
            "LDDS/PartitionQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_topicData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "topic_data",
            "LDDS/TopicDataQosPolicy;"
        )
    );
    SET_CACHED(subscriptionBuiltinTopicData_groupData_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "group_data",
            "LDDS/GroupDataQosPolicy;"
        )
    );

    if (GET_CACHED(subscriptionBuiltinTopicData_key_fid) == NULL              ||
        GET_CACHED(subscriptionBuiltinTopicData_participantKey_fid) == NULL   ||
        GET_CACHED(subscriptionBuiltinTopicData_topicName_fid) == NULL        ||
        GET_CACHED(subscriptionBuiltinTopicData_typeName_fid) == NULL         ||
        GET_CACHED(subscriptionBuiltinTopicData_durability_fid) == NULL       ||
        GET_CACHED(subscriptionBuiltinTopicData_latencyBudget_fid) == NULL    ||
        GET_CACHED(subscriptionBuiltinTopicData_liveliness_fid) == NULL       ||
        GET_CACHED(subscriptionBuiltinTopicData_reliability_fid) == NULL      ||
        GET_CACHED(subscriptionBuiltinTopicData_ownership_fid) == NULL        ||
        GET_CACHED(subscriptionBuiltinTopicData_destinationOrder_fid) == NULL ||
        GET_CACHED(subscriptionBuiltinTopicData_userData_fid) == NULL         ||
        GET_CACHED(subscriptionBuiltinTopicData_timeBasedFilter_fid) == NULL  ||
        GET_CACHED(subscriptionBuiltinTopicData_deadline_fid) == NULL         ||
        GET_CACHED(subscriptionBuiltinTopicData_presentation_fid) == NULL     ||
        GET_CACHED(subscriptionBuiltinTopicData_partition_fid) == NULL        ||
        GET_CACHED(subscriptionBuiltinTopicData_topicData_fid) == NULL        ||
        GET_CACHED(subscriptionBuiltinTopicData_groupData_fid) == NULL )
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;

}

saj_returnCode saj_InitializeDomainParticipantFactoryQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DomainParticipantFactoryQosHolder");
    SET_CACHED(domainParticipantFactoryQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/DomainParticipantFactoryQos;"
        )
    );

    if (GET_CACHED(domainParticipantFactoryQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDomainParticipantQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DomainParticipantQosHolder");
    SET_CACHED(domainParticipantQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/DomainParticipantQos;"
        )
    );

    if (GET_CACHED(domainParticipantQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeNamedDomainParticipantQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/NamedDomainParticipantQosHolder");
    SET_CACHED(namedDomainParticipantQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/NamedDomainParticipantQos;"
        )
    );

    if (GET_CACHED(namedDomainParticipantQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializePublisherQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/PublisherQosHolder");
    SET_CACHED(publisherQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/PublisherQos;"
        )
    );

    if (GET_CACHED(publisherQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeNamedPublisherQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/NamedPublisherQosHolder");
    SET_CACHED(namedPublisherQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/NamedPublisherQos;"
        )
    );

    if (GET_CACHED(namedPublisherQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeStatusHolders(JNIEnv *env)
{
    jclass tempClass;

    tempClass = (*env)->FindClass(env, "DDS/InconsistentTopicStatusHolder");
    SET_CACHED(inconsistentTopicStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/InconsistentTopicStatus;"
        )
    );

    if (GET_CACHED(inconsistentTopicStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/AllDataDisposedTopicStatusHolder");
    SET_CACHED(allDataDisposedTopicStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/AllDataDisposedTopicStatus;"
        )
    );

    if (GET_CACHED(allDataDisposedTopicStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/LivelinessLostStatusHolder");
    SET_CACHED(livelinessLostStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/LivelinessLostStatus;"
        )
    );

    if (GET_CACHED(livelinessLostStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/OfferedDeadlineMissedStatusHolder");
    SET_CACHED(offeredDeadlineMissedStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/OfferedDeadlineMissedStatus;"
        )
    );

    if (GET_CACHED(offeredDeadlineMissedStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/OfferedIncompatibleQosStatusHolder");
    SET_CACHED(offeredIncompatibleQosStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/OfferedIncompatibleQosStatus;"
        )
    );

    if (GET_CACHED(offeredIncompatibleQosStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/PublicationMatchedStatusHolder");
    SET_CACHED(publicationMatchedStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/PublicationMatchedStatus;"
        )
    );

    if (GET_CACHED(publicationMatchedStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/LivelinessChangedStatusHolder");
    SET_CACHED(livelinessChangedStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/LivelinessChangedStatus;"
        )
    );

    if (GET_CACHED(livelinessChangedStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/RequestedDeadlineMissedStatusHolder");
    SET_CACHED(requestedDeadlineMissedStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/RequestedDeadlineMissedStatus;"
        )
    );

    if (GET_CACHED(requestedDeadlineMissedStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/RequestedIncompatibleQosStatusHolder");
    SET_CACHED(requestedIncompatibleQosStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/RequestedIncompatibleQosStatus;"
        )
    );

    if (GET_CACHED(requestedIncompatibleQosStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/SubscriptionMatchedStatusHolder");
    SET_CACHED(subscriptionMatchedStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/SubscriptionMatchedStatus;"
        )
    );

    if (GET_CACHED(subscriptionMatchedStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/SampleRejectedStatusHolder");
    SET_CACHED(sampleRejectedStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/SampleRejectedStatus;"
        )
    );

    if (GET_CACHED(sampleRejectedStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    /**********************************************************************/
    tempClass = (*env)->FindClass(env, "DDS/SampleLostStatusHolder");
    SET_CACHED(sampleLostStatusHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/SampleLostStatus;"
        )
    );

    if (GET_CACHED(sampleLostStatusHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    (*env)->DeleteLocalRef(env, tempClass);

    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSubscriberQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SubscriberQosHolder");
    SET_CACHED(subscriberQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/SubscriberQos;"
        )
    );

    if (GET_CACHED(subscriberQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeNamedSubscriberQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/NamedSubscriberQosHolder");
    SET_CACHED(namedSubscriberQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/NamedSubscriberQos;"
        )
    );

    if (GET_CACHED(namedSubscriberQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeTopicQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/TopicQosHolder");
    SET_CACHED(topicQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/TopicQos;"
        )
    );

    if (GET_CACHED(topicQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeNamedTopicQosHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/NamedTopicQosHolder");
    SET_CACHED(namedTopicQosHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/NamedTopicQos;"
        )
    );

    if (GET_CACHED(namedTopicQosHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeDataReaderSeqHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/DataReaderSeqHolder");
    SET_CACHED(dataReaderSeqHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "[LDDS/DataReader;"
        )
    );

    if (GET_CACHED(dataReaderSeqHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeConditionSeqHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ConditionSeqHolder");
    SET_CACHED(conditionSeqHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "[LDDS/Condition;"
        )
    );

    if (GET_CACHED(conditionSeqHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSampleInfo(JNIEnv *env)
{
    jclass grClass;
    jclass tempClass;

    tempClass = (*env)->FindClass(env, "DDS/SampleInfo");
    grClass = (*env)->NewGlobalRef (env, tempClass);

    SET_CACHED(sampleInfo_class, grClass);
    if (GET_CACHED(sampleInfo_class) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_constructor_mid, (*env)->GetMethodID (env, tempClass, "<init>", "()V"));
    if (GET_CACHED(sampleInfo_constructor_mid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_sample_state_fid, (*env)->GetFieldID( env, tempClass, "sample_state", "I"));
    if (GET_CACHED(sampleInfo_sample_state_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_view_state_fid, (*env)->GetFieldID( env, tempClass, "view_state", "I"));
    if (GET_CACHED(sampleInfo_view_state_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_instance_state_fid, (*env)->GetFieldID( env, tempClass, "instance_state", "I"));
    if (GET_CACHED(sampleInfo_instance_state_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_valid_data_fid, (*env)->GetFieldID( env, tempClass, "valid_data", "Z" /* boolean */));
    if (GET_CACHED(sampleInfo_instance_state_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_source_timestamp_fid, (*env)->GetFieldID( env, tempClass, "source_timestamp", "LDDS/Time_t;"));
    if (GET_CACHED(sampleInfo_source_timestamp_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_instance_handle_fid, (*env)->GetFieldID( env, tempClass, "instance_handle", "J"));
    if (GET_CACHED(sampleInfo_instance_handle_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_publication_handle_fid, (*env)->GetFieldID( env, tempClass, "publication_handle", "J"));
    if (GET_CACHED(sampleInfo_publication_handle_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_disposed_generation_count_fid, (*env)->GetFieldID( env, tempClass, "disposed_generation_count", "I"));
    if (GET_CACHED(sampleInfo_disposed_generation_count_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_no_writers_generation_count_fid, (*env)->GetFieldID( env, tempClass, "no_writers_generation_count", "I"));
    if (GET_CACHED(sampleInfo_no_writers_generation_count_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_sample_rank_fid, (*env)->GetFieldID( env, tempClass, "sample_rank", "I"));
    if (GET_CACHED(sampleInfo_sample_rank_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_generation_rank_fid, (*env)->GetFieldID( env, tempClass, "generation_rank", "I"));
    if (GET_CACHED(sampleInfo_generation_rank_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_absolute_generation_rank_fid, (*env)->GetFieldID( env, tempClass, "absolute_generation_rank", "I"));
    if (GET_CACHED(sampleInfo_absolute_generation_rank_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(sampleInfo_reception_timestamp_fid, (*env)->GetFieldID( env, tempClass, "reception_timestamp", "LDDS/Time_t;"));
    if (GET_CACHED(sampleInfo_reception_timestamp_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSampleInfoHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SampleInfoHolder");
    SET_CACHED(sampleInfoHolder_value_fid, (*env)->GetFieldID( env, tempClass, "value", "LDDS/SampleInfo;"));
    if (GET_CACHED(sampleInfoHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeSampleInfoSeqHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/SampleInfoSeqHolder");
    SET_CACHED(sampleInfoSeqHolder_value_fid, (*env)->GetFieldID( env, tempClass, "value", "[LDDS/SampleInfo;"));
    if (GET_CACHED(sampleInfoSeqHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_InitializeDuration_t(
    JNIEnv *env)
{
    jclass grClass;
    jclass tempClass;

    tempClass = (*env)->FindClass(env, "DDS/Duration_t");
    grClass = (*env)->NewGlobalRef (env, tempClass);

    SET_CACHED(duration_t_class, grClass);

    SET_CACHED(
        duration_t_constructor_mid,
        (*env)->GetMethodID (env, tempClass, "<init>", "(II)V")
    );

    SET_CACHED(duration_t_sec_fid,
        (*env)->GetFieldID(env, tempClass, "sec", "I"));

    SET_CACHED(duration_t_nanosec_fid,
        (*env)->GetFieldID(env, tempClass, "nanosec", "I"));

    if (GET_CACHED(duration_t_sec_fid) == NULL     ||
        GET_CACHED(duration_t_nanosec_fid) == NULL ||
        GET_CACHED(duration_t_class) == NULL       ||
        GET_CACHED(duration_t_constructor_mid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_InitializeTime_t(
    JNIEnv *env)
{
    jclass grClass;
    jclass tempClass;

    tempClass = (*env)->FindClass(env, "DDS/Time_t");
    grClass = (*(env))->NewGlobalRef (env, tempClass);

    SET_CACHED(time_t_class, grClass);
    if (GET_CACHED(time_t_class) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(time_t_constructor_mid, (*env)->GetMethodID (env, tempClass, "<init>", "(II)V"));
    if (GET_CACHED(time_t_constructor_mid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }
    SET_CACHED(time_t_sec_fid, (*env)->GetFieldID(env, tempClass, "sec", "I"));
    SET_CACHED(time_t_nanosec_fid, (*env)->GetFieldID(env, tempClass, "nanosec", "I"));

    if (GET_CACHED(time_t_sec_fid) == NULL ||
        GET_CACHED(time_t_nanosec_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeTime_tHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/Time_tHolder");
    SET_CACHED(time_tHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "LDDS/Time_t;"
        )
    );

    if (GET_CACHED(time_tHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeStringHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/StringHolder");
    SET_CACHED(stringHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "Ljava/lang/String;"
        )
    );

    if (GET_CACHED(stringHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode saj_InitializeErrorCodeHolder(JNIEnv *env)
{
    jclass tempClass = (*env)->FindClass(env, "DDS/ErrorCodeHolder");
    SET_CACHED(errorCodeHolder_value_fid,
        (*env)->GetFieldID(
            env,
            tempClass,
            "value",
            "I"
        )
    );

    if (GET_CACHED(errorCodeHolder_value_fid) == NULL)
    {
        return SAJ_RETCODE_ERROR; /* VM has thrown an exception */
    }

    (*env)->DeleteLocalRef(env, tempClass);
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_InitializeInconsistentTopicStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/InconsistentTopicStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(inconsistentTopicStatus_class, grClass);

    if(GET_CACHED(inconsistentTopicStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(inconsistentTopicStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(II)V"));

        if(GET_CACHED(inconsistentTopicStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeGetAllDataDisposedTopicStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/AllDataDisposedTopicStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(allDataDisposedTopicStatus_class, grClass);

    if(GET_CACHED(allDataDisposedTopicStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(allDataDisposedTopicStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(II)V"));

        if(GET_CACHED(allDataDisposedTopicStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeLivelinessLostStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/LivelinessLostStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(livelinessLostStatus_class, grClass);

    if(GET_CACHED(livelinessLostStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(livelinessLostStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(II)V"));

        if(GET_CACHED(livelinessLostStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeOfferedDeadlineMissedStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/OfferedDeadlineMissedStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(offeredDeadlineMissedStatus_class, grClass);

    if(GET_CACHED(offeredDeadlineMissedStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(offeredDeadlineMissedStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(IIJ)V"));

        if(GET_CACHED(offeredDeadlineMissedStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeOfferedIncompatibleQosStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/OfferedIncompatibleQosStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(offeredIncompatibleQosStatus_class, grClass);

    if(GET_CACHED(offeredIncompatibleQosStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(offeredIncompatibleQosStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(III[LDDS/QosPolicyCount;)V"));

        if(GET_CACHED(offeredIncompatibleQosStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializePublicationMatchStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/PublicationMatchedStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(publicationMatchStatus_class, grClass);

    if(GET_CACHED(publicationMatchStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(publicationMatchStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(IIIIJ)V"));

        if(GET_CACHED(publicationMatchStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeQosPolicyCount(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/QosPolicyCount");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(qosPolicyCount_class, grClass);

    if(GET_CACHED(qosPolicyCount_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(qosPolicyCount_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(II)V"));

        if(GET_CACHED(qosPolicyCount_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeSampleRejectedStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/SampleRejectedStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(sampleRejectedStatus_class, grClass);

    if(GET_CACHED(sampleRejectedStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(sampleRejectedStatus_constructor_mid,
                                (*env)->GetMethodID (env, cls, "<init>",
                                "(IILDDS/SampleRejectedStatusKind;J)V"));

        if(GET_CACHED(sampleRejectedStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        } else {
            cls = (*env)->FindClass(env, "DDS/SampleRejectedStatusKind");
            grClass = (*(env))->NewGlobalRef (env, cls);
            SET_CACHED(sampleRejectedStatusKind_class, grClass);

            if(GET_CACHED(sampleRejectedStatusKind_class == NULL)){
                rc = SAJ_RETCODE_ERROR;
            } else {
                SET_CACHED(sampleRejectedStatusKind_fromInt_mid,
                        (*env)->GetStaticMethodID(env, cls, "from_int",
                                        "(I)LDDS/SampleRejectedStatusKind;"));

                if(GET_CACHED(sampleRejectedStatusKind_fromInt_mid == NULL)){
                    rc = SAJ_RETCODE_ERROR;
                }
            }
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeLivelinessChangedStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/LivelinessChangedStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(livelinessChangedStatus_class, grClass);

    if(GET_CACHED(livelinessChangedStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(livelinessChangedStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(IIIIJ)V"));

        if(GET_CACHED(livelinessChangedStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeRequestedDeadlineMissedStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/RequestedDeadlineMissedStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(requestedDeadlineMissedStatus_class, grClass);

    if(GET_CACHED(requestedDeadlineMissedStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(requestedDeadlineMissedStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(IIJ)V"));

        if(GET_CACHED(requestedDeadlineMissedStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeRequestedIncompatibleQosStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/RequestedIncompatibleQosStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(requestedIncompatibleQosStatus_class, grClass);

    if(GET_CACHED(requestedIncompatibleQosStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(requestedIncompatibleQosStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(III[LDDS/QosPolicyCount;)V"));

        if(GET_CACHED(requestedIncompatibleQosStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeSubscriptionMatchStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/SubscriptionMatchedStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(subscriptionMatchStatus_class, grClass);

    if(GET_CACHED(subscriptionMatchStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(subscriptionMatchStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(IIIIJ)V"));

        if(GET_CACHED(subscriptionMatchStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeSampleLostStatus(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "DDS/SampleLostStatus");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(sampleLostStatus_class, grClass);

    if(GET_CACHED(sampleLostStatus_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else {
        SET_CACHED(sampleLostStatus_constructor_mid,
                    (*env)->GetMethodID (env, cls, "<init>", "(II)V"));

        if(GET_CACHED(sampleLostStatus_constructor_mid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeTypeSupport(
    JNIEnv* env)
{
    saj_returnCode rc;
    jclass grClass;
    jclass cls;

    rc = SAJ_RETCODE_OK;
    cls = (*env)->FindClass(env, "org/opensplice/dds/dcps/TypeSupportImpl");
    grClass = (*(env))->NewGlobalRef (env, cls);

    SET_CACHED(typeSupport_class, grClass);

    if(GET_CACHED(typeSupport_class == NULL)){
        rc = SAJ_RETCODE_ERROR;
    } else
    {
        SET_CACHED(typeSupportDataReader_fid,
                    (*env)->GetFieldID(env, cls, "dataReaderClass",
                                                 "Ljava/lang/String;"));

        if(GET_CACHED(typeSupportDataReader_fid == NULL)){
            rc = SAJ_RETCODE_ERROR;
        } else{
            SET_CACHED(typeSupportDataWriter_fid,
                        (*env)->GetFieldID(env, cls, "dataWriterClass",
                                                     "Ljava/lang/String;"));

            if(GET_CACHED(typeSupportDataWriter_fid == NULL)){
                rc = SAJ_RETCODE_ERROR;
            } else {
                SET_CACHED(typeSupportConstructorSignature_fid,
                            (*env)->GetFieldID(env, cls, "constructorSignature",
                                                         "Ljava/lang/String;"));
                if(GET_CACHED(typeSupportConstructorSignature_fid == NULL)){
                    rc = SAJ_RETCODE_ERROR;
                }
                else {
                    SET_CACHED(typeSupportDataReaderView_fid,
                                (*env)->GetFieldID(env, cls, "dataReaderViewClass",
                                                             "Ljava/lang/String;"));

                    if(GET_CACHED(typeSupportDataReaderView_fid == NULL)){
                        rc = SAJ_RETCODE_ERROR;
                    }
                }
            }
        }
    }

    (*env)->DeleteLocalRef(env, cls);

    return rc;
}

saj_returnCode
saj_InitializeTopicListener(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls, grClass;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/TopicListenerOperations");
    grClass = (*(env))->NewGlobalRef (env, cls);
    SET_CACHED(listener_topic_class, grClass);

    if(GET_CACHED(listener_topic_class) != NULL){
        SET_CACHED(listener_onInconsistentTopic_mid,
                    (*env)->GetMethodID (env, cls,  "on_inconsistent_topic",
                                    "(LDDS/Topic;LDDS/InconsistentTopicStatus;)V"));

        (*env)->DeleteLocalRef(env, cls);

        if(GET_CACHED(listener_onInconsistentTopic_mid) != NULL){
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeExtTopicListener(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls, grClass;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/ExtTopicListenerOperations");
    grClass = (*(env))->NewGlobalRef (env, cls);
    SET_CACHED(listener_topic_class, grClass);

    if(GET_CACHED(listener_topic_class) != NULL){
        SET_CACHED(listener_onAllDataDisposed_mid,
                    (*env)->GetMethodID (env, cls, "on_all_data_disposed",
                                    "(LDDS/Topic;)V"));

        (*env)->DeleteLocalRef(env, cls);

        if(GET_CACHED(listener_onAllDataDisposed_mid) != NULL){
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeDataReaderListener(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls, grClass;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/DataReaderListenerOperations");
    grClass = (*(env))->NewGlobalRef (env, cls);
    SET_CACHED(listener_datareader_class, grClass);

    if(GET_CACHED(listener_datareader_class) != NULL){
        SET_CACHED(listener_onRequestedDeadlineMissed_mid,
                    (*env)->GetMethodID (env, cls,
                    "on_requested_deadline_missed",
                    "(LDDS/DataReader;LDDS/RequestedDeadlineMissedStatus;)V"));

        if(GET_CACHED(listener_onRequestedDeadlineMissed_mid) != NULL){
            SET_CACHED(listener_onRequestedIncompatibleQos_mid,
                    (*env)->GetMethodID (env, cls,
                    "on_requested_incompatible_qos",
                    "(LDDS/DataReader;LDDS/RequestedIncompatibleQosStatus;)V"));

            if(GET_CACHED(listener_onRequestedIncompatibleQos_mid) != NULL){
                SET_CACHED(listener_onSampleRejected_mid,
                        (*env)->GetMethodID (env, cls,
                        "on_sample_rejected",
                        "(LDDS/DataReader;LDDS/SampleRejectedStatus;)V"));

                if(GET_CACHED(listener_onSampleRejected_mid) != NULL){
                    SET_CACHED(listener_onLivelinessChanged_mid,
                        (*env)->GetMethodID (env, cls,
                        "on_liveliness_changed",
                        "(LDDS/DataReader;LDDS/LivelinessChangedStatus;)V"));

                    if(GET_CACHED(listener_onLivelinessChanged_mid) != NULL){
                        SET_CACHED(listener_onDataAvailable_mid,
                            (*env)->GetMethodID (env, cls,
                                    "on_data_available",
                                    "(LDDS/DataReader;)V"));

                        if(GET_CACHED(listener_onDataAvailable_mid) != NULL){
                            SET_CACHED(listener_onSubscriptionMatch_mid,
                                (*env)->GetMethodID (env, cls,
                                "on_subscription_matched",
                                "(LDDS/DataReader;LDDS/SubscriptionMatchedStatus;)V"));

                            if(GET_CACHED(listener_onSubscriptionMatch_mid) != NULL){
                                SET_CACHED(listener_onSampleLost_mid,
                                    (*env)->GetMethodID (env, cls,
                                    "on_sample_lost",
                                    "(LDDS/DataReader;LDDS/SampleLostStatus;)V"));

                                (*env)->DeleteLocalRef(env, cls);

                                if(GET_CACHED(listener_onSampleLost_mid) != NULL){
                                    rc = SAJ_RETCODE_OK;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeDataWriterListener(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls, grClass;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/DataWriterListenerOperations");
    grClass = (*(env))->NewGlobalRef (env, cls);
    SET_CACHED(listener_datawriter_class, grClass);

    if(GET_CACHED(listener_datawriter_class) != NULL){
        SET_CACHED(listener_onOfferedDeadlineMissed_mid,
                    (*env)->GetMethodID (env, cls,
                    "on_offered_deadline_missed",
                    "(LDDS/DataWriter;LDDS/OfferedDeadlineMissedStatus;)V"));

        if(GET_CACHED(listener_onOfferedDeadlineMissed_mid) != NULL){
            SET_CACHED(listener_onOfferedIncompatibleQos_mid,
                    (*env)->GetMethodID (env, cls,
                    "on_offered_incompatible_qos",
                    "(LDDS/DataWriter;LDDS/OfferedIncompatibleQosStatus;)V"));

            if(GET_CACHED(listener_onOfferedIncompatibleQos_mid) != NULL){
                SET_CACHED(listener_onLivelinessLost_mid,
                        (*env)->GetMethodID (env, cls,
                        "on_liveliness_lost",
                        "(LDDS/DataWriter;LDDS/LivelinessLostStatus;)V"));

                if(GET_CACHED(listener_onLivelinessLost_mid) != NULL){
                    SET_CACHED(listener_onPublicationMatch_mid,
                        (*env)->GetMethodID (env, cls,
                        "on_publication_matched",
                        "(LDDS/DataWriter;LDDS/PublicationMatchedStatus;)V"));
                    (*env)->DeleteLocalRef(env, cls);

                    if(GET_CACHED(listener_onPublicationMatch_mid) != NULL){
                        rc = SAJ_RETCODE_OK;
                    }
                }
            }
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeSubscriberListener(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls, grClass;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/SubscriberListenerOperations");
    grClass = (*(env))->NewGlobalRef (env, cls);
    SET_CACHED(listener_subscriber_class, grClass);

    if(GET_CACHED(listener_subscriber_class) != NULL){
        SET_CACHED(listener_onDataOnReaders_mid,
                    (*env)->GetMethodID (env, cls,
                    "on_data_on_readers",
                    "(LDDS/Subscriber;)V"));
        (*env)->DeleteLocalRef(env, cls);

        if(GET_CACHED(listener_onDataOnReaders_mid) != NULL){
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeUtilities(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls, grClass;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, PACKAGENAME "Utilities");
    grClass = (*(env))->NewGlobalRef (env, cls);
    SET_CACHED(utilities_class, grClass);

    if(GET_CACHED(utilities_class) != NULL){
        SET_CACHED(utilities_throwException_mid,
                    (*env)->GetStaticMethodID (env, cls,
                    "createException",
                    "(ILjava/lang/String;)Ljava/lang/RuntimeException;"));

        (*env)->DeleteLocalRef(env, cls);

        if(GET_CACHED(utilities_throwException_mid) != NULL){
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_InitializeConstantObjects(
    JNIEnv *env)
{
    saj_returnCode rc;
    jclass cls;
    jfieldID fid;
    jobject obj;

    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/PARTICIPANT_QOS_DEFAULT");
    if (cls) {
    fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/DomainParticipantQos;");
    if (fid != NULL) {
        obj = (*env)->GetStaticObjectField (env, cls, fid);
        SET_CACHED (PARTICIPANT_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
            (*env)->DeleteLocalRef(env, obj);
        rc = SAJ_RETCODE_OK;
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/TOPIC_QOS_DEFAULT");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/TopicQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (TOPIC_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/PUBLISHER_QOS_DEFAULT");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/PublisherQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (PUBLISHER_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/SUBSCRIBER_QOS_DEFAULT");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/SubscriberQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (SUBSCRIBER_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/DATAWRITER_QOS_DEFAULT");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/DataWriterQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (DATAWRITER_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/DATAREADER_QOS_DEFAULT");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/DataReaderQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (DATAREADER_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = SAJ_RETCODE_ERROR;
        cls = (*env)->FindClass(env, "DDS/DATAREADERVIEW_QOS_DEFAULT");
        if (cls) {
            fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/DataReaderViewQos;");
            if (fid != NULL) {
                obj = (*env)->GetStaticObjectField (env, cls, fid);
                SET_CACHED (DATAREADERVIEW_QOS_DEFAULT, (*(env))->NewGlobalRef (env, obj));
                    (*env)->DeleteLocalRef(env, obj);
                rc = SAJ_RETCODE_OK;
            }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/DATAWRITER_QOS_USE_TOPIC_QOS");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/DataWriterQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (DATAWRITER_QOS_USE_TOPIC_QOS, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }
    if (rc == SAJ_RETCODE_OK) {
    rc = SAJ_RETCODE_ERROR;
    cls = (*env)->FindClass(env, "DDS/DATAREADER_QOS_USE_TOPIC_QOS");
        if (cls) {
        fid = (*env)->GetStaticFieldID(env, cls, "value", "LDDS/DataReaderQos;");
        if (fid != NULL) {
            obj = (*env)->GetStaticObjectField (env, cls, fid);
            SET_CACHED (DATAREADER_QOS_USE_TOPIC_QOS, (*(env))->NewGlobalRef (env, obj));
                (*env)->DeleteLocalRef(env, obj);
            rc = SAJ_RETCODE_OK;
        }
        }
    }

    if (rc == SAJ_RETCODE_OK) {
        rc = SAJ_RETCODE_ERROR;
        cls = (*env)->FindClass(env, "DDS/StringSeqHolder");

        if (cls) {
            SET_CACHED (stringSeqHolder_stringSeq_fid,
                            (*env)->GetFieldID(env, cls,
                                "value", "[Ljava/lang/String;"));

            if (GET_CACHED(stringSeqHolder_stringSeq_fid) != NULL) {
                rc = SAJ_RETCODE_OK;
            }
        }
    }
    return rc;
}
