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
#include "saj_Topic.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_status.h"
#include "saj_topicListener.h"
#include "saj_extTopicListener.h"

#include "gapi.h"

#include "os_report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_TopicImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetInconsistentTopicStatus
 * Signature: ()LDDS/InconsistentTopicStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetInconsistentTopicStatus)(
    JNIEnv *env, 
    jobject jtopic,
    jobject jstatusHolder)
{
    gapi_topic topic;
    jobject jstatus;
    gapi_inconsistentTopicStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jstatusHolder){
        topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
        result = gapi_topic_get_inconsistent_topic_status(topic, &status);
        
        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutInconsistentTopicStatus(env, &status, &jstatus);
            
            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder, 
                            GET_CACHED(inconsistentTopicStatusHolder_value_fid), jstatus);
                (*env)->DeleteLocalRef(env, jstatus);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetAllDataDisposedTopicStatus
 * Signature: ()LDDS/AllDataDisposedTopicStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetAllDataDisposedTopicStatus)(
    JNIEnv *env, 
    jobject jtopic,
    jobject jstatusHolder)
{
    gapi_topic topic;
    jobject jstatus;
    gapi_allDataDisposedTopicStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    if(jstatusHolder){
        topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
        result = gapi_topic_get_all_data_disposed_topic_status(topic, &status);
        
        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutAllDataDisposedTopicStatus(env, &status, &jstatus);
            
            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder, 
                            GET_CACHED(allDataDisposedTopicStatusHolder_value_fid), jstatus);
                (*env)->DeleteLocalRef(env, jstatus);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetQos
 * Signature: (LDDS/TopicQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jtopic,
    jobject jqosHolder)
{
    gapi_topicQos* qos;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jqos;
    gapi_topic topic;
    
    if(jqosHolder != NULL){
        topic = (gapi_topic)saj_read_gapi_address(env, jtopic);
        jqos = NULL;
    
        qos = gapi_topicQos__alloc();
        result = gapi_topic_get_qos(topic, qos);
        
        if(result == GAPI_RETCODE_OK){
            rc = saj_TopicQosCopyOut(env, qos, &jqos);
            gapi_free(qos);
            
            if(rc == SAJ_RETCODE_OK){        
                (*env)->SetObjectField(env, jqosHolder, 
                        GET_CACHED(topicQosHolder_value_fid), jqos);
                (*env)->DeleteLocalRef(env, jqos);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniSetQos
 * Signature: (LDDS/TopicQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jtopic,
    jobject jqos)
{
    gapi_topicQos* qos;
    gapi_topic topic;
    saj_returnCode rc;
    jint result;
    
    qos = gapi_topicQos__alloc();
    topic = (gapi_topic)saj_read_gapi_address(env, jtopic);
    rc = saj_TopicQosCopyIn(env, jqos, qos);
    result = (jint)GAPI_RETCODE_ERROR;
    
    if(rc == SAJ_RETCODE_OK){
        result = (jint)gapi_topic_set_qos(topic, qos); 
    }
    gapi_free(qos);
    
    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetListener
 * Signature: ()LDDS/TopicListener;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetListener)(
    JNIEnv *env,
    jobject jtopic)
{
    jobject jlistener;
    struct gapi_topicListener listener;
    gapi_topic topic;
    
    jlistener = NULL;
    topic = (gapi_topic)saj_read_gapi_address(env, jtopic);
    listener = gapi_topic_get_listener(topic);
    
    jlistener = saj_read_java_listener_address(listener.listener_data);

    return jlistener;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniSetListener
 * Signature: (LDDS/TopicListener;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener)(
    JNIEnv *env,
    jobject jtopic,
    jobject jlistener,
    jint jmask)
{
    struct gapi_topicListener* listener;
    gapi_topic topic;
    gapi_returnCode_t grc = GAPI_RETCODE_OK;

    jclass tempClass;
    jboolean result;

    /* We can check Java instances in the jni layer, so here we check
     * that if the mask is set to GAPI_ALL_DATA_DISPOSED_STATUS we have
     * been given an ExtDomainParticipantListener to call. If not then
     * an error is reported and a GAPI_RETCODE_BAD_PARAMETER status is
     * returned. */
    if(jmask & GAPI_ALL_DATA_DISPOSED_STATUS) {
        tempClass = (*env)->FindClass(env, "DDS/ExtTopicListener");
        result = (*env)->IsInstanceOf(env, jlistener, tempClass);
        if(result == JNI_FALSE) {
            OS_REPORT(OS_ERROR, "dcpssaj", 0, "ExtTopicListener must be used when the ALL_DATA_DISPOSED_STATUS bit is set.");
            grc = GAPI_RETCODE_BAD_PARAMETER;
        }  
    }

    if(grc == GAPI_RETCODE_OK){
        topic = (gapi_topic)saj_read_gapi_address(env, jtopic);

        if(jmask & GAPI_ALL_DATA_DISPOSED_STATUS) {
            listener = saj_extTopicListenerNew(env, jlistener);
        } else {
            listener = saj_topicListenerNew(env, jlistener);
        }

        grc = gapi_topic_set_listener(topic, listener, (unsigned long int)jmask);
    
        if(grc == GAPI_RETCODE_OK){
            if(listener != NULL){
                saj_write_java_listener_address(env, topic, listener->listener_data);
            }
        }
    } 
    return (jint)grc; 
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetTypeName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetTypeName)(
    JNIEnv *env,
    jobject jtopic)
{
    gapi_topic topic;
    jstring jtypeName = NULL;
    gapi_string typeName;
    
    topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
    typeName = gapi_topic_get_type_name(topic);
    
    if(typeName != NULL){
        jtypeName = (*env)->NewStringUTF(env, typeName);
        gapi_free(typeName);
    }
    return jtypeName;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetName)(
    JNIEnv *env,
    jobject jtopic)
{
    gapi_topic topic;
    jstring jname = NULL;
    gapi_string name;
    
    topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
    name = gapi_topic_get_name(topic);
    
    if(name != NULL){
        jname = (*env)->NewStringUTF(env, name);
        gapi_free(name);
    }
    return jname;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetParticipant
 * Signature: ()LDDS/DomainParticipant;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetParticipant)(
    JNIEnv *env,
    jobject jtopic)
{
    gapi_topic topic;
    gapi_domainParticipant participant;
    jobject jparticipant;
    
    jparticipant = NULL;
    topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
    participant = gapi_topic_get_participant(topic);
    
    if(participant != NULL){
        jparticipant = saj_read_java_address(participant);
    }
    return jparticipant;
}  

/*
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniDisposeAllData
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDisposeAllData)(
    JNIEnv *env,
    jobject jtopic)
{
    gapi_topic topic;
    gapi_returnCode_t result;

    topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
    result = gapi_topic_dispose_all_data(topic);

    return (jint)result;
}
  
#undef SAJ_FUNCTION
