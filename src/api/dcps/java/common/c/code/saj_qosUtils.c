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

#include "saj_qosUtils.h"
#include "c_stringSupport.h"
#include "kernelModule.h"

#define _COPYIN_(_qos, _policy, _dst) \
        if(rc == SAJ_RETCODE_OK) { \
            jobject policy = GET_OBJECT_FIELD(env, src, _qos##_##_policy); \
            rc = saj_##_policy##QosPolicyICopyIn(env, policy, &_dst); \
            DELETE_LOCAL_REF(env, policy); \
        }

#define _COPYOUT_(_src, _qos, _policy) \
    if(rc == SAJ_RETCODE_OK) { \
        jobject policy = NULL; \
        rc = saj_##_policy##QosPolicyICopyOut(env, &_src, &policy); \
        SET_OBJECT_FIELD(env, *dst, _qos##_##_policy, policy); \
        DELETE_LOCAL_REF(env, policy); \
        assert(rc == SAJ_RETCODE_OK); \
    }



/*################ COPY-IN QOS POLICY #############################*/


saj_returnCode
saj_userDataQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_userDataPolicy *dst)
{
    jbyteArray policy;
    void *buffer;
    jsize length;
    saj_returnCode rc = SAJ_RETCODE_ERROR;

    assert(dst != NULL);

    if (src != NULL) {
        policy = GET_OBJECT_FIELD(env, src, userDataQosPolicy_value);
        length = GET_ARRAY_LENGTH(env, policy);

        if (length > 0) {
            dst->value = os_malloc(length);
            buffer = GET_PRIMITIVE_ARRAY_CRITICAL(env, policy, NULL);
            memcpy(dst->value, buffer, length);
            dst->size = length;
            RELEASE_PRIMITIVE_ARRAY_CRITICAL(env, policy, buffer, JNI_ABORT);
        } else {
            dst->value = NULL;
            dst->size  = 0;
        }
        CHECK_EXCEPTION(env);
        DELETE_LOCAL_REF(env, policy);
        rc = SAJ_RETCODE_OK;
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_entityFactoryQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_entityFactoryPolicy *dst)
{
    assert(dst != NULL);

    if (src != NULL) {
        dst->autoenable_created_entities =
            GET_BOOLEAN_FIELD(env, src, entityFactoryQosPolicy_autoenableCreatedEntities);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_presentationQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_presentationPolicy *dst)
{
    jobject kind;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, presentationQosPolicy_accessScope);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &dst->access_scope);
        if (rc == SAJ_RETCODE_OK) {
            dst->coherent_access = GET_BOOLEAN_FIELD(env, src, presentationQosPolicy_coherentAccess);
            dst->ordered_access = GET_BOOLEAN_FIELD(env, src, presentationQosPolicy_orderedAccess);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durabilityQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_durabilityPolicy *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, durabilityQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->kind));
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durabilityServiceQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_durabilityServicePolicy *dst)
{
    jobject kind = NULL;
    jobject delay = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        dst->history_depth = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_historyDepth);
        dst->max_samples = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_maxSamples);
        dst->max_instances = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_maxInstances);
        dst->max_samples_per_instance = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_maxSamplesPerInstance);

        kind = GET_OBJECT_FIELD(env, src, durabilityServiceQosPolicy_historyKind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->history_kind));
        if (rc == SAJ_RETCODE_OK){
            delay = GET_OBJECT_FIELD(env, src, durabilityServiceQosPolicy_serviceCleanupDelay);
            rc = saj_vDurationCopyIn(env, delay, &(dst->service_cleanup_delay));
            DELETE_LOCAL_REF(env, delay);
        }
        DELETE_LOCAL_REF(env, kind);
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_deadlineQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_deadlinePolicy *dst)
{
    jobject period = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        period = GET_OBJECT_FIELD(env, src, deadlineQosPolicy_period);
        rc = saj_vDurationCopyIn(env, period, &(dst->period));
        DELETE_LOCAL_REF(env, period);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_latencyBudgetQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_latencyPolicy *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, latencyBudgetQosPolicy_duration);
        rc = saj_vDurationCopyIn(env, duration, &(dst->duration));
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_livelinessQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_livelinessPolicy *dst)
{
    jobject kind = NULL;
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, livelinessQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->kind));
        if (rc == SAJ_RETCODE_OK) {
            duration = GET_OBJECT_FIELD(env, src, livelinessQosPolicy_leaseDuration);
            rc = saj_vDurationCopyIn(env, duration, &(dst->lease_duration));
            DELETE_LOCAL_REF(env, duration);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_reliabilityQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_reliabilityPolicy *dst)
{
    jobject kind = NULL;
    jobject period = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, reliabilityQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->kind));
        if (rc == SAJ_RETCODE_OK) {
            period = GET_OBJECT_FIELD(env, src, reliabilityQosPolicy_maxBlockingTime);
            rc = saj_vDurationCopyIn(env, period, &(dst->max_blocking_time));
            if(rc == SAJ_RETCODE_OK){
                dst->synchronous = GET_BOOLEAN_FIELD(env, src, reliabilityQosPolicy_synchronous);
            }
            DELETE_LOCAL_REF(env, period);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_destinationOrderQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_orderbyPolicy *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, destinationOrderQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->kind));
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_historyQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_historyPolicy *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, historyQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->kind));
        if (rc == SAJ_RETCODE_OK) {
            dst->depth = GET_INT_FIELD(env, src, historyQosPolicy_depth);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}



saj_returnCode
saj_resourceLimitsQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_resourcePolicy *dst)
{
    assert(dst != NULL);

    if (src != NULL) {
        dst->max_samples = GET_INT_FIELD(env, src, resourceLimitsQosPolicy_maxSamples);
        dst->max_instances = GET_INT_FIELD(env, src, resourceLimitsQosPolicy_maxInstances);
        dst->max_samples_per_instance = GET_INT_FIELD(env, src, resourceLimitsQosPolicy_maxSamplesPerInstance);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_transportPriorityQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_transportPolicy *dst)
{
    assert(dst != NULL);

    if (src != NULL) {
        dst->value = GET_INT_FIELD(env, src, transportPriorityQosPolicy_value);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_lifespanQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_lifespanPolicy *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, lifespanQosPolicy_duration);
        rc = saj_vDurationCopyIn(env, duration, &(dst->duration));
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_ownershipQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_ownershipPolicy *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, ownershipQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->kind));
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_ownershipStrengthQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_strengthPolicy *dst)
{
    assert(dst != NULL);

    if (dst != NULL) {
        dst->value = GET_INT_FIELD(env, src, ownershipStrengthQosPolicy_value);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_readerDataLifecycleQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    struct v_readerLifecyclePolicy *dst)
{
    jobject duration = NULL;
    jobject visibility;
    jobject kind;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay);
        rc = saj_vDurationCopyIn(env, duration, &(dst->autopurge_nowriter_samples_delay));
        DELETE_LOCAL_REF(env, duration);
        if (rc == SAJ_RETCODE_OK) {
            duration = GET_OBJECT_FIELD(env, src, readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay);
            rc = saj_vDurationCopyIn(env, duration, &(dst->autopurge_disposed_samples_delay));
            DELETE_LOCAL_REF(env, duration);
        }
        if (rc == SAJ_RETCODE_OK) {
            dst->autopurge_dispose_all = GET_BOOLEAN_FIELD(env, src, readerDataLifecycleQosPolicy_autopurge_dispose_all);
        }
        if (rc == SAJ_RETCODE_OK) {
            dst->enable_invalid_samples = GET_BOOLEAN_FIELD(env, src, readerDataLifecycleQosPolicy_enable_invalid_samples);
        }
        if (rc == SAJ_RETCODE_OK) {
            os_uint32 tmp;
            visibility = GET_OBJECT_FIELD(env, src, readerDataLifecycleQosPolicy_invalid_sample_visibility);
            kind = GET_OBJECT_FIELD(env, visibility, invalidSampleVisibilityQosPolicy_kind);
            rc = saj_EnumCopyIn(env, kind, &tmp);
            dst->enable_invalid_samples = tmp;
            DELETE_LOCAL_REF(env, kind);
            DELETE_LOCAL_REF(env, visibility);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_readerLifespanQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    struct v_readerLifespanPolicy *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject duration = NULL;

    assert(dst != NULL);

    if(src != NULL) {
        dst->used = GET_BOOLEAN_FIELD(env, src, readerLifespanQosPolicy_useLifespan);
        duration = GET_OBJECT_FIELD(env, src, readerLifespanQosPolicy_duration);
        if(duration != NULL){
            rc = saj_vDurationCopyIn(env, duration, &dst->duration);
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_shareQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    struct v_sharePolicy *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jstring share_name;
    const char *string_data;

    assert(dst != NULL);

    if(src != NULL) {
        dst->enable = GET_BOOLEAN_FIELD(env, src, shareQosPolicy_enable);
        share_name = GET_OBJECT_FIELD(env, src, shareQosPolicy_name);
        if(share_name != NULL){
            string_data = GET_STRING_UTFCHAR(env, share_name, 0);
            dst->name = os_strdup(string_data);
            RELEASE_STRING_UTFCHAR(env, share_name, string_data);
            DELETE_LOCAL_REF(env, share_name);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_subscriptionKeysQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    struct v_userKeyPolicy *dst)
{
    jobjectArray keyList = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject *keys;
    jsize length;
    int i, strlength;
    const char **vm_strings; /* Const because it will hold const char *elements. */

    assert(dst != NULL);

    if (src != NULL) {
        dst->enable = GET_BOOLEAN_FIELD(env, src, subscriptionKeyQosPolicy_useKeyList);

        keyList = GET_OBJECT_FIELD(env, src, subscriptionKeyQosPolicy_keyList);
        if (keyList != NULL) {
            length = GET_ARRAY_LENGTH(env, keyList);
            if (length > 0) {
                vm_strings = os_malloc(length * sizeof(char *));
                keys = os_malloc(length * sizeof(jobject));
                strlength = 1;
                for (i = 0; i < length; i++) {
                    keys[i] = GET_OBJECTARRAY_ELEMENT(env, keyList, i);
                    if (keys[i] != NULL) {
                        vm_strings[i] = GET_STRING_UTFCHAR(env, keys[i], 0);
                        strlength += strlen(vm_strings[i]) + 1;
                    } else {
                        vm_strings[i] = NULL;
                    }
                }
                dst->expression = os_malloc(strlength);
                os_strcpy(dst->expression, vm_strings[0]);
                RELEASE_STRING_UTFCHAR(env, keys[0], vm_strings[0]);
                for (i = 1; i < length; i++) {
                    strcat(dst->expression, ",");
                    strcat(dst->expression, vm_strings[i]);
                    RELEASE_STRING_UTFCHAR(env, keys[i], vm_strings[i]);
                    DELETE_LOCAL_REF(env, keys[i]);
                }
                os_free((void *) vm_strings);
                os_free(keys);
            } else {
                dst->expression = os_malloc(1);
                dst->expression[0] = '\0';
            }
            DELETE_LOCAL_REF(env, keyList);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_viewKeysQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    struct v_userKeyPolicy *dst)
{
    jobjectArray keyList = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    jobject *keyBuf;
    jsize length;
    int i, strlength;
    const char **strBuf; /* Const because it will hold const char *elements. */

    assert(dst != NULL);

    if (src != NULL) {
        dst->enable = GET_BOOLEAN_FIELD(env, src, viewKeyQosPolicy_useKeyList);
        keyList = GET_OBJECT_FIELD(env, src, viewKeyQosPolicy_keyList);
        if (keyList != NULL) {
            length = GET_ARRAY_LENGTH(env, keyList);
            if (length > 0) {
                strBuf = os_malloc(length * sizeof(char *));
                keyBuf = os_malloc(length * sizeof(jobject));
                strlength = 1;
                for (i = 0; i < length; i++) {
                    keyBuf[i] = GET_OBJECTARRAY_ELEMENT(env, keyList, i);
                    if (keyBuf[i] != NULL) {
                        strBuf[i] = GET_STRING_UTFCHAR(env, keyBuf[i], 0);
                        strlength += strlen(strBuf[i]) + 1;
                    } else {
                        strBuf[i] = NULL;
                    }
                }
                dst->expression = os_malloc(strlength);
                os_strcpy(dst->expression, strBuf[0]);
                RELEASE_STRING_UTFCHAR(env, keyBuf[0], strBuf[0]);
                for (i = 1; i < length; i++) {
                    strcat(dst->expression, ",");
                    strcat(dst->expression, strBuf[i]);
                    RELEASE_STRING_UTFCHAR(env, keyBuf[i], strBuf[i]);
                    DELETE_LOCAL_REF(env, keyBuf[i]);
                }
                os_free((void *) strBuf);
                os_free(keyBuf);
            }
            DELETE_LOCAL_REF(env, keyList);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_timeBasedFilterQosPolicyCopyIn(
    JNIEnv *env,
    const jobject src,
    struct v_pacingPolicy *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, timeBasedFilterQosPolicy_minimumSeparation);
        rc = saj_vDurationCopyIn(env, duration, &(dst->minSeperation));
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_writerDataLifecycleQosPolicyCopyIn(
    JNIEnv *env,
    jobject src,
    struct v_writerLifecyclePolicy *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject duration = NULL;

    assert(dst != NULL);

    if(src != NULL) {
        dst->autodispose_unregistered_instances = GET_BOOLEAN_FIELD(env, src, writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances);
        duration = GET_OBJECT_FIELD(env, src, writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay);
        rc = saj_vDurationCopyIn(env, duration, &(dst->autopurge_suspended_samples_delay));
        DELETE_LOCAL_REF(env, duration);

        if (rc == SAJ_RETCODE_OK) {
            duration = GET_OBJECT_FIELD(env, src, writerDataLifecycleQosPolicy_autounregisterInstanceDelay);
            rc = saj_vDurationCopyIn(env, duration, &(dst->autounregister_instance_delay));
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

/*################ COPY-OUT QOS POLICY #############################*/

saj_returnCode
saj_shareQosPolicyCopyOut(
    JNIEnv *env,
    struct v_sharePolicy *src,
    jobject *dst)
{
    jobject name = NULL;
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
        SET_BOOLEAN_FIELD(env, *dst, shareQosPolicy_enable, value);

        name = NEW_STRING_UTF(env, src->name ? src->name : "");
        SET_OBJECT_FIELD(env, *dst, shareQosPolicy_name, name);
        DELETE_LOCAL_REF(env, name);
    }
    return rc;

CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_latencyBudgetQosPolicyCopyOut(
    JNIEnv *env,
    struct v_latencyPolicy *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        /* create a new java LatencyBudgetQosPolicy object */
        rc = saj_create_new_java_object(env, "DDS/LatencyBudgetQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        /* create a new java Duration_t object */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            /* copy the content of the user object to the java object */
            rc = saj_vDurationCopyOut(env, &src->duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                /* store duration in the LatencyBudgetQosPolicy object */
                SET_OBJECT_FIELD(env, *dst, latencyBudgetQosPolicy_duration, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_livelinessQosPolicyCopyOut(
    JNIEnv *env,
    struct v_livelinessPolicy *src,
    jobject *dst)
{
    jobject duration = NULL;
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/LivelinessQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        /* create a new java objects for LeaseDuration and LivelinessQosKind */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            /* copy the content of the user objects to the java objects */
            rc = saj_vDurationCopyOut(env, &src->lease_duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                rc = saj_EnumCopyOut(env, "DDS/LivelinessQosPolicyKind", src->kind, &kind);
                if (rc == SAJ_RETCODE_OK) {
                    /* store the objects in LivelinessQosPolicy attributes */
                    SET_OBJECT_FIELD(env, *dst, livelinessQosPolicy_leaseDuration, duration);
                    SET_OBJECT_FIELD(env, *dst, livelinessQosPolicy_kind, kind);
                    DELETE_LOCAL_REF(env, kind);
                }
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_readerLifespanQosPolicyCopyOut(
    JNIEnv *env,
    struct v_readerLifespanPolicy *src,
    jobject *dst)
{
    jboolean use_lifespan;
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(src != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ReaderLifespanQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK) {
        if (src->used == 0) {
            use_lifespan = JNI_FALSE;
        } else {
            use_lifespan = JNI_TRUE;
        }
        SET_BOOLEAN_FIELD(env, *dst, readerLifespanQosPolicy_useLifespan, use_lifespan);
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_vDurationCopyOut(env, &src->duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, readerLifespanQosPolicy_duration, duration);
            }
        }
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_userDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userDataPolicy *src,
    jobject *dst)
{
    jbyteArray value = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    /* check if a EntityFactoryQosPolicy already exists */
    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/UserDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        value = NEW_BYTE_ARRAY(env, src->size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, src->size, (jbyte *)src->value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, userDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_builtinUserDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinUserDataPolicy *src,
    jobject *dst)
{
    jbyteArray value = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;
    int size = 0;

    assert(dst != NULL);

    /* check if a EntityFactoryQosPolicy already exists */
    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/UserDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        size = c_arraySize(src->value);
        value = NEW_BYTE_ARRAY(env, size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, size, (jbyte *)src->value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, userDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_entityFactoryQosPolicyCopyOut(
    JNIEnv *env,
    struct v_entityFactoryPolicy *src,
    jobject *dst)
{
    jboolean value;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    /* create a new java EntityFactoryQosPolicy object */
    rc = saj_create_new_java_object(env, "DDS/EntityFactoryQosPolicy", dst);
    if (rc == SAJ_RETCODE_OK) {
        if (src->autoenable_created_entities == 1) {
            value = JNI_TRUE;
        } else {
            value = JNI_FALSE;
        }
        SET_BOOLEAN_FIELD(env, *dst, entityFactoryQosPolicy_autoenableCreatedEntities, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_deadlineQosPolicyCopyOut(
    JNIEnv *env,
    struct v_deadlinePolicy *src,
    jobject *dst)
{
    jobject period = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DeadlineQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        /* create a new java Duration_t object */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &period);
        if (rc == SAJ_RETCODE_OK) {
            /* copy the content of the user object to the java Duration_t object */
            rc = saj_vDurationCopyOut(env, &src->period, &period);
            if (rc == SAJ_RETCODE_OK) {
                /* store the Duration_t java object in the DeadLineQosPolicy object */
                SET_OBJECT_FIELD(env, *dst, deadlineQosPolicy_period, period);
            }
            DELETE_LOCAL_REF(env, period);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durabilityQosPolicyCopyOut(
    JNIEnv *env,
    struct v_durabilityPolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DurabilityQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_EnumCopyOut(env, "DDS/DurabilityQosPolicyKind", src->kind, &kind);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, durabilityQosPolicy_kind, kind);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durabilityServiceQosPolicyCopyOut(
    JNIEnv *env,
    struct v_durabilityServicePolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    jobject delay = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DurabilityServiceQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &delay);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EnumCopyOut(env, "DDS/HistoryQosPolicyKind", src->history_kind, &kind);
            if (rc == SAJ_RETCODE_OK) {
                rc = saj_vDurationCopyOut(env, &src->service_cleanup_delay, &delay);
                if (rc == SAJ_RETCODE_OK) {
                    SET_OBJECT_FIELD(env, *dst, durabilityServiceQosPolicy_historyKind, kind);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_historyDepth, src->history_depth);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_maxSamples, src->max_samples);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_maxInstances, src->max_instances);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_maxSamplesPerInstance, src->max_samples_per_instance);
                    SET_OBJECT_FIELD(env, *dst, durabilityServiceQosPolicy_serviceCleanupDelay, delay);
                }
                DELETE_LOCAL_REF(env, kind);
            }
            DELETE_LOCAL_REF(env, delay);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_builtinGroupDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinGroupDataPolicy *src,
    jobject *dst)
{
    jbyteArray value = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;
    int size;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/GroupDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        size = c_arraySize(src->value);
        value = NEW_BYTE_ARRAY(env, size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, size, (jbyte *)src->value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, groupDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_historyQosPolicyCopyOut(
    JNIEnv *env,
    struct v_historyPolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/HistoryQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_EnumCopyOut(env, "DDS/HistoryQosPolicyKind", src->kind, &kind);
        if(rc == SAJ_RETCODE_OK) {
            SET_INT_FIELD(env, *dst, historyQosPolicy_depth, src->depth);
            SET_OBJECT_FIELD(env, *dst, historyQosPolicy_kind, kind);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_lifespanQosPolicyCopyOut(
    JNIEnv *env,
    struct v_lifespanPolicy *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/LifespanQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_vDurationCopyOut(env, &src->duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, lifespanQosPolicy_duration, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_ownershipQosPolicyCopyOut(
    JNIEnv *env,
    struct v_ownershipPolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/OwnershipQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_EnumCopyOut(env, "DDS/OwnershipQosPolicyKind", src->kind, &kind);
        if(rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, ownershipQosPolicy_kind, kind);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_ownershipStrengthQosPolicyCopyOut(
    JNIEnv *env,
    struct v_strengthPolicy *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/OwnershipStrengthQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        SET_INT_FIELD(env, *dst, ownershipStrengthQosPolicy_value, src->value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_builtinPartitionQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinPartitionPolicy *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;
    jobjectArray partitionArray = NULL;
    jclass stringArrCls;
    jstring jPartition;
    os_int32 length, i;

    assert(dst != NULL);

    if ((src != NULL) && (dst != NULL)) {
        rc = SAJ_RETCODE_OK;
        if (*dst == NULL) {
            rc = saj_create_new_java_object(env, "DDS/PartitionQosPolicy", dst);
        }
        if (rc == SAJ_RETCODE_OK) {
            length = c_arraySize(src->name);

            stringArrCls = FIND_CLASS(env, "java/lang/String");
            CHECK_EXCEPTION(env);
            partitionArray = NEW_OBJECTARRAY(env, length, stringArrCls, NULL);

            for (i=0; i<length; i++) {
                jPartition = NEW_STRING_UTF(env, src->name[i]);
                if (jPartition != NULL) {
                    (*env)->SetObjectArrayElement(env, partitionArray, i, jPartition);
                    CHECK_EXCEPTION(env);
                    DELETE_LOCAL_REF(env, jPartition);
                }
            }
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, partitionQosPolicy_name, partitionArray);
            }
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_presentationQosPolicyCopyOut(
    JNIEnv *env,
    struct v_presentationPolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/PresentationQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_EnumCopyOut(env, "DDS/PresentationQosPolicyAccessScopeKind", src->access_scope, &kind);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, presentationQosPolicy_accessScope, kind);
            SET_BOOLEAN_FIELD(env, *dst, presentationQosPolicy_coherentAccess, src->coherent_access);
            SET_BOOLEAN_FIELD(env, *dst, presentationQosPolicy_orderedAccess, src->ordered_access);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_readerDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    struct v_readerLifecyclePolicy *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ReaderDataLifecycleQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            duration = NULL;
            rc = saj_vDurationCopyOut(env, &src->autopurge_nowriter_samples_delay, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            duration = NULL;
            rc = saj_vDurationCopyOut(env, &src->autopurge_disposed_samples_delay, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }

    if (rc == SAJ_RETCODE_OK) {
        jobject visibility = NULL;
        rc = saj_create_new_java_object(env, "DDS/InvalidSampleVisibilityQosPolicy", &visibility);
        if (rc == SAJ_RETCODE_OK) {
#if 1 /* TODO : temporary patch until this policy is supporten by the kernel. */
            jobject kind = NULL;
            rc = saj_EnumCopyOut(env, "DDS/InvalidSampleVisibilityQosPolicyKind", src->enable_invalid_samples, &kind);
            SET_OBJECT_FIELD(env, visibility, invalidSampleVisibilityQosPolicy_kind, kind);
            DELETE_LOCAL_REF(env, kind);
#else
            saj_invalidSampleVisibilityQosPolicyCopyOut(env, &src->invalid_sample_visibility, &visibility);
#endif
            SET_OBJECT_FIELD(env, *dst, readerDataLifecycleQosPolicy_invalid_sample_visibility, visibility);
            DELETE_LOCAL_REF(env, visibility);
        }
    }

    if (rc == SAJ_RETCODE_OK) {
        SET_BOOLEAN_FIELD(env, *dst, readerDataLifecycleQosPolicy_autopurge_dispose_all, src->autopurge_dispose_all);
        SET_BOOLEAN_FIELD(env, *dst, readerDataLifecycleQosPolicy_enable_invalid_samples, src->enable_invalid_samples);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_subscriptionKeysQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userKeyPolicy *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobjectArray keyList = NULL;
    jboolean value;

    jclass stringArrCls;
    jstring jKey;
    os_char *key;
    os_int32 length, i;
    c_iter list;

    assert(dst != NULL);

    if (*dst == NULL) {
       rc = saj_create_new_java_object(env, "DDS/SubscriptionKeyQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        if (src->enable == 0) {
            value = JNI_FALSE;
        } else {
            value = JNI_TRUE;
        }
        SET_BOOLEAN_FIELD(env, *dst, subscriptionKeyQosPolicy_useKeyList, value);
    }
    if (rc == SAJ_RETCODE_OK) {
        list = c_splitString(src->expression, ",");
        length = c_iterLength(list);
        stringArrCls = FIND_CLASS(env, "java/lang/String");
        CHECK_EXCEPTION(env);
        keyList = NEW_OBJECTARRAY(env, length, stringArrCls, NULL);

        assert(keyList);

        for (i=0; i<length; i++) {
            key = c_iterTakeFirst(list);
            jKey = NEW_STRING_UTF(env, key);
            if (jKey != NULL) {
                (*env)->SetObjectArrayElement(env, keyList, i, jKey);
                CHECK_EXCEPTION(env);
                DELETE_LOCAL_REF(env, jKey);
            }
        }
        c_iterFree(list);
        if (rc == SAJ_RETCODE_OK) {
           SET_OBJECT_FIELD(env, *dst, subscriptionKeyQosPolicy_keyList, keyList);
        }
        DELETE_LOCAL_REF(env, keyList);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_viewKeysQosPolicyCopyOut(
    JNIEnv *env,
    struct v_userKeyPolicy *src,
    jobject *dst)
{
    jobjectArray keyList = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;
    jboolean value;

    jclass stringArrCls;
    jstring jKey;
    os_char *key;
    os_int32 length, i;
    c_iter list;

    assert(dst != NULL);

    if (*dst == NULL) {
       rc = saj_create_new_java_object(env, "DDS/ViewKeyQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        if (src->enable == 0) {
            value = JNI_FALSE;
        } else {
            value = JNI_TRUE;
        }
        SET_BOOLEAN_FIELD(env, *dst, viewKeyQosPolicy_useKeyList, value);
    }
    if (rc == SAJ_RETCODE_OK) {
        list = c_splitString(src->expression, ",");
        length = c_iterLength(list);
        stringArrCls = FIND_CLASS(env, "java/lang/String");
        CHECK_EXCEPTION(env);
        keyList = NEW_OBJECTARRAY(env, length, stringArrCls, NULL);

        assert(keyList);

        for (i=0; i<length; i++) {
            key = c_iterTakeFirst(list);
            jKey = NEW_STRING_UTF(env, key);
            if (jKey != NULL) {
                (*env)->SetObjectArrayElement(env, keyList, i, jKey);
                CHECK_EXCEPTION(env);
                DELETE_LOCAL_REF(env, jKey);
            }
        }
        c_iterFree(list);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, viewKeyQosPolicy_keyList, keyList);
        }
        DELETE_LOCAL_REF(env, keyList);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_reliabilityQosPolicyCopyOut(
    JNIEnv *env,
    struct v_reliabilityPolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    jobject javaMaxBlockingTime = NULL;
    jboolean javaSynchronous;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ReliabilityQosPolicy", dst);
    }
    if (!src->synchronous) {
        javaSynchronous = JNI_FALSE;
    } else {
        javaSynchronous = JNI_TRUE;
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &javaMaxBlockingTime);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EnumCopyOut(env, "DDS/ReliabilityQosPolicyKind", src->kind, &kind);
            if (rc == SAJ_RETCODE_OK) {
                rc = saj_vDurationCopyOut(env, &src->max_blocking_time, &javaMaxBlockingTime);
            }
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, reliabilityQosPolicy_kind, kind);
                SET_OBJECT_FIELD(env, *dst, reliabilityQosPolicy_maxBlockingTime, javaMaxBlockingTime);
                SET_BOOLEAN_FIELD(env, *dst, reliabilityQosPolicy_synchronous, javaSynchronous);
                DELETE_LOCAL_REF(env, kind);
            }
            DELETE_LOCAL_REF(env, javaMaxBlockingTime);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_resourceLimitsQosPolicyCopyOut(
    JNIEnv *env,
    struct v_resourcePolicy *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ResourceLimitsQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        SET_INT_FIELD(env, *dst, resourceLimitsQosPolicy_maxSamples, src->max_samples);
        SET_INT_FIELD(env, *dst, resourceLimitsQosPolicy_maxInstances, src->max_instances);
        SET_INT_FIELD(env, *dst, resourceLimitsQosPolicy_maxSamplesPerInstance, src->max_samples_per_instance);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_timeBasedFilterQosPolicyCopyOut(
    JNIEnv *env,
    struct v_pacingPolicy *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/TimeBasedFilterQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_vDurationCopyOut(env, &src->minSeperation, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, timeBasedFilterQosPolicy_minimumSeparation, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_builtinTopicDataQosPolicyCopyOut(
    JNIEnv *env,
    struct v_builtinTopicDataPolicy *src,
    jobject *dst)
{
    jbyteArray value;
    saj_returnCode rc = SAJ_RETCODE_OK;
    int size;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/TopicDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        size = c_arraySize(src->value);
        value = NEW_BYTE_ARRAY(env, size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, size, (jbyte *)src->value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, topicDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_transportPriorityQosPolicyCopyOut(
    JNIEnv *env,
    struct v_transportPolicy *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/TransportPriorityQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        SET_INT_FIELD(env, *dst, transportPriorityQosPolicy_value, src->value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_writerDataLifecycleQosPolicyCopyOut(
    JNIEnv *env,
    struct v_writerLifecyclePolicy *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject duration = NULL;

    assert(dst != NULL);
    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/WriterDataLifecycleQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        SET_BOOLEAN_FIELD(env, *dst, writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances, src->autodispose_unregistered_instances);
    }
    if (rc == SAJ_RETCODE_OK) {
        duration = NULL;
        rc = saj_vDurationCopyOut(env, &src->autopurge_suspended_samples_delay, &duration);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env,*dst, writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay, duration);
            DELETE_LOCAL_REF(env, duration);
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        duration = NULL;
        rc = saj_vDurationCopyOut(env, &src->autounregister_instance_delay, &duration);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, writerDataLifecycleQosPolicy_autounregisterInstanceDelay, duration);
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_destinationOrderQosPolicyCopyOut(
    JNIEnv *env,
    struct v_orderbyPolicy *src,
    jobject *dst)
{
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DestinationOrderQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_EnumCopyOut(env, "DDS/DestinationOrderQosPolicyKind", src->kind, &kind);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, destinationOrderQosPolicy_kind, kind);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

/* ############################ POLICYI COPY IN ####################### */

saj_returnCode
saj_userDataQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_userDataPolicyI *dst)
{
    return saj_userDataQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_entityFactoryQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_entityFactoryPolicyI *dst)
{
    return saj_entityFactoryQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_presentationQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_presentationPolicyI *dst)
{
    return saj_presentationQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_partitionQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_partitionPolicyI *dst)
{
    jobjectArray partitionList = NULL;
    jobject *partitions;
    jsize length;
    saj_returnCode rc = SAJ_RETCODE_OK;
    int i, strlength;
    const char **vm_strings; /* Const because it will hold const char *elements. */

    assert(dst != NULL);
    assert(dst->v == NULL);

    if (src != NULL) {
        partitionList = GET_OBJECT_FIELD(env, src, partitionQosPolicy_name);
        if (partitionList != NULL) {
            length = GET_ARRAY_LENGTH(env, partitionList);
            if (length > 0) {
                vm_strings = os_malloc(length * sizeof(char *));
                partitions = os_malloc(length * sizeof(jobject));
                strlength = 1;
                for (i = 0; i < length; i++) {
                    partitions[i] = GET_OBJECTARRAY_ELEMENT(env, partitionList, i);
                    if (partitions[i] != NULL) {
                        vm_strings[i] = GET_STRING_UTFCHAR(env, partitions[i], 0);
                        strlength += strlen(vm_strings[i]) + 1;
                    } else {
                        vm_strings[i] = NULL;
                    }
                }
                dst->v = os_malloc(strlength);
                os_strcpy(dst->v, vm_strings[0]);
                RELEASE_STRING_UTFCHAR(env, partitions[0], vm_strings[0]);
                for (i = 1; i < length; i++) {
                    strcat(dst->v, ",");
                    strcat(dst->v, vm_strings[i]);
                    RELEASE_STRING_UTFCHAR(env, partitions[i], vm_strings[i]);
                    DELETE_LOCAL_REF(env, partitions[i]);
                }
                os_free((void *) vm_strings);
                os_free(partitions);
            } else {
                dst->v = os_malloc(1);
                *dst->v = '\0';
            }
        }
        DELETE_LOCAL_REF(env, partitionList);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_groupDataQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_groupDataPolicyI *dst)
{
    jbyteArray policy = NULL;
    jbyte *buffer;
    jsize length;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        policy = GET_OBJECT_FIELD(env, src, groupDataQosPolicy_value);
        length = GET_ARRAY_LENGTH(env, policy);
        if (length > 0) {
            dst->v.value = os_malloc(length);
            buffer = GET_PRIMITIVE_ARRAY_CRITICAL(env, policy, NULL);
            memcpy(dst->v.value, buffer, length);
            dst->v.size = length;
            RELEASE_PRIMITIVE_ARRAY_CRITICAL(env, policy, buffer, JNI_ABORT);
        } else {
            dst->v.value = NULL;
            dst->v.size  = 0;
        }
        CHECK_EXCEPTION(env);
        DELETE_LOCAL_REF(env, policy);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_topicDataQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_topicDataPolicyI *dst)
{
    jbyteArray policy = NULL;
    jbyte *buffer;
    jsize length;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        policy = GET_OBJECT_FIELD(env, src, topicDataQosPolicy_value);
        length = GET_ARRAY_LENGTH(env, policy);
        if (length > 0) {
            dst->v.value = os_malloc(length);
            buffer = GET_PRIMITIVE_ARRAY_CRITICAL(env, policy, NULL);
            memcpy(dst->v.value, buffer, length);
            dst->v.size = length;
            RELEASE_PRIMITIVE_ARRAY_CRITICAL(env, policy, buffer, JNI_ABORT);
        } else {
            dst->v.value = NULL;
            dst->v.size  = 0;
        }
        CHECK_EXCEPTION(env);
        DELETE_LOCAL_REF(env, policy);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durabilityQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_durabilityPolicyI *dst)
{
    return saj_durabilityQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_durabilityServiceQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_durabilityServicePolicyI *dst)
{
    jobject kind = NULL;
    jobject delay = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        dst->v.history_depth = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_historyDepth);
        dst->v.max_samples = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_maxSamples);
        dst->v.max_instances = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_maxInstances);
        dst->v.max_samples_per_instance = GET_INT_FIELD(env, src, durabilityServiceQosPolicy_maxSamplesPerInstance);

        kind = GET_OBJECT_FIELD(env, src, durabilityServiceQosPolicy_historyKind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->v.history_kind));
        if (rc == SAJ_RETCODE_OK){
            delay = GET_OBJECT_FIELD(env, src, durabilityServiceQosPolicy_serviceCleanupDelay);
            rc = saj_durationCopyIn(env, delay, &(dst->v.service_cleanup_delay));
            DELETE_LOCAL_REF(env, delay);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_deadlineQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_deadlinePolicyI *dst)
{
    jobject period = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        period = GET_OBJECT_FIELD(env, src, deadlineQosPolicy_period);
        rc = saj_durationCopyIn(env, period, &(dst->v.period));
        DELETE_LOCAL_REF(env, period);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_latencyBudgetQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_latencyPolicyI *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, latencyBudgetQosPolicy_duration);
        rc = saj_durationCopyIn(env, duration, &(dst->v.duration));
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_livelinessQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_livelinessPolicyI *dst)
{
    jobject kind = NULL;
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, livelinessQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->v.kind));
        if (rc == SAJ_RETCODE_OK) {
            duration = GET_OBJECT_FIELD(env, src, livelinessQosPolicy_leaseDuration);
            rc = saj_durationCopyIn(env, duration, &(dst->v.lease_duration));
            DELETE_LOCAL_REF(env, duration);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_reliabilityQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_reliabilityPolicyI *dst)
{
    jobject kind = NULL;
    jobject period = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        kind = GET_OBJECT_FIELD(env, src, reliabilityQosPolicy_kind);
        rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->v.kind));
        if (rc == SAJ_RETCODE_OK) {
            period = GET_OBJECT_FIELD(env, src, reliabilityQosPolicy_maxBlockingTime);
            rc = saj_durationCopyIn(env, period, &(dst->v.max_blocking_time));
            if(rc == SAJ_RETCODE_OK){
                dst->v.synchronous = GET_BOOLEAN_FIELD(env, src, reliabilityQosPolicy_synchronous);
            }
            DELETE_LOCAL_REF(env, period);
        }
        DELETE_LOCAL_REF(env, kind);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_destinationOrderQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_orderbyPolicyI *dst)
{
    return saj_destinationOrderQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_historyQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_historyPolicyI *dst)
{
    return saj_historyQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_resourceLimitsQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_resourcePolicyI *dst)
{
    return saj_resourceLimitsQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_transportPriorityQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_transportPolicyI *dst)
{
    return saj_transportPriorityQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_lifespanQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_lifespanPolicyI *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, lifespanQosPolicy_duration);
        rc = saj_durationCopyIn(env, duration, &(dst->v.duration));
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_ownershipQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_ownershipPolicyI *dst)
{
    return saj_ownershipQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_ownershipStrengthQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_strengthPolicyI *dst)
{
    return saj_ownershipStrengthQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_readerDataLifecycleQosPolicyICopyIn(
    JNIEnv *env,
    const jobject src,
    v_readerLifecyclePolicyI *dst)
{
    jobject duration = NULL;
    jobject visibility;
    jobject kind;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay);
        rc = saj_durationCopyIn(env, duration, &(dst->v.autopurge_nowriter_samples_delay));
        DELETE_LOCAL_REF(env, duration);
        if (rc == SAJ_RETCODE_OK) {
            duration = GET_OBJECT_FIELD(env, src, readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay);
            rc = saj_durationCopyIn(env, duration, &(dst->v.autopurge_disposed_samples_delay));
            DELETE_LOCAL_REF(env, duration);
        }
        if (rc == SAJ_RETCODE_OK) {
            dst->v.autopurge_dispose_all = GET_BOOLEAN_FIELD(env, src, readerDataLifecycleQosPolicy_autopurge_dispose_all);
        }
        if (rc == SAJ_RETCODE_OK) {
            dst->v.enable_invalid_samples = GET_BOOLEAN_FIELD(env, src, readerDataLifecycleQosPolicy_enable_invalid_samples);
        }
        if (rc == SAJ_RETCODE_OK) {
            os_uint32 tmp;
            visibility = GET_OBJECT_FIELD(env, src, readerDataLifecycleQosPolicy_invalid_sample_visibility);
            kind = GET_OBJECT_FIELD(env, visibility, invalidSampleVisibilityQosPolicy_kind);
            rc = saj_EnumCopyIn(env, kind, &tmp);
            dst->v.enable_invalid_samples = tmp;
            DELETE_LOCAL_REF(env, kind);
            DELETE_LOCAL_REF(env, visibility);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_readerLifespanQosPolicyICopyIn(
    JNIEnv *env,
    const jobject src,
    v_readerLifespanPolicyI *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject duration = NULL;

    assert(dst != NULL);

    if(src != NULL) {
        dst->v.used = GET_BOOLEAN_FIELD(env, src, readerLifespanQosPolicy_useLifespan);
        duration = GET_OBJECT_FIELD(env, src, readerLifespanQosPolicy_duration);
        if(duration != NULL){
            rc = saj_durationCopyIn(env, duration, &dst->v.duration);
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_shareQosPolicyICopyIn(
    JNIEnv *env,
    const jobject src,
    v_sharePolicyI *dst)
{
    return saj_shareQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_subscriptionKeysQosPolicyICopyIn(
    JNIEnv *env,
    const jobject src,
    v_userKeyPolicyI *dst)
{
    return saj_subscriptionKeysQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_viewKeysQosPolicyICopyIn(
    JNIEnv *env,
    const jobject src,
    v_userKeyPolicyI *dst)
{
    return saj_viewKeysQosPolicyCopyIn (env, src, &dst->v);
}

saj_returnCode
saj_timeBasedFilterQosPolicyICopyIn(
    JNIEnv *env,
    const jobject src,
    v_pacingPolicyI *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        duration = GET_OBJECT_FIELD(env, src, timeBasedFilterQosPolicy_minimumSeparation);
        rc = saj_durationCopyIn(env, duration, &(dst->v.minSeperation));
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_writerDataLifecycleQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_writerLifecyclePolicyI *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject duration = NULL;

    assert(dst != NULL);

    if(src != NULL) {
        dst->v.autodispose_unregistered_instances = GET_BOOLEAN_FIELD(env, src, writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances);
        duration = GET_OBJECT_FIELD(env, src, writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay);
        rc = saj_durationCopyIn(env, duration, &(dst->v.autopurge_suspended_samples_delay));
        DELETE_LOCAL_REF(env, duration);

        if (rc == SAJ_RETCODE_OK) {
            duration = GET_OBJECT_FIELD(env, src, writerDataLifecycleQosPolicy_autounregisterInstanceDelay);
            rc = saj_durationCopyIn(env, duration, &(dst->v.autounregister_instance_delay));
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_schedulingQosPolicyICopyIn(
    JNIEnv *env,
    jobject src,
    v_schedulePolicyI *dst)
{
    jobject policy = NULL;
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if(src != NULL) {
        dst->v.priority = GET_INT_FIELD(env, src, schedulingQosPolicy_schedulingPriority);
        policy = GET_OBJECT_FIELD(env, src, schedulingQosPolicy_schedulingClass);
        if (policy != NULL) {
            kind = GET_OBJECT_FIELD(env, policy, schedulingClassQosPolicy_kind);
            if (kind != NULL) {
                rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->v.kind));
                DELETE_LOCAL_REF(env, kind);
            }
            DELETE_LOCAL_REF(env, policy);
        }
        if (rc == SAJ_RETCODE_OK) {
            policy = GET_OBJECT_FIELD(env, src, schedulingQosPolicy_schedulingPriorityKind);
            if (policy != NULL) {
                kind = GET_OBJECT_FIELD(env, policy, schedulingPriorityQosPolicy_kind);
                if (kind != NULL) {
                    rc = saj_EnumCopyIn(env, kind, (os_uint32 *) &(dst->v.priorityKind));
                    DELETE_LOCAL_REF(env, kind);
                }
                DELETE_LOCAL_REF(env, policy);
            }
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

#define saj_watchdogSchedulingQosPolicyICopyIn saj_schedulingQosPolicyICopyIn
#define saj_listenerSchedulingQosPolicyICopyIn saj_schedulingQosPolicyICopyIn

/*################ COPY-OUT QOS POLICY #############################*/

saj_returnCode
saj_shareQosPolicyICopyOut(
    JNIEnv *env,
    v_sharePolicyI *src,
    jobject *dst)
{
    return saj_shareQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_latencyBudgetQosPolicyICopyOut(
    JNIEnv *env,
    v_latencyPolicyI *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        /* create a new java LatencyBudgetQosPolicy object */
        rc = saj_create_new_java_object(env, "DDS/LatencyBudgetQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        /* create a new java Duration_t object */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            /* copy the content of the user object to the java object */
            rc = saj_durationCopyOut(env, src->v.duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                /* store duration in the LatencyBudgetQosPolicy object */
                SET_OBJECT_FIELD(env, *dst, latencyBudgetQosPolicy_duration, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_livelinessQosPolicyICopyOut(
    JNIEnv *env,
    v_livelinessPolicyI *src,
    jobject *dst)
{
    jobject duration = NULL;
    jobject kind = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/LivelinessQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        /* create a new java objects for LeaseDuration and LivelinessQosKind */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            /* copy the content of the user objects to the java objects */
            rc = saj_durationCopyOut(env, src->v.lease_duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                rc = saj_EnumCopyOut(env, "DDS/LivelinessQosPolicyKind", src->v.kind, &kind);
                if (rc == SAJ_RETCODE_OK) {
                    /* store the objects in LivelinessQosPolicy attributes */
                    SET_OBJECT_FIELD(env, *dst, livelinessQosPolicy_leaseDuration, duration);
                    SET_OBJECT_FIELD(env, *dst, livelinessQosPolicy_kind, kind);
                    DELETE_LOCAL_REF(env, kind);
                }
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_readerLifespanQosPolicyICopyOut(
    JNIEnv *env,
    v_readerLifespanPolicyI *src,
    jobject *dst)
{
    jboolean use_lifespan;
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(src != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ReaderLifespanQosPolicy", dst);
    }

    if (rc == SAJ_RETCODE_OK) {
        if (src->v.used == 0) {
            use_lifespan = JNI_FALSE;
        } else {
            use_lifespan = JNI_TRUE;
        }
        SET_BOOLEAN_FIELD(env, *dst, readerLifespanQosPolicy_useLifespan, use_lifespan);
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_durationCopyOut(env, src->v.duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, readerLifespanQosPolicy_duration, duration);
            }
        }
        DELETE_LOCAL_REF(env, duration);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_userDataQosPolicyICopyOut(
    JNIEnv *env,
    v_userDataPolicyI *src,
    jobject *dst)
{
    jbyteArray value = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    /* check if a EntityFactoryQosPolicy already exists */
    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/UserDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        value = NEW_BYTE_ARRAY(env, src->v.size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, src->v.size, (jbyte *)src->v.value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, userDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_entityFactoryQosPolicyICopyOut(
    JNIEnv *env,
    v_entityFactoryPolicyI *src,
    jobject *dst)
{
    return saj_entityFactoryQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_deadlineQosPolicyICopyOut(
    JNIEnv *env,
    v_deadlinePolicyI *src,
    jobject *dst)
{
    jobject period = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DeadlineQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        /* create a new java Duration_t object */
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &period);
        if (rc == SAJ_RETCODE_OK) {
            /* copy the content of the user object to the java Duration_t object */
            rc = saj_durationCopyOut(env, src->v.period, &period);
            if (rc == SAJ_RETCODE_OK) {
                /* store the Duration_t java object in the DeadLineQosPolicy object */
                SET_OBJECT_FIELD(env, *dst, deadlineQosPolicy_period, period);
            }
            DELETE_LOCAL_REF(env, period);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durabilityQosPolicyICopyOut(
    JNIEnv *env,
    v_durabilityPolicyI *src,
    jobject *dst)
{
    return saj_durabilityQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_durabilityServiceQosPolicyICopyOut(
    JNIEnv *env,
    v_durabilityServicePolicyI *src,
    jobject *dst)
{
    jobject kind = NULL;
    jobject delay = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DurabilityServiceQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &delay);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EnumCopyOut(env, "DDS/HistoryQosPolicyKind", src->v.history_kind, &kind);
            if (rc == SAJ_RETCODE_OK) {
                rc = saj_durationCopyOut(env, src->v.service_cleanup_delay, &delay);
                if (rc == SAJ_RETCODE_OK) {
                    SET_OBJECT_FIELD(env, *dst, durabilityServiceQosPolicy_historyKind, kind);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_historyDepth, src->v.history_depth);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_maxSamples, src->v.max_samples);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_maxInstances, src->v.max_instances);
                    SET_INT_FIELD(env, *dst, durabilityServiceQosPolicy_maxSamplesPerInstance, src->v.max_samples_per_instance);
                    SET_OBJECT_FIELD(env, *dst, durabilityServiceQosPolicy_serviceCleanupDelay, delay);
                }
                DELETE_LOCAL_REF(env, kind);
            }
            DELETE_LOCAL_REF(env, delay);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_groupDataQosPolicyICopyOut(
    JNIEnv *env,
    v_groupDataPolicyI *src,
    jobject *dst)
{
    jbyteArray value = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/GroupDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        value = NEW_BYTE_ARRAY(env, src->v.size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, src->v.size, (jbyte *)src->v.value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, groupDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_historyQosPolicyICopyOut(
    JNIEnv *env,
    v_historyPolicyI *src,
    jobject *dst)
{
    return saj_historyQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_lifespanQosPolicyICopyOut(
    JNIEnv *env,
    v_lifespanPolicyI *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/LifespanQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_durationCopyOut(env, src->v.duration, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, lifespanQosPolicy_duration, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_ownershipQosPolicyICopyOut(
    JNIEnv *env,
    v_ownershipPolicyI *src,
    jobject *dst)
{
    return saj_ownershipQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_ownershipStrengthQosPolicyICopyOut(
    JNIEnv *env,
    v_strengthPolicyI *src,
    jobject *dst)
{
    return saj_ownershipStrengthQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_partitionQosPolicyICopyOut(
    JNIEnv *env,
    v_partitionPolicyI *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;
    jobjectArray partitionArray = NULL;
    jclass stringArrCls;
    jstring jPartition;
    os_char *partition;
    os_int32 length, i;
    c_iter list;

    assert(dst != NULL);

    if ((src != NULL) && (dst != NULL)) {
        rc = SAJ_RETCODE_OK;
        if (*dst == NULL) {
            rc = saj_create_new_java_object(env, "DDS/PartitionQosPolicy", dst);
        }
        if (rc == SAJ_RETCODE_OK) {
            list = c_splitString(src->v, ",");
            length = c_iterLength(list);

            stringArrCls = FIND_CLASS(env, "java/lang/String");
            CHECK_EXCEPTION(env);
            partitionArray = NEW_OBJECTARRAY(env, length, stringArrCls, NULL);

            for (i=0; i<length; i++) {
                partition = c_iterTakeFirst(list);
                jPartition = NEW_STRING_UTF(env, partition);
                os_free(partition);
                if (jPartition != NULL) {
                    (*env)->SetObjectArrayElement(env, partitionArray, i, jPartition);
                    CHECK_EXCEPTION(env);
                    DELETE_LOCAL_REF(env, jPartition);
                }
            }
            c_iterFree(list);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, partitionQosPolicy_name, partitionArray);
            }
        }
    }
    return rc;

    CATCH_EXCEPTION:
    for (i=0; i<length; i++) {
        partition = c_iterTakeFirst(list);
        os_free(partition);
    }
    c_iterFree(list);
    return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_presentationQosPolicyICopyOut(
    JNIEnv *env,
    v_presentationPolicyI *src,
    jobject *dst)
{
    return saj_presentationQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_readerDataLifecycleQosPolicyICopyOut(
    JNIEnv *env,
    v_readerLifecyclePolicyI *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ReaderDataLifecycleQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            duration = NULL;
            rc = saj_durationCopyOut(env, src->v.autopurge_nowriter_samples_delay, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, readerDataLifecycleQosPolicy_autopurgeNowriterSamplesDelay, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            duration = NULL;
            rc = saj_durationCopyOut(env, src->v.autopurge_disposed_samples_delay, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, readerDataLifecycleQosPolicy_autopurgeDisposedSamplesDelay, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }

    if (rc == SAJ_RETCODE_OK) {
        jobject visibility = NULL;
        rc = saj_create_new_java_object(env, "DDS/InvalidSampleVisibilityQosPolicy", &visibility);
        if (rc == SAJ_RETCODE_OK) {
#if 1 /* TODO : temporary patch until this policy is supporten by the kernel. */
            jobject kind = NULL;
            rc = saj_EnumCopyOut(env, "DDS/InvalidSampleVisibilityQosPolicyKind", src->v.enable_invalid_samples, &kind);
            SET_OBJECT_FIELD(env, visibility, invalidSampleVisibilityQosPolicy_kind, kind);
            DELETE_LOCAL_REF(env, kind);
#else
            saj_invalidSampleVisibilityQosPolicyCopyOut(env, &src->v.invalid_sample_visibility, &visibility);
#endif
            SET_OBJECT_FIELD(env, *dst, readerDataLifecycleQosPolicy_invalid_sample_visibility, visibility);
            DELETE_LOCAL_REF(env, visibility);
        }
    }

    if (rc == SAJ_RETCODE_OK) {
        SET_BOOLEAN_FIELD(env, *dst, readerDataLifecycleQosPolicy_autopurge_dispose_all, src->v.autopurge_dispose_all);
        SET_BOOLEAN_FIELD(env, *dst, readerDataLifecycleQosPolicy_enable_invalid_samples, src->v.enable_invalid_samples);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_subscriptionKeysQosPolicyICopyOut(
    JNIEnv *env,
    v_userKeyPolicyI *src,
    jobject *dst)
{
    return saj_subscriptionKeysQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_viewKeysQosPolicyICopyOut(
    JNIEnv *env,
    v_userKeyPolicyI *src,
    jobject *dst)
{
    return saj_viewKeysQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_reliabilityQosPolicyICopyOut(
    JNIEnv *env,
    v_reliabilityPolicyI *src,
    jobject *dst)
{
    jobject kind = NULL;
    jobject javaMaxBlockingTime = NULL;
    jboolean javaSynchronous;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/ReliabilityQosPolicy", dst);
    }
    if (!src->v.synchronous) {
        javaSynchronous = JNI_FALSE;
    } else {
        javaSynchronous = JNI_TRUE;
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &javaMaxBlockingTime);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EnumCopyOut(env, "DDS/ReliabilityQosPolicyKind", src->v.kind, &kind);
            if (rc == SAJ_RETCODE_OK) {
                rc = saj_durationCopyOut(env, src->v.max_blocking_time, &javaMaxBlockingTime);
            }
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, reliabilityQosPolicy_kind, kind);
                SET_OBJECT_FIELD(env, *dst, reliabilityQosPolicy_maxBlockingTime, javaMaxBlockingTime);
                SET_BOOLEAN_FIELD(env, *dst, reliabilityQosPolicy_synchronous, javaSynchronous);
                DELETE_LOCAL_REF(env, kind);
            }
            DELETE_LOCAL_REF(env, javaMaxBlockingTime);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_resourceLimitsQosPolicyICopyOut(
    JNIEnv *env,
    v_resourcePolicyI *src,
    jobject *dst)
{
    return saj_resourceLimitsQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_timeBasedFilterQosPolicyICopyOut(
    JNIEnv *env,
    v_pacingPolicyI *src,
    jobject *dst)
{
    jobject duration = NULL;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/TimeBasedFilterQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/Duration_t", &duration);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_durationCopyOut(env, src->v.minSeperation, &duration);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, *dst, timeBasedFilterQosPolicy_minimumSeparation, duration);
            }
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_topicDataQosPolicyICopyOut(
    JNIEnv *env,
    v_topicDataPolicyI *src,
    jobject *dst)
{
    jbyteArray value;
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/TopicDataQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        value = NEW_BYTE_ARRAY(env, src->v.size);
        CHECK_EXCEPTION(env);
        SET_BYTE_ARRAY_REGION(env, value, 0, src->v.size, (jbyte *)src->v.value);
        CHECK_EXCEPTION(env);
        SET_OBJECT_FIELD(env, *dst, topicDataQosPolicy_value, value);
        DELETE_LOCAL_REF(env, value);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_transportPriorityQosPolicyICopyOut(
    JNIEnv *env,
    v_transportPolicyI *src,
    jobject *dst)
{
    return saj_transportPriorityQosPolicyCopyOut (env, &src->v, dst);
}

saj_returnCode
saj_writerDataLifecycleQosPolicyICopyOut(
    JNIEnv *env,
    v_writerLifecyclePolicyI *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject duration = NULL;

    assert(dst != NULL);
    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/WriterDataLifecycleQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        SET_BOOLEAN_FIELD(env, *dst, writerDataLifecycleQosPolicy_autodisposeUnregisteredInstances, src->v.autodispose_unregistered_instances);
    }
    if (rc == SAJ_RETCODE_OK) {
        duration = NULL;
        rc = saj_durationCopyOut(env, src->v.autopurge_suspended_samples_delay, &duration);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env,*dst, writerDataLifecycleQosPolicy_autopurgeSuspendedSamplesDelay, duration);
            DELETE_LOCAL_REF(env, duration);
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        duration = NULL;
        rc = saj_durationCopyOut(env, src->v.autounregister_instance_delay, &duration);
        if (rc == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, *dst, writerDataLifecycleQosPolicy_autounregisterInstanceDelay, duration);
            DELETE_LOCAL_REF(env, duration);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

#define saj_watchdogSchedulingQosPolicyICopyOut saj_schedulingQosPolicyICopyOut
#define saj_listenerSchedulingQosPolicyICopyOut saj_schedulingQosPolicyICopyOut

saj_returnCode
saj_schedulingQosPolicyICopyOut(
    JNIEnv *env,
    v_schedulePolicyI *src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    jobject policy = NULL;
    jobject kind = NULL;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/SchedulingQosPolicy", dst);
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/SchedulingClassQosPolicy", &policy);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EnumCopyOut(env, "DDS/SchedulingClassQosPolicyKind", src->v.kind, &kind);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, policy, schedulingClassQosPolicy_kind, kind);
                DELETE_LOCAL_REF(env, kind);
                SET_OBJECT_FIELD(env, *dst, schedulingQosPolicy_schedulingClass, policy);
            }
            DELETE_LOCAL_REF(env, policy);
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        rc = saj_create_new_java_object(env, "DDS/SchedulingPriorityQosPolicy", &policy);
        if (rc == SAJ_RETCODE_OK) {
            rc = saj_EnumCopyOut(env, "DDS/SchedulingPriorityQosPolicyKind", src->v.priorityKind, &kind);
            if (rc == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, policy, schedulingPriorityQosPolicy_kind, kind);
                DELETE_LOCAL_REF(env, kind);
                SET_OBJECT_FIELD(env, *dst, schedulingQosPolicy_schedulingPriorityKind, policy);
            }
            DELETE_LOCAL_REF(env, policy);
        }
    }
    if (rc == SAJ_RETCODE_OK) {
        SET_INT_FIELD( env, *dst, schedulingQosPolicy_schedulingPriority, src->v.priority);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_destinationOrderQosPolicyICopyOut(
    JNIEnv *env,
    v_orderbyPolicyI *src,
    jobject *dst)
{
    return saj_destinationOrderQosPolicyCopyOut (env, &src->v, dst);
}

/*################ COPY-IN ENTITY QOS #############################*/

saj_returnCode
saj_domainParticipantQosCopyIn(
    JNIEnv *env,
    const jobject src,
    u_participantQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;

    assert(dst != NULL);

    if (src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(domainParticipantQos, userData, dst->userData);
        _COPYIN_(domainParticipantQos, entityFactory, dst->entityFactory);
        _COPYIN_(domainParticipantQos, watchdogScheduling, dst->watchdogScheduling);
/* TODO : check if this is required :     _COPYIN_(domainParticipantQos, listenerSchedulingI); */
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_subscriberQosCopyIn(
    JNIEnv *env,
    const jobject src,
    u_subscriberQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;

    assert(dst != NULL);

    if(src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(subscriberQos, presentation, dst->presentation);
        _COPYIN_(subscriberQos, partition, dst->partition);
        _COPYIN_(subscriberQos, groupData, dst->groupData);
        _COPYIN_(subscriberQos, entityFactory, dst->entityFactory);
        _COPYIN_(subscriberQos, share, dst->share);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_topicQosCopyIn(
    JNIEnv *env,
    const jobject src,
    u_topicQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;

    assert(dst != NULL);

    if(src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(topicQos, topicData, dst->topicData);
        _COPYIN_(topicQos, durability, dst->durability);
        _COPYIN_(topicQos, durabilityService, dst->durabilityService);
        _COPYIN_(topicQos, deadline, dst->deadline);
        _COPYIN_(topicQos, latencyBudget, dst->latency);
        _COPYIN_(topicQos, liveliness, dst->liveliness);
        _COPYIN_(topicQos, reliability, dst->reliability);
        _COPYIN_(topicQos, destinationOrder, dst->orderby);
        _COPYIN_(topicQos, history, dst->history);
        _COPYIN_(topicQos, resourceLimits, dst->resource);
        _COPYIN_(topicQos, transportPriority, dst->transport);
        _COPYIN_(topicQos, lifespan, dst->lifespan);
        _COPYIN_(topicQos, ownership, dst->ownership);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_publisherQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_publisherQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(publisherQos, presentation, dst->presentation);
        _COPYIN_(publisherQos, partition, dst->partition);
        _COPYIN_(publisherQos, groupData, dst->groupData);
        _COPYIN_(publisherQos, entityFactory, dst->entityFactory);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_dataWriterQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_writerQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;

    assert(src != NULL);

    if (src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(dataWriterQos, durability, dst->durability);
        _COPYIN_(dataWriterQos, deadline, dst->deadline);
        _COPYIN_(dataWriterQos, latencyBudget, dst->latency);
        _COPYIN_(dataWriterQos, liveliness, dst->liveliness);
        _COPYIN_(dataWriterQos, reliability, dst->reliability);
        _COPYIN_(dataWriterQos, destinationOrder, dst->orderby);
        _COPYIN_(dataWriterQos, history, dst->history);
        _COPYIN_(dataWriterQos, resourceLimits, dst->resource);
        _COPYIN_(dataWriterQos, transportPriority, dst->transport);
        _COPYIN_(dataWriterQos, lifespan, dst->lifespan);
        _COPYIN_(dataWriterQos, userData, dst->userData);
        _COPYIN_(dataWriterQos, ownership, dst->ownership);
        _COPYIN_(dataWriterQos, ownershipStrength, dst->strength);
        _COPYIN_(dataWriterQos, writerDataLifecycle, dst->lifecycle);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_dataReaderQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_readerQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;

    assert(src != NULL);

    if (src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(dataReaderQos, durability, dst->durability);
        _COPYIN_(dataReaderQos, deadline, dst->deadline);
        _COPYIN_(dataReaderQos, latencyBudget, dst->latency);
        _COPYIN_(dataReaderQos, liveliness, dst->liveliness);
        _COPYIN_(dataReaderQos, reliability, dst->reliability);
        _COPYIN_(dataReaderQos, destinationOrder, dst->orderby);
        _COPYIN_(dataReaderQos, history, dst->history);
        _COPYIN_(dataReaderQos, resourceLimits, dst->resource);
        _COPYIN_(dataReaderQos, userData, dst->userData);
        _COPYIN_(dataReaderQos, ownership, dst->ownership);
        _COPYIN_(dataReaderQos, timeBasedFilter, dst->pacing);
        _COPYIN_(dataReaderQos, readerDataLifecycle, dst->lifecycle);
        _COPYIN_(dataReaderQos, share, dst->share);
        _COPYIN_(dataReaderQos, readerLifespan, dst->lifespan);
        _COPYIN_(dataReaderQos, subscriptionKeys, dst->userKey);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_dataReaderViewQosCopyIn(
    JNIEnv *env,
    jobject src,
    u_dataViewQos dst)
{
    saj_returnCode rc = SAJ_RETCODE_BAD_PARAMETER;

    assert(src != NULL);

    if (src != NULL) {
        rc = SAJ_RETCODE_OK;
        _COPYIN_(dataReaderViewQos, viewKeys, dst->userKey);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

#undef _COPYIN_

/*################ COPY-OUT ENTITY QOS ############################*/

saj_returnCode
saj_domainParticipantQosCopyOut(
    JNIEnv *env,
    u_participantQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DomainParticipantQos", dst);
        _COPYOUT_(src->userData, domainParticipantQos, userData);
        _COPYOUT_(src->entityFactory, domainParticipantQos, entityFactory);
        _COPYOUT_(src->watchdogScheduling, domainParticipantQos, watchdogScheduling);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_topicQosCopyOut(
    JNIEnv *env,
    u_topicQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/TopicQos", dst);
        _COPYOUT_(src->topicData, topicQos, topicData);
        _COPYOUT_(src->durability, topicQos, durability);
        _COPYOUT_(src->durabilityService, topicQos, durabilityService);
        _COPYOUT_(src->deadline, topicQos, deadline);
        _COPYOUT_(src->latency, topicQos, latencyBudget);
        _COPYOUT_(src->liveliness, topicQos, liveliness);
        _COPYOUT_(src->reliability, topicQos, reliability);
        _COPYOUT_(src->orderby, topicQos, destinationOrder);
        _COPYOUT_(src->history, topicQos, history);
        _COPYOUT_(src->resource, topicQos, resourceLimits);
        _COPYOUT_(src->transport, topicQos, transportPriority);
        _COPYOUT_(src->lifespan, topicQos, lifespan);
        _COPYOUT_(src->ownership, topicQos, ownership);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_subscriberQosCopyOut(
    JNIEnv *env,
    u_subscriberQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/SubscriberQos", dst);
        _COPYOUT_(src->presentation, subscriberQos, presentation);
        _COPYOUT_(src->partition, subscriberQos, partition);
        _COPYOUT_(src->groupData, subscriberQos, groupData);
        _COPYOUT_(src->entityFactory, subscriberQos, entityFactory);
        _COPYOUT_(src->share, subscriberQos, share);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_publisherQosCopyOut(
    JNIEnv *env,
    u_publisherQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/PublisherQos", dst);
        _COPYOUT_(src->presentation, publisherQos, presentation);
        _COPYOUT_(src->partition, publisherQos, partition);
        _COPYOUT_(src->groupData, publisherQos, groupData);
        _COPYOUT_(src->entityFactory, publisherQos, entityFactory);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_dataWriterQosCopyOut(
    JNIEnv *env,
    u_writerQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DataWriterQos", dst);
        _COPYOUT_(src->durability, dataWriterQos, durability);
        _COPYOUT_(src->deadline, dataWriterQos, deadline);
        _COPYOUT_(src->latency, dataWriterQos, latencyBudget);
        _COPYOUT_(src->liveliness, dataWriterQos, liveliness);
        _COPYOUT_(src->reliability, dataWriterQos, reliability);
        _COPYOUT_(src->orderby, dataWriterQos, destinationOrder);
        _COPYOUT_(src->history, dataWriterQos, history);
        _COPYOUT_(src->resource, dataWriterQos, resourceLimits);
        _COPYOUT_(src->transport, dataWriterQos, transportPriority);
        _COPYOUT_(src->lifespan, dataWriterQos, lifespan);
        _COPYOUT_(src->userData, dataWriterQos, userData);
        _COPYOUT_(src->ownership, dataWriterQos, ownership);
        _COPYOUT_(src->strength, dataWriterQos, ownershipStrength);
        _COPYOUT_(src->lifecycle, dataWriterQos, writerDataLifecycle);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_dataReaderQosCopyOut(
    JNIEnv *env,
    u_readerQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DataReaderQos", dst);
        _COPYOUT_(src->durability, dataReaderQos, durability);
        _COPYOUT_(src->deadline, dataReaderQos, deadline);
        _COPYOUT_(src->latency, dataReaderQos, latencyBudget);
        _COPYOUT_(src->liveliness, dataReaderQos, liveliness);
        _COPYOUT_(src->reliability, dataReaderQos, reliability);
        _COPYOUT_(src->orderby, dataReaderQos, destinationOrder);
        _COPYOUT_(src->resource, dataReaderQos, resourceLimits);
        _COPYOUT_(src->userData, dataReaderQos, userData);
        _COPYOUT_(src->ownership, dataReaderQos, ownership);
        _COPYOUT_(src->pacing, dataReaderQos, timeBasedFilter);
        _COPYOUT_(src->lifecycle, dataReaderQos, readerDataLifecycle);
        _COPYOUT_(src->userKey, dataReaderQos, subscriptionKeys);
        _COPYOUT_(src->share, dataReaderQos, share);
        _COPYOUT_(src->lifespan, dataReaderQos, readerLifespan);
        _COPYOUT_(src->history, dataReaderQos, history);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_dataReaderViewQosCopyOut(
    JNIEnv *env,
    u_dataViewQos src,
    jobject *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;

    assert(dst != NULL);

    if (*dst == NULL) {
        rc = saj_create_new_java_object(env, "DDS/DataReaderViewQos", dst);
        _COPYOUT_(src->userKey, dataReaderViewQos, viewKeys);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

#undef _COPYOUT_
