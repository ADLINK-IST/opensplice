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
#include "saj_status.h"
#include "saj_listener.h"
#include "saj_dataReaderListener.h"
#include "gapi.h"

struct gapi_dataReaderListener *
saj_dataReaderListenerNew(
    JNIEnv *env,
    jobject jlistener)
{
    struct gapi_dataReaderListener* listener;
    saj_listenerData ld;
    
    listener = NULL;
    
    ld = saj_listenerDataNew(env, jlistener);
    
    if(ld != NULL){
        listener = gapi_dataReaderListener__alloc();
        saj_listenerInit((struct gapi_listener*)listener);
        listener->listener_data = ld;
        
        listener->on_requested_deadline_missed  = saj_dataReaderListenerOnRequestedDeadlineMissed;
        listener->on_requested_incompatible_qos = saj_dataReaderListenerOnRequestedIncompatibleQos;
        listener->on_sample_rejected            = saj_dataReaderListenerOnSampleRejected;
        listener->on_liveliness_changed         = saj_dataReaderListenerOnLivelinessChanged;
        listener->on_data_available             = saj_dataReaderListenerOnDataAvailable;
        listener->on_subscription_match         = saj_dataReaderListenerOnSubscriptionMatch;
        listener->on_sample_lost                = saj_dataReaderListenerOnSampleLost;
        
    }
    return listener;
}

void
saj_dataReaderListenerOnRequestedDeadlineMissed(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_requestedDeadlineMissedStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataReader;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);
    
    rc = saj_statusCopyOutRequestedDeadlineMissedStatus(env, 
                        (gapi_requestedDeadlineMissedStatus *)status, 
                        &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jdataReader = saj_read_java_address(dataReader);
        (*env)->CallVoidMethod(env, ld->jlistener, 
                        GET_CACHED(listener_onRequestedDeadlineMissed_mid), 
                        jdataReader, jstatus);
    }
}

void
saj_dataReaderListenerOnRequestedIncompatibleQos(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_requestedIncompatibleQosStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataReader;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutRequestedIncompatibleQosStatus(env, 
                        (gapi_requestedIncompatibleQosStatus *)status, 
                        &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jdataReader = saj_read_java_address(dataReader);
        (*env)->CallVoidMethod(env, ld->jlistener, 
                        GET_CACHED(listener_onRequestedIncompatibleQos_mid), 
                        jdataReader, jstatus);
    }
}

void
saj_dataReaderListenerOnSampleRejected(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_sampleRejectedStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataReader;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutSampleRejectedStatus(env, 
                        (gapi_sampleRejectedStatus *)status, 
                        &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jdataReader = saj_read_java_address(dataReader);
        (*env)->CallVoidMethod(env, ld->jlistener, 
                        GET_CACHED(listener_onSampleRejected_mid), 
                        jdataReader, jstatus);
    }
}

void
saj_dataReaderListenerOnLivelinessChanged(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_livelinessChangedStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataReader;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutLivelinessChangedStatus(env, 
                        (gapi_livelinessChangedStatus *)status, 
                        &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jdataReader = saj_read_java_address(dataReader);
        (*env)->CallVoidMethod(env, ld->jlistener, 
                        GET_CACHED(listener_onLivelinessChanged_mid), 
                        jdataReader, jstatus);
    }
}

void
saj_dataReaderListenerOnDataAvailable(
    void* listenerData, 
    gapi_dataReader dataReader)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jdataReader;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);
    
    jdataReader = saj_read_java_address(dataReader);
    (*env)->CallVoidMethod(env, ld->jlistener, 
                                GET_CACHED(listener_onDataAvailable_mid), 
                                jdataReader);
}

void
saj_dataReaderListenerOnSubscriptionMatch(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_subscriptionMatchedStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataReader;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutSubscriptionMatchStatus(env, 
                        (gapi_subscriptionMatchedStatus *)status, 
                        &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jdataReader = saj_read_java_address(dataReader);
        (*env)->CallVoidMethod(env, ld->jlistener, 
                        GET_CACHED(listener_onSubscriptionMatch_mid), 
                        jdataReader, jstatus);
    }
}

void
saj_dataReaderListenerOnSampleLost(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_sampleLostStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataReader;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutSampleLostStatus(env, 
                        (gapi_sampleLostStatus *)status, 
                        &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jdataReader = saj_read_java_address(dataReader);
        (*env)->CallVoidMethod(env, ld->jlistener, 
                        GET_CACHED(listener_onSampleLost_mid), 
                        jdataReader, jstatus);
    }
}
