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
  
#include "saj_status.h" 
#include <jni.h>
#include "saj_utilities.h"

saj_returnCode
saj_statusCopyOutSampleRejectedStatusKind(
    JNIEnv *env,
    gapi_sampleRejectedStatusKind kind,
    jobject *jkind)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(sampleRejectedStatusKind_class);
    mid = GET_CACHED(sampleRejectedStatusKind_fromInt_mid);
    
    *jkind = (*env)->CallStaticObjectMethod(env, cls, mid, (jint)kind);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutQosPolicyCount(
    JNIEnv *env,
    gapi_qosPolicyCount* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(qosPolicyCount_class);
    mid = GET_CACHED(qosPolicyCount_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->policy_id, 
                                            (jint)status->count);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutQosPolicyCountSeq(
    JNIEnv *env,
    gapi_qosPolicyCountSeq *src,
    jobjectArray* dst)
{
    saj_returnCode rc; 
    jclass cls;
    unsigned int i;
    jobject jcount;
    
    rc = SAJ_RETCODE_OK;
    assert(dst != NULL);

    cls = GET_CACHED(qosPolicyCount_class);
    *dst = (*env)->NewObjectArray(env, src->_length, cls, NULL);
    saj_exceptionCheck(env);
    

    for(i=0; (i<src->_length) && (rc == SAJ_RETCODE_OK); i++){
        rc = saj_statusCopyOutQosPolicyCount(env, 
                                    &src->_buffer[i], 
                                    &jcount);
        (*env)->SetObjectArrayElement(env, *dst, (jsize)i, jcount);
        saj_exceptionCheck(env);
        (*env)->DeleteLocalRef(env, jcount);
        saj_exceptionCheck(env);
    }    
    return rc;
}

saj_returnCode
saj_statusCopyOutInconsistentTopicStatus(
    JNIEnv *env,
    gapi_inconsistentTopicStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(inconsistentTopicStatus_class);
    mid = GET_CACHED(inconsistentTopicStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutAllDataDisposedTopicStatus(
    JNIEnv *env,
    gapi_allDataDisposedTopicStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(allDataDisposedTopicStatus_class);
    mid = GET_CACHED(allDataDisposedTopicStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutLivelinessLostStatus(
    JNIEnv *env,
    gapi_livelinessLostStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(livelinessLostStatus_class);
    mid = GET_CACHED(livelinessLostStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutOfferedDeadlineMissedStatus(
    JNIEnv *env,
    gapi_offeredDeadlineMissedStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(offeredDeadlineMissedStatus_class);
    mid = GET_CACHED(offeredDeadlineMissedStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change,
                                            (jlong)status->last_instance_handle);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutOfferedIncompatibleQosStatus(
    JNIEnv *env,
    gapi_offeredIncompatibleQosStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    jobjectArray jqosCount;
    saj_returnCode rc;
    
    cls = GET_CACHED(offeredIncompatibleQosStatus_class);
    mid = GET_CACHED(offeredIncompatibleQosStatus_constructor_mid);
    rc = saj_statusCopyOutQosPolicyCountSeq(env, &status->policies, &jqosCount);
    
    if(rc == SAJ_RETCODE_OK){
        *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change,
                                            (jint)status->last_policy_id,
                                            jqosCount);
        saj_exceptionCheck(env);
    }
    return rc;
}
    
saj_returnCode
saj_statusCopyOutPublicationMatchStatus(
    JNIEnv *env,
    gapi_publicationMatchedStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(publicationMatchStatus_class);
    mid = GET_CACHED(publicationMatchStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change,
                                            (jint)status->current_count, 
                                            (jint)status->current_count_change,
                                            (jlong)status->last_subscription_handle);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutSampleRejectedStatus(
    JNIEnv *env,
    gapi_sampleRejectedStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    saj_returnCode rc;
    jobject jkind;
    
    cls = GET_CACHED(sampleRejectedStatus_class);
    mid = GET_CACHED(sampleRejectedStatus_constructor_mid);
    
    rc = saj_statusCopyOutSampleRejectedStatusKind(env, status->last_reason, 
                                                                    &jkind);
    if(rc == SAJ_RETCODE_OK){
        *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change,
                                            jkind,
                                            (jlong)status->last_instance_handle);
        saj_exceptionCheck(env);
    }
    return rc;
}

saj_returnCode
saj_statusCopyOutLivelinessChangedStatus(
    JNIEnv *env,
    gapi_livelinessChangedStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(livelinessChangedStatus_class);
    mid = GET_CACHED(livelinessChangedStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                        (jint)status->alive_count, 
                                        (jint)status->not_alive_count,
                                        (jint)status->alive_count_change,
                                        (jint)status->not_alive_count_change,
                                        (jlong)status->last_publication_handle);
    
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutRequestedDeadlineMissedStatus(
    JNIEnv *env,
    gapi_requestedDeadlineMissedStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(requestedDeadlineMissedStatus_class);
    mid = GET_CACHED(requestedDeadlineMissedStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                        (jint)status->total_count, 
                                        (jint)status->total_count_change,
                                        (jlong)status->last_instance_handle);
    
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutRequestedIncompatibleQosStatus(
    JNIEnv *env,
    gapi_requestedIncompatibleQosStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    jobjectArray jqosCount;
    saj_returnCode rc;
    
    cls = GET_CACHED(requestedIncompatibleQosStatus_class);
    mid = GET_CACHED(requestedIncompatibleQosStatus_constructor_mid);
    rc = saj_statusCopyOutQosPolicyCountSeq(env, &status->policies, &jqosCount);
    
    if(rc == SAJ_RETCODE_OK){
        *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change,
                                            (jint)status->last_policy_id,
                                            jqosCount);
        saj_exceptionCheck(env);
    }
    return rc;
}

saj_returnCode
saj_statusCopyOutSubscriptionMatchStatus(
    JNIEnv *env,
    gapi_subscriptionMatchedStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(subscriptionMatchStatus_class);
    mid = GET_CACHED(subscriptionMatchStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change,
                                            (jint)status->current_count, 
                                            (jint)status->current_count_change,
                                            (jlong)status->last_publication_handle);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_statusCopyOutSampleLostStatus(
    JNIEnv *env,
    gapi_sampleLostStatus* status,
    jobject* jstatus)
{
    jclass cls;
    jmethodID mid;
    
    cls = GET_CACHED(sampleLostStatus_class);
    mid = GET_CACHED(sampleLostStatus_constructor_mid);
    
    *jstatus = (*env)->NewObject(env, cls, mid, 
                                            (jint)status->total_count, 
                                            (jint)status->total_count_change);
    saj_exceptionCheck(env);
    
    return SAJ_RETCODE_OK;
}
