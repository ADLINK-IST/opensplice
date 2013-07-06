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

#include "saj_Publisher.h"
#include "saj_publisherListener.h"
#include "saj_dataWriterListener.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_PublisherImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniCreateDatawriter
 * Signature: (LDDS/Topic;LDDS/DataWriterQos;LDDS/DataWriterListener;)LDDS/DataWriter;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateDatawriter)(
    JNIEnv *env, 
    jobject jpublisher,
    jobject jtopic,
    jobject jqos,
    jobject jlistener,
    jint jmask)
{
    jobject jwriter;
    gapi_publisher publisher;
    gapi_dataWriter writer;
    gapi_dataWriterQos* writerQos;
    gapi_topic topic;
    gapi_domainParticipant participant;
    gapi_typeSupport typeSupport;
    const struct gapi_dataWriterListener *listener;
    gapi_char* dataWriterClassName;
    saj_returnCode rc;
    gapi_string typeName;
    gapi_char* signature;
    jobject jtypeSupport;
    
    jwriter = NULL;
    listener = NULL;
    writer = GAPI_OBJECT_NIL;
    
    publisher = (gapi_publisher) saj_read_gapi_address(env, jpublisher);
    participant = gapi_publisher_get_participant(publisher);
    topic = (gapi_topic) saj_read_gapi_address(env, jtopic);
    
    typeName = gapi_topicDescription_get_type_name((gapi_topicDescription)topic);
    typeSupport = gapi_domainParticipant_get_typesupport(participant, (const gapi_char*) typeName);
    gapi_free(typeName);
                                                
    jtypeSupport = saj_read_java_address((gapi_object)typeSupport);
    rc = saj_LookupTypeSupportDataWriter(env, jtypeSupport, &dataWriterClassName);
    
    if(rc == SAJ_RETCODE_OK){
        if ((*env)->IsSameObject (env, jqos, GET_CACHED(DATAWRITER_QOS_DEFAULT)) == JNI_TRUE) {
            writerQos = (gapi_dataWriterQos *)GAPI_DATAWRITER_QOS_DEFAULT;
            rc = SAJ_RETCODE_OK;
        } else if ((*env)->IsSameObject (env, jqos, GET_CACHED(DATAWRITER_QOS_USE_TOPIC_QOS)) == JNI_TRUE) {
            writerQos = (gapi_dataWriterQos *)GAPI_DATAWRITER_QOS_USE_TOPIC_QOS;
            rc = SAJ_RETCODE_OK;
        } else {
            writerQos = gapi_dataWriterQos__alloc();
            rc = saj_DataWriterQosCopyIn(env, jqos, writerQos);
	}
        
        if(rc == SAJ_RETCODE_OK){
            listener = saj_dataWriterListenerNew(env, jlistener);
            writer = gapi_publisher_create_datawriter(publisher, topic, writerQos, 
                                                                        listener, (gapi_statusMask)jmask);
                    
            if (writer != GAPI_OBJECT_NIL){
                rc = saj_LookupTypeSupportConstructorSignature(env, jtypeSupport, &signature);
                
                if(rc == SAJ_RETCODE_OK){
                    gapi_publisherQos *pqos = gapi_publisherQos__alloc();
                    rc = saj_construct_typed_java_object(env, 
                                                        dataWriterClassName, 
                                                        (PA_ADDRCAST)writer, 
                                                        &jwriter, signature,
                                                        jtypeSupport);

                    gapi_free(signature);
                    
                    if(listener != NULL){
                        saj_write_java_listener_address(env, writer, listener->listener_data);
                    }

                    if(pqos){
                        if(gapi_publisher_get_qos(publisher, pqos) == GAPI_RETCODE_OK){
                            if(pqos->entity_factory.autoenable_created_entities) {
                                gapi_entity_enable(writer);
                            }
                        }
                        gapi_free(pqos);
                    }

                }
            } else if(listener != NULL){
                saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
            }
        }
        if ((writerQos != (gapi_dataWriterQos *)GAPI_DATAWRITER_QOS_DEFAULT) &&
	    (writerQos != (gapi_dataWriterQos *)GAPI_DATAWRITER_QOS_USE_TOPIC_QOS)) {
	    gapi_free(writerQos);
	}
        gapi_free(dataWriterClassName);
    } 
    return jwriter;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniDeleteDatawriter
 * Signature: (LDDS/DataWriter;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteDatawriter)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jwriter)
{
    gapi_publisher publisher;
    gapi_dataWriter writer;
    gapi_returnCode_t grc;
    c_bool must_free;

    publisher = (gapi_publisher) saj_read_gapi_address(env, jpublisher);
    writer = (gapi_dataWriter) saj_read_gapi_address(env, jwriter);

    must_free = saj_setThreadEnv(env);
    grc = gapi_publisher_delete_datawriter(publisher, writer);
    saj_delThreadEnv(must_free);
    
    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniLookupDatawriter
 * Signature: (Ljava/lang/String;)LDDS/DataWriter;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniLookupDatawriter)(
    JNIEnv *env,
    jobject jpublisher,
    jstring jtopicName)
{
    jobject jwriter;
    gapi_publisher publisher;
    gapi_dataWriter writer;
    const gapi_char* topicName;
    
    jwriter = NULL;
    writer = GAPI_OBJECT_NIL;
    topicName = NULL;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    
    if(jtopicName != NULL){
        topicName = (*env)->GetStringUTFChars(env, jtopicName, 0);
    }
    writer = gapi_publisher_lookup_datawriter(publisher, topicName);
    
    if (writer != GAPI_OBJECT_NIL){
        jwriter = saj_read_java_address(writer);
    }
    if(jtopicName != NULL){
        (*env)->ReleaseStringUTFChars(env, jtopicName, topicName);
    }
    return jwriter;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniDeleteContainedEntities
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContainedEntities)(
    JNIEnv *env,
    jobject jpublisher)
{
    gapi_publisher publisher;
    jint result;
    c_bool must_free;

    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
   
    must_free = saj_setThreadEnv(env);
    result = (jint)gapi_publisher_delete_contained_entities(publisher);
    saj_delThreadEnv(must_free);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniSetQos
 * Signature: (LDDS/PublisherQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jqos)
{
    gapi_publisherQos* qos;
    gapi_publisher publisher;
    saj_returnCode rc;
    jint result;
    
    qos = gapi_publisherQos__alloc();
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    rc = saj_PublisherQosCopyIn(env, jqos, qos);
    result = (jint)GAPI_RETCODE_ERROR;
    
    if(rc == SAJ_RETCODE_OK){
        result = (jint)gapi_publisher_set_qos(publisher, qos); 
    }
    gapi_free(qos);
    
    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniGetQos
 * Signature: (LDDS/PublisherQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jqosHolder)
{
    gapi_publisherQos* qos;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jqos;
    gapi_publisher publisher;
    
    if(jqosHolder != NULL){
        publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
        jqos = NULL;
    
        qos = gapi_publisherQos__alloc();
        result = gapi_publisher_get_qos(publisher, qos);
        
        if(result == GAPI_RETCODE_OK){
            rc = saj_PublisherQosCopyOut(env, qos, &jqos);
            gapi_free(qos);
            
            if(rc == SAJ_RETCODE_OK){        
                (*env)->SetObjectField(env, jqosHolder, 
                        GET_CACHED(publisherQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniSetListener
 * Signature: (LDDS/PublisherListener;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jlistener,
    jint jmask)
{
    struct gapi_publisherListener *listener;
    gapi_publisher publisher;
    gapi_returnCode_t grc;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    listener = saj_publisherListenerNew(env, jlistener);
    grc = gapi_publisher_set_listener(publisher, listener, 
                                                    (unsigned long int)jmask);
    
    if(grc == GAPI_RETCODE_OK){
        if(listener != NULL){
            saj_write_java_listener_address(env, publisher, listener->listener_data);
        }
    } else if(listener != NULL){
        saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
    } 
    return (jint)grc; 
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniGetListener
 * Signature: ()LDDS/PublisherListener;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetListener)(
    JNIEnv *env,
    jobject jpublisher)
{
    jobject jlistener;
    struct gapi_publisherListener listener;
    gapi_publisher publisher;
    
    jlistener = NULL;
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    listener = gapi_publisher_get_listener(publisher);
    
    jlistener = saj_read_java_listener_address(listener.listener_data);

    return jlistener;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniSuspendPublications
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSuspendPublications)(
    JNIEnv *env,
    jobject jpublisher)
{
    gapi_publisher publisher;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    
    return (jint)gapi_publisher_suspend_publications(publisher);
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniResumePublications
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniResumePublications)(
    JNIEnv *env,
    jobject jpublisher)
{
    gapi_publisher publisher;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    
    return (jint)gapi_publisher_resume_publications(publisher);
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniBeginCoherentChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniBeginCoherentChanges)(
    JNIEnv *env,
    jobject jpublisher)
{
    gapi_publisher publisher;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    
    return (jint)gapi_publisher_begin_coherent_changes(publisher);
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniEndCoherentChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniEndCoherentChanges)(
    JNIEnv *env,
    jobject jpublisher)
{
    gapi_publisher publisher;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    
    return (jint)gapi_publisher_end_coherent_changes(publisher);
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniGetParticipant
 * Signature: ()LDDS/DomainParticipant;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetParticipant)(
    JNIEnv *env,
    jobject jpublisher)
{
    gapi_publisher publisher;
    gapi_domainParticipant participant;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
    participant = gapi_publisher_get_participant(publisher);
    
    return saj_read_java_address(participant);
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniSetDefaultDatawriterQos
 * Signature: (LDDS/DataWriterQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultDatawriterQos)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jqos)
{
    gapi_dataWriterQos* qos;
    gapi_publisher publisher;
    saj_returnCode rc;
    jint result;
    
    result = (jint)GAPI_RETCODE_ERROR;
    qos = gapi_dataWriterQos__alloc();
    rc = saj_DataWriterQosCopyIn(env, jqos, qos);
    
    if (rc == SAJ_RETCODE_OK){
        publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
        result = (jint)gapi_publisher_set_default_datawriter_qos(publisher, qos);
    }
    gapi_free(qos);
    
    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniGetDefaultDatawriterQos
 * Signature: (LDDS/DataWriterQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDefaultDatawriterQos)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jqosHolder)
{
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jqos;
    gapi_publisher publisher;
    gapi_dataWriterQos *qos;
    
    jqos = NULL;
    
    if(jqosHolder != NULL){
        qos = gapi_dataWriterQos__alloc();
        publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
        result = gapi_publisher_get_default_datawriter_qos(publisher, qos); 
        
        if(result == GAPI_RETCODE_OK){
            rc = saj_DataWriterQosCopyOut(env, qos, &jqos);
            gapi_free(qos);
        
            if (rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder, 
                                       GET_CACHED(dataWriterQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniCopyFromTopicQos
 * Signature: (LDDS/DataWriterQosHolder;LDDS/TopicQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniCopyFromTopicQos)(
    JNIEnv *env,
    jobject jpublisher,
    jobject jqosHolder,
    jobject jtopicQos)
{
    saj_returnCode rc;
    gapi_returnCode_t grc;
    jobject jqos, oldQos;
    gapi_publisher publisher;
    gapi_dataWriterQos *qos;
    gapi_topicQos *topicQos;
    jint result;

    oldQos = NULL;
    
    if(jqosHolder != NULL){    
        result = (jint)GAPI_RETCODE_ERROR;
        rc = SAJ_RETCODE_ERROR;
    
        if ((*env)->IsSameObject (env, jtopicQos, GET_CACHED(TOPIC_QOS_DEFAULT)) == JNI_TRUE) {
            topicQos = (gapi_topicQos *)GAPI_TOPIC_QOS_DEFAULT;
            rc = SAJ_RETCODE_OK;
        } else {
            topicQos = gapi_topicQos__alloc();
            rc = saj_TopicQosCopyIn(env, jtopicQos, topicQos);
        }
        
        if(rc == SAJ_RETCODE_OK){
            oldQos = (*env)->GetObjectField(env, jqosHolder, 
                                  GET_CACHED(dataWriterQosHolder_value_fid));
            qos = gapi_dataWriterQos__alloc();
            
            if(oldQos){
                rc = saj_DataWriterQosCopyIn(env, oldQos, qos);
                
                if(rc != SAJ_RETCODE_OK) {
                    gapi_free(qos);
                    qos = NULL;
                }
            }
        } else {
            qos = NULL;
        }
        publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);
        grc = gapi_publisher_copy_from_topic_qos(publisher, qos, topicQos);
        result = (jint)grc;
        
        if(grc == GAPI_RETCODE_OK){
            if(oldQos){
                jqos = oldQos;
            } else {
                jqos = NULL;
            }
            rc = saj_DataWriterQosCopyOut(env, qos, &jqos);
            
            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder, 
                                  GET_CACHED(dataWriterQosHolder_value_fid), jqos);
            }
        }
        if(qos) {
            gapi_free(qos);
        }
        gapi_free(topicQos);
    } else {
        result = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniWaitForAcknowledgments
 * Signature: (LDDS/Duration_t;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitForAcknowledgments)(
    JNIEnv *env, 
    jobject jpublisher, 
    jobject jduration)
{
    gapi_publisher publisher;
    gapi_duration_t* duration;
    saj_returnCode rc;
    gapi_returnCode_t result;
    
    publisher = (gapi_publisher)saj_read_gapi_address(env, jpublisher);    
    
    if(jduration != NULL){
        duration = gapi_duration_t__alloc();
        rc = saj_durationCopyIn(env, jduration, duration);
    
        if(rc == SAJ_RETCODE_OK){
            result = gapi_publisher_wait_for_acknowledgments(publisher, duration);
        } else {
            result = GAPI_RETCODE_ERROR;
        }
        gapi_free(duration);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }    
    return (jint)result;
}

#undef SAJ_FUNCTION
