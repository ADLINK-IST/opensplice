/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include <jni.h>
#include "saj_utilities.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"
#include "saj_qosUtils.h"
#include "u_object.h"
#include "u_observable.h"
#include "v_status.h"
#include "v_entity.h"
#include "c_stringSupport.h"

/* Enabling CDR copy out requires compiling the type, for which it
   needs the copy cache - and so we need these two include files. */
#include "saj_copyCache.h"
#include "sd_cdr.h"
#include "saj__report.h"

#define TIME_INVALID_SEC       (-1L)
#define TIME_INVALID_NSEC      (-1L)

/* jni_cache jniCache; */
jni_cache jniCache;


#define SAJ_CHECK_RESULT(rc) \
    if (rc != SAJ_RETCODE_OK) { \
        printf("saj_publicationBuiltinTopicDataCopyOut: rc = %d at line %d\n", rc, __LINE__); \
        THROW_EXCEPTION; \
    }

/**
 * @brief private function to check the java_object.
 * If it is an instance of org.opensplice.dds.dcps.ObjectImpl
 * SAJ_RETCODE_OK will be returned otherwise SAJ_RETCODE_ERROR is returned.
 * @param env The JNI environment.
 * @param java_object The object that will be checked.
 * @return SAJ_RETCODE_OK if java_object is an instance of ObjectImpl.
 */
saj_returnCode  checkJavaObject(JNIEnv *env, jobject java_object);

saj_returnCode
saj_retcode_from_user_result(
    u_result result)
{
    switch(result) {
    case U_RESULT_UNDEFINED:            return SAJ_RETCODE_ERROR;
    case U_RESULT_OK:                   return SAJ_RETCODE_OK;
    case U_RESULT_INTERRUPTED:          return SAJ_RETCODE_ERROR;
    case U_RESULT_OUT_OF_MEMORY:        return SAJ_RETCODE_OUT_OF_RESOURCES;
    case U_RESULT_INTERNAL_ERROR:       return SAJ_RETCODE_ERROR;
    case U_RESULT_ILL_PARAM:            return SAJ_RETCODE_BAD_PARAMETER;
    case U_RESULT_CLASS_MISMATCH:       return SAJ_RETCODE_PRECONDITION_NOT_MET;
    case U_RESULT_DETACHING:            return SAJ_RETCODE_ALREADY_DELETED;
    case U_RESULT_TIMEOUT:              return SAJ_RETCODE_TIMEOUT;
    case U_RESULT_OUT_OF_RESOURCES:     return SAJ_RETCODE_OUT_OF_RESOURCES;
    case U_RESULT_INCONSISTENT_QOS:     return SAJ_RETCODE_INCONSISTENT_POLICY;
    case U_RESULT_IMMUTABLE_POLICY:     return SAJ_RETCODE_IMMUTABLE_POLICY;
    case U_RESULT_PRECONDITION_NOT_MET: return SAJ_RETCODE_PRECONDITION_NOT_MET;
    case U_RESULT_ALREADY_DELETED:      return SAJ_RETCODE_ALREADY_DELETED;
    case U_RESULT_HANDLE_EXPIRED:       return SAJ_RETCODE_HANDLE_EXPIRED;
    case U_RESULT_NO_DATA:              return SAJ_RETCODE_NO_DATA;
    case U_RESULT_UNSUPPORTED:          return SAJ_RETCODE_UNSUPPORTED;
    default: break;
    }
    return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_create_new_java_object(
    JNIEnv *env,
    const char *classname,
    jobject *new_java_object)
{
    jclass newClass;
    jmethodID constructorId;

    assert(new_java_object);
    assert(classname);

    newClass = FIND_CLASS(env, classname);
    constructorId = GET_METHOD_ID(env, newClass, "<init>", "()V");
    *new_java_object = NEW_OBJECT(env, newClass, constructorId);
    DELETE_LOCAL_REF(env, newClass);

    return SAJ_RETCODE_OK;

CATCH_EXCEPTION:
    *new_java_object = NULL;
    return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_create_new_typed_java_object(
    JNIEnv *env,
    const char *classname,
    jobject *new_java_object,
    const char *constructorSignature,
    jobject typeSupport)
{
    jclass  newClass;
    jmethodID constructorId;
    jobject newObject;
    saj_returnCode rc;

    assert(new_java_object != NULL);

    rc = SAJ_RETCODE_ERROR;
    newClass = FIND_CLASS(env, classname);

    if (newClass != NULL){
        constructorId = (*env)->GetMethodID(env, newClass, "<init>", constructorSignature);
        CHECK_EXCEPTION(env);

        if (constructorId != NULL){
            newObject = NEW_OBJECT(env, newClass, constructorId, typeSupport);

            if(newObject != NULL){
                *new_java_object = newObject;
                rc = SAJ_RETCODE_OK;
            }
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
checkJavaObject(
    JNIEnv *env,
    jobject java_object)
{
    saj_returnCode rc;
    jclass tempClass;
    os_boolean isInstanceOf;

    rc = SAJ_RETCODE_ERROR;

    if(java_object != NULL){
        /* make sure there is a reference to ObjectImpl */
        if (GET_CACHED(ObjectImpl_class) == NULL){
            tempClass = FIND_CLASS(env, "org/opensplice/dds/dcps/ObjectImpl");
            SET_CACHED(ObjectImpl_class, NEW_GLOBAL_REF(env, tempClass));
            DELETE_LOCAL_REF(env, tempClass);
        }

        /* Verify the java_object is an instance of ObjectImpl */
        isInstanceOf = IS_INSTANCE_OF(env, java_object, GET_CACHED(ObjectImpl_class));
        if (isInstanceOf) {
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

void
saj_exceptionCheck (
    JNIEnv *javaEnv)
{
    if ((*javaEnv)->ExceptionOccurred(javaEnv))
        assert(0);
}

saj_returnCode
saj_durationCopyIn_deprecated(
    JNIEnv *env,
    jobject javaDuration,
    os_duration *out)
{
    saj_returnCode rc;
    os_int32 seconds, nanoseconds;

    assert(out != NULL);

    rc = SAJ_RETCODE_OK;

    if (javaDuration != NULL) {
        seconds = GET_INT_FIELD(env, javaDuration, duration_t_sec);
        nanoseconds = GET_INT_FIELD(env, javaDuration, duration_t_nanosec);
        *out = SAJ_DURATION_INIT(seconds, nanoseconds);
        if (OS_DURATION_ISINVALID(*out)) {
            rc = SAJ_RETCODE_BAD_PARAMETER;
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durationCopyIn(
    JNIEnv *env,
    jobject javaDuration,
    os_duration *out)
{
    saj_returnCode rc;
    c_long sec;
    c_ulong nsec;

    assert(out != NULL);

    rc = SAJ_RETCODE_OK;

    if (javaDuration != NULL) {
        sec  = GET_INT_FIELD(env, javaDuration, duration_t_sec);
        nsec = GET_INT_FIELD(env, javaDuration, duration_t_nanosec);

        *out = SAJ_DURATION_INIT(sec, nsec);
        if (OS_DURATION_ISINVALID(*out)) {
            rc = SAJ_RETCODE_BAD_PARAMETER;
        }
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durationCopyOut_deprecated(
    JNIEnv *env,
    os_duration *src,
    jobject *dst)
{
    saj_returnCode rc;
    os_int32 seconds, nanoseconds;

    assert(dst);

    rc = SAJ_RETCODE_OK;

    if (OS_DURATION_ISINFINITE(*src)) {
        seconds = SAJ_DURATION_INFINITE_SEC;
        nanoseconds = SAJ_DURATION_INFINITE_NSEC;
    } else {
        seconds = (os_int32)OS_DURATION_GET_SECONDS(*src);
        nanoseconds = OS_DURATION_GET_NANOSECONDS(*src);
    }
    if (*dst == NULL) {
        *dst = NEW_OBJECT(env, GET_CACHED(duration_t_class),
                          GET_CACHED(duration_t_constructor_mid),
                          seconds,
                          nanoseconds);
        rc = *dst == NULL ? SAJ_RETCODE_ERROR : SAJ_RETCODE_OK;
    }
    else
    {
        /* set the nanosec and sec fields of the java object */
        SET_INT_FIELD(env, *dst, duration_t_sec, seconds);
        SET_INT_FIELD(env, *dst, duration_t_nanosec, nanoseconds);
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_durationCopyOut(
    JNIEnv *env,
    os_duration src,
    jobject *dst)
{
    saj_returnCode rc;
    os_int32 seconds, nanoseconds;

    assert(dst);

    rc = SAJ_RETCODE_OK;

    if (OS_DURATION_ISINFINITE(src)) {
        seconds = SAJ_DURATION_INFINITE_SEC;
        nanoseconds = SAJ_DURATION_INFINITE_NSEC;
    } else {
        seconds = (os_int32)OS_DURATION_GET_SECONDS(src);
        nanoseconds = OS_DURATION_GET_NANOSECONDS(src);
    }
    if (*dst == NULL) {
        *dst = NEW_OBJECT(env, GET_CACHED(duration_t_class),
                          GET_CACHED(duration_t_constructor_mid),
                          seconds, nanoseconds);
        rc = *dst == NULL ? SAJ_RETCODE_ERROR : SAJ_RETCODE_OK;
    }
    else
    {
        /* set the nanosec and sec fields of the java object */
        SET_INT_FIELD(env, *dst, duration_t_sec, seconds);
        SET_INT_FIELD(env, *dst, duration_t_nanosec, nanoseconds);
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_vDurationCopyIn(
    JNIEnv *env,
    jobject src,
    v_duration *dst)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    c_long nsec;

    assert (dst);
    assert (src);

    dst->seconds = GET_INT_FIELD(env, src, time_t_sec);
    nsec = GET_INT_FIELD(env, src, time_t_nanosec);

    if(dst->seconds == -1 && nsec == -1){
        dst->seconds = C_TIME_INVALID.seconds;
        dst->nanoseconds = C_TIME_INVALID.nanoseconds;
    } else {
        dst->nanoseconds = (c_ulong)nsec;
    if (!c_timeValid(*dst)) {
            retcode = SAJ_RETCODE_BAD_PARAMETER;
            SAJ_REPORT(retcode, "Invalid time supplied.");
        }
    }
    return retcode;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_vDurationCopyOut(
    JNIEnv *env,
    v_duration *src,
    jobject *dst)
{
    assert(src);
    assert(dst);

    if (*dst == NULL) {
       *dst = NEW_OBJECT(env, GET_CACHED(duration_t_class),
                         GET_CACHED(duration_t_constructor_mid),
                         src->seconds, src->nanoseconds);
        if (*dst == NULL) {
            return SAJ_RETCODE_ERROR;
        }
    } else {
        /* set the nanosec and sec fields of the java object */
        SET_INT_FIELD(env, *dst, duration_t_sec, src->seconds);
        SET_INT_FIELD(env, *dst, duration_t_nanosec, src->nanoseconds);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_timeCopyIn(
    JNIEnv *env,
    jobject src,
    os_timeW *dst)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    os_int64  sec;
    os_int32  nsec;
    os_int64  max_seconds =0;

    assert (dst);
    assert (src);

    if (src != NULL) {
        sec = GET_SECOND_FIELD(env, src, time_t_sec);
        nsec = GET_INT_FIELD(env, src, time_t_nanosec);

        if (GET_CACHED(y2038_enabled) && GET_CACHED(time_t_constructor_mid_time64) != NULL) {
            max_seconds = OS_TIME_MAX_VALID_SECONDS;
        } else if (!GET_CACHED(y2038_enabled) && GET_CACHED(time_t_constructor_mid_time64) != NULL) {
            if (sec > INT32_MAX) {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
                SAJ_REPORT(retcode, "Time is not supported, support for time beyond year 2038 is not enabled");
            }
        } else {
            max_seconds = INT32_MAX;
        }

        if (retcode == SAJ_RETCODE_OK) {
            if (sec >= 0 && sec <= max_seconds && nsec < 1000000000 && nsec >= 0) {
                *dst = OS_TIMEW_INIT(sec, nsec);
            } else if(sec == TIME_INVALID_SEC && nsec == (TIME_INVALID_NSEC -1)){
                /* TIME_CURRENT should map to TIME_INVALID to get the current time in the kernel */
                *dst = OS_TIMEW_INVALID;
            } else {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
                SAJ_REPORT(retcode, "Invalid time supplied.");
            }
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(retcode, "Invalid time supplied.");
    }

    return retcode;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_timeCopyInAcceptInvalidTime(
    JNIEnv *env,
    jobject src,
    os_timeW *dst)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    os_int64  sec;
    os_int32  nsec;
    os_int64  max_seconds =0;

    assert (dst);
    assert (src);

    if (src != NULL) {
        sec = GET_SECOND_FIELD(env, src, time_t_sec);
        nsec = GET_INT_FIELD(env, src, time_t_nanosec);

        if (GET_CACHED(y2038_enabled) && GET_CACHED(time_t_constructor_mid_time64) != NULL) {
            max_seconds = OS_TIME_MAX_VALID_SECONDS;
        } else if (!GET_CACHED(y2038_enabled) && GET_CACHED(time_t_constructor_mid_time64) != NULL) {
            if (sec > INT32_MAX) {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
                SAJ_REPORT(retcode, "Time is not supported, support for time beyond year 2038 is not enabled");
            }
        } else {
            max_seconds = INT32_MAX;
        }

        if (retcode == SAJ_RETCODE_OK) {
            if(sec == TIME_INVALID_SEC && nsec == TIME_INVALID_NSEC){
                *dst = OS_TIMEW_INVALID;
            } else if (sec >= 0 && sec <= max_seconds && nsec < 1000000000 && nsec >= 0) {
                *dst = OS_TIMEW_INIT(sec, nsec);
            } else {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
                SAJ_REPORT(retcode, "Invalid time supplied.");
            }
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(retcode, "Invalid time supplied.");
    }

    return retcode;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_timeCopyOut(
    JNIEnv *env,
    os_timeW src,
    jobject *dst)
{
    os_int64 sec;
    os_int32 nsec;

    assert(dst);

    if (OS_TIMEW_ISINVALID(src)) {
        sec = TIME_INVALID_SEC;
        nsec = TIME_INVALID_NSEC;
    } else if (OS_TIMEW_ISINFINITE(src)) {
        sec = SAJ_TIME_INFINITE_SEC;
        nsec = SAJ_TIME_INFINITE_NSEC;
    } else {
        sec = (os_int64)OS_TIMEW_GET_SECONDS(src);
        nsec = (os_int32)OS_TIMEW_GET_NANOSECONDS(src);
    }
    if (*dst == NULL) {
       if (GET_CACHED(time_t_constructor_mid_time64) != NULL) {
           *dst = NEW_OBJECT(env, GET_CACHED(time_t_class),
                             GET_CACHED(time_t_constructor_mid_time64),
                             sec, nsec);

       } else {
           *dst = NEW_OBJECT(env, GET_CACHED(time_t_class),
                             GET_CACHED(time_t_constructor_mid),
                             (int)sec, nsec);
       }
       if (*dst == NULL) {
           return SAJ_RETCODE_ERROR;
       }
    } else {
        /* set the nanosec and sec fields of the java object */
        SET_SECOND_FIELD(env, *dst, time_t_sec, sec);
        SET_INT_FIELD(env, *dst, time_t_nanosec, nsec);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_timeCopyOutE(
    JNIEnv *env,
    os_timeE src,
    jobject *dst)
{
    os_int64 sec;
    os_int32 nsec;

    assert(dst);

    if (OS_TIMEE_ISINVALID(src)) {
        sec = TIME_INVALID_SEC;
        nsec = TIME_INVALID_NSEC;
    } else if (OS_TIMEE_ISINFINITE(src)) {
        sec = SAJ_DURATION_INFINITE_SEC;
        nsec = SAJ_DURATION_INFINITE_NSEC;
    } else {
        sec = (os_int64)OS_TIMEE_GET_SECONDS(src);
        nsec = (os_int32)OS_TIMEE_GET_NANOSECONDS(src);
    }
    if (*dst == NULL) {
       if (GET_CACHED(time_t_constructor_mid_time64) != NULL) {
           *dst = NEW_OBJECT(env, GET_CACHED(time_t_class),
                             GET_CACHED(time_t_constructor_mid_time64),
                             sec, nsec);

       } else {
           *dst = NEW_OBJECT(env, GET_CACHED(time_t_class),
                             GET_CACHED(time_t_constructor_mid),
                             (int)sec, nsec);
       }
       if (*dst == NULL) {
           return SAJ_RETCODE_ERROR;
       }
    } else {
        /* set the nanosec and sec fields of the java object */
        SET_SECOND_FIELD(env, *dst, time_t_sec, sec);
        SET_INT_FIELD(env, *dst, time_t_nanosec, nsec);
    }
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_stringSequenceCopyIn(
    JNIEnv *env,
    jobjectArray stringArray,
    os_char **out)
{
    jobject             javaString;
    const os_char*    vm_managed_c_string;
    jsize               arrayLength;
    int                 i;
    saj_returnCode      rc;

    assert(stringArray != NULL);
    OS_UNUSED_ARG(out);

    javaString = NULL;
    vm_managed_c_string = NULL;
    arrayLength = 0;
    i = 0;
    rc = SAJ_RETCODE_OK;
    arrayLength = GET_ARRAY_LENGTH(env, stringArray);

    if (stringArray != NULL)
    {
        /* fill the string buffer with strings */
        for (i = 0; i < arrayLength && rc == SAJ_RETCODE_OK; i++)
        {
            /* get the java String from the array */
            javaString = GET_OBJECTARRAY_ELEMENT(env, stringArray, i);

            /* translate the java string to a c string */
            vm_managed_c_string = GET_STRING_UTFCHAR(env, javaString, 0);

            if(vm_managed_c_string != NULL)
            {
                /* copy the c sting to the buffer */
            }
            else
            {
                rc = SAJ_RETCODE_ERROR; /* VM has thrown an OutOfMemoryError */
            }

            /* release local references */
            RELEASE_STRING_UTFCHAR(env, javaString, vm_managed_c_string);
        }
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_builtinTopicKeyCopyOut(
    JNIEnv *env,
    v_builtinTopicKey *src,
    jintArray *dst)
{
    saj_returnCode rc = SAJ_RETCODE_OK;
    assert(dst != NULL);

    *dst = NEW_INTARRAY(env, 3);
    if (*dst != NULL)
    {
        (*env)->SetIntArrayRegion(env, *dst, 0, 3, (jint *)src);
        CHECK_EXCEPTION(env);
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

v_result
saj_subscriptionBuiltinTopicDataCopyOut(
    JNIEnv* env,
    u_subscriptionInfo *src,
    jobject *dst)
{
    jobject policy;
    saj_returnCode rc;
    v_result result = V_RESULT_UNDEFINED;

    assert(dst != NULL);

    rc = SAJ_RETCODE_OK;

    /* copy the attributes from the gapi object to the java object */

    rc = saj_create_new_java_object(env, "DDS/SubscriptionBuiltinTopicData", dst);
    SAJ_CHECK_RESULT(rc);

    /* copy the attributes from the gapi object to the java object */

    policy=NULL;
    rc = saj_builtinTopicKeyCopyOut(env, &src->key, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_key, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinTopicKeyCopyOut(env, &src->participant_key, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_participantKey, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    policy = NEW_STRING_UTF(env, src->topic_name ? src->topic_name : "");
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_topicName, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    policy = NEW_STRING_UTF(env, src->type_name ? src->type_name : "");
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_typeName, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_durabilityQosPolicyCopyOut(env, &src->durability, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_durability, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_latencyBudgetQosPolicyCopyOut(env, &src->latency_budget, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_latencyBudget, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_livelinessQosPolicyCopyOut(env, &src->liveliness, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_liveliness, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_reliabilityQosPolicyCopyOut(env, &src->reliability, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_reliability, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_ownershipQosPolicyCopyOut(env, &src->ownership, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_ownership, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_destinationOrderQosPolicyCopyOut(env, &src->destination_order, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_destinationOrder, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinUserDataQosPolicyCopyOut(env, &src->user_data, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_userData, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_timeBasedFilterQosPolicyCopyOut(env, &src->time_based_filter, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_timeBasedFilter, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_deadlineQosPolicyCopyOut(env, &src->deadline, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_deadline, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_presentationQosPolicyCopyOut(env, &src->presentation, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_presentation, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinPartitionQosPolicyCopyOut(env, &src->partition, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_partition, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinTopicDataQosPolicyCopyOut(env, &src->topic_data, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_topicData, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinGroupDataQosPolicyCopyOut(env, &src->group_data, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, subscriptionBuiltinTopicData_groupData, policy);
    DELETE_LOCAL_REF(env, policy);

    if (rc == SAJ_RETCODE_OK) {
        result = V_RESULT_OK;
    }
    return result;
    CATCH_EXCEPTION: return result;
}


saj_returnCode
saj_publicationBuiltinTopicDataCopyOut(
    JNIEnv* env,
    u_publicationInfo *src,
    jobject *dst)
{
    jobject policy;
    saj_returnCode rc;

    assert(dst != NULL);

    rc = saj_create_new_java_object(env, "DDS/PublicationBuiltinTopicData", dst);
    SAJ_CHECK_RESULT(rc);

    /* copy the attributes from the gapi object to the java object */

    policy=NULL;
    rc = saj_builtinTopicKeyCopyOut(env, &src->key, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_key, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinTopicKeyCopyOut(env, &src->participant_key, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_participantKey, policy);
    DELETE_LOCAL_REF(env, policy);

    policy = NEW_STRING_UTF(env, src->topic_name ? src->topic_name : "");
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_topicName, policy);
    DELETE_LOCAL_REF(env, policy);

    policy = NEW_STRING_UTF(env, src->type_name ? src->type_name : "");
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_typeName, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_durabilityQosPolicyCopyOut(env, &src->durability, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_durability, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_latencyBudgetQosPolicyCopyOut(env, &src->latency_budget, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_latencyBudget, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_livelinessQosPolicyCopyOut(env, &src->liveliness, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_liveliness, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_reliabilityQosPolicyCopyOut(env, &src->reliability, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_reliability, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_ownershipQosPolicyCopyOut(env, &src->ownership, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_ownership, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_ownershipStrengthQosPolicyCopyOut(env, &src->ownership_strength, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_ownershipStrength, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_lifespanQosPolicyCopyOut(env, &src->lifespan, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_lifespan, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_destinationOrderQosPolicyCopyOut(env, &src->destination_order, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_destinationOrder, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinUserDataQosPolicyCopyOut(env, &src->user_data, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_userData, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_deadlineQosPolicyCopyOut(env, &src->deadline, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_deadline, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_presentationQosPolicyCopyOut(env, &src->presentation, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_presentation, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinPartitionQosPolicyCopyOut(env, &src->partition, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_partition, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinTopicDataQosPolicyCopyOut(env, &src->topic_data, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_topicData, policy);
    DELETE_LOCAL_REF(env, policy);

    policy=NULL;
    rc = saj_builtinGroupDataQosPolicyCopyOut(env, &src->group_data, &policy);
    SAJ_CHECK_RESULT(rc);
    SET_OBJECT_FIELD(env, *dst, publicationBuiltinTopicData_groupData, policy);
    DELETE_LOCAL_REF(env, policy);

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_EnumCopyIn(
    JNIEnv      *env,
    jobject     src,
    os_uint32 *dst)
{
    jclass enumClass;
    jfieldID enumValue_fid;
    jmethodID valueMethodId;
    saj_returnCode rc;
    jthrowable jexception = NULL;

    assert(dst != NULL);

    enumClass = NULL;
    enumValue_fid = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL) {
        enumClass = GET_OBJECT_CLASS(env, src);

        /* get __value fieldid from the enum class */
        enumValue_fid = (*env)->GetFieldID(env, enumClass, "__value", "I");
        jexception = EXCEPTION_OCCURRED(env);

        if (jexception) {
            /*  clear the exception */
            EXCEPTION_CLEAR(env);
            valueMethodId = GET_METHOD_ID(env, enumClass, "value", "()I");
            if(valueMethodId != NULL) {
                *dst = CALL_INT_METHOD(env, src, valueMethodId);
            } else {
                rc = SAJ_RETCODE_ERROR;
            }
        } else {
            *dst = (*env)->GetIntField(env, src, enumValue_fid);
            CHECK_EXCEPTION(env);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;

}

saj_returnCode
saj_EnumCopyOut(
    JNIEnv              *env,
    const char          *classname,
    os_uint32  src,
    jobject             *dst)
{
    jclass enumClassId;
    jmethodID from_intMethodId;
    saj_returnCode rc;
    char methodString[255];

    assert(dst != NULL && classname != NULL);
    assert(strlen(classname) < 249);

    enumClassId = NULL;
    from_intMethodId = NULL;
    rc = SAJ_RETCODE_ERROR;

    /* construct a method signature */
    snprintf(methodString, 255, "(I)L%s;", classname);

    enumClassId = FIND_CLASS(env, classname);

    if (enumClassId != NULL){
        from_intMethodId = GET_STATIC_METHOD_ID(env, enumClassId, "from_int", methodString);

        if (from_intMethodId != NULL){
            *dst = CALL_STATIC_OBJECT_METHOD(env, enumClassId, from_intMethodId, src);
            if(*dst != NULL){
                rc = SAJ_RETCODE_OK;
            }
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_stringSequenceCopyOut(
    JNIEnv *env,
    c_string src,
    jobjectArray *dst)
{
    jclass stringArrCls;
    jstring jStr;
    c_iter list;
    os_uint32 i, length;
    char *str;
    saj_returnCode rc;

    assert(dst != NULL);

    jStr = NULL;
    rc = SAJ_RETCODE_OK;

    stringArrCls = FIND_CLASS(env, "java/lang/String");

    assert(stringArrCls != NULL);

    list = c_splitString(src, ",");
    length = c_iterLength(list);

    *dst = NEW_OBJECTARRAY(env, length, stringArrCls, NULL);

    /* get the c strings from the buffer */
    for (i = 0; i < length && rc == SAJ_RETCODE_OK; i++)
    {
        str = c_iterTakeFirst(list);
        jStr = NEW_STRING_UTF(env, str);
        os_free(str);

        if (jStr != NULL) {
            /* store the string object in the string array */
            SET_OBJECTARRAY_ELEMENT(env, *dst, i, jStr);
        } else {
            rc = SAJ_RETCODE_ERROR;
        }
        DELETE_LOCAL_REF(env, jStr);
    }
    c_iterFree(list);
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_sampleInfoCopyOut(
    JNIEnv              *env,
    cmn_sampleInfo       src,
    jobject             *dst)
{
    jobject source_timestamp;
    jobject reception_timestamp;

    assert (src);
    assert (dst);

    if (*dst == NULL) {
       *dst = NEW_OBJECT(env, GET_CACHED(sampleInfo_class),
                         GET_CACHED(sampleInfo_constructor_mid), NULL);

       if (*dst == NULL) {
           return SAJ_RETCODE_ERROR;
       }
    }
    SET_INT_FIELD(env, *dst, sampleInfo_sample_state, src->sample_state);
    SET_INT_FIELD(env, *dst, sampleInfo_view_state, src->view_state);
    SET_INT_FIELD(env, *dst, sampleInfo_instance_state, src->instance_state);
    SET_BOOLEAN_FIELD(env, *dst, sampleInfo_valid_data, src->valid_data);
    source_timestamp = (*env)->GetObjectField (env, *dst, GET_CACHED(sampleInfo_source_timestamp_fid));
    CHECK_EXCEPTION(env);
    saj_timeCopyOut (env,  src->source_timestamp, &source_timestamp);
    CHECK_EXCEPTION(env);

    if (source_timestamp == NULL) {
       return SAJ_RETCODE_ERROR;
    }
    (*env)->SetObjectField (env, *dst, GET_CACHED(sampleInfo_source_timestamp_fid), source_timestamp);
    CHECK_EXCEPTION(env);
    DELETE_LOCAL_REF(env, source_timestamp);

    SET_LONG_FIELD(env, *dst, sampleInfo_instance_handle, src->instance_handle);
    SET_LONG_FIELD(env, *dst, sampleInfo_publication_handle, src->publication_handle);

    SET_INT_FIELD(env, *dst, sampleInfo_disposed_generation_count, src->disposed_generation_count);
    SET_INT_FIELD(env, *dst, sampleInfo_no_writers_generation_count, src->no_writers_generation_count);
    SET_INT_FIELD(env, *dst, sampleInfo_sample_rank, src->sample_rank);
    SET_INT_FIELD(env, *dst, sampleInfo_generation_rank, src->generation_rank);
    SET_INT_FIELD(env, *dst, sampleInfo_absolute_generation_rank, src->absolute_generation_rank);

    reception_timestamp = (*env)->GetObjectField (env, *dst, GET_CACHED(sampleInfo_reception_timestamp_fid));
    CHECK_EXCEPTION(env);
    saj_timeCopyOut (env,  src->reception_timestamp, &reception_timestamp);
    CHECK_EXCEPTION(env);

    if (reception_timestamp == NULL) {
       return SAJ_RETCODE_ERROR;
    }
    (*env)->SetObjectField (env, *dst, GET_CACHED(sampleInfo_reception_timestamp_fid), reception_timestamp);
    CHECK_EXCEPTION(env);
    DELETE_LOCAL_REF(env, reception_timestamp);
    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_conditionSeqCopy(
    JNIEnv *env,
    c_iter *src,
    jobjectArray *dst)
{
    jobject object;
    jclass classId;
    os_uint32 i, length;
    saj_returnCode rc;

    assert(dst != NULL);

    classId = NULL;
    rc = SAJ_RETCODE_ERROR;

    /* find the class id of the sequence class */
    classId = FIND_CLASS(env, "DDS/Condition");

    assert(classId != NULL);

    if (classId != NULL) {
        length = c_iterLength(*src);
        *dst = NEW_OBJECTARRAY(env, length, classId, NULL);

        if (*dst != NULL) {
            rc = SAJ_RETCODE_OK;
            object = c_iterTakeFirst(*src);
            i = 0;
            while ((object != NULL) && (i<length)) {
                (*env)->SetObjectArrayElement(env, *dst, i++, object);
                CHECK_EXCEPTION(env);
                object = c_iterTakeFirst(*src);
            }
        }
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_LookupTypeSupportDataReader(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportDataReader_fid);

        if(fid != NULL)
        {
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            CHECK_EXCEPTION(env);

            if(jresult != NULL){
               data = GET_STRING_UTFCHAR(env, jresult, 0);
               *result = NULL; /* TODO : copy data */
               RELEASE_STRING_UTFCHAR(env, jresult, data);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_LookupTypeSupportDataReaderView(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportDataReaderView_fid);

        if(fid != NULL)
        {
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            CHECK_EXCEPTION(env);

            if(jresult != NULL){
               data = GET_STRING_UTFCHAR(env, jresult, 0);
               *result = NULL; /* TODO : copy out data. */
               RELEASE_STRING_UTFCHAR(env, jresult, data);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_LookupTypeSupportDataWriter(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    assert(result != NULL);

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportDataWriter_fid);

        if(fid != NULL){
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            CHECK_EXCEPTION(env);

            if(jresult != NULL){
                data = GET_STRING_UTFCHAR(env, jresult, 0);
                *result = NULL; /* TODO : copy ou data */;
                RELEASE_STRING_UTFCHAR(env, jresult, data);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_LookupTypeSupportConstructorSignature(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportConstructorSignature_fid);

        if(fid != NULL)
        {
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            CHECK_EXCEPTION(env);

            if(jresult != NULL){
                data = GET_STRING_UTFCHAR(env, jresult, 0);
                *result = NULL; /* TODO : copy out data. */
                RELEASE_STRING_UTFCHAR(env, jresult, data);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_write_parallelDemarshallingContext_address(
    JNIEnv *env,
    jobject java_object,
    sajParDemContext address)
{
    saj_returnCode rc = SAJ_RETCODE_ERROR;
    os_boolean isInstanceOf;

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if(rc == SAJ_RETCODE_OK){
            /* Verify the java_object is an instance of DataReaderImpl */
            assert(GET_CACHED(dataReaderImpl_class));
            isInstanceOf = IS_INSTANCE_OF(env, java_object, GET_CACHED(dataReaderImpl_class));
            if (!isInstanceOf) {
                THROW_EXCEPTION;
            }

            assert(GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
            /* write value to java object */
            SET_LONG_FIELD(env, java_object, dataReaderImplClassParallelDemarshallingContext, (PA_ADDRCAST)address);
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_read_parallelDemarshallingContext_address(
    JNIEnv *env,
    jobject java_object,
    sajParDemContext *address)
{
    saj_returnCode rc = SAJ_RETCODE_ERROR;
    os_boolean isInstanceOf;

    assert(address);

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if(rc == SAJ_RETCODE_OK){
            /* Verify the java_object is an instance of DataReaderImpl */
            assert(GET_CACHED(dataReaderImpl_class));
            isInstanceOf = IS_INSTANCE_OF(env, java_object, GET_CACHED(dataReaderImpl_class));
            if (!isInstanceOf) {
                rc = SAJ_RETCODE_ERROR;
            }

            if( rc == SAJ_RETCODE_OK){
                assert(GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
                /* get value from java object */
                *address = (sajParDemContext)SAJ_VOIDP((*env)->GetLongField(env, java_object, GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid)));
                CHECK_EXCEPTION(env);
            }
        }
    }
    if(rc != SAJ_RETCODE_OK){
        *address = NULL; /* Ensure out-param is always initialized */
    }

    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

static saj_returnCode saj_prepare_CDRCopy (JNIEnv *env, jobject obj)
{
    /* obj is an instance of <Type>DataReaderImpl, I'm pretty sure. That
     * one has a "private long copyCache" containing the address of the
     * relevant copyCache.
     */
    jclass objClass;
    jfieldID fid;
    saj_copyCache copyCache;

    objClass = GET_OBJECT_CLASS(env, obj);
    fid = GET_FIELD_ID(env, objClass, "copyCache", "J");
    if (fid == NULL) {
        return SAJ_RETCODE_ERROR;
    }
    copyCache = (saj_copyCache) SAJ_VOIDP((*env)->GetLongField (env, obj, fid));
    CHECK_EXCEPTION(env);
    if (sd_cdrCompile (saj_copyCacheCdrInfo (copyCache)) < 0) {
        return SAJ_RETCODE_ERROR;
    } else {
        return SAJ_RETCODE_OK;
    }
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

saj_returnCode
saj_write_CDRCopy_value(
    JNIEnv *env,
    jobject jdatareader,
    long value)
{
    saj_returnCode rc = SAJ_RETCODE_ERROR;
    os_boolean isInstanceOf;
    jboolean setupOk;

    if(jdatareader != NULL){
        rc = checkJavaObject(env, jdatareader);

        if(rc == SAJ_RETCODE_OK){
            /* Verify the java_object is an instance of DataReaderImpl */
            assert(GET_CACHED(dataReaderImpl_class));
            isInstanceOf = IS_INSTANCE_OF(env, jdatareader, GET_CACHED(dataReaderImpl_class));
            if (!isInstanceOf) {
                THROW_EXCEPTION;
            }

            assert(GET_CACHED(dataReaderImplClassCDRCopy_fid));
            assert(GET_CACHED(dataReaderImplClassCDRCopySetupHelper_mid));

            setupOk = CALL_BOOLEAN_METHOD(env, jdatareader, GET_CACHED(dataReaderImplClassCDRCopySetupHelper_mid));
            if(!setupOk){
                THROW_EXCEPTION;
            }

            if (value) {
                rc = saj_prepare_CDRCopy (env, jdatareader);
            }

            if (rc == SAJ_RETCODE_OK) {
                SET_LONG_FIELD(env, jdatareader, dataReaderImplClassCDRCopy, value);
            } else {
                OS_REPORT (OS_ERROR, "dcpssaj", 0, "saj_write_CDRCopy_value: unsupported type");
            }
        }
    }
    return rc;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

jobject
saj_inconsistentTopicStatus_new(
    JNIEnv  *env,
    struct v_inconsistentTopicInfo *info)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(inconsistentTopicStatus_class),
                      GET_CACHED(inconsistentTopicStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_livelinessLostStatus_new(
    JNIEnv  *env,
    struct v_livelinessLostInfo *info)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(livelinessLostStatus_class),
                      GET_CACHED(livelinessLostStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_sampleLostStatus_new(
    JNIEnv  *env,
    struct v_sampleLostInfo *info)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(sampleLostStatus_class),
                      GET_CACHED(sampleLostStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_offeredDeadlineMissedStatus_new(
    JNIEnv *env,
    struct v_deadlineMissedInfo *info)
{
    v_handleResult handleResult;
    v_public instance;
    u_instanceHandle handle;
    jobject jobj;

    handle = U_INSTANCEHANDLE_NIL;
    handleResult = v_handleClaim(info->instanceHandle, (v_object *) &instance);
    if (handleResult == V_HANDLE_OK) {
        handle = u_instanceHandleNew(v_public(instance));
        handleResult = v_handleRelease(info->instanceHandle);
    }

    jobj = NEW_OBJECT(env, GET_CACHED(offeredDeadlineMissedStatus_class),
                      GET_CACHED(offeredDeadlineMissedStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      (jlong)handle);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_requestedDeadlineMissedStatus_new(
    JNIEnv  *env,
    struct v_deadlineMissedInfo *info)
{
    v_handleResult handleResult;
    v_public instance;
    u_instanceHandle handle;
    jobject jobj;

    handle = U_INSTANCEHANDLE_NIL;
    handleResult = v_handleClaim(info->instanceHandle, (v_object *) &instance);
    if (handleResult == V_HANDLE_OK) {
        handle = u_instanceHandleNew(v_public(instance));
        handleResult = v_handleRelease(info->instanceHandle);
    }
    jobj = NEW_OBJECT(env, GET_CACHED(requestedDeadlineMissedStatus_class),
                      GET_CACHED(requestedDeadlineMissedStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      (jlong)handle);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_sampleRejectedStatus_new(
    JNIEnv  *env,
    struct v_sampleRejectedInfo *info)
{
    jclass cls;
    jmethodID mid;
    jobject jkind, jobj;

    cls = GET_CACHED(sampleRejectedStatusKind_class);
    mid = GET_CACHED(sampleRejectedStatusKind_fromInt_mid);

    jkind = CALL_STATIC_OBJECT_METHOD(env, cls, mid, (jint)info->lastReason);

    jobj = NEW_OBJECT(env, GET_CACHED(sampleRejectedStatus_class),
                      GET_CACHED(sampleRejectedStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      jkind,
                      (jlong)u_instanceHandleFromGID(info->instanceHandle));

    DELETE_LOCAL_REF(env, jkind);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_offeredIncompatibleQosStatus_new(
    JNIEnv  *env,
    struct v_incompatibleQosInfo *info)
{
    jobjectArray jqosCount;
    jobject jcount;
    jobject jstatus;
    unsigned long i;

    jqosCount = NEW_OBJECTARRAY(env, V_POLICY_ID_COUNT, GET_CACHED(qosPolicyCount_class), NULL);
    for(i=0; i<V_POLICY_ID_COUNT; i++) {
        jcount = NEW_OBJECT(env, GET_CACHED(qosPolicyCount_class),
                            GET_CACHED(qosPolicyCount_constructor_mid),
                            i, ((c_long *)info->policyCount)[i]);
        SET_OBJECTARRAY_ELEMENT(env, jqosCount, i, jcount);
        DELETE_LOCAL_REF(env, jcount);
    }
    jstatus = NEW_OBJECT(env, GET_CACHED(offeredIncompatibleQosStatus_class),
                      GET_CACHED(offeredIncompatibleQosStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      (jint)info->lastPolicyId,
                      jqosCount);
    DELETE_LOCAL_REF(env, jqosCount);

    return jstatus;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_requestedIncompatibleQosStatus_new(
    JNIEnv  *env,
    struct v_incompatibleQosInfo *info)
{
    jobjectArray jqosCount;
    jobject jcount;
    jobject jstatus;
    unsigned long i;

    jqosCount = NEW_OBJECTARRAY(env, V_POLICY_ID_COUNT, GET_CACHED(qosPolicyCount_class), NULL);
    for(i=0; i<V_POLICY_ID_COUNT; i++) {
        jcount = NEW_OBJECT(env, GET_CACHED(qosPolicyCount_class),
                      GET_CACHED(qosPolicyCount_constructor_mid),
                      i, info->policyCount[i]);
        SET_OBJECTARRAY_ELEMENT(env, jqosCount, i, jcount);
        DELETE_LOCAL_REF(env, jcount);
    }
    jstatus =  NEW_OBJECT(env, GET_CACHED(requestedIncompatibleQosStatus_class),
                      GET_CACHED(requestedIncompatibleQosStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      (jint)info->lastPolicyId,
                      jqosCount);
    DELETE_LOCAL_REF(env, jqosCount);

    return jstatus;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_livelinessChangedStatus_new(
    JNIEnv  *env,
    struct v_livelinessChangedInfo *info)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(livelinessChangedStatus_class),
                      GET_CACHED(livelinessChangedStatus_constructor_mid),
                      (jint)info->activeCount,
                      (jint)info->inactiveCount,
                      (jint)info->activeChanged,
                      (jint)info->inactiveChanged,
                      (jlong)u_instanceHandleFromGID(info->instanceHandle));

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_publicationMatchStatus_new(
    JNIEnv  *env,
    struct v_topicMatchInfo *info)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(publicationMatchStatus_class),
                      GET_CACHED(publicationMatchStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      (jint)info->currentCount,
                      (jint)info->currentChanged,
                      (jlong)u_instanceHandleFromGID(info->instanceHandle));

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

jobject
saj_subscriptionMatchStatus_new(
    JNIEnv  *env,
    struct v_topicMatchInfo *info)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(subscriptionMatchStatus_class),
                      GET_CACHED(subscriptionMatchStatus_constructor_mid),
                      (jint)info->totalCount,
                      (jint)info->totalChanged,
                      (jint)info->currentCount,
                      (jint)info->currentChanged,
                      (jlong)u_instanceHandleFromGID(info->instanceHandle));

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

#define DDS_INCONSISTENT_TOPIC_STATUS         (0x0001L << 0L)
#define DDS_OFFERED_DEADLINE_MISSED_STATUS    (0x0001L << 1L)
#define DDS_REQUESTED_DEADLINE_MISSED_STATUS  (0x0001L << 2L)
#define DDS_OFFERED_INCOMPATIBLE_QOS_STATUS   (0x0001L << 5L)
#define DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS (0x0001L << 6L)
#define DDS_SAMPLE_LOST_STATUS                (0x0001L << 7L)
#define DDS_SAMPLE_REJECTED_STATUS            (0x0001L << 8L)
#define DDS_DATA_ON_READERS_STATUS            (0x0001L << 9L)
#define DDS_DATA_AVAILABLE_STATUS             (0x0001L << 10L)
#define DDS_LIVELINESS_LOST_STATUS            (0x0001L << 11L)
#define DDS_LIVELINESS_CHANGED_STATUS         (0x0001L << 12L)
#define DDS_PUBLICATION_MATCHED_STATUS        (0x0001L << 13L)
#define DDS_SUBSCRIPTION_MATCHED_STATUS       (0x0001L << 14L)
#define DDS_PREPARE_DELETE                    (0x0001L << 28L)
#define DDS_OBJECT_DESTROYED                  (0x0001L << 30L)
#define DDS_ALL_DATA_DISPOSED_TOPIC_STATUS    (0x0001L << 31L)

jobject
saj_eventNew(
    JNIEnv *env,
    unsigned int kind,
    jobject jentity,
    jobject jobserver,
    jobject jstatus)
{
    jobject jobj;

    jobj = NEW_OBJECT(env, GET_CACHED(event_class), GET_CACHED(event_constructor_mid),
                      (jint)kind, jentity, jobserver, jstatus);

    return jobj;
    CATCH_EXCEPTION: return NULL;
}

c_iter
saj_eventListNew(
    JNIEnv *env,
    v_listenerEvent event)
{
    jobject jentity = NULL;
    jobject jobserver = NULL;
    jobject jstatus = NULL;
    jobject jevent = NULL;
    unsigned int kind = 0;
    c_iter eventList = NULL;

    /* bootstrap issue, avoid crash */
    if (event->source != NULL) {
        jentity = u_observableGetUserData(u_observable(event->source));
    }
    jobserver = u_observableGetUserData(u_observable(event->userData));

    if (jobserver == NULL) {
        SAJ_REPORT(SAJ_RETCODE_ERROR, "JNI Observer null for kind %d",event->kind);
    }

    if (event->kind & (V_EVENT_OBJECT_DESTROYED | V_EVENT_PREPARE_DELETE)) {
        if (event->kind & V_EVENT_OBJECT_DESTROYED) {
            jevent = saj_eventNew(env, DDS_OBJECT_DESTROYED, jentity, jobserver, NULL);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_PREPARE_DELETE) {
            jevent = saj_eventNew (env, DDS_PREPARE_DELETE, jentity, jobserver, NULL);
            eventList = c_iterInsert (eventList, jevent);
        }
        return eventList;
    }
    if (event->kind & V_EVENT_TRIGGER) {
        /* Nothing to deliver so ignore. */
        return NULL;
    }
    switch(u_objectKind(event->userData)) {
    case U_PARTICIPANT:
        if (event->kind & V_EVENT_ON_DATA_ON_READERS) {
            kind = DDS_DATA_ON_READERS_STATUS;
            jstatus = NULL;
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_DATA_AVAILABLE) {
            kind = DDS_DATA_AVAILABLE_STATUS;
            jstatus = NULL;
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_INCONSISTENT_TOPIC) {
            kind = DDS_INCONSISTENT_TOPIC_STATUS;
            jstatus = saj_inconsistentTopicStatus_new(
                          env, &v_topicStatus(event->eventData)->inconsistentTopic);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_SAMPLE_REJECTED) {
            kind = DDS_SAMPLE_REJECTED_STATUS;
            jstatus = saj_sampleRejectedStatus_new(
                          env, &v_readerStatus(event->eventData)->sampleRejected);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_LIVELINESS_CHANGED) {
            kind = DDS_LIVELINESS_CHANGED_STATUS;
            jstatus = saj_livelinessChangedStatus_new(
                          env, &v_readerStatus(event->eventData)->livelinessChanged);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_LIVELINESS_LOST) {
            kind = DDS_LIVELINESS_LOST_STATUS;
            jstatus = saj_livelinessLostStatus_new(
                            env,  &v_writerStatus(event->eventData)->livelinessLost);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_OFFERED_DEADLINE_MISSED) {
            kind = DDS_OFFERED_DEADLINE_MISSED_STATUS;
            jstatus = saj_offeredDeadlineMissedStatus_new(
                          env, &v_writerStatus(event->eventData)->deadlineMissed);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            kind = DDS_REQUESTED_DEADLINE_MISSED_STATUS;
            jstatus = saj_requestedDeadlineMissedStatus_new(
                          env, &v_readerStatus(event->eventData)->deadlineMissed);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            kind = DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
            jstatus = saj_offeredIncompatibleQosStatus_new(
                          env, &v_writerStatus(event->eventData)->incompatibleQos);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            kind = DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
            jstatus = saj_requestedIncompatibleQosStatus_new(
                          env, &v_readerStatus(event->eventData)->incompatibleQos);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_SAMPLE_LOST) {
            kind = DDS_SAMPLE_LOST_STATUS;
            jstatus = saj_sampleLostStatus_new(
                          env, &v_readerStatus(event->eventData)->sampleLost);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_PUBLICATION_MATCHED) {
            kind = DDS_PUBLICATION_MATCHED_STATUS;
            jstatus = saj_publicationMatchStatus_new(
                          env, &v_writerStatus(event->eventData)->publicationMatch);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_SUBSCRIPTION_MATCHED) {
            kind = DDS_SUBSCRIPTION_MATCHED_STATUS;
            jstatus = saj_subscriptionMatchStatus_new(
                          env, &v_readerStatus(event->eventData)->subscriptionMatch);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_ALL_DATA_DISPOSED) {
            kind = DDS_ALL_DATA_DISPOSED_TOPIC_STATUS;
            jstatus = NULL;
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
    break;
    case U_TOPIC:
    {
        v_topicStatus status = v_topicStatus(event->eventData);
        if (event->kind & V_EVENT_INCONSISTENT_TOPIC) {
            kind = DDS_INCONSISTENT_TOPIC_STATUS;
            jstatus = saj_inconsistentTopicStatus_new(env, &status->inconsistentTopic);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_ALL_DATA_DISPOSED) {
            kind = DDS_ALL_DATA_DISPOSED_TOPIC_STATUS;
            jstatus = NULL;
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
    }
    break;
    case U_PUBLISHER:
    case U_WRITER:
    {
        v_writerStatus status = v_writerStatus(event->eventData);
        if (event->kind & V_EVENT_LIVELINESS_LOST) {
            kind = DDS_LIVELINESS_LOST_STATUS;
            jstatus = saj_livelinessLostStatus_new(env, &status->livelinessLost);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_OFFERED_DEADLINE_MISSED) {
            kind = DDS_OFFERED_DEADLINE_MISSED_STATUS;
            jstatus = saj_offeredDeadlineMissedStatus_new(env, &status->deadlineMissed);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            kind = DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
            jstatus = saj_offeredIncompatibleQosStatus_new(env, &status->incompatibleQos);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_PUBLICATION_MATCHED) {
            kind = DDS_PUBLICATION_MATCHED_STATUS;

            jstatus = saj_publicationMatchStatus_new(env, &status->publicationMatch);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
    }
    break;
    case U_SUBSCRIBER:
    {
        if (event->kind & V_EVENT_ON_DATA_ON_READERS) {
            kind = DDS_DATA_ON_READERS_STATUS;
            jstatus = NULL;
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
            break;
        }
    }
    /* If not ON_DATA_ON_READERS then the event is from a reader so fall through to reader part
     * to copy the status information.
     */
    case U_READER:
    {
        v_readerStatus status = v_readerStatus(event->eventData);
        if (event->kind & V_EVENT_DATA_AVAILABLE) {
            kind = DDS_DATA_AVAILABLE_STATUS;
            jstatus = NULL;
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_SAMPLE_REJECTED) {
            kind = DDS_SAMPLE_REJECTED_STATUS;
            jstatus = saj_sampleRejectedStatus_new(env, &status->sampleRejected);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_LIVELINESS_CHANGED) {
            kind = DDS_LIVELINESS_CHANGED_STATUS;
            jstatus = saj_livelinessChangedStatus_new(env, &status->livelinessChanged);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            kind = DDS_REQUESTED_DEADLINE_MISSED_STATUS;
            jstatus = saj_requestedDeadlineMissedStatus_new(env, &status->deadlineMissed);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            kind = DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
            jstatus = saj_requestedIncompatibleQosStatus_new(env, &status->incompatibleQos);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_SAMPLE_LOST) {
            kind = DDS_SAMPLE_LOST_STATUS;
            jstatus = saj_sampleLostStatus_new(env, &status->sampleLost);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
        if (event->kind & V_EVENT_SUBSCRIPTION_MATCHED) {
            kind = DDS_SUBSCRIPTION_MATCHED_STATUS;
            jstatus = saj_subscriptionMatchStatus_new(env, &status->subscriptionMatch);
            jevent = saj_eventNew(env, kind, jentity, jobserver, jstatus);
            eventList = c_iterInsert(eventList, jevent);
        }
    }
    break;
    default:
    break;
    }
    return eventList;
}

u_eventMask
saj_statusMask_to_eventMask(
    jint mask)
{
    jint uMask = 0;

    if (mask & DDS_INCONSISTENT_TOPIC_STATUS) {
        uMask |= V_EVENT_INCONSISTENT_TOPIC;
    }
    if (mask & DDS_LIVELINESS_LOST_STATUS) {
        uMask |= V_EVENT_LIVELINESS_LOST;
    }
    if (mask & DDS_OFFERED_DEADLINE_MISSED_STATUS) {
        uMask |= V_EVENT_OFFERED_DEADLINE_MISSED;
    }
    if (mask & DDS_OFFERED_INCOMPATIBLE_QOS_STATUS) {
        uMask |= V_EVENT_OFFERED_INCOMPATIBLE_QOS;
    }
    if (mask & DDS_DATA_ON_READERS_STATUS) {
        uMask |= V_EVENT_ON_DATA_ON_READERS;
    }
    if (mask & DDS_SAMPLE_LOST_STATUS) {
        uMask |= V_EVENT_SAMPLE_LOST;
    }
    if (mask & DDS_DATA_AVAILABLE_STATUS) {
        uMask |= V_EVENT_DATA_AVAILABLE;
    }
    if (mask & DDS_SAMPLE_REJECTED_STATUS) {
        uMask |= V_EVENT_SAMPLE_REJECTED;
    }
    if (mask & DDS_LIVELINESS_CHANGED_STATUS) {
        uMask |= V_EVENT_LIVELINESS_CHANGED;
    }
    if (mask & DDS_REQUESTED_DEADLINE_MISSED_STATUS) {
        uMask |= V_EVENT_REQUESTED_DEADLINE_MISSED;
    }
    if (mask & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS) {
        uMask |= V_EVENT_REQUESTED_INCOMPATIBLE_QOS;
    }
    if (mask & DDS_PUBLICATION_MATCHED_STATUS) {
        uMask |= V_EVENT_PUBLICATION_MATCHED;
    }
    if (mask & DDS_SUBSCRIPTION_MATCHED_STATUS) {
        uMask |= V_EVENT_SUBSCRIPTION_MATCHED;
    }
    if (mask & DDS_ALL_DATA_DISPOSED_TOPIC_STATUS) {
        uMask |= V_EVENT_ALL_DATA_DISPOSED;
    }
    return uMask;
}

void
saj_eventMask_to_statusMask(
    v_public p,
    void *arg)
{
    jint *mask = (jint *) arg;
    c_ulong vMask = v_entityStatusGetMask(v_entity(p));
    v_kind kind = v_objectKind(p);

    *mask = 0;
    switch(kind) {
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        if (vMask & V_EVENT_INCONSISTENT_TOPIC) {
            *mask |= DDS_INCONSISTENT_TOPIC_STATUS;
        }
        if (vMask & V_EVENT_ALL_DATA_DISPOSED) {
            *mask |= DDS_ALL_DATA_DISPOSED_TOPIC_STATUS;
        }
        break;
    case K_WRITER:
    case K_PUBLISHER:
        if (vMask & V_EVENT_LIVELINESS_LOST) {
            *mask |= DDS_LIVELINESS_LOST_STATUS;
        }
        if (vMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            *mask |= DDS_OFFERED_DEADLINE_MISSED_STATUS;
        }
        if (vMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            *mask |= DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
        }
        if (vMask & V_EVENT_PUBLICATION_MATCHED) {
            *mask |= DDS_PUBLICATION_MATCHED_STATUS;
        }
        break;
    case K_SUBSCRIBER:
        if (vMask & V_EVENT_ON_DATA_ON_READERS) {
            *mask |= DDS_DATA_ON_READERS_STATUS;
        }
        /* If not ON_DATA_ON_READERS then the event is from a reader so fall through to reader part */
     case K_DATAREADER:
        if (vMask & V_EVENT_SAMPLE_REJECTED) {
            *mask |= DDS_SAMPLE_REJECTED_STATUS;
        }
        if (vMask & V_EVENT_LIVELINESS_CHANGED) {
            *mask |= DDS_LIVELINESS_CHANGED_STATUS;
        }
        if (vMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            *mask |= DDS_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if (vMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            *mask |= DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if (vMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            *mask |= DDS_SUBSCRIPTION_MATCHED_STATUS;
        }
        if (vMask & V_EVENT_DATA_AVAILABLE) {
            *mask |= DDS_DATA_AVAILABLE_STATUS;
        }
        if (vMask & V_EVENT_SAMPLE_LOST) {
            *mask |= DDS_SAMPLE_LOST_STATUS;
        }
        break;
    case K_PARTICIPANT:
        if (vMask & V_EVENT_INCONSISTENT_TOPIC) {
            *mask |= DDS_INCONSISTENT_TOPIC_STATUS;
        }
        if (vMask & V_EVENT_ALL_DATA_DISPOSED) {
            *mask |= DDS_ALL_DATA_DISPOSED_TOPIC_STATUS;
        }
        if (vMask & V_EVENT_ON_DATA_ON_READERS) {
            *mask |= DDS_DATA_ON_READERS_STATUS;
        }
        if (vMask & V_EVENT_LIVELINESS_LOST) {
            *mask |= DDS_LIVELINESS_LOST_STATUS;
        }
        if (vMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            *mask |= DDS_OFFERED_DEADLINE_MISSED_STATUS;
        }
        if (vMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            *mask |= DDS_OFFERED_INCOMPATIBLE_QOS_STATUS;
        }
        if (vMask & V_EVENT_PUBLICATION_MATCHED) {
            *mask |= DDS_PUBLICATION_MATCHED_STATUS;
        }
        if (vMask & V_EVENT_SAMPLE_REJECTED) {
            *mask |= DDS_SAMPLE_REJECTED_STATUS;
        }
        if (vMask & V_EVENT_LIVELINESS_CHANGED) {
            *mask |= DDS_LIVELINESS_CHANGED_STATUS;
        }
        if (vMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            *mask |= DDS_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if (vMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            *mask |= DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if (vMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            *mask |= DDS_SUBSCRIPTION_MATCHED_STATUS;
        }
        if (vMask & V_EVENT_DATA_AVAILABLE) {
            *mask |= DDS_DATA_AVAILABLE_STATUS;
        }
        if (vMask & V_EVENT_SAMPLE_LOST) {
            *mask |= DDS_SAMPLE_LOST_STATUS;
        }
        break;
    default:
        assert(0);
    }
}

void* saj_createCopyCache(void* arg) {
    void* pvReturn = NULL;
    saj_createCopyCacheArg *args = NULL;
    if (arg != NULL) {
        args = arg;
        pvReturn = (void*)saj_copyCacheNew (args->Env, args->typeMeta, args->redirects);
    }
    return pvReturn;
}

#undef SAJ_CHECK_RESULT
