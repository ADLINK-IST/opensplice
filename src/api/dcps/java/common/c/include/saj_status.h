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
#ifndef SAJ_STATUS_H
#define SAJ_STATUS_H
 
#include "gapi.h" 
#include <jni.h>
#include "saj_utilities.h"

saj_returnCode
saj_statusCopyOutSampleRejectedStatusKind(
    JNIEnv *env,
    gapi_sampleRejectedStatusKind kind,
    jobject *jkind);

saj_returnCode
saj_statusCopyOutQosPolicyCount(
    JNIEnv *env,
    gapi_qosPolicyCount* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutQosPolicyCountSeq(
    JNIEnv *env,
    gapi_qosPolicyCountSeq *src,
    jobjectArray* dst);
    
saj_returnCode
saj_statusCopyOutInconsistentTopicStatus(
    JNIEnv *env,
    gapi_inconsistentTopicStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutAllDataDisposedTopicStatus(
    JNIEnv *env,
    gapi_allDataDisposedTopicStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutLivelinessLostStatus(
    JNIEnv *env,
    gapi_livelinessLostStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutOfferedDeadlineMissedStatus(
    JNIEnv *env,
    gapi_offeredDeadlineMissedStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutOfferedIncompatibleQosStatus(
    JNIEnv *env,
    gapi_offeredIncompatibleQosStatus* status,
    jobject* jstatus);
    
saj_returnCode
saj_statusCopyOutPublicationMatchStatus(
    JNIEnv *env,
    gapi_publicationMatchedStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutSampleRejectedStatus(
    JNIEnv *env,
    gapi_sampleRejectedStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutLivelinessChangedStatus(
    JNIEnv *env,
    gapi_livelinessChangedStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutRequestedDeadlineMissedStatus(
    JNIEnv *env,
    gapi_requestedDeadlineMissedStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutRequestedIncompatibleQosStatus(
    JNIEnv *env,
    gapi_requestedIncompatibleQosStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutSubscriptionMatchStatus(
    JNIEnv *env,
    gapi_subscriptionMatchedStatus* status,
    jobject* jstatus);

saj_returnCode
saj_statusCopyOutSampleLostStatus(
    JNIEnv *env,
    gapi_sampleLostStatus* status,
    jobject* jstatus);
    
#endif /* SAJ_STATUS_H */
