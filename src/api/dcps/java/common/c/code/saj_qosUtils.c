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

#include "saj_qosUtils.h"

saj_returnCode
saj_UserDataQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_userDataQosPolicy  *dst)
{
    jbyteArray javaUserDataQosPolicy_value;
    saj_returnCode rc;

    assert(dst != NULL);

    if (src != NULL) {
        javaUserDataQosPolicy_value = (*env)->GetObjectField(env, src,
                                    GET_CACHED(userDataQosPolicy_value_fid));
        saj_exceptionCheck(env);
        rc = saj_octetSequenceCopyIn(env, javaUserDataQosPolicy_value, &(dst->value));
        (*env)->DeleteLocalRef (env, javaUserDataQosPolicy_value);
    } else {
        rc = SAJ_RETCODE_ERROR;
    }

    return rc;
}

saj_returnCode
saj_UserDataQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_userDataQosPolicy  *src,
    jobject                 *dst)
{
    jbyteArray value;
    saj_returnCode rc;

    assert(dst != NULL);

    value = NULL;
    rc = SAJ_RETCODE_OK;

    /* check if a EntityFactoryQosPolicy already exists */
    if (*dst == NULL)
    {
        /* create a new java EntityFactoryQosPolicy object */
        rc = saj_create_new_java_object(env, "DDS/UserDataQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_octetSequenceCopyOut(env, &src->value, &value);

        if (rc == SAJ_RETCODE_OK)
        {
            /* Set the attribute of the UserDataQos object */
            (*env)->SetObjectField(
                env, *dst, GET_CACHED(userDataQosPolicy_value_fid), value);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, value);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_EntityFactoryQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_entityFactoryQosPolicy *dst)
{
    assert(dst != NULL);

    if (src != NULL)
    {
        dst->autoenable_created_entities =
            (*env)->GetBooleanField(env, src,
                GET_CACHED(entityFactoryQosPolicy_autoenableCreatedEntities_fid));
        saj_exceptionCheck(env);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_EntityFactoryQosPolicyCopyOut(
    JNIEnv *env,
    gapi_entityFactoryQosPolicy *src,
    jobject *dst)
{
    jboolean value;
    saj_returnCode rc;

    assert(dst != NULL);

    /* create a new java EntityFactoryQosPolicy object */
    rc = saj_create_new_java_object(env, "DDS/EntityFactoryQosPolicy", dst);

    if (rc == SAJ_RETCODE_OK)
    {
        if (src->autoenable_created_entities == 1) {
            value = JNI_TRUE;
        } else {
            value = JNI_FALSE;
        }

        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(entityFactoryQosPolicy_autoenableCreatedEntities_fid),
            value);
        saj_exceptionCheck(env);
    }

    return rc;
}


saj_returnCode
saj_PresentationQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_presentationQosPolicy *dst)
{
    jobject javaPresentationQosPolicyAccessScopeKind;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaPresentationQosPolicyAccessScopeKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(presentationQosPolicy_accessScope_fid));
        saj_exceptionCheck(env);

        rc = saj_EnumCopyIn(env,
                            javaPresentationQosPolicyAccessScopeKind,
                            (gapi_unsigned_long *) &dst->access_scope);

        if (rc == SAJ_RETCODE_OK)
        {
            dst->coherent_access =
                (*env)->GetBooleanField(env, src,
                    GET_CACHED(presentationQosPolicy_coherentAccess_fid));
            saj_exceptionCheck(env);
            dst->ordered_access =
                (*env)->GetBooleanField(env, src,
                    GET_CACHED(presentationQosPolicy_orderedAccess_fid));
            saj_exceptionCheck(env);
        }
    }

    return rc;
}

saj_returnCode
saj_PartitionQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_partitionQosPolicy *dst)
{
    jobjectArray jStringArray = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        jStringArray =
            (*env)->GetObjectField(env, src,
                GET_CACHED(partitionQosPolicy_name_fid));
        saj_exceptionCheck(env);
        rc = saj_stringSequenceCopyIn(env, jStringArray, &(dst->name));
    (*env)->DeleteLocalRef (env, jStringArray);
    }

    return rc;
}

saj_returnCode
saj_GroupDataQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_groupDataQosPolicy *dst)
{
    jbyteArray groupDataQosPolicy_value = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        groupDataQosPolicy_value =
            (*env)->GetObjectField(env, src,
                GET_CACHED(groupDataQosPolicy_value_fid));
        saj_exceptionCheck(env);
        rc = saj_octetSequenceCopyIn(env,
                                     groupDataQosPolicy_value,
                                     &(dst->value));
    (*env)->DeleteLocalRef (env, groupDataQosPolicy_value);
    }

    return rc;
}

saj_returnCode
saj_TopicDataQosPolicyCopyIn(
    JNIEnv              *env,
    jobject             src,
    gapi_topicDataQosPolicy *dst)
{
    jbyteArray topicQosPolicy_value = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        topicQosPolicy_value =
            (*env)->GetObjectField(env, src,
                GET_CACHED(topicDataQosPolicy_value_fid));
        saj_exceptionCheck(env);
        rc = saj_octetSequenceCopyIn(env, topicQosPolicy_value, &(dst->value));
    (*env)->DeleteLocalRef (env, topicQosPolicy_value);
    }

    return rc;
}

saj_returnCode
saj_DurabilityQosPolicyCopyIn(
    JNIEnv                      *env,
    jobject                     src,
    gapi_durabilityQosPolicy    *dst)
{
    jobject javaDurabilityQosKind = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaDurabilityQosKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(durabilityQosPolicy_kind_fid));
        saj_exceptionCheck(env);

        rc = saj_EnumCopyIn(env, javaDurabilityQosKind,
                                    (gapi_unsigned_long *) &(dst->kind));

    (*env)->DeleteLocalRef (env, javaDurabilityQosKind);
    }

    return rc;
}

saj_returnCode
saj_DurabilityServiceQosPolicyCopyIn(
    JNIEnv                          *env,
    jobject                          src,
    gapi_durabilityServiceQosPolicy *dst)
{
    jobject javaHistoryQosKind = NULL;
    jobject javaServiceCleanupDelay = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaHistoryQosKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(durabilityServiceQosPolicy_historyKind_fid));
        saj_exceptionCheck(env);

        dst->history_depth =
            (*env)->GetIntField(env, src,
                GET_CACHED(durabilityServiceQosPolicy_historyDepth_fid));
        saj_exceptionCheck(env);
        dst->max_samples =
            (*env)->GetIntField(env, src,
                GET_CACHED(durabilityServiceQosPolicy_maxSamples_fid));
        saj_exceptionCheck(env);
        dst->max_instances =
            (*env)->GetIntField(env, src,
                GET_CACHED(durabilityServiceQosPolicy_maxInstances_fid));
        saj_exceptionCheck(env);
        dst->max_samples_per_instance =
            (*env)->GetIntField(env, src,
                GET_CACHED(durabilityServiceQosPolicy_maxSamplesPerInstance_fid));
        saj_exceptionCheck(env);

        javaServiceCleanupDelay =
            (*env)->GetObjectField(env, src,
                GET_CACHED(durabilityServiceQosPolicy_serviceCleanupDelay_fid));
        saj_exceptionCheck(env);

        rc = saj_EnumCopyIn(env, javaHistoryQosKind,
                                    (gapi_unsigned_long *) &(dst->history_kind));
        if (rc == SAJ_RETCODE_OK){
            saj_durationCopyIn(env, javaServiceCleanupDelay,
                                        &(dst->service_cleanup_delay));
        }
    (*env)->DeleteLocalRef (env, javaHistoryQosKind);
    (*env)->DeleteLocalRef (env, javaServiceCleanupDelay);
    }

    return rc;
}

saj_returnCode
saj_DeadlineQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_deadlineQosPolicy  *dst)
{
    jobject javaPeriod = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaPeriod =
            (*env)->GetObjectField(env, src,
                GET_CACHED(deadlineQosPolicy_period_fid));
        saj_exceptionCheck(env);
        saj_durationCopyIn(env, javaPeriod, &(dst->period));
    (*env)->DeleteLocalRef (env, javaPeriod);
    }

    return rc;
}

saj_returnCode
saj_DeadlineQosPolicyCopyOut(
    JNIEnv *env,
    gapi_deadlineQosPolicy *src,
    jobject *dst)
{
    jobject javaPeriod;
    saj_returnCode rc;

    assert(dst != NULL);

    javaPeriod = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DeadlineQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* create a new java Duration_t object */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &javaPeriod);

        if (rc == SAJ_RETCODE_OK)
        {
            /* copy the content of the gapi object to the java Duration_t object */
            saj_durationCopyOut(env, &src->period, &javaPeriod);

            /* store the Duration_t java object in the DeadLineQosPolicy object */
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(deadlineQosPolicy_period_fid),
                javaPeriod);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaPeriod);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_LatencyBudgetQosPolicyCopyIn(
    JNIEnv                      *env,
    jobject                     src,
    gapi_latencyBudgetQosPolicy *dst)
{
    jobject javaDuration;
    saj_returnCode rc;

    assert(dst != NULL);

    javaDuration = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        javaDuration =
            (*env)->GetObjectField(env, src,
                GET_CACHED(latencyBudgetQosPolicy_duration_fid));
        saj_exceptionCheck(env);
        rc = saj_durationCopyIn(env, javaDuration, &(dst->duration));
    (*env)->DeleteLocalRef (env, javaDuration);
    }

    return rc;
}

saj_returnCode
saj_LatencyBudgetQosPolicyCopyOut(
    JNIEnv *env,
    gapi_latencyBudgetQosPolicy *src,
    jobject *dst)
{
    jobject javaDuration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL)
    {
        /* create a new java LatencyBudgetQosPolicy object */
        rc = saj_create_new_java_object(env, "DDS/LatencyBudgetQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* create a new java Duration_t object */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &javaDuration);

        if (rc == SAJ_RETCODE_OK)
        {
            /* copy the content of the gapi object to the java object */
            saj_durationCopyOut(env, &src->duration, &javaDuration);

            /* store javaDuration in the LatencyBudgetQosPolicy object */
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(latencyBudgetQosPolicy_duration_fid),
                javaDuration);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaDuration);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_LivelinessQosPolicyCopyIn(
    JNIEnv                      *env,
    jobject                     src,
    gapi_livelinessQosPolicy    *dst)
{
    jobject javaKind;
    jobject javaLeaseDuration;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    javaLeaseDuration = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        javaKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(livelinessQosPolicy_kind_fid));
        saj_exceptionCheck(env);
        javaLeaseDuration =
            (*env)->GetObjectField(env, src,
                GET_CACHED(livelinessQosPolicy_leaseDuration_fid));
        saj_exceptionCheck(env);
        rc = saj_EnumCopyIn(env, javaKind, (gapi_unsigned_long *) &(dst->kind));

        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_durationCopyIn(
                env, javaLeaseDuration, &(dst->lease_duration));
        }
    (*env)->DeleteLocalRef (env, javaKind);
    (*env)->DeleteLocalRef (env, javaLeaseDuration);
    }
    return rc;
}

saj_returnCode
saj_LivelinessQosPolicyCopyOut(
    JNIEnv                   *env,
    gapi_livelinessQosPolicy *src,
    jobject                  *dst)
{
    jobject javaLeaseDuration;
    jobject javaKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaLeaseDuration = NULL;
    javaKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/LivelinessQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* create a new java objects for LeaseDuration and LivelinessQosKind */
        rc = saj_create_new_java_object(
            env, "DDS/Duration_t", &javaLeaseDuration);

        if (rc == SAJ_RETCODE_OK)
        {
            /* copy the content of the gapi objects to the java objects */
            saj_durationCopyOut(env, &src->lease_duration, &javaLeaseDuration);

            rc = saj_EnumCopyOut(
                env, "DDS/LivelinessQosPolicyKind", src->kind, &javaKind);

            if (rc == SAJ_RETCODE_OK)
            {

                /* store the objects in LivelinessQosPolicy attributes */
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(livelinessQosPolicy_leaseDuration_fid),
                    javaLeaseDuration);
                saj_exceptionCheck(env);
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(livelinessQosPolicy_kind_fid),
                    javaKind);
                saj_exceptionCheck(env);
            }
        }
    }

    (*env)->DeleteLocalRef(env, javaLeaseDuration);
    saj_exceptionCheck(env);
    (*env)->DeleteLocalRef(env, javaKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ReliabilityQosPolicyCopyIn(
    JNIEnv                      *env,
    jobject                     src,
    gapi_reliabilityQosPolicy   *dst)
{
    jobject javaKind;
    jobject javaMaxBlockingTime;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    javaMaxBlockingTime = NULL;
    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {

        javaKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(reliabilityQosPolicy_kind_fid));
        saj_exceptionCheck(env);
        javaMaxBlockingTime =
            (*env)->GetObjectField(env, src,
                GET_CACHED(reliabilityQosPolicy_maxBlockingTime_fid));
        saj_exceptionCheck(env);
        rc = saj_EnumCopyIn(env, javaKind, (gapi_unsigned_long *) &(dst->kind));

        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_durationCopyIn(
                env, javaMaxBlockingTime, &(dst->max_blocking_time));
        }
        if(rc == SAJ_RETCODE_OK){
            dst->synchronous = (*env)->GetBooleanField(env, src,
                 GET_CACHED(reliabilityQosPolicy_synchronous_fid)) == JNI_TRUE;
            saj_exceptionCheck(env);
        }
    (*env)->DeleteLocalRef (env, javaKind);
    (*env)->DeleteLocalRef (env, javaMaxBlockingTime);
    }

    return rc;
}

saj_returnCode
saj_DestinationOrderQosPolicyCopyIn(
    JNIEnv                          *env,
    jobject                         src,
    gapi_destinationOrderQosPolicy  *dst)
{
    jobject javaKind = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(destinationOrderQosPolicy_kind_fid));
        saj_exceptionCheck(env);
        rc = saj_EnumCopyIn(
            env, javaKind, (gapi_unsigned_long *) &(dst->kind));
    (*env)->DeleteLocalRef (env, javaKind);
    }

    return rc;
}

saj_returnCode
saj_HistoryQosPolicyCopyIn(
    JNIEnv                  *env,
    jobject                 src,
    gapi_historyQosPolicy   *dst)
{
    jobject javaKind = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {

        javaKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(historyQosPolicy_kind_fid));
        saj_exceptionCheck(env);
        rc = saj_EnumCopyIn(env, javaKind, (gapi_unsigned_long *) &(dst->kind));

        if (rc == SAJ_RETCODE_OK)
        {
            dst->depth =
                (*env)->GetIntField(env, src,
                    GET_CACHED(historyQosPolicy_depth_fid));
            saj_exceptionCheck(env);
        }
    (*env)->DeleteLocalRef (env, javaKind);
    }

    return rc;
}

saj_returnCode
saj_ResourceLimitsQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_resourceLimitsQosPolicy *dst)
{
    assert(dst != NULL);

    if (src != NULL)
    {
        dst->max_samples =
            (*env)->GetIntField(env, src,
                GET_CACHED(resourceLimitsQosPolicy_maxSamples_fid));
        saj_exceptionCheck(env);
        dst->max_instances =
            (*env)->GetIntField(env, src,
                GET_CACHED(resourceLimitsQosPolicy_maxInstances_fid));
        saj_exceptionCheck(env);
        dst->max_samples_per_instance =
            (*env)->GetIntField(env, src,
                GET_CACHED(resourceLimitsQosPolicy_maxSamplesPerInstance_fid));
        saj_exceptionCheck(env);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_TransportPriorityQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_transportPriorityQosPolicy *dst)
{
    assert(dst != NULL);

    if (src != NULL)
    {
        dst->value =
            (*env)->GetIntField(env, src,
                GET_CACHED(transportPriorityQosPolicy_value_fid));
        saj_exceptionCheck(env);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_LifespanQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_lifespanQosPolicy *dst)
{
    jobject duration;
    saj_returnCode rc;

    assert(dst != NULL);

    duration = NULL;
    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        duration =
            (*env)->GetObjectField(env, src,
                GET_CACHED(lifespanQosPolicy_duration_fid));
        saj_exceptionCheck(env);
        rc = saj_durationCopyIn(env, duration, &(dst->duration));
    (*env)->DeleteLocalRef (env, duration);
    }

    return rc;
}

saj_returnCode
saj_OwnershipQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_ownershipQosPolicy *dst)
{
    jobject javaKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(ownershipQosPolicy_kind_fid));
        saj_exceptionCheck(env);
        rc = saj_EnumCopyIn(env, javaKind, (gapi_unsigned_long *) &(dst->kind));
    (*env)->DeleteLocalRef (env, javaKind);
    }

    return rc;
}

saj_returnCode
saj_OwnershipStrengthQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_ownershipStrengthQosPolicy *dst)
{
    assert(dst != NULL);

    if (dst != NULL)
    {
        dst->value =
            (*env)->GetIntField(env, src,
                GET_CACHED(ownershipStrengthQosPolicy_value_fid));
        saj_exceptionCheck(env);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_InvalidSampleVisibilityQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_invalidSampleVisibilityQosPolicy *dst)
{
    jobject kind = NULL;
    saj_returnCode rc;

    assert(dst != NULL);
    rc = SAJ_RETCODE_OK;

    if (src != NULL) {
        kind = (*env)->GetObjectField(env, src,
            GET_CACHED(invalidSampleVisibilityQosPolicy_kind_fid)
        );
        saj_exceptionCheck(env);

        rc = saj_EnumCopyIn(env, kind, (gapi_unsigned_long*) &(dst->kind));

        (*env)->DeleteLocalRef(env, kind);
    }

    return rc;
}

saj_returnCode
saj_ReaderDataLifecycleQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_readerDataLifecycleQosPolicy *dst)
{
    jobject duration;
    jobject invalid_sample_visibility;
    saj_returnCode rc;

    assert(dst != NULL);

    duration = NULL;
    invalid_sample_visibility = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        duration = (*env)->GetObjectField(env, src,
            GET_CACHED(
                readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay_fid)
            );
        saj_exceptionCheck(env);
        rc = saj_durationCopyIn(
            env, duration, &(dst->autopurge_nowriter_samples_delay));
    (*env)->DeleteLocalRef (env, duration);

        if (rc == SAJ_RETCODE_OK) {
            duration = (*env)->GetObjectField(env, src,
                GET_CACHED(
                    readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay_fid)
                );
            saj_exceptionCheck(env);
            rc = saj_durationCopyIn(
                env, duration, &(dst->autopurge_disposed_samples_delay));
            (*env)->DeleteLocalRef (env, duration);
        }
        if (rc == SAJ_RETCODE_OK) {
            dst->enable_invalid_samples = (*env)->GetBooleanField(
                env,
                src,
                GET_CACHED(
                    readerDataLifecycleQosPolicy_enable_invalid_samples_fid
                )
            );
        }

        if (rc == SAJ_RETCODE_OK) {
            invalid_sample_visibility = (*env)->GetObjectField(env, src,
                GET_CACHED(
                    readerDataLifecycleQosPolicy_invalid_sample_visibility_fid)
                );
            saj_exceptionCheck(env);
            rc = saj_InvalidSampleVisibilityQosPolicyCopyIn(
                env, invalid_sample_visibility, &(dst->invalid_sample_visibility));
        }
    }

    return rc;
}

saj_returnCode
saj_ReaderLifespanQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_readerLifespanQosPolicy *dst)
{
    saj_returnCode rc;

    jobject duration;

    assert(dst != NULL);

    duration = NULL;

    rc = SAJ_RETCODE_OK;


    if(src != NULL)
    {
        dst->use_lifespan = (*env)->GetBooleanField(env, src,
                 GET_CACHED(readerLifespanQosPolicy_useLifespan_fid));
        saj_exceptionCheck(env);


        duration = (*env)->GetObjectField(env, src, GET_CACHED(readerLifespanQosPolicy_duration_fid));
        saj_exceptionCheck(env);

        if(duration != NULL){
            saj_durationCopyIn(env, duration, &dst->duration);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, duration);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ReaderLifespanQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_readerLifespanQosPolicy *src,
    jobject                 *dst)
{
    jboolean use_lifespan;
    jobject duration;
    saj_returnCode rc;

    assert(src != NULL);

    duration = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/ReaderLifespanQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        if (src->use_lifespan == 0) {
            use_lifespan = JNI_FALSE;
        } else {
            use_lifespan = JNI_TRUE;
        }

        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(readerLifespanQosPolicy_useLifespan_fid),
            use_lifespan);
        saj_exceptionCheck(env);

        rc = saj_create_new_java_object(
                env, "DDS/Duration_t", &duration);

        if (rc == SAJ_RETCODE_OK)
        {
            saj_durationCopyOut(env, &src->duration, &duration);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(readerLifespanQosPolicy_duration_fid),
                duration
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, duration);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ShareQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_shareQosPolicy *dst)
{
    saj_returnCode rc;
    jstring share_name;
    const char *string_data;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if(src != NULL) {
        dst->enable = (*env)->GetBooleanField(env, src, GET_CACHED(shareQosPolicy_enable_fid));
        saj_exceptionCheck(env);

        share_name = (jstring)((*env)->GetObjectField(env, src, GET_CACHED(shareQosPolicy_name_fid)));
        saj_exceptionCheck(env);

        if(share_name != NULL){
            string_data = (*env)->GetStringUTFChars(env, share_name, 0);
            saj_exceptionCheck(env);
            dst->name = gapi_string_dup(string_data);
            (*env)->ReleaseStringUTFChars(env, share_name, string_data);
            saj_exceptionCheck(env);

            (*env)->DeleteLocalRef (env, share_name);
        }
    }
    return rc;
}

saj_returnCode
saj_ShareQosPolicyCopyOut(
    JNIEnv *env,
    gapi_shareQosPolicy *src,
    jobject *dst)
{
    jboolean value;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    /* create a new java ShareQosPolicy object */
    if(*dst == NULL){
        rc = saj_create_new_java_object(env, "DDS/ShareQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK) {
        if (src->enable == 0) {
            value = JNI_FALSE;
        } else {
            value = JNI_TRUE;
        }

        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(shareQosPolicy_enable_fid),
            value);
        saj_exceptionCheck(env);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        jobject javaString = NULL;
        if (src != NULL && src->name != NULL) {
            javaString = (*env)->NewStringUTF(env, src->name);
            saj_exceptionCheck(env);
        }

        if(javaString == NULL){ /* src->name was also NULL */
            javaString = (*env)->NewStringUTF(env, "");
        }

        if (javaString != NULL) /* not a null string */
        {
           /* store the string object in the string field */
           (*env)->SetObjectField(env, *dst, GET_CACHED(shareQosPolicy_name_fid), javaString);
           saj_exceptionCheck(env);

        }
        else
        {
            rc = SAJ_RETCODE_ERROR;
        }
        (*env)->DeleteLocalRef (env, javaString);
    }

    return rc;
}

saj_returnCode
saj_SubscriptionKeyQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_subscriptionKeyQosPolicy *dst)
{
    jobjectArray jStringArray = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        dst->use_key_list = (*env)->GetBooleanField(env, src,
                         GET_CACHED(subscriptionKeyQosPolicy_useKeyList_fid));
                saj_exceptionCheck(env);

        jStringArray =
            (*env)->GetObjectField(env, src,
                GET_CACHED(subscriptionKeyQosPolicy_keyList_fid));
        saj_exceptionCheck(env);
        rc = saj_stringSequenceCopyIn(env, jStringArray, &(dst->key_list));
        (*env)->DeleteLocalRef (env, jStringArray);
    }

    return rc;
}

saj_returnCode
saj_ViewKeyQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_viewKeyQosPolicy *dst)
{
    jobjectArray jStringArray = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        dst->use_key_list = (*env)->GetBooleanField(env, src,
                         GET_CACHED(viewKeyQosPolicy_useKeyList_fid));
                saj_exceptionCheck(env);

        jStringArray =
            (*env)->GetObjectField(env, src,
                GET_CACHED(viewKeyQosPolicy_keyList_fid));
        saj_exceptionCheck(env);
        rc = saj_stringSequenceCopyIn(env, jStringArray, &(dst->key_list));
        (*env)->DeleteLocalRef (env, jStringArray);
    }

    return rc;
}

saj_returnCode
saj_TimeBasedFilterQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    gapi_timeBasedFilterQosPolicy *dst)
{
    jobject duration;
    saj_returnCode rc;

    assert(dst != NULL);

    duration = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        duration = (*env)->GetObjectField(env, src,
            GET_CACHED(timeBasedFilterQosPolicy_minimumSeparation_fid));
        saj_exceptionCheck(env);
        rc = saj_durationCopyIn(env, duration, &(dst->minimum_separation));
        (*env)->DeleteLocalRef (env, duration);
    }

    return rc;
}

saj_returnCode
saj_WriterDataLifecycleQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_writerDataLifecycleQosPolicy *dst)
{
    jobject assdDuration;
    jobject aidDuration;

    assert(dst != NULL);

    assdDuration = NULL;
    aidDuration = NULL;

    if(src != NULL)
    {
        dst->autodispose_unregistered_instances = (*env)->GetBooleanField(
            env,
            src,
            GET_CACHED(
                writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances_fid
            )
        );
        saj_exceptionCheck(env);

        assdDuration = (*env)->GetObjectField(env, src,
            GET_CACHED(writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay_fid));
        saj_exceptionCheck(env);
        saj_durationCopyIn(env, assdDuration, &(dst->autopurge_suspended_samples_delay));

        aidDuration = (*env)->GetObjectField(env, src,
           GET_CACHED(writerDataLifecycleQosPolicy_autounregisterInstanceDelay_fid));
        saj_exceptionCheck(env);
        saj_durationCopyIn(env, aidDuration, &(dst->autounregister_instance_delay));

        (*env)->DeleteLocalRef (env, assdDuration);
        (*env)->DeleteLocalRef (env, aidDuration);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_SchedulingClassQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_schedulingClassQosPolicy *dst)
{
    jobject javaSchedulingClassQosKind = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaSchedulingClassQosKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(schedulingClassQosPolicy_kind_fid));
        saj_exceptionCheck(env);

        rc = saj_EnumCopyIn(env, javaSchedulingClassQosKind,
                                    (gapi_unsigned_long *) &(dst->kind));

    (*env)->DeleteLocalRef (env, javaSchedulingClassQosKind);
    }
    return rc;
}

saj_returnCode
saj_SchedulingPriorityQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_schedulingPriorityQosPolicy *dst)
{
    jobject javaSchedulingPriorityQosKind = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        javaSchedulingPriorityQosKind =
            (*env)->GetObjectField(env, src,
                GET_CACHED(schedulingPriorityQosPolicy_kind_fid));
        saj_exceptionCheck(env);

        rc = saj_EnumCopyIn(env, javaSchedulingPriorityQosKind,
                                    (gapi_unsigned_long *) &(dst->kind));

    (*env)->DeleteLocalRef (env, javaSchedulingPriorityQosKind);
    }
    return rc;
}

saj_returnCode
saj_SchedulingQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_schedulingQosPolicy *dst)
{
    jobject policy;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;
    if(src != NULL)
    {
        dst->scheduling_priority = (*env)->GetIntField(env, src,
            GET_CACHED( schedulingQosPolicy_schedulingPriority_fid)
        );
        saj_exceptionCheck(env);
        policy = (*env)->GetObjectField(env, src,
            GET_CACHED(schedulingQosPolicy_schedulingClass_fid));
        saj_exceptionCheck(env);
        rc = saj_SchedulingClassQosPolicyCopyIn(env, policy, &(dst->scheduling_class));
    (*env)->DeleteLocalRef (env, policy);
        if (rc == SAJ_RETCODE_OK) {
            policy = (*env)->GetObjectField(env, src,
                GET_CACHED(schedulingQosPolicy_schedulingPriorityKind_fid));
            saj_exceptionCheck(env);
            rc = saj_SchedulingPriorityQosPolicyCopyIn(env, policy, &(dst->scheduling_priority_kind));
        (*env)->DeleteLocalRef (env, policy);
    }
    }
    return rc;
}

/*################ COPY-IN / COPY-OUT ENTITY QOS #############################*/

saj_returnCode
saj_DomainParticipantFactoryQosCopyIn(
    JNIEnv          *env,
    const jobject   src,
    gapi_domainParticipantFactoryQos *dst)
{
    jobject javaEntityFactory;
    saj_returnCode rc;

    assert(dst != NULL);

    javaEntityFactory = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
         /* get the QosPolicy attributes */
        javaEntityFactory = (*env)->GetObjectField(env, src,
            GET_CACHED(domainParticipantFactoryQos_entityFactory_fid));
        saj_exceptionCheck(env);
        rc = saj_EntityFactoryQosPolicyCopyIn(env, javaEntityFactory, &dst->entity_factory);
        (*env)->DeleteLocalRef (env, javaEntityFactory);

    }
    return rc;
}

saj_returnCode
saj_DomainParticipantFactoryQosCopyOut(
    JNIEnv      *env,
    gapi_domainParticipantFactoryQos *src,
    jobject     *dst)
{
    jobject javaEntityFactory = NULL;
    saj_returnCode rc;

    assert(dst != NULL);
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DomainParticipantFactoryQos", dst);
    }

    if(rc == SAJ_RETCODE_OK){

        rc = saj_create_new_java_object(
            env, "DDS/EntityFactoryQosPolicy", &javaEntityFactory);

        if(rc == SAJ_RETCODE_OK){
            /* copy the values of the gapi objects to the java object */
            saj_EntityFactoryQosPolicyCopyOut(
                env, &src->entity_factory, &javaEntityFactory);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(domainParticipantFactoryQos_entityFactory_fid),
                javaEntityFactory
            );
            saj_exceptionCheck(env);
            /* Free the local references to the newly create objects */
            (*env)->DeleteLocalRef(env, javaEntityFactory);
            saj_exceptionCheck(env);
        }
    }
    return rc;
}

saj_returnCode
saj_DomainParticipantQosCopyIn(
    JNIEnv          *env,
    const jobject   src,
    gapi_domainParticipantQos *dst)
{
    jobject javaUserDataQosPolicy;
    jobject javaEntityFactory;
    jobject javaWatchdogScheduling;
    jobject javaListenerScheduling;
    saj_returnCode rc;

    assert(dst != NULL);

    javaUserDataQosPolicy = NULL;
    javaEntityFactory = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
         /* get the QosPolicy attributes */
        javaUserDataQosPolicy = (*env)->GetObjectField(env, src,
            GET_CACHED(domainParticipantQos_userData_fid));
        saj_exceptionCheck(env);
        javaEntityFactory = (*env)->GetObjectField(env, src,
            GET_CACHED(domainParticipantQos_entityFactory_fid));
        saj_exceptionCheck(env);
        javaWatchdogScheduling = (*env)->GetObjectField(env, src,
            GET_CACHED(domainParticipantQos_watchdogScheduling_fid));
        saj_exceptionCheck(env);
        javaListenerScheduling = (*env)->GetObjectField(env, src,
            GET_CACHED(domainParticipantQos_listenerScheduling_fid));
        saj_exceptionCheck(env);

        rc = saj_UserDataQosPolicyCopyIn(env, javaUserDataQosPolicy, &dst->user_data);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EntityFactoryQosPolicyCopyIn(env, javaEntityFactory, &dst->entity_factory);
        }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_SchedulingQosPolicyCopyIn(env, javaWatchdogScheduling, &dst->watchdog_scheduling);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_SchedulingQosPolicyCopyIn(env, javaListenerScheduling, &dst->listener_scheduling);
    }

    (*env)->DeleteLocalRef (env, javaUserDataQosPolicy);
    (*env)->DeleteLocalRef (env, javaEntityFactory);
    (*env)->DeleteLocalRef (env, javaWatchdogScheduling);
    (*env)->DeleteLocalRef (env, javaListenerScheduling);
    }

    return rc;
}

saj_returnCode
saj_SubscriberQosCopyIn(
    JNIEnv          *env,
    const jobject   src,
    gapi_subscriberQos *dst)
{
    jobject     javaPresentationQosPolicy;
    jobject     javaPartitionQosPolicy;
    jobject     javaGroupDataQosPolicy;
    jobject     javaEntityFactoryQosPolicy;
    jobject     javaShareQosPolicy;
    saj_returnCode rc;

    assert(dst != NULL);

    javaPresentationQosPolicy = NULL;
    javaPartitionQosPolicy = NULL;
    javaGroupDataQosPolicy = NULL;
    javaEntityFactoryQosPolicy = NULL;
    javaShareQosPolicy = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        /* get the QosPolicy attributes from the java objects */
        javaPresentationQosPolicy =
            (*env)->GetObjectField(
                env,
                src,
                GET_CACHED(subscriberQos_presentation_fid)
            );
        saj_exceptionCheck(env);
        javaPartitionQosPolicy =
            (*env)->GetObjectField(env, src,
                GET_CACHED(subscriberQos_partition_fid));
        saj_exceptionCheck(env);
        javaGroupDataQosPolicy =
            (*env)->GetObjectField(
                env,
                src,
                GET_CACHED(subscriberQos_groupData_fid)
            );
        saj_exceptionCheck(env);
        javaEntityFactoryQosPolicy =
            (*env)->GetObjectField(env, src,
                GET_CACHED(subscriberQos_entityFactory_fid));
        saj_exceptionCheck(env);
        javaShareQosPolicy =
            (*env)->GetObjectField(
                env,
                src,
                GET_CACHED(subscriberQos_share_fid)
            );
        saj_exceptionCheck(env);
        /* copy the attributes from the java object to the gapi object */
        rc = saj_PresentationQosPolicyCopyIn(
                env, javaPresentationQosPolicy, &dst->presentation);
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_PartitionQosPolicyCopyIn(
                env, javaPartitionQosPolicy, &dst->partition);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_GroupDataQosPolicyCopyIn(
                env, javaGroupDataQosPolicy, &dst->group_data);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_EntityFactoryQosPolicyCopyIn(
                env, javaEntityFactoryQosPolicy, &dst->entity_factory);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ShareQosPolicyCopyIn(
                env, javaShareQosPolicy, &dst->share);
        }
    (*env)->DeleteLocalRef (env, javaPresentationQosPolicy);
    (*env)->DeleteLocalRef (env, javaPartitionQosPolicy);
    (*env)->DeleteLocalRef (env, javaGroupDataQosPolicy);
    (*env)->DeleteLocalRef (env, javaEntityFactoryQosPolicy);
    (*env)->DeleteLocalRef (env, javaShareQosPolicy);
    }

    return rc;
}

saj_returnCode
saj_TopicQosCopyIn(
    JNIEnv          *env,
    const jobject   src,
    gapi_topicQos   *dst)
{
    jobject javaTopic_data;
    jobject javaDurability;
    jobject javaDurabilityService;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaHistory;
    jobject javaResource_limits;
    jobject javaTransport_priority;
    jobject javaLifespan;
    jobject javaOwnership;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        javaTopic_data = NULL;
        javaDurability = NULL;
        javaDurabilityService = NULL;
        javaDeadline = NULL;
        javaLatency_budget = NULL;
        javaLiveliness = NULL;
        javaReliability = NULL;
        javaDestination_order = NULL;
        javaHistory = NULL;
        javaResource_limits = NULL;
        javaTransport_priority = NULL;
        javaLifespan = NULL;
        javaOwnership = NULL;

        /* get the QosPolicy attributes from the java objects */
        javaTopic_data =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_topicData_fid));
        saj_exceptionCheck(env);
        javaDurability =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_durability_fid));
        saj_exceptionCheck(env);
        javaDurabilityService =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_durabilityService_fid));
        saj_exceptionCheck(env);
        javaDeadline =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_deadline_fid));
        saj_exceptionCheck(env);
        javaLatency_budget =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_latencyBudget_fid));
        saj_exceptionCheck(env);
        javaLiveliness =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_liveliness_fid));
        saj_exceptionCheck(env);
        javaReliability =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_reliability_fid));
        saj_exceptionCheck(env);
        javaDestination_order =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_destinationOrder_fid));
        saj_exceptionCheck(env);
        javaHistory =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_history_fid));
        saj_exceptionCheck(env);
        javaResource_limits =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_resourceLimits_fid));
        saj_exceptionCheck(env);
        javaTransport_priority =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_transportPriority_fid));
        saj_exceptionCheck(env);
        javaLifespan =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_lifespan_fid));
        saj_exceptionCheck(env);
        javaOwnership =
            (*env)->GetObjectField(env, src, GET_CACHED(topicQos_ownership_fid));
        saj_exceptionCheck(env);
        /* copy the attributes from the java object to the gapi object */
        rc = saj_TopicDataQosPolicyCopyIn(env, javaTopic_data, &dst->topic_data);
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_DurabilityQosPolicyCopyIn(env, javaDurability, &dst->durability);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_DurabilityServiceQosPolicyCopyIn(env, javaDurabilityService, &dst->durability_service);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_DeadlineQosPolicyCopyIn(env, javaDeadline, &dst->deadline);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_LatencyBudgetQosPolicyCopyIn(env, javaLatency_budget, &dst->latency_budget);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_LivelinessQosPolicyCopyIn(env, javaLiveliness, &dst->liveliness);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_ReliabilityQosPolicyCopyIn(env, javaReliability, &dst->reliability);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_DestinationOrderQosPolicyCopyIn(env, javaDestination_order, &dst->destination_order);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_HistoryQosPolicyCopyIn(env, javaHistory, &dst->history);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_ResourceLimitsQosPolicyCopyIn(env, javaResource_limits, &dst->resource_limits);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_TransportPriorityQosPolicyCopyIn(env, javaTransport_priority, &dst->transport_priority);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_LifespanQosPolicyCopyIn(env, javaLifespan, &dst->lifespan);
        }
        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_OwnershipQosPolicyCopyIn(env, javaOwnership, &dst->ownership);
        }
        (*env)->DeleteLocalRef (env, javaTopic_data);
        (*env)->DeleteLocalRef (env, javaDurability);
        (*env)->DeleteLocalRef (env, javaDurabilityService);
        (*env)->DeleteLocalRef (env, javaDeadline);
        (*env)->DeleteLocalRef (env, javaLatency_budget);
        (*env)->DeleteLocalRef (env, javaLiveliness);
        (*env)->DeleteLocalRef (env, javaReliability);
        (*env)->DeleteLocalRef (env, javaDestination_order);
        (*env)->DeleteLocalRef (env, javaHistory);
        (*env)->DeleteLocalRef (env, javaResource_limits);
        (*env)->DeleteLocalRef (env, javaTransport_priority);
        (*env)->DeleteLocalRef (env, javaLifespan);
        (*env)->DeleteLocalRef (env, javaOwnership);
    }
    return rc;
}

saj_returnCode
saj_DestinationOrderQosPolicyCopyOut(
    JNIEnv                          *env,
    gapi_destinationOrderQosPolicy  *src,
    jobject                         *dst)
{
    jobject javaKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/DestinationOrderQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_EnumCopyOut(
            env, "DDS/DestinationOrderQosPolicyKind", src->kind, &javaKind);

        if (rc == SAJ_RETCODE_OK)
        {
            /* store the new java object in the DestinationOrderQosPolicyKind obj */
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(destinationOrderQosPolicy_kind_fid),
                javaKind);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_DurabilityQosPolicyCopyOut(
    JNIEnv                      *env,
    gapi_durabilityQosPolicy    *src,
    jobject                     *dst)
{
    jobject javaDurabilityQosKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaDurabilityQosKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DurabilityQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_EnumCopyOut(
            env,
            "DDS/DurabilityQosPolicyKind",
            src->kind,
            &javaDurabilityQosKind
        );

        if (rc == SAJ_RETCODE_OK)
        {
            (*env)->SetObjectField(env, *dst,
                GET_CACHED(durabilityQosPolicy_kind_fid),
                javaDurabilityQosKind);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaDurabilityQosKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_DurabilityServiceQosPolicyCopyOut(
    JNIEnv                             *env,
    gapi_durabilityServiceQosPolicy    *src,
    jobject                            *dst)
{
    jobject javaHistoryQosKind;
    jobject javaServiceCleanupDelay;
    saj_returnCode rc;

    assert(dst != NULL);

    javaHistoryQosKind = NULL;
    javaServiceCleanupDelay = NULL;;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DurabilityServiceQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
            env, "DDS/Duration_t", &javaServiceCleanupDelay);

        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_EnumCopyOut(
                env,
                "DDS/HistoryQosPolicyKind",
                src->history_kind,
                &javaHistoryQosKind
            );

            if (rc == SAJ_RETCODE_OK)
            {
                saj_durationCopyOut(
                    env,
                    &src->service_cleanup_delay,
                    &javaServiceCleanupDelay
                );

                (*env)->SetObjectField(env, *dst,
                    GET_CACHED(durabilityServiceQosPolicy_historyKind_fid),
                    javaHistoryQosKind);
                saj_exceptionCheck(env);

               (*env)->SetIntField(
                    env,
                    *dst,
                    GET_CACHED(durabilityServiceQosPolicy_historyDepth_fid),
                    src->history_depth
                );
               (*env)->SetIntField(
                    env,
                    *dst,
                    GET_CACHED(durabilityServiceQosPolicy_maxSamples_fid),
                    src->max_samples
                );
                saj_exceptionCheck(env);
                (*env)->SetIntField(
                    env,
                    *dst,
                    GET_CACHED(durabilityServiceQosPolicy_maxInstances_fid),
                    src->max_instances
                );
                saj_exceptionCheck(env);
                (*env)->SetIntField(
                    env,
                    *dst,
                    GET_CACHED(durabilityServiceQosPolicy_maxSamplesPerInstance_fid),
                    src->max_samples_per_instance
                );
                saj_exceptionCheck(env);


                (*env)->SetObjectField(env, *dst,
                    GET_CACHED(durabilityServiceQosPolicy_serviceCleanupDelay_fid),
                    javaServiceCleanupDelay);
                saj_exceptionCheck(env);
            }
        }
    }

    (*env)->DeleteLocalRef(env, javaHistoryQosKind);
    saj_exceptionCheck(env);
    (*env)->DeleteLocalRef(env, javaServiceCleanupDelay);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_GroupDataQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_groupDataQosPolicy *src,
    jobject                 *dst)
{
    jbyteArray groupDataQosPolicy_value;
    saj_returnCode rc;

    assert(dst != NULL);

    groupDataQosPolicy_value = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/GroupDataQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_octetSequenceCopyOut(env, &src->value, &groupDataQosPolicy_value);

        if (rc == SAJ_RETCODE_OK)
        {
            /* store the attribute in the GroupDataQosPolicy object */
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(groupDataQosPolicy_value_fid),
                groupDataQosPolicy_value
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, groupDataQosPolicy_value);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_HistoryQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_historyQosPolicy   *src,
    jobject                 *dst)
{
    jobject javaKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/HistoryQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* convert from gapi to java HistoryQosPolicyKind */
        rc = saj_EnumCopyOut(
            env, "DDS/HistoryQosPolicyKind", src->kind, &javaKind);

        if(rc == SAJ_RETCODE_OK)
        {
            /* store the attributes in the HistoryQosPolicy object */
            (*env)->SetIntField(
                env,
                *dst,
                GET_CACHED(historyQosPolicy_depth_fid),
                src->depth
            );
            saj_exceptionCheck(env);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(historyQosPolicy_kind_fid),
                javaKind
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_LifespanQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_lifespanQosPolicy  *src,
    jobject                 *dst)
{
    jobject duration;
    saj_returnCode rc;

    assert(dst != NULL);

    duration = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/LifespanQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
                env, "DDS/Duration_t", &duration);

        if (rc == SAJ_RETCODE_OK)
        {
            saj_durationCopyOut(env, &src->duration, &duration);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(lifespanQosPolicy_duration_fid),
                duration
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, duration);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_OwnershipQosPolicyCopyOut(
    JNIEnv *env,
    gapi_ownershipQosPolicy *src,
    jobject *dst)
{
    jobject javaKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/OwnershipQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* convert gapi to java HistoryQosPolicyKind */
        rc = saj_EnumCopyOut(
            env, "DDS/OwnershipQosPolicyKind", src->kind, &javaKind);

        if(rc == SAJ_RETCODE_OK)
        {
            /* store the attributes in the HistoryQosPolicy object */
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(ownershipQosPolicy_kind_fid),
                javaKind
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_OwnershipStrengthQosPolicyCopyOut(
    JNIEnv *env,
    gapi_ownershipStrengthQosPolicy *src,
    jobject *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/OwnershipStrengthQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        (*env)->SetIntField(
            env,
            *dst,
            GET_CACHED(ownershipStrengthQosPolicy_value_fid),
            src->value
        );
        saj_exceptionCheck(env);
    }

    return rc;
}

saj_returnCode
saj_PartitionQosPolicyCopyOut(
    JNIEnv                  *env,
    gapi_partitionQosPolicy *src,
    jobject                 *dst)
{
    jobjectArray jStringArray;
    saj_returnCode rc;

    assert(dst != NULL);

    jStringArray = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/PartitionQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_stringSequenceCopyOut(env, src->name, &jStringArray);

        if (rc == SAJ_RETCODE_OK)
        {
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(partitionQosPolicy_name_fid),
                jStringArray
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, jStringArray);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_PresentationQosPolicyCopyOut(
    JNIEnv  *env,
    gapi_presentationQosPolicy  *src,
    jobject *dst)
{
    jobject javaKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/PresentationQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_EnumCopyOut(
            env,
            "DDS/PresentationQosPolicyAccessScopeKind",
            src->access_scope,
            &javaKind
        );

        if (rc == SAJ_RETCODE_OK)
        {
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(presentationQosPolicy_accessScope_fid),
                javaKind
            );
            saj_exceptionCheck(env);
            (*env)->SetBooleanField(
                env,
                *dst,
                GET_CACHED(presentationQosPolicy_coherentAccess_fid),
                src->coherent_access
            );
            saj_exceptionCheck(env);
            (*env)->SetBooleanField(
                env,
                *dst,
                GET_CACHED(presentationQosPolicy_orderedAccess_fid),
                src->ordered_access
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_InvalidSampleVisibilityQosPolicyCopyOut(
    JNIEnv *env,
    gapi_invalidSampleVisibilityQosPolicy *src,
    jobject *dst)
{
    jobject kind;
    saj_returnCode rc;

    assert(dst != NULL);

    kind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/InvalidSampleVisibilityQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK) {
        rc = saj_EnumCopyOut(
            env,
            "DDS/InvalidSampleVisibilityQosPolicyKind",
            src->kind,
            &kind
        );

        if (rc == SAJ_RETCODE_OK) {
            (*env)->SetObjectField(env, *dst,
                GET_CACHED(invalidSampleVisibilityQosPolicy_kind_fid),
                kind);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, kind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ReaderDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    gapi_readerDataLifecycleQosPolicy *src,
    jobject *dst)
{
    jobject duration;
    jobject invalid_sample_visibility;
    saj_returnCode rc;

    assert(dst != NULL);

    duration = NULL;
    invalid_sample_visibility = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/ReaderDataLifecycleQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
                env, "DDS/Duration_t", &duration);

        if (rc == SAJ_RETCODE_OK)
        {
            saj_durationCopyOut(
                env, &src->autopurge_nowriter_samples_delay, &duration);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay_fid),
                duration
            );
            saj_exceptionCheck(env);
            (*env)->DeleteLocalRef(env, duration);
            saj_exceptionCheck(env);
        }
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
                env, "DDS/Duration_t", &duration);

        if (rc == SAJ_RETCODE_OK)
        {
            saj_durationCopyOut(
                env, &src->autopurge_disposed_samples_delay, &duration);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay_fid),
                duration
            );
            saj_exceptionCheck(env);
            (*env)->DeleteLocalRef(env, duration);
            saj_exceptionCheck(env);
        }
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
                env, "DDS/InvalidSampleVisibilityQosPolicy", &invalid_sample_visibility);

        if (rc == SAJ_RETCODE_OK)
        {
            saj_InvalidSampleVisibilityQosPolicyCopyOut(
                env, &src->invalid_sample_visibility, &invalid_sample_visibility);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(readerDataLifecycleQosPolicy_invalid_sample_visibility_fid),
                invalid_sample_visibility
            );
            saj_exceptionCheck(env);
            (*env)->DeleteLocalRef(env, invalid_sample_visibility);
            saj_exceptionCheck(env);
        }
    }

    if (rc == SAJ_RETCODE_OK) {
        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(readerDataLifecycleQosPolicy_enable_invalid_samples_fid),
            src->enable_invalid_samples
        );
        saj_exceptionCheck(env);
    }

    return rc;
}

saj_returnCode
saj_SubscriptionKeyQosPolicyCopyOut(
    JNIEnv *env,
    gapi_subscriptionKeyQosPolicy *src,
    jobject *dst)
{
    jobjectArray jStringArray;
    saj_returnCode rc;
    jboolean value;

    assert(dst != NULL);

    jStringArray = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
       rc = saj_create_new_java_object(env, "DDS/SubscriptionKeyQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        if (src->use_key_list == 0) {
            value = JNI_FALSE;
        } else {
            value = JNI_TRUE;
        }

        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(subscriptionKeyQosPolicy_useKeyList_fid),
            value);
        saj_exceptionCheck(env);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_stringSequenceCopyOut(env, src->key_list, &jStringArray);

        if (rc == SAJ_RETCODE_OK)
        {
           (*env)->SetObjectField(
               env,
               *dst,
               GET_CACHED(subscriptionKeyQosPolicy_keyList_fid),
               jStringArray
           );
           saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, jStringArray);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ViewKeyQosPolicyCopyOut(
    JNIEnv *env,
    gapi_viewKeyQosPolicy *src,
    jobject *dst)
{
    jobjectArray jStringArray;
    saj_returnCode rc;
    jboolean value;

    assert(dst != NULL);

    jStringArray = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
       rc = saj_create_new_java_object(env, "DDS/ViewKeyQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        if (src->use_key_list == 0) {
            value = JNI_FALSE;
        } else {
            value = JNI_TRUE;
        }

        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(viewKeyQosPolicy_useKeyList_fid),
            value);
        saj_exceptionCheck(env);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_stringSequenceCopyOut(env, src->key_list, &jStringArray);

        if (rc == SAJ_RETCODE_OK)
        {
           (*env)->SetObjectField(
               env,
               *dst,
               GET_CACHED(viewKeyQosPolicy_keyList_fid),
               jStringArray
           );
           saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, jStringArray);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ReliabilityQosPolicyCopyOut(
    JNIEnv *env,
    gapi_reliabilityQosPolicy *src,
    jobject *dst)
{
    jobject javaKind;
    jobject javaMaxBlockingTime;
    jboolean javaSynchronous;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKind = NULL;
    javaMaxBlockingTime = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/ReliabilityQosPolicy", dst);
    }

    if (!src->synchronous) {
        javaSynchronous = JNI_FALSE;
    } else {
        javaSynchronous = JNI_TRUE;
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
            env, "DDS/Duration_t", &javaMaxBlockingTime);

        if (rc == SAJ_RETCODE_OK)
        {
            rc = saj_EnumCopyOut(
                env, "DDS/ReliabilityQosPolicyKind", src->kind, &javaKind);

            if (rc == SAJ_RETCODE_OK)
            {
                saj_durationCopyOut(
                    env, &src->max_blocking_time, &javaMaxBlockingTime);

                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(reliabilityQosPolicy_kind_fid),
                    javaKind
                );
                saj_exceptionCheck(env);
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(reliabilityQosPolicy_maxBlockingTime_fid),
                    javaMaxBlockingTime
                );
                saj_exceptionCheck(env);
                (*env)->SetBooleanField(
                    env,
                    *dst,
                    GET_CACHED(reliabilityQosPolicy_synchronous_fid),
                    javaSynchronous);
                saj_exceptionCheck(env);
            }
        }
    }

    (*env)->DeleteLocalRef(env, javaKind);
    saj_exceptionCheck(env);
    (*env)->DeleteLocalRef(env, javaMaxBlockingTime);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_ResourceLimitsQosPolicyCopyOut(
    JNIEnv *env,
    gapi_resourceLimitsQosPolicy *src,
    jobject *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/ResourceLimitsQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        (*env)->SetIntField(
            env,
            *dst,
            GET_CACHED(resourceLimitsQosPolicy_maxSamples_fid),
            src->max_samples
        );
        saj_exceptionCheck(env);
        (*env)->SetIntField(
            env,
            *dst,
            GET_CACHED(resourceLimitsQosPolicy_maxInstances_fid),
            src->max_instances
        );
        saj_exceptionCheck(env);
        (*env)->SetIntField(
            env,
            *dst,
            GET_CACHED(resourceLimitsQosPolicy_maxSamplesPerInstance_fid),
            src->max_samples_per_instance
        );
        saj_exceptionCheck(env);
    }
    return rc;
}

saj_returnCode
saj_TimeBasedFilterQosPolicyCopyOut(
    JNIEnv *env,
    gapi_timeBasedFilterQosPolicy *src,
    jobject *dst)
{
    jobject duration;
    saj_returnCode rc;

    assert(dst != NULL);

    duration = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/TimeBasedFilterQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_create_new_java_object(
                env, "DDS/Duration_t", &duration);

        if (rc == SAJ_RETCODE_OK)
        {
            saj_durationCopyOut(
                env, &src->minimum_separation, &duration);

            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(timeBasedFilterQosPolicy_minimumSeparation_fid),
                duration
            );
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, duration);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_TopicDataQosPolicyCopyOut(
    JNIEnv *env,
    gapi_topicDataQosPolicy *src,
    jobject *dst)
{
    jbyteArray value;
    saj_returnCode rc;

    assert(dst != NULL);
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/TopicDataQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = (saj_octetSequenceCopyOut(env, &src->value, &value));

        if (rc == SAJ_RETCODE_OK) {
            (*env)->SetObjectField(
                env,
                *dst,
                GET_CACHED(topicDataQosPolicy_value_fid),
                value
            );
            saj_exceptionCheck(env);

            (*env)->DeleteLocalRef(env, value);
            saj_exceptionCheck(env);
        }
    }

    return rc;


}

saj_returnCode
saj_TransportPriorityQosPolicyCopyOut(
    JNIEnv *env,
    gapi_transportPriorityQosPolicy *src,
    jobject *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/TransportPriorityQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        (*env)->SetIntField(
            env,
            *dst,
            GET_CACHED(transportPriorityQosPolicy_value_fid),
            src->value
        );
        saj_exceptionCheck(env);
    }
    return rc;
}

saj_returnCode
saj_WriterDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    gapi_writerDataLifecycleQosPolicy *src,
    jobject *dst)
{
    saj_returnCode rc;
    jobject assdDuration;
    jobject aidDuration;

    assert(dst != NULL);

    assdDuration = NULL;
    aidDuration = NULL;

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(
            env, "DDS/WriterDataLifecycleQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        (*env)->SetBooleanField(
            env,
            *dst,
            GET_CACHED(writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances_fid),
            src->autodispose_unregistered_instances
        );
        saj_exceptionCheck(env);

        saj_durationCopyOut(
            env, &src->autopurge_suspended_samples_delay, &assdDuration);

        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay_fid),
            assdDuration
        );
        saj_exceptionCheck(env);

        saj_durationCopyOut(
            env, &src->autounregister_instance_delay, &aidDuration);

        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(writerDataLifecycleQosPolicy_autounregisterInstanceDelay_fid),
            aidDuration
        );
        saj_exceptionCheck(env);

        (*env)->DeleteLocalRef(env, assdDuration);
        (*env)->DeleteLocalRef(env, aidDuration);
    }

    return rc;
}

saj_returnCode
saj_SchedulingClassQosPolicyCopyOut(
    JNIEnv *env,
    gapi_schedulingClassQosPolicy *src,
    jobject *dst)
{
    jobject javaSchedulingClassQosKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaSchedulingClassQosKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/SchedulingClassQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_EnumCopyOut(
            env,
            "DDS/SchedulingClassQosPolicyKind",
            src->kind,
            &javaSchedulingClassQosKind
        );

        if (rc == SAJ_RETCODE_OK)
        {
            (*env)->SetObjectField(env, *dst,
                GET_CACHED(schedulingClassQosPolicy_kind_fid),
                javaSchedulingClassQosKind);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaSchedulingClassQosKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_SchedulingPriorityQosPolicyCopyOut(
    JNIEnv *env,
    gapi_schedulingPriorityQosPolicy *src,
    jobject *dst)
{
    jobject javaSchedulingPriorityQosKind;
    saj_returnCode rc;

    assert(dst != NULL);

    javaSchedulingPriorityQosKind = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/SchedulingPriorityQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_EnumCopyOut(
            env,
            "DDS/SchedulingPriorityQosPolicyKind",
            src->kind,
            &javaSchedulingPriorityQosKind
        );

        if (rc == SAJ_RETCODE_OK)
        {
            (*env)->SetObjectField(env, *dst,
                GET_CACHED(schedulingPriorityQosPolicy_kind_fid),
                javaSchedulingPriorityQosKind);
            saj_exceptionCheck(env);
        }
    }

    (*env)->DeleteLocalRef(env, javaSchedulingPriorityQosKind);
    saj_exceptionCheck(env);

    return rc;
}

saj_returnCode
saj_SchedulingQosPolicyCopyOut(
    JNIEnv *env,
    gapi_schedulingQosPolicy *src,
    jobject *dst)
{
    saj_returnCode rc;
    jobject javaSchedulingClassQosPolicy = NULL;
    jobject javaSchedulingPriorityQosPolicy = NULL;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/SchedulingQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK)
    {
    rc = saj_SchedulingClassQosPolicyCopyOut (env,
        &src->scheduling_class, &javaSchedulingClassQosPolicy);
    }
    if (rc == SAJ_RETCODE_OK)
    {
    rc = saj_SchedulingPriorityQosPolicyCopyOut (env,
        &src->scheduling_priority_kind, &javaSchedulingPriorityQosPolicy);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        (*env)->SetObjectField(env, *dst, GET_CACHED(schedulingQosPolicy_schedulingClass_fid),
        javaSchedulingClassQosPolicy);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(env, *dst, GET_CACHED(schedulingQosPolicy_schedulingPriorityKind_fid),
        javaSchedulingPriorityQosPolicy);
        saj_exceptionCheck(env);
        (*env)->SetIntField( env, *dst, GET_CACHED(schedulingQosPolicy_schedulingPriority_fid),
        src->scheduling_priority);
        saj_exceptionCheck(env);
    }
    (*env)->DeleteLocalRef (env, javaSchedulingClassQosPolicy);
    (*env)->DeleteLocalRef (env, javaSchedulingPriorityQosPolicy);

    return rc;
}

saj_returnCode
saj_DomainParticipantQosCopyOut(
    JNIEnv      *env,
    gapi_domainParticipantQos *src,
    jobject     *dst)
{
    jobject javaUserDataQosPolicy = NULL;
    jobject javaEntityFactory = NULL;
    jobject javaWatchdogScheduling = NULL;
    jobject javaListenerScheduling = NULL;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DomainParticipantQos", dst);
    }

    if(rc == SAJ_RETCODE_OK){
        rc = saj_create_new_java_object(env, "DDS/UserDataQosPolicy", &javaUserDataQosPolicy);

        if(rc == SAJ_RETCODE_OK){
            rc = saj_create_new_java_object(
                env, "DDS/EntityFactoryQosPolicy", &javaEntityFactory);

            if(rc == SAJ_RETCODE_OK){
                /* copy the values of the gapi objects to the java object */
                saj_UserDataQosPolicyCopyOut(
                    env, &src->user_data, &javaUserDataQosPolicy);
                saj_EntityFactoryQosPolicyCopyOut(
                    env, &src->entity_factory, &javaEntityFactory);
            saj_SchedulingQosPolicyCopyOut(
            env, &src->watchdog_scheduling, &javaWatchdogScheduling);
            saj_SchedulingQosPolicyCopyOut(
            env, &src->listener_scheduling, &javaListenerScheduling);

                /* Set DomainParticipantQos attributes to reference the new objects */
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(domainParticipantQos_userData_fid),
                    javaUserDataQosPolicy
                );
                saj_exceptionCheck(env);
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(domainParticipantQos_entityFactory_fid),
                    javaEntityFactory
                );
                saj_exceptionCheck(env);
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(domainParticipantQos_watchdogScheduling_fid),
                    javaWatchdogScheduling
                );
                saj_exceptionCheck(env);
                (*env)->SetObjectField(
                    env,
                    *dst,
                    GET_CACHED(domainParticipantQos_listenerScheduling_fid),
                    javaListenerScheduling
                );
                saj_exceptionCheck(env);
                /* Free the local references to the newly create objects */
                (*env)->DeleteLocalRef(env, javaUserDataQosPolicy);
                saj_exceptionCheck(env);
                (*env)->DeleteLocalRef(env, javaEntityFactory);
                saj_exceptionCheck(env);
                (*env)->DeleteLocalRef(env, javaWatchdogScheduling);
                saj_exceptionCheck(env);
                (*env)->DeleteLocalRef(env, javaListenerScheduling);
                saj_exceptionCheck(env);
            }
        }
    }
    return rc;
}

saj_returnCode
saj_TopicQosCopyOut(
    JNIEnv          *env,
    gapi_topicQos   *src,
    jobject         *dst)
{
    jobject javaTopic_data;
    jobject javaDurability;
    jobject javaDurabilityService;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaHistory;
    jobject javaResource_limits;
    jobject javaTransport_priority;
    jobject javaLifespan;
    jobject javaOwnership;
    saj_returnCode rc;

    assert(dst != NULL);

    javaTopic_data = NULL;
    javaDurability = NULL;
    javaDurabilityService = NULL;
    javaDeadline = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaDestination_order = NULL;
    javaHistory = NULL;
    javaResource_limits = NULL;
    javaTransport_priority = NULL;
    javaLifespan = NULL;
    javaOwnership = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/TopicQos", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_TopicDataQosPolicyCopyOut(
            env, &src->topic_data, &javaTopic_data);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_DurabilityQosPolicyCopyOut(
            env, &src->durability, &javaDurability);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_DurabilityServiceQosPolicyCopyOut(
            env, &src->durability_service, &javaDurabilityService);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_DeadlineQosPolicyCopyOut(env, &src->deadline, &javaDeadline);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_LatencyBudgetQosPolicyCopyOut(
            env, &src->latency_budget, &javaLatency_budget);
    }
    if (rc == SAJ_RETCODE_OK)
    {
         rc = saj_LivelinessQosPolicyCopyOut(env, &src->liveliness, &javaLiveliness);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReliabilityQosPolicyCopyOut(env, &src->reliability, &javaReliability);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_DestinationOrderQosPolicyCopyOut(
            env, &src->destination_order, &javaDestination_order);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_HistoryQosPolicyCopyOut(env, &src->history, &javaHistory);
    }

    saj_ResourceLimitsQosPolicyCopyOut(
        env, &src->resource_limits, &javaResource_limits);

    saj_TransportPriorityQosPolicyCopyOut(
        env, &src->transport_priority, &javaTransport_priority);

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_LifespanQosPolicyCopyOut(env, &src->lifespan, &javaLifespan);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipQosPolicyCopyOut(env, &src->ownership, &javaOwnership);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* Set the QosPolicy attributes */
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_topicData_fid), javaTopic_data);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_durability_fid), javaDurability);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_durabilityService_fid), javaDurabilityService);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_deadline_fid), javaDeadline);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_latencyBudget_fid), javaLatency_budget);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_liveliness_fid), javaLiveliness);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_reliability_fid), javaReliability);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(topicQos_destinationOrder_fid),
            javaDestination_order
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_history_fid), javaHistory);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(topicQos_resourceLimits_fid),
            javaResource_limits
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(topicQos_transportPriority_fid),
            javaTransport_priority
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_lifespan_fid), javaLifespan);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst, GET_CACHED(topicQos_ownership_fid), javaOwnership);
        saj_exceptionCheck(env);
    }
    (*env)->DeleteLocalRef (env, javaTopic_data);
    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaDurabilityService);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaHistory);
    (*env)->DeleteLocalRef (env, javaResource_limits);
    (*env)->DeleteLocalRef (env, javaTransport_priority);
    (*env)->DeleteLocalRef (env, javaLifespan);
    (*env)->DeleteLocalRef (env, javaOwnership);

    return rc;
}

saj_returnCode
saj_SubscriberQosCopyOut(
    JNIEnv *env,
    gapi_subscriberQos *src,
    jobject *dst)
{
    jobject javaPresentationQosPolicy;
    jobject javaPartitionQosPolicy;
    jobject javaGroupDataQosPolicy;
    jobject javaEntityFactoryQosPolicy;
    jobject javaShareQosPolicy;
    saj_returnCode rc;

    assert(dst != NULL);

    javaPresentationQosPolicy = NULL;
    javaPartitionQosPolicy = NULL;
    javaGroupDataQosPolicy = NULL;
    javaEntityFactoryQosPolicy = NULL;
    javaShareQosPolicy = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/SubscriberQos", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* copy the attributes from the gapi object to the java object */
        rc = saj_PresentationQosPolicyCopyOut(
                env, &src->presentation, &javaPresentationQosPolicy);
    }

    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_PartitionQosPolicyCopyOut(
            env, &src->partition, &javaPartitionQosPolicy);
    }

    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_GroupDataQosPolicyCopyOut(
            env, &src->group_data, &javaGroupDataQosPolicy);
    }
    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_EntityFactoryQosPolicyCopyOut(
                env, &src->entity_factory, &javaEntityFactoryQosPolicy);
    }
    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_ShareQosPolicyCopyOut(
                env, &src->share, &javaShareQosPolicy);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        /* set the QosPolicy attributes */
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(subscriberQos_presentation_fid),
            javaPresentationQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(subscriberQos_partition_fid),
            javaPartitionQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(subscriberQos_groupData_fid),
            javaGroupDataQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(subscriberQos_entityFactory_fid),
            javaEntityFactoryQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(subscriberQos_share_fid),
            javaShareQosPolicy
        );
        saj_exceptionCheck(env);

        (*env)->DeleteLocalRef (env, javaPresentationQosPolicy);
        (*env)->DeleteLocalRef (env, javaPartitionQosPolicy);
        (*env)->DeleteLocalRef (env, javaGroupDataQosPolicy);
        (*env)->DeleteLocalRef (env, javaEntityFactoryQosPolicy);
        (*env)->DeleteLocalRef (env, javaShareQosPolicy);
    }
    return rc;
}

saj_returnCode
saj_PublisherQosCopyOut(
    JNIEnv *env,
    gapi_publisherQos *src,
    jobject *dst)
{
    jobject javaPresentationQosPolicy;
    jobject javaPartitionQosPolicy;
    jobject javaGroupDataQosPolicy;
    jobject javaEntityFactoryQosPolicy;
    saj_returnCode rc;

    assert(dst != NULL);

    javaPresentationQosPolicy = NULL;
    javaPartitionQosPolicy = NULL;
    javaGroupDataQosPolicy = NULL;
    javaEntityFactoryQosPolicy = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/PublisherQos", dst);
    }

    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_PresentationQosPolicyCopyOut(
                env, &src->presentation, &javaPresentationQosPolicy);
    }

    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_PartitionQosPolicyCopyOut(
            env, &src->partition, &javaPartitionQosPolicy);
    }

    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_GroupDataQosPolicyCopyOut(
            env, &src->group_data, &javaGroupDataQosPolicy);
    }

    saj_EntityFactoryQosPolicyCopyOut(
        env, &src->entity_factory, &javaEntityFactoryQosPolicy);

    if (rc == SAJ_RETCODE_OK)
    {
        /* set the QosPolicy attributes */
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(publisherQos_presentation_fid),
            javaPresentationQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(publisherQos_partition_fid),
            javaPartitionQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(publisherQos_groupData_fid),
            javaGroupDataQosPolicy
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(publisherQos_entityFactory_fid),
            javaEntityFactoryQosPolicy
        );
        saj_exceptionCheck(env);
    }
    (*env)->DeleteLocalRef (env, javaPresentationQosPolicy);
    (*env)->DeleteLocalRef (env, javaPartitionQosPolicy);
    (*env)->DeleteLocalRef (env, javaGroupDataQosPolicy);
    (*env)->DeleteLocalRef (env, javaEntityFactoryQosPolicy);
    return rc;
}

saj_returnCode
saj_PublisherQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_publisherQos *dst)
{
    jobject javaPresentationQosPolicy;
    jobject javaPartitionQosPolicy;
    jobject javaGroupDataQosPolicy;
    jobject javaEntityFactoryQosPolicy;
    saj_returnCode rc;

    assert(dst != NULL);

    javaPresentationQosPolicy = NULL;
    javaPartitionQosPolicy = NULL;
    javaGroupDataQosPolicy = NULL;
    javaEntityFactoryQosPolicy = NULL;
    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        /* get the QosPolicy attributes from the java objects */
        javaPresentationQosPolicy =
            (*env)->GetObjectField(
                env,
                src,
                GET_CACHED(publisherQos_presentation_fid)
            );
        saj_exceptionCheck(env);
        javaPartitionQosPolicy =
            (*env)->GetObjectField(env, src,
                GET_CACHED(publisherQos_partition_fid));
        saj_exceptionCheck(env);
        javaGroupDataQosPolicy =
            (*env)->GetObjectField(
                env,
                src,
                GET_CACHED(publisherQos_groupData_fid)
            );
        saj_exceptionCheck(env);
        javaEntityFactoryQosPolicy =
            (*env)->GetObjectField(env, src,
                GET_CACHED(publisherQos_entityFactory_fid));
        saj_exceptionCheck(env);
        /* copy the attributes from the java object to the gapi object */
        rc = saj_PresentationQosPolicyCopyIn(
                env, javaPresentationQosPolicy, &dst->presentation);

        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_PartitionQosPolicyCopyIn(
                env, javaPartitionQosPolicy, &dst->partition);
        }

        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_GroupDataQosPolicyCopyIn(
                env, javaGroupDataQosPolicy, &dst->group_data);
        }

        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_EntityFactoryQosPolicyCopyIn(
                env, javaEntityFactoryQosPolicy, &dst->entity_factory);
        }
        (*env)->DeleteLocalRef (env, javaPresentationQosPolicy);
        (*env)->DeleteLocalRef (env, javaPartitionQosPolicy);
        (*env)->DeleteLocalRef (env, javaGroupDataQosPolicy);
        (*env)->DeleteLocalRef (env, javaEntityFactoryQosPolicy);
    }
    return rc;
}

saj_returnCode
saj_DataWriterQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_dataWriterQos *dst)
{
    jobject javaDurability;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaHistory;
    jobject javaResource_limits;
    jobject javaTransport_priority;
    jobject javaLifespan;
    jobject javaUser_data;
    jobject javaOwnership;
    jobject javaOwnership_strength;
    jobject javaWriter_data_lifecycle;
    saj_returnCode rc;

    assert(src != NULL);

    javaDurability = NULL;
    javaDeadline = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaDestination_order = NULL;
    javaHistory = NULL;
    javaResource_limits = NULL;
    javaTransport_priority = NULL;
    javaLifespan = NULL;
    javaUser_data = NULL;
    javaOwnership_strength = NULL;
    javaOwnership = NULL;
    javaWriter_data_lifecycle = NULL;
    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        /* get the QosPolicy attributes from the java objects */
        javaDurability =
            (*env)->GetObjectField( env, src, GET_CACHED(dataWriterQos_durability_fid));
        saj_exceptionCheck(env);
        javaDeadline =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_deadline_fid));
        saj_exceptionCheck(env);
        javaLatency_budget =
            (*env)->GetObjectField( env, src, GET_CACHED(dataWriterQos_latencyBudget_fid));
        saj_exceptionCheck(env);
        javaLiveliness =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_liveliness_fid));
        saj_exceptionCheck(env);
        javaReliability =
            (*env)->GetObjectField( env, src, GET_CACHED(dataWriterQos_reliability_fid));
        saj_exceptionCheck(env);
        javaDestination_order =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_destinationOrder_fid));
        saj_exceptionCheck(env);
        javaHistory =
            (*env)->GetObjectField( env, src, GET_CACHED(dataWriterQos_history_fid));
        saj_exceptionCheck(env);
        javaResource_limits =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_resourceLimits_fid));
        saj_exceptionCheck(env);
        javaTransport_priority =
            (*env)->GetObjectField( env, src, GET_CACHED(dataWriterQos_transportPriority_fid));
        saj_exceptionCheck(env);
        javaLifespan =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_lifespan_fid));
        saj_exceptionCheck(env);
        javaUser_data =
            (*env)->GetObjectField( env, src, GET_CACHED(dataWriterQos_userData_fid));
        saj_exceptionCheck(env);
        javaOwnership =
                    (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_ownership_fid));
        saj_exceptionCheck(env);
        javaOwnership_strength =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_ownershipStrength_fid));
        saj_exceptionCheck(env);
        javaWriter_data_lifecycle =
            (*env)->GetObjectField(env, src, GET_CACHED(dataWriterQos_writerDataLifecycle_fid));
        saj_exceptionCheck(env);
        /* copy the attributes from the java object to the gapi object */
        rc = saj_DurabilityQosPolicyCopyIn(env, javaDurability, &dst->durability);
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_DeadlineQosPolicyCopyIn(env, javaDeadline, &dst->deadline);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_LatencyBudgetQosPolicyCopyIn(env, javaLatency_budget, &dst->latency_budget);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_LivelinessQosPolicyCopyIn(env, javaLiveliness, &dst->liveliness);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ReliabilityQosPolicyCopyIn(env, javaReliability, &dst->reliability);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_DestinationOrderQosPolicyCopyIn(env, javaDestination_order, &dst->destination_order);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_HistoryQosPolicyCopyIn(env, javaHistory, &dst->history);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ResourceLimitsQosPolicyCopyIn(env, javaResource_limits, &dst->resource_limits);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_TransportPriorityQosPolicyCopyIn(env, javaTransport_priority, &dst->transport_priority);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_LifespanQosPolicyCopyIn(env, javaLifespan, &dst->lifespan);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_UserDataQosPolicyCopyIn(env, javaUser_data, &dst->user_data);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_OwnershipQosPolicyCopyIn(env, javaOwnership, &dst->ownership);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_OwnershipStrengthQosPolicyCopyIn(env, javaOwnership_strength, &dst->ownership_strength);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_WriterDataLifecycleQosPolicyCopyIn(env, javaWriter_data_lifecycle, &dst->writer_data_lifecycle);
        }
    }
    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaHistory);
    (*env)->DeleteLocalRef (env, javaResource_limits);
    (*env)->DeleteLocalRef (env, javaTransport_priority);
    (*env)->DeleteLocalRef (env, javaLifespan);
    (*env)->DeleteLocalRef (env, javaUser_data);
    (*env)->DeleteLocalRef (env, javaOwnership);
    (*env)->DeleteLocalRef (env, javaOwnership_strength);
    (*env)->DeleteLocalRef (env, javaWriter_data_lifecycle);

    return rc;
}

saj_returnCode
saj_DataWriterQosCopyOut(
    JNIEnv *env,
    gapi_dataWriterQos *src,
    jobject *dst)
{
    jobject javaDurability;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaHistory;
    jobject javaResource_limits;
    jobject javaTransport_priority;
    jobject javaLifespan;
    jobject javaUser_data;
    jobject javaOwnership;
    jobject javaOwnership_strength;
    jobject javaWriter_data_lifecycle;
    saj_returnCode rc;

    assert(dst != NULL);

    javaDurability = NULL;
    javaDeadline = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaDestination_order = NULL;
    javaHistory = NULL;
    javaResource_limits = NULL;
    javaTransport_priority = NULL;
    javaLifespan = NULL;
    javaUser_data = NULL;
    javaOwnership = NULL;
    javaOwnership_strength = NULL;
    javaWriter_data_lifecycle = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DataWriterQos", dst);
    }

    /* copy the attributes from the java object to the gapi object */
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DurabilityQosPolicyCopyOut(
            env, &src->durability, &javaDurability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DeadlineQosPolicyCopyOut(
            env, &src->deadline, &javaDeadline);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LatencyBudgetQosPolicyCopyOut(
            env, &src->latency_budget, &javaLatency_budget);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LivelinessQosPolicyCopyOut(
            env, &src->liveliness, &javaLiveliness);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReliabilityQosPolicyCopyOut(
            env, &src->reliability, &javaReliability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DestinationOrderQosPolicyCopyOut(
            env, &src->destination_order, &javaDestination_order);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_HistoryQosPolicyCopyOut(
            env, &src->history, &javaHistory);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        saj_ResourceLimitsQosPolicyCopyOut(
            env, &src->resource_limits, &javaResource_limits);

        saj_TransportPriorityQosPolicyCopyOut(
            env, &src->transport_priority, &javaTransport_priority);

        rc = saj_LifespanQosPolicyCopyOut(
            env, &src->lifespan, &javaLifespan);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_UserDataQosPolicyCopyOut(
            env, &src->user_data, &javaUser_data);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipQosPolicyCopyOut(
                    env, &src->ownership, &javaOwnership);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipStrengthQosPolicyCopyOut(
            env, &src->ownership_strength, &javaOwnership_strength);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_WriterDataLifecycleQosPolicyCopyOut(
            env, &src->writer_data_lifecycle, &javaWriter_data_lifecycle);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        /* Set the QosPolicy attributes from the java objects */
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_durability_fid),
            javaDurability
        );
        saj_exceptionCheck(env);

        (*env)->SetObjectField(env, *dst,
            GET_CACHED(dataWriterQos_deadline_fid), javaDeadline);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_latencyBudget_fid),
            javaLatency_budget
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(env, *dst,
            GET_CACHED(dataWriterQos_liveliness_fid), javaLiveliness);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_reliability_fid),
            javaReliability
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env, *dst,
            GET_CACHED(dataWriterQos_destinationOrder_fid),
            javaDestination_order
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_history_fid),
            javaHistory
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(env, *dst,
            GET_CACHED(dataWriterQos_resourceLimits_fid), javaResource_limits);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_transportPriority_fid),
            javaTransport_priority
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(env, *dst,
            GET_CACHED(dataWriterQos_lifespan_fid), javaLifespan);
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_userData_fid),
            javaUser_data
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_ownership_fid),
            javaOwnership
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_ownershipStrength_fid),
            javaOwnership_strength
        );
        saj_exceptionCheck(env);
        (*env)->SetObjectField(
            env,
            *dst,
            GET_CACHED(dataWriterQos_writerDataLifecycle_fid),
            javaWriter_data_lifecycle
        );
        saj_exceptionCheck(env);
    }
    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaHistory);
    (*env)->DeleteLocalRef (env, javaResource_limits);
    (*env)->DeleteLocalRef (env, javaTransport_priority);
    (*env)->DeleteLocalRef (env, javaLifespan);
    (*env)->DeleteLocalRef (env, javaUser_data);
    (*env)->DeleteLocalRef (env, javaOwnership_strength);
    (*env)->DeleteLocalRef (env, javaOwnership);
    (*env)->DeleteLocalRef (env, javaWriter_data_lifecycle);

    return rc;
}

saj_returnCode
saj_DataReaderQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_dataReaderQos *dst)
{
    jobject javaDurability;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaHistory;
    jobject javaResource_limits;
    jobject javaUser_data;
    jobject javaOwnership;
    jobject javaTime_based_filter;
    jobject javaReader_data_lifecycle;
    jobject javaShare;
    jobject javaReaderLifespan;
    jobject javaSubscriptionKeys;
    saj_returnCode rc;

    assert(src != NULL);

    javaDurability = NULL;
    javaDeadline = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaDestination_order = NULL;
    javaHistory = NULL;
    javaResource_limits = NULL;
    javaUser_data = NULL;
    javaOwnership = NULL;
    javaTime_based_filter = NULL;
    javaReader_data_lifecycle = NULL;
    javaShare = NULL;
    javaReaderLifespan = NULL;
    javaSubscriptionKeys = NULL;

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {
        /* get the QosPolicy attributes from the java objects */
        javaDurability =
            (*env)->GetObjectField( env, src, GET_CACHED(dataReaderQos_durability_fid));
        saj_exceptionCheck(env);
        javaDeadline =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_deadline_fid));
        saj_exceptionCheck(env);
        javaLatency_budget =
            (*env)->GetObjectField( env, src, GET_CACHED(dataReaderQos_latencyBudget_fid));
        saj_exceptionCheck(env);
        javaLiveliness =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_liveliness_fid));
        saj_exceptionCheck(env);
        javaReliability =
            (*env)->GetObjectField( env, src, GET_CACHED(dataReaderQos_reliability_fid));
        saj_exceptionCheck(env);
        javaDestination_order =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_destinationOrder_fid));
        saj_exceptionCheck(env);
        javaHistory =
            (*env)->GetObjectField( env, src, GET_CACHED(dataReaderQos_history_fid));
        saj_exceptionCheck(env);
        javaResource_limits =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_resourceLimits_fid));
        saj_exceptionCheck(env);
        javaUser_data =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_userData_fid));
        saj_exceptionCheck(env);
        javaOwnership =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_ownership_fid));
        saj_exceptionCheck(env);
        javaTime_based_filter =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_timeBasedFilter_fid));
        saj_exceptionCheck(env);
        javaReader_data_lifecycle =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_readerDataLifecycle_fid));
        saj_exceptionCheck(env);
        javaShare =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_share_fid));
        saj_exceptionCheck(env);
        javaReaderLifespan =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_readerLifespan_fid));
        saj_exceptionCheck(env);
        javaSubscriptionKeys =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderQos_subscriptionKeys_fid));
        saj_exceptionCheck(env);
        /* copy the attributes from the java object to the gapi object */
        rc = saj_DurabilityQosPolicyCopyIn(env, javaDurability, &dst->durability);

        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_DeadlineQosPolicyCopyIn(env, javaDeadline, &dst->deadline);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_LatencyBudgetQosPolicyCopyIn(env, javaLatency_budget, &dst->latency_budget);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_LivelinessQosPolicyCopyIn(env, javaLiveliness, &dst->liveliness);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ReliabilityQosPolicyCopyIn(env, javaReliability, &dst->reliability);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_DestinationOrderQosPolicyCopyIn(env, javaDestination_order, &dst->destination_order);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_HistoryQosPolicyCopyIn(env, javaHistory, &dst->history);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ResourceLimitsQosPolicyCopyIn(env, javaResource_limits, &dst->resource_limits);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_UserDataQosPolicyCopyIn(env, javaUser_data, &dst->user_data);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_OwnershipQosPolicyCopyIn(env, javaOwnership, &dst->ownership);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_TimeBasedFilterQosPolicyCopyIn(env, javaTime_based_filter, &dst->time_based_filter);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ReaderDataLifecycleQosPolicyCopyIn(env, javaReader_data_lifecycle, &dst->reader_data_lifecycle);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            /* TODO: Add support for subscriber-defined-keyson Java API,
             * currently pass NULL */
            rc = saj_SubscriptionKeyQosPolicyCopyIn(env, javaSubscriptionKeys, &dst->subscription_keys);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ReaderLifespanQosPolicyCopyIn(env, javaReaderLifespan, &dst->reader_lifespan);
        }
        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ShareQosPolicyCopyIn(env, javaShare, &dst->share);
        }
    }
    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaHistory);
    (*env)->DeleteLocalRef (env, javaResource_limits);
    (*env)->DeleteLocalRef (env, javaUser_data);
    (*env)->DeleteLocalRef (env, javaOwnership);
    (*env)->DeleteLocalRef (env, javaTime_based_filter);
    (*env)->DeleteLocalRef (env, javaReader_data_lifecycle);
    (*env)->DeleteLocalRef (env, javaShare);
    (*env)->DeleteLocalRef (env, javaReaderLifespan);
    (*env)->DeleteLocalRef (env, javaSubscriptionKeys);

    return rc;
}

saj_returnCode
saj_DataReaderQosCopyOut(
    JNIEnv *env,
    gapi_dataReaderQos *src,
    jobject *dst)
{
    jobject javaDurability;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaHistory;
    jobject javaResource_limits;
    jobject javaUser_data;
    jobject javaOwnership;
    jobject javaTime_based_filter;
    jobject javaReader_data_lifecycle;
    jobject javaShare;
    jobject javaReaderLifespan;
    jobject javaSubscriptionKeys;
    saj_returnCode rc;

    assert(dst != NULL);

    javaDurability = NULL;
    javaDeadline = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaDestination_order = NULL;
    javaHistory = NULL;
    javaResource_limits = NULL;
    javaUser_data = NULL;
    javaOwnership = NULL;
    javaTime_based_filter = NULL;
    javaReader_data_lifecycle = NULL;
    javaShare = NULL;
    javaReaderLifespan = NULL;
    javaSubscriptionKeys = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DataReaderQos", dst);
    }

    /* copy the attributes from the gapi object to the java object */
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DurabilityQosPolicyCopyOut(
            env, &src->durability, &javaDurability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DeadlineQosPolicyCopyOut(
            env, &src->deadline, &javaDeadline);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LatencyBudgetQosPolicyCopyOut(
            env, &src->latency_budget, &javaLatency_budget);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LivelinessQosPolicyCopyOut(
            env, &src->liveliness, &javaLiveliness);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReliabilityQosPolicyCopyOut(
            env, &src->reliability, &javaReliability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DestinationOrderQosPolicyCopyOut(
            env, &src->destination_order, &javaDestination_order);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_HistoryQosPolicyCopyOut(
            env, &src->history, &javaHistory);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        saj_ResourceLimitsQosPolicyCopyOut(
            env, &src->resource_limits, &javaResource_limits);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_UserDataQosPolicyCopyOut(
            env, &src->user_data, &javaUser_data);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipQosPolicyCopyOut(
            env, &src->ownership, &javaOwnership);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_TimeBasedFilterQosPolicyCopyOut(
            env, &src->time_based_filter, &javaTime_based_filter);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReaderDataLifecycleQosPolicyCopyOut(
            env, &src->reader_data_lifecycle, &javaReader_data_lifecycle);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_SubscriptionKeyQosPolicyCopyOut(
            env, &src->subscription_keys, &javaSubscriptionKeys);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ShareQosPolicyCopyOut(
            env, &src->share, &javaShare);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReaderLifespanQosPolicyCopyOut(
            env, &src->reader_lifespan, &javaReaderLifespan);
    }

    /* set the QosPolicy attributes from the java objects */
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_durability_fid),
        javaDurability
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_deadline_fid), javaDeadline);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_latencyBudget_fid),
        javaLatency_budget
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_liveliness_fid), javaLiveliness);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_reliability_fid),
        javaReliability
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_destinationOrder_fid), javaDestination_order);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_history_fid),
        javaHistory
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_resourceLimits_fid), javaResource_limits);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_userData_fid), javaUser_data);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_ownership_fid), javaOwnership);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(dataReaderQos_timeBasedFilter_fid), javaTime_based_filter);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_readerDataLifecycle_fid),
        javaReader_data_lifecycle
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_share_fid),
        javaShare
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_readerLifespan_fid),
        javaReaderLifespan
    );
    saj_exceptionCheck(env);
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderQos_subscriptionKeys_fid),
        javaSubscriptionKeys
    );
    saj_exceptionCheck(env);

    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaHistory);
    (*env)->DeleteLocalRef (env, javaResource_limits);
    (*env)->DeleteLocalRef (env, javaUser_data);
    (*env)->DeleteLocalRef (env, javaOwnership);
    (*env)->DeleteLocalRef (env, javaTime_based_filter);
    (*env)->DeleteLocalRef (env, javaReader_data_lifecycle);
    (*env)->DeleteLocalRef (env, javaShare);
    (*env)->DeleteLocalRef (env, javaReaderLifespan);
    (*env)->DeleteLocalRef (env, javaSubscriptionKeys);

    return rc;
}

saj_returnCode
saj_DataReaderViewQosCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_dataReaderViewQos *dst)
{
    jobject javaViewKeys;
    saj_returnCode rc;

    assert(src != NULL);

    javaViewKeys = NULL;

    rc = SAJ_RETCODE_OK;

    if (src != NULL)
    {

        javaViewKeys =
            (*env)->GetObjectField(env, src, GET_CACHED(dataReaderViewQos_viewKeys_fid));
        saj_exceptionCheck(env);



        if(rc == SAJ_RETCODE_OK)
        {
            rc = saj_ViewKeyQosPolicyCopyIn(env, javaViewKeys, &dst->view_keys);
        }

    }

    (*env)->DeleteLocalRef (env, javaViewKeys);

    return rc;
}

saj_returnCode
saj_DataReaderViewQosCopyOut(
    JNIEnv *env,
    gapi_dataReaderViewQos *src,
    jobject *dst)
{
    jobject javaViewKeys;
    saj_returnCode rc;

    assert(dst != NULL);

    javaViewKeys = NULL;
    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/DataReaderViewQos", dst);
    }

    /* copy the attributes from the gapi object to the java object */

    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ViewKeyQosPolicyCopyOut(
            env, &src->view_keys, &javaViewKeys);
    }


    /* set the QosPolicy attributes from the java objects */
    (*env)->SetObjectField(
        env,
        *dst,
        GET_CACHED(dataReaderViewQos_viewKeys_fid),
        javaViewKeys
    );
    saj_exceptionCheck(env);

    (*env)->DeleteLocalRef (env, javaViewKeys);

    return rc;
}
