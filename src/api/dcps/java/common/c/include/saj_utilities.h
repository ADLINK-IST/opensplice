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
/**
 * @file api/dcps/saj/include
 * @brief Utility functions for use the saj JNI implementation.
 */

#ifndef SAJ_UTILITIES_H
#define SAJ_UTILITIES_H

#include <jni.h>
#include <stdio.h>
#include "u_user.h"
#include "os_if.h"
#include "os_abstract.h"
#include "cmn_samplesList.h"

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

#define DDS_READ_SAMPLE_STATE 1U
#define DDS_NOT_READ_SAMPLE_STATE 2U
#define DDS_NEW_VIEW_STATE 1U
#define DDS_NOT_NEW_VIEW_STATE 2U
#define DDS_ALIVE_INSTANCE_STATE 1U
#define DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE 2U
#define DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE 4U

#define DDS_SAMPLE_STATE_FLAGS \
        (DDS_READ_SAMPLE_STATE | \
         DDS_NOT_READ_SAMPLE_STATE)

#define DDS_VIEW_STATE_FLAGS \
        (DDS_NEW_VIEW_STATE | \
         DDS_NOT_NEW_VIEW_STATE)

#define DDS_INSTANCE_STATE_FLAGS \
        (DDS_ALIVE_INSTANCE_STATE | \
         DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE | \
         DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)

#define DDS_SAMPLE_MASK_ANY (65535)
#define DDS_SAMPLE_MASK(sample_states, view_states, instance_states) \
        (sample_states & DDS_SAMPLE_STATE_FLAGS) | \
        ((view_states & DDS_VIEW_STATE_FLAGS) << 2) | \
        ((instance_states & DDS_INSTANCE_STATE_FLAGS) << 4)

#define DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states) \
        (((sample_states == DDS_SAMPLE_MASK_ANY ? 0 : sample_states & ~DDS_SAMPLE_STATE_FLAGS) | \
          (view_states == DDS_SAMPLE_MASK_ANY ? 0 : view_states & ~DDS_VIEW_STATE_FLAGS) | \
          (instance_states == DDS_SAMPLE_MASK_ANY ? 0 : instance_states & ~DDS_INSTANCE_STATE_FLAGS)) == 0 ? SAJ_RETCODE_OK : SAJ_RETCODE_BAD_PARAMETER)

#define SET_CACHED(var, value) jniCache.var = value
#define GET_CACHED(var) jniCache.var

#define THROW_EXCEPTION goto CATCH_EXCEPTION

#define CHECK_EXCEPTION(env) \
        do { \
            if ((*env)->ExceptionCheck(env)) { \
                fprintf(stderr, "Exception at line %d in file %s\n", __LINE__, __FILE__); \
                fflush(stderr); \
                (*env)->ExceptionDescribe(env); \
                (*env)->ExceptionClear(env); \
                THROW_EXCEPTION; \
            } \
        } while (0)

/* In a C99 VARARGS macro, the VARARGS may not be empty. By including the last
 * required argument to NewObject in the __VA_ARGS__, we don't need to add two
 * macro's for the case with and without additional arguments. */
#define NEW_OBJECT(env, cls, /* mid,*/ ...) \
        (*env)->NewObject(env, cls, __VA_ARGS__); \
        CHECK_EXCEPTION(env)

#define NEW_OBJECTARRAY(env, len, cls, arg) \
        (*env)->NewObjectArray(env, len, cls, arg); \
        CHECK_EXCEPTION(env)

#define NEW_LONGARRAY(env, len) \
        (*env)->NewLongArray(env, len); \
        CHECK_EXCEPTION(env)

#define NEW_INTARRAY(env, len) \
        (*env)->NewIntArray(env, len); \
        CHECK_EXCEPTION(env)

#define NEW_BYTE_ARRAY(env, len) \
        (*env)->NewByteArray(env, len); \
        CHECK_EXCEPTION(env)

#define SET_BYTE_ARRAY_REGION(env, byteArray, idx, size, arr) \
        (*env)->SetByteArrayRegion(env, byteArray, idx, size, arr); \
        CHECK_EXCEPTION(env)

#define SET_LONG_ARRAY_REGION(env, longArray, idx, size, arr) \
        (*env)->SetLongArrayRegion(env, longArray, idx, size, arr); \
        CHECK_EXCEPTION(env)

#define GET_ARRAY_LENGTH(env, arr) \
        (*env)->GetArrayLength(env, arr); \
        CHECK_EXCEPTION(env)

#define SET_OBJECTARRAY_ELEMENT(env, arr, idx, obj) \
        (*env)->SetObjectArrayElement(env, arr, idx, obj); \
        CHECK_EXCEPTION(env)

#define GET_OBJECTARRAY_ELEMENT(env, arr, idx) \
        (*env)->GetObjectArrayElement(env, arr, idx); \
        CHECK_EXCEPTION(env)

#define GET_FIELD(env, type, obj, fid) \
        (*env)->Get##type##Field(env, obj, GET_CACHED(fid##_fid)); \
        CHECK_EXCEPTION(env)

#define GET_STATIC_FIELD(env, type, cls, fid) \
        (*env)->GetStatic##type##Field(env, GET_CACHED(cls##_class), GET_CACHED(fid##_fid)); \
        CHECK_EXCEPTION(env)

#define SET_FIELD(env, type, obj, fid, val) \
        (*env)->Set##type##Field(env, obj, GET_CACHED(fid##_fid), val); \
        CHECK_EXCEPTION(env)

#define SET_STATIC_FIELD(env, type, cls, fid, val) \
        (*env)->SetStatic##type##Field(env, GET_CACHED(cls##_class), GET_CACHED(fid##_fid), val); \
        CHECK_EXCEPTION(env)

#define GET_OBJECT_FIELD(env, obj, fid) GET_FIELD(env, Object, obj, fid)
#define GET_BOOLEAN_FIELD(env, obj, fid) GET_FIELD(env, Boolean, obj, fid)
#define GET_INT_FIELD(env, obj, fid) GET_FIELD(env, Int, obj, fid)
#define GET_LONG_FIELD(env, obj, fid) GET_FIELD(env, Long, obj, fid)

#define GET_SECOND_FIELD(env, obj, fid) \
    GET_CACHED(time_t_sec_fid_time64) != NULL ? (*env)->GetLongField(env, obj, GET_CACHED(fid##_fid_time64)) : (int)GET_FIELD(env, Int, obj, fid)

#define SET_SECOND_FIELD(env, obj, fid, val) \
    GET_CACHED(time_t_sec_fid_time64) != NULL ? (*env)->SetLongField(env, obj, GET_CACHED(fid##_fid_time64), val) : SET_FIELD(env, Int, obj, fid, (int)val)

#define SET_OBJECT_FIELD(env, obj, fid, val) SET_FIELD(env, Object, obj, fid, val)
#define SET_BOOLEAN_FIELD(env, obj, fid, val) SET_FIELD(env, Boolean, obj, fid, val)
#define SET_INT_FIELD(env, obj, fid, val) SET_FIELD(env, Int, obj, fid, val)
#define SET_LONG_FIELD(env, obj, fid, val) SET_FIELD(env, Long, obj, fid, val)

#define SAJ_THROW(env, excep) \
        (*env)->Throw(env, excep); \
        CHECK_EXCEPTION(env)

#define EXCEPTION_CLEAR(env) \
        (*env)->ExceptionClear(env); \
        CHECK_EXCEPTION(env)

#define EXCEPTION_OCCURRED(env) \
        (*env)->ExceptionOccurred(env)
        /* Don't check exception here! */

/* See note on C99 VARARGS at definition of NEW_OBJECT(...) macro. */
#define CALL_STATIC_OBJECT_METHOD(env, cls, /* method, */ ...) \
        (*env)->CallStaticObjectMethod(env, cls, __VA_ARGS__); \
        CHECK_EXCEPTION(env)

/* See note on C99 VARARGS at definition of NEW_OBJECT(...) macro. */
#define CALL_VOID_METHOD(env, obj, /* method, */ ...) \
        (*env)->CallVoidMethod(env, obj, __VA_ARGS__); \
        CHECK_EXCEPTION(env)

/* See note on C99 VARARGS at definition of NEW_OBJECT(...) macro. */
#define CALL_INT_METHOD(env, obj, /* method, */ ...) \
        (*env)->CallIntMethod(env, obj, __VA_ARGS__); \
        CHECK_EXCEPTION(env)

/* See note on C99 VARARGS at definition of NEW_OBJECT(...) macro. */
#define CALL_BOOLEAN_METHOD(env, obj, /* method, */ ...) \
        (*env)->CallBooleanMethod(env, obj, __VA_ARGS__); \
        CHECK_EXCEPTION(env)

#define FIND_CLASS(env, cls) \
        (*env)->FindClass(env, cls); \
        CHECK_EXCEPTION(env)

#define GET_FIELD_ID(env, cls, field, sign) \
        (*env)->GetFieldID(env, cls, field, sign); \
        CHECK_EXCEPTION(env)

#define GET_STATIC_FIELD_ID(env, cls, field, sign) \
        (*env)->GetStaticFieldID(env, cls, field, sign); \
        CHECK_EXCEPTION(env)

#define GET_BYTE_ARRAY_ELEMENTS(env, byteArray, isCopyHolder) \
        (*env)->GetByteArrayElements(env, byteArray, isCopyHolder); \
        CHECK_EXCEPTION(env)

#define RELEASE_BYTE_ARRAY_ELEMENTS(env, byteArrayObject, byteArray, releaseMode) \
        (*env)->ReleaseByteArrayElements(env, byteArrayObject, byteArray, releaseMode)

#define SET_BYTE_ARRAY_ELEMENTS(env, byteArray, head, length, source) \
        (*env)->SetByteArrayElements(env, byteArray, head, length, source); \
        CHECK_EXCEPTION(env)

#define GET_LONG_ARRAY_ELEMENTS(env, longArray, isCopyHolder) \
        (*env)->GetLongArrayElements(env, longArray, isCopyHolder); \
        CHECK_EXCEPTION(env)

#define RELEASE_LONG_ARRAY_ELEMENTS(env, longArrayObject, longArray, releaseMode) \
        (*env)->ReleaseLongArrayElements(env, longArrayObject, longArray, releaseMode)

#define SET_LONG_ARRAY_ELEMENTS(env, longArray, head, length, source) \
        (*env)->SetLongArrayElements(env, longArray, head, length, source); \
        CHECK_EXCEPTION(env)

#define GET_OBJECT_CLASS(env, obj) \
        (*env)->GetObjectClass(env, obj); \
        CHECK_EXCEPTION(env)

#define GET_STATIC_OBJECT_FIELD(env, obj, fieldId) \
        (*env)->GetStaticObjectField(env, obj, fieldId); \
        CHECK_EXCEPTION(env)

#define GET_PRIMITIVE_ARRAY_CRITICAL(env, arrayObject, isCopyHolder) \
        (*env)->GetPrimitiveArrayCritical(env, arrayObject, isCopyHolder);

#define RELEASE_PRIMITIVE_ARRAY_CRITICAL(env, arrayObject, arr, mode) \
        (*env)->ReleasePrimitiveArrayCritical(env, arrayObject, arr, mode); \
        CHECK_EXCEPTION(env)

#define GET_STRING_UTFCHAR(env, strobj, str) \
        (*env)->GetStringUTFChars(env, strobj, str); \
        CHECK_EXCEPTION(env)

#define RELEASE_STRING_UTFCHAR(env, strobj, str) \
        (*env)->ReleaseStringUTFChars(env, strobj, str)

#define NEW_LOCAL_REF(env, obj) \
        (*env)->NewLocalRef(env, obj)

#define NEW_GLOBAL_REF(env, obj) \
        (*env)->NewGlobalRef(env, obj)

#define NEW_WEAK_GLOBAL_REF(env, obj) \
        (*env)->NewWeakGlobalRef(env, obj); \
        CHECK_EXCEPTION(env)

/* This macro does string-interning for the empty string.
 * @pre str != NULL */
#define NEW_STRING_UTF(env, str) \
        (assert(str), ((*((char *)(str)) == '\0') ? saj_getEmptyStringRef(env) : (*env)->NewStringUTF(env, str))); \
        CHECK_EXCEPTION(env)

#define GET_METHOD_ID(env, cls, method_name, signature) \
        (*env)->GetMethodID(env, cls, method_name, signature); \
        CHECK_EXCEPTION(env)

#define GET_METHOD_ID_NO_EX_CHECK(env, cls, method_name, signature) \
        (*env)->GetMethodID(env, cls, method_name, signature); \

#define GET_STATIC_METHOD_ID(env, cls, method_name, signature) \
        (*env)->GetStaticMethodID(env, cls, method_name, signature); \
        CHECK_EXCEPTION(env)

#define DELETE_LOCAL_REF(env, obj) \
        (*env)->DeleteLocalRef(env, obj)

#define DELETE_GLOBAL_REF(env, obj) \
        (*env)->DeleteGlobalRef(env, obj)

#define DELETE_WEAK_GLOBAL_REF(env, obj) \
        (*env)->DeleteWeakGlobalRef(env, obj)

#define IS_INSTANCE_OF(env, obj, cls) \
        (*env)->IsInstanceOf(env, obj, cls); \
        CHECK_EXCEPTION(env)


#define SAJ_JLONG(_address) ((jlong)(PA_ADDRCAST)_address)
#define SAJ_VOIDP(_long) ((void *)((size_t)(_long)))

#define SAJ_DURATION_INFINITE_SEC  (0x7FFFFFFFL)
#define SAJ_DURATION_INFINITE_NSEC (0x7FFFFFFFUL)
#define SAJ_TIME_INFINITE_SEC (0x7fffffffffffffffll)
#define SAJ_TIME_INFINITE_NSEC (0x7FFFFFFFL)

#define SAJ_DURATION_INIT(sec,nsec) \
    (sec == SAJ_DURATION_INFINITE_SEC && nsec == SAJ_DURATION_INFINITE_NSEC ? OS_DURATION_INFINITE : \
    (nsec >= 1000000000 ? OS_DURATION_INVALID : OS_DURATION_INIT(sec,nsec)))

/* Defines the package of the java implementation classes */
#define PACKAGENAME "org/opensplice/dds/dcps/"

typedef struct sajParDemContext_s * sajParDemContext;

/**
 *
 * Retcode mapping support
 *
 */
typedef jint saj_returnCode;

#define SAJ_RETCODE_OK                   (0)
#define SAJ_RETCODE_ERROR                (1)
#define SAJ_RETCODE_UNSUPPORTED          (2)
#define SAJ_RETCODE_BAD_PARAMETER        (3)
#define SAJ_RETCODE_PRECONDITION_NOT_MET (4)
#define SAJ_RETCODE_OUT_OF_RESOURCES     (5)
#define SAJ_RETCODE_NOT_ENABLED          (6)
#define SAJ_RETCODE_IMMUTABLE_POLICY     (7)
#define SAJ_RETCODE_INCONSISTENT_POLICY  (8)
#define SAJ_RETCODE_ALREADY_DELETED      (9)
#define SAJ_RETCODE_TIMEOUT              (10)
#define SAJ_RETCODE_NO_DATA              (11)
#define SAJ_RETCODE_ILLEGAL_OPERATION    (12)
#define SAJ_RETCODE_HANDLE_EXPIRED       (13)

typedef jint saj_errorCode;

#define SAJ_ERRORCODE_UNDEFINED                       (0)
#define SAJ_ERRORCODE_ERROR                           (1)
#define SAJ_ERRORCODE_OUT_OF_RESOURCES                (2)
#define SAJ_ERRORCODE_CREATION_KERNEL_ENTITY_FAILED   (3)
#define SAJ_ERRORCODE_INVALID_VALUE                   (4)
#define SAJ_ERRORCODE_INVALID_DURATION                (5)
#define SAJ_ERRORCODE_INVALID_TIME                    (6)
#define SAJ_ERRORCODE_ENTITY_INUSE                    (7)
#define SAJ_ERRORCODE_CONTAINS_ENTITIES               (8)
#define SAJ_ERRORCODE_ENTITY_UNKNOWN                  (9)
#define SAJ_ERRORCODE_HANDLE_NOT_REGISTERED           (10)
#define SAJ_ERRORCODE_HANDLE_NOT_MATCH                (11)
#define SAJ_ERRORCODE_HANDLE_INVALID                  (12)
#define SAJ_ERRORCODE_INVALID_SEQUENCE                (13)
#define SAJ_ERRORCODE_UNSUPPORTED_VALUE               (14)
#define SAJ_ERRORCODE_INCONSISTENT_VALUE              (15)
#define SAJ_ERRORCODE_IMMUTABLE_QOS_POLICY            (16)
#define SAJ_ERRORCODE_INCONSISTENT_QOS                (17)
#define SAJ_ERRORCODE_UNSUPPORTED_QOS_POLICY          (18)
#define SAJ_ERRORCODE_CONTAINS_CONDITIONS             (19)
#define SAJ_ERRORCODE_CONTAINS_LOANS                  (20)
#define SAJ_ERRORCODE_INCONSISTENT_TOPIC              (21)


typedef struct {
    os_int32 sec;
    os_uint32 nanosec;
} saj_time_t;

C_CLASS(saj_queryData);
C_STRUCT(saj_queryData) {
    jobject condition;
    u_sampleMask mask;
};

saj_returnCode saj_retcode_from_user_result(u_result result);

typedef struct jni_cache_t {
    /* cache the class reference to a GapiSuperClass object */
    jclass ObjectImpl_class;

    /* cache the common parent of all Condition objects. */
    jclass condition_class;
    jclass condition_ops_class;
    jmethodID  conditionGetTriggerValue_mid;

    /* field id of the gapiPeer field */
    jfieldID sajSuperClassGapiPeer_fid;

    /* cache the class reference to a FooDataReader object */
    jclass dataReaderImpl_class;

    /* field-/method-id's of the parallelDemarshallingContext field of the DR */
    jfieldID dataReaderImplClassParallelDemarshallingContext_fid;
    jmethodID dataReaderImplClassStartWorkers_mid;
    jmethodID dataReaderImplClassJoinWorkers_mid;
    jfieldID dataReaderImplClassCDRCopy_fid;
    jmethodID dataReaderImplClassCDRCopySetupHelper_mid;
    jmethodID dataReaderImplClassCDRDeserializeByteBuffer_mid;

    /* field-/method-id's of the dataReaderView needed for parallelDemarshallingContext */
    jfieldID dataReaderViewImplClassReader_fid;
    jclass   dataReaderViewImpl_class;

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
    jfieldID readerDataLifecycleQosPolicy_autopurge_dispose_all_fid;
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
    jfieldID returnCodeHolder_value_fid;

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

    /* caching the field id's of the Event and EventList class */
    jclass    event_class;
    jmethodID event_constructor_mid;
    jfieldID  eventList_value_fid;

    /* caching the field id's of the Time_t class */
    jclass    time_t_class;
    jmethodID time_t_constructor_mid;
    jmethodID time_t_constructor_mid_time64;
    jfieldID  time_t_sec_fid;
    jfieldID  time_t_sec_fid_time64;
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

    /*errorInfo*/
    jfieldID  errorInfoImpl_code_fid;
    jfieldID  errorInfoImpl_message_fid;
    jfieldID  errorInfoImpl_location_fid;
    jfieldID  errorInfoImpl_sourceLine_fid;
    jfieldID  errorInfoImpl_stackTrace_fid;

    /*utilities cache*/
    jclass    utilities_class;
    jmethodID utilities_throwException_mid;
    jboolean  y2038_enabled;

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

    jclass   DOMAIN_ID_INVALID_class;
    jfieldID DOMAIN_ID_INVALID_value_fid;

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
 * @brief Copies the duration defined in the java object to an object
 * os_duration struct.
 * @param env The JNI environment.
 * @param in The java Duration object.
 * @param out The os_duration struct.
 */
saj_returnCode
saj_durationCopyIn_deprecated(
    JNIEnv *env,
    jobject in,
    os_duration *out);

/**
 * @brief Copies the duration defined in the os_duration object to a java
 * Duration_t object.
 * @param env The JNI environment.
 * @param src The Duration_t object.
 * @param dst The java Duration_t object.
 * @return Return code.
 */
saj_returnCode
saj_durationCopyOut_deprecated(
    JNIEnv *env,
    os_duration *src,
    jobject *dst
);

/**
 * @brief Copies the duration defined in the java object to an object
 * os_duration struct.
 * @param env The JNI environment.
 * @param in The java Duration object.
 * @param out The os_duration struct.
 */
saj_returnCode
saj_durationCopyIn(
    JNIEnv *env,
    jobject in,
    os_duration *out);

/**
 * @brief Copies the duration defined in the java object to an object
 * os_duration struct.
 * @param env The JNI environment.
 * @param in The java Duration object.
 * @param out The os_duration struct.
 */
saj_returnCode
saj_durationCopyOut(
    JNIEnv *env,
    os_duration src,
    jobject *dst);

/**
 * @brief Copies the time defined in the java object to a
 * os_timeW struct.
 * @param env The JNI environment.
 * @param in The java Time object.
 * @param out The os_timeW struct.
 */
saj_returnCode
saj_timeCopyIn(
    JNIEnv *env,
    jobject src,
    os_timeW *dst);

/**
 * @brief Copies the time defined in the java object to a
 * os_timeW struct.
 * @param env The JNI environment.
 * @param in The java Time object.
 * @param out The os_timeW struct.
 */
saj_returnCode
saj_timeCopyInAcceptInvalidTime(
    JNIEnv *env,
    jobject src,
    os_timeW *dst);

/**
 * @brief Copies the time defined in the os_timeW object to a java
 * Time_t object.
 * @param env The JNI environment.
 * @param src The os_timeW object.
 * @param dst The java Time_t object.
 * @return Return code.
 */
saj_returnCode
saj_timeCopyOut(
    JNIEnv *env,
    os_timeW src,
    jobject *dst);

/**
 * @brief Copies the time defined in the v_duration object to a java
 * Time_t object.
 * @param env The JNI environment.
 * @param src The os_timeW object.
 * @param dst The java Time_t object.
 * @return Return code.
 */
saj_returnCode
saj_vDurationCopyOut(
    JNIEnv *env,
    v_duration *src,
    jobject *dst);

/**
 * @brief Copies the time defined in the java object to a
 * v_duration struct.
 * @param env The JNI environment.
 * @param in The java Time object.
 * @param out The os_timeW struct.
 */
saj_returnCode
saj_vDurationCopyIn(
    JNIEnv *env,
    jobject src,
    v_duration *dst);

saj_returnCode
saj_instanceHandleSequenceCopyOut(
    JNIEnv* env,
    c_iter src,
    jintArray *dst);

v_result
saj_subscriptionBuiltinTopicDataCopyOut(
    JNIEnv* env,
    u_subscriptionInfo *src,
    jobject *dst);

saj_returnCode
saj_publicationBuiltinTopicDataCopyOut(
    JNIEnv* env,
    u_publicationInfo *src,
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
    os_uint32 *dst);

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
    os_uint32  src,
    jobject             *dst);

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
    cmn_sampleInfo src,
    jobject *dst);

saj_returnCode
saj_LookupTypeSupportDataReader(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result);

saj_returnCode
saj_LookupTypeSupportDataReaderView(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result);

saj_returnCode
saj_LookupTypeSupportDataWriter(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result);

saj_returnCode
saj_LookupTypeSupportConstructorSignature(
    JNIEnv* env,
    jobject jtypeSupport,
    os_char** result);

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

saj_returnCode
saj_conditionSeqCopy(
    JNIEnv *env,
    c_iter *src,
    jobjectArray *dst);

c_iter
saj_eventListNew(
    JNIEnv  *env,
    v_listenerEvent event);

u_eventMask
saj_statusMask_to_eventMask(
    jint mask);

void
saj_eventMask_to_statusMask(
    v_public p,
    void *arg);

jobject saj_inconsistentTopicStatus_new(JNIEnv *env, struct v_inconsistentTopicInfo *info);
jobject saj_livelinessLostStatus_new(JNIEnv *env, struct v_livelinessLostInfo *info);
jobject saj_sampleLostStatus_new(JNIEnv *env, struct v_sampleLostInfo *info);
jobject saj_offeredDeadlineMissedStatus_new(JNIEnv *env, struct v_deadlineMissedInfo *info);
jobject saj_requestedDeadlineMissedStatus_new(JNIEnv *env, struct v_deadlineMissedInfo *info);
jobject saj_sampleRejectedStatus_new(JNIEnv *env, struct v_sampleRejectedInfo *info);
jobject saj_offeredIncompatibleQosStatus_new(JNIEnv *env, struct v_incompatibleQosInfo *info);
jobject saj_requestedIncompatibleQosStatus_new(JNIEnv *env, struct v_incompatibleQosInfo *info);
jobject saj_livelinessChangedStatus_new(JNIEnv *env, struct v_livelinessChangedInfo *info);
jobject saj_publicationMatchStatus_new(JNIEnv *env, struct v_topicMatchInfo *info);
jobject saj_subscriptionMatchStatus_new(JNIEnv *env, struct v_topicMatchInfo *info);

typedef struct {
    JNIEnv *Env;
    c_metaObject typeMeta;
    os_char *redirects;
} saj_createCopyCacheArg;

void*
saj_createCopyCache(
        void* arg);

#undef OS_API

#endif /* SAJ_UTILITIES_H */
