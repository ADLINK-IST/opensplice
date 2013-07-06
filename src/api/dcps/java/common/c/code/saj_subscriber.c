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
#include "saj_Subscriber.h"
#include "saj_subscriberListener.h"
#include "saj_dataReaderListener.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"

#include "saj__fooDataReader.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_SubscriberImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniCreateDatareader
 * Signature: (LDDS/TopicDescription;LDDS/DataReaderQos;LDDS/DataReaderListener;)LDDS/DataReader;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateDatareader)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jdescription,
    jobject jqos,
    jobject jlistener,
    jint jmask)
{
    jobject jreader;
    jobject jtypeSupport;
    gapi_subscriber subscriber;
    gapi_dataReader reader;
    gapi_dataReaderQos* readerQos;
    gapi_domainParticipant participant;
    gapi_typeSupport typeSupport;
    gapi_string typeName;
    gapi_topicDescription description;
    struct gapi_dataReaderListener *listener;
    gapi_char* dataReaderClassName;
    gapi_char* signature;
    saj_returnCode rc;

    listener = NULL;
    jreader = NULL;
    reader = GAPI_OBJECT_NIL;

    subscriber = (gapi_subscriber) saj_read_gapi_address(env, jsubscriber);
    participant = gapi_subscriber_get_participant(subscriber);
    description = (gapi_topicDescription) saj_read_gapi_address(env, jdescription);

    typeName = gapi_topicDescription_get_type_name(description);
    typeSupport = gapi_domainParticipant_get_typesupport(participant, (const gapi_char*) typeName);
    gapi_free(typeName);

    jtypeSupport = saj_read_java_address((gapi_object)typeSupport);
    rc = saj_LookupTypeSupportDataReader(env, jtypeSupport, &dataReaderClassName);

    if(rc == SAJ_RETCODE_OK){
        if ((*env)->IsSameObject (env, jqos, GET_CACHED(DATAREADER_QOS_DEFAULT)) == JNI_TRUE) {
            readerQos = (gapi_dataReaderQos *)GAPI_DATAREADER_QOS_DEFAULT;
            rc = SAJ_RETCODE_OK;
        } else if ((*env)->IsSameObject (env, jqos, GET_CACHED(DATAREADER_QOS_USE_TOPIC_QOS)) == JNI_TRUE) {
            readerQos = (gapi_dataReaderQos *)GAPI_DATAREADER_QOS_USE_TOPIC_QOS;
            rc = SAJ_RETCODE_OK;
        } else {
            readerQos = gapi_dataReaderQos__alloc();
            rc = saj_DataReaderQosCopyIn(env, jqos, readerQos);
        }

        if(rc == SAJ_RETCODE_OK){
            listener = saj_dataReaderListenerNew(env, jlistener);
            reader = gapi_subscriber_create_datareader(subscriber, description,
                                                        readerQos, listener, (gapi_statusMask)jmask);

            if (reader != GAPI_OBJECT_NIL){
                rc = saj_LookupTypeSupportConstructorSignature(env, jtypeSupport, &signature);

                if(rc == SAJ_RETCODE_OK){
                    gapi_subscriberQos *sqos = gapi_subscriberQos__alloc();
                    rc = saj_construct_typed_java_object(env, dataReaderClassName,
                                                        (PA_ADDRCAST)reader,
                                                        &jreader, signature,
                                                        jtypeSupport);
                    gapi_free(signature);

                    if(listener != NULL){
                        saj_write_java_listener_address(env, reader, listener->listener_data);
                    }

                    if(sqos){
                        if(gapi_subscriber_get_qos(subscriber, sqos) == GAPI_RETCODE_OK){
                            if(sqos->entity_factory.autoenable_created_entities) {
                                gapi_entity_enable(reader);
                            }
                        }
                        gapi_free(sqos);
                    }
                }
            } else if(listener != NULL){
                saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
            }
        }
        if ((readerQos != (gapi_dataReaderQos *)GAPI_DATAREADER_QOS_DEFAULT) &&
            (readerQos != (gapi_dataReaderQos *)GAPI_DATAREADER_QOS_USE_TOPIC_QOS)) {
            gapi_free(readerQos);
        }
        gapi_free(dataReaderClassName);
    }

    return jreader;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniDeleteDatareader
 * Signature: (LDDS/DataReader;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteDatareader)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jdataReader)
{
    gapi_subscriber subscriber;
    gapi_dataReader dataReader;
    gapi_returnCode_t grc;
    c_bool must_free;

    subscriber = (gapi_subscriber) saj_read_gapi_address(env, jsubscriber);
    dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);

    must_free = saj_setThreadEnv(env);
    grc = gapi_subscriber_delete_datareader(subscriber, dataReader);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniDeleteContainedEntities
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContainedEntities)(
    JNIEnv *env,
    jobject jsubscriber)
{
    gapi_subscriber subscriber;
    jint result;
    c_bool must_free;

    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);

    must_free = saj_setThreadEnv(env);
    result = (jint)gapi_subscriber_delete_contained_entities(subscriber);
    saj_delThreadEnv(must_free);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniLookupDatareader
 * Signature: (Ljava/lang/String;)LDDS/DataReader;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniLookupDatareader)(
    JNIEnv *env,
    jobject jsubscriber,
    jstring jtopicName)
{
    jobject jreader;
    gapi_subscriber subscriber;
    gapi_dataReader reader;
    const gapi_char* topicName;

    jreader = NULL;
    reader = GAPI_OBJECT_NIL;
    topicName = NULL;

    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);

    if(jtopicName != NULL){
        topicName = (*env)->GetStringUTFChars(env, jtopicName, 0);
    }
    reader = gapi_subscriber_lookup_datareader(subscriber, topicName);

    if (reader != GAPI_OBJECT_NIL){
        jreader = saj_read_java_address(reader);
    }
    if(jtopicName != NULL){
        (*env)->ReleaseStringUTFChars(env, jtopicName, topicName);
    }
    return jreader;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniGetDatareaders
 * Signature: (LDDS/DataReaderSeqHolder;III)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDatareaders)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jseqHolder,
    jint jsampleStates,
    jint jviewStates,
    jint jinstanceStates)
{
    gapi_subscriber subscriber;
    gapi_returnCode_t grc;
    gapi_dataReaderSeq *readerSeq;
    saj_returnCode rc;
    jobjectArray jreaderSeq;
    jint jresult;

    if(jseqHolder != NULL){
        readerSeq = gapi_dataReaderSeq__alloc();
        if (readerSeq)
        {
            subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
            grc = gapi_subscriber_get_datareaders(subscriber, readerSeq,
                                        (const gapi_sampleStateMask)jsampleStates,
                                        (const gapi_viewStateMask)jviewStates,
                                        (const gapi_instanceStateMask)jinstanceStates);

            if(grc == GAPI_RETCODE_OK){
                rc = saj_LookupExistingDataReaderSeq(env, readerSeq, &jreaderSeq);

                if(rc == SAJ_RETCODE_OK){
                    (*env)->SetObjectField(env, jseqHolder,
                                GET_CACHED(dataReaderSeqHolder_value_fid), jreaderSeq);
                }
                gapi_free(readerSeq);
            }
            jresult = (jint)grc;
        } else {
            jresult = (jint)GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    } else {
        jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return jresult;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniNotifyDatareaders
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniNotifyDatareaders)(
    JNIEnv *env,
    jobject jsubscriber)
{
    gapi_subscriber subscriber;
    c_bool must_free;
    jint result;

    must_free = saj_setThreadEnv(env);
    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
    result = (jint)gapi_subscriber_notify_datareaders(subscriber);
    saj_delThreadEnv(must_free);
    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniSetQos
 * Signature: (LDDS/SubscriberQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jqos)
{
    gapi_subscriberQos* qos;
    gapi_subscriber subscriber;
    saj_returnCode rc;
    jint result;

    qos = gapi_subscriberQos__alloc();
    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
    rc = saj_SubscriberQosCopyIn(env, jqos, qos);
    result = (jint)GAPI_RETCODE_ERROR;

    if(rc == SAJ_RETCODE_OK){
        result = (jint)gapi_subscriber_set_qos(subscriber, qos);
    }
    gapi_free(qos);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniGetQos
 * Signature: (LDDS/SubscriberQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jqosHolder)
{
    gapi_subscriberQos* qos;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jqos;
    gapi_subscriber subscriber;

    if(jqosHolder != NULL){
        subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
        jqos = NULL;

        qos = gapi_subscriberQos__alloc();
        result = gapi_subscriber_get_qos(subscriber, qos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_SubscriberQosCopyOut(env, qos, &jqos);
            gapi_free(qos);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                        GET_CACHED(subscriberQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniSetListener
 * Signature: (LDDS/SubscriberListener;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jlistener,
    jint jmask)
{
    struct gapi_subscriberListener *listener;
    gapi_subscriber subscriber;
    gapi_returnCode_t grc;

    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
    listener = saj_subscriberListenerNew(env, jlistener);
    grc = gapi_subscriber_set_listener(subscriber, listener,
                                                    (unsigned long int)jmask);

    if(grc == GAPI_RETCODE_OK){
        if(listener != NULL){
            saj_write_java_listener_address(env, subscriber, listener->listener_data);
        }
    } else if(listener != NULL){
        saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
    }
    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniGetListener
 * Signature: ()LDDS/SubscriberListener;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetListener)(
    JNIEnv *env,
    jobject jsubscriber)
{
    jobject jlistener;
    struct gapi_subscriberListener listener;
    gapi_subscriber subscriber;

    jlistener = NULL;
    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
    listener = gapi_subscriber_get_listener(subscriber);

    jlistener = saj_read_java_listener_address(listener.listener_data);

    return jlistener;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniBeginAccess
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniBeginAccess)(
    JNIEnv *env,
    jobject jsubscriber)
{
    gapi_subscriber subscriber;

    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);

    return (jint)gapi_subscriber_begin_access(subscriber);
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniEndAccess
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniEndAccess)(
    JNIEnv *env,
    jobject jsubscriber)
{
    gapi_subscriber subscriber;

    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);

    return (jint)gapi_subscriber_end_access(subscriber);
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniGetParticipant
 * Signature: ()LDDS/DomainParticipant;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetParticipant)(
    JNIEnv *env,
    jobject jsubscriber)
{
    gapi_subscriber subscriber;
    gapi_domainParticipant participant;

    subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
    participant = gapi_subscriber_get_participant(subscriber);

    return saj_read_java_address(participant);
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniSetDefaultDatareaderQos
 * Signature: (LDDS/DataReaderQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultDatareaderQos)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jqos)
{
    gapi_dataReaderQos* qos;
    gapi_subscriber subscriber;
    saj_returnCode rc;
    jint result;

    result = (jint)GAPI_RETCODE_ERROR;
    qos = gapi_dataReaderQos__alloc();
    rc = saj_DataReaderQosCopyIn(env, jqos, qos);

    if (rc == SAJ_RETCODE_OK){
        subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
        result = (jint)gapi_subscriber_set_default_datareader_qos(subscriber, qos);
    }
    gapi_free(qos);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniGetDefaultDatareaderQos
 * Signature: (LDDS/DataReaderQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDefaultDatareaderQos)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jqosHolder)
{
    saj_returnCode rc;
    jobject jqos;
    gapi_subscriber subscriber;
    gapi_returnCode_t result;
    gapi_dataReaderQos *qos;

    jqos = NULL;
    rc = SAJ_RETCODE_ERROR;

    if(jqosHolder != NULL){
        qos = gapi_dataReaderQos__alloc();

        subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
        result = gapi_subscriber_get_default_datareader_qos(subscriber, qos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DataReaderQosCopyOut(env, qos, &jqos);
            gapi_free(qos);

            if (rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                                       GET_CACHED(dataReaderQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniCopyFromTopicQos
 * Signature: (LDDS/DataReaderQosHolder;LDDS/TopicQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniCopyFromTopicQos)(
    JNIEnv *env,
    jobject jsubscriber,
    jobject jqosHolder,
    jobject jtopicQos)
{
    saj_returnCode rc;
    gapi_returnCode_t grc;
    jobject jqos, oldQos;
    gapi_subscriber subscriber;
    gapi_dataReaderQos *qos;
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
            topicQos = gapi_topicQos__alloc ();
            rc = saj_TopicQosCopyIn(env, jtopicQos, topicQos);
        }

        if(rc == SAJ_RETCODE_OK){
            oldQos = (*env)->GetObjectField(env, jqosHolder,
                                  GET_CACHED(dataReaderQosHolder_value_fid));
            qos = gapi_dataReaderQos__alloc();

            if(oldQos){
                rc = saj_DataReaderQosCopyIn(env, oldQos, qos);

                if(rc != SAJ_RETCODE_OK) {
                    gapi_free(qos);
                    qos = NULL;
                }
            }
        } else {
            qos = NULL;
        }
        subscriber = (gapi_subscriber)saj_read_gapi_address(env, jsubscriber);
        grc = gapi_subscriber_copy_from_topic_qos(subscriber, qos, topicQos);
        result = (jint)grc;

        if(grc == GAPI_RETCODE_OK){
            if(oldQos){
                jqos = oldQos;
            } else {
                jqos = NULL;
            }
            rc = saj_DataReaderQosCopyOut(env, qos, &jqos);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                                  GET_CACHED(dataReaderQosHolder_value_fid), jqos);
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

#undef SAJ_FUNCTION
