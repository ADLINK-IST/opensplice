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
#include "saj_DataWriter.h"
#include "saj_dataWriterListener.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_status.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DataWriterImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniSetQos
 * Signature: (LDDS/DataWriterQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jqos)
{
    gapi_dataWriterQos* qos;
    gapi_dataWriter dataWriter;
    saj_returnCode rc;
    jint result;

    qos = gapi_dataWriterQos__alloc();
    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
    rc = saj_DataWriterQosCopyIn(env, jqos, qos);
    result = (jint)GAPI_RETCODE_ERROR;

    if(rc == SAJ_RETCODE_OK){
        result = (jint)gapi_dataWriter_set_qos(dataWriter, qos);
    }
    gapi_free(qos);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetQos
 * Signature: (LDDS/DataWriterQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jqosHolder)
{
    gapi_dataWriterQos* qos;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jqos;
    gapi_dataWriter dataWriter;


    if(jqosHolder != NULL){
        dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
        jqos = NULL;

        qos = gapi_dataWriterQos__alloc();
        result = gapi_dataWriter_get_qos(dataWriter, qos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DataWriterQosCopyOut(env, qos, &jqos);
            gapi_free(qos);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                        GET_CACHED(dataWriterQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniSetListener
 * Signature: (LDDS/DataWriterListener;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jlistener,
    jint jmask)
{
    struct gapi_dataWriterListener *listener;
    gapi_dataWriter dataWriter;
    gapi_returnCode_t grc;

    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
    listener = saj_dataWriterListenerNew(env, jlistener);

    if(listener != NULL){
        saj_write_java_listener_address(env, dataWriter, listener->listener_data);
    }
    grc = gapi_dataWriter_set_listener(dataWriter, listener,
                                                    (unsigned long int)jmask);

    if((grc != GAPI_RETCODE_OK) && (listener != NULL)){
        saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
    }
    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetListener
 * Signature: ()LDDS/DataWriterListener;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetListener)(
    JNIEnv *env,
    jobject jdataWriter)
{
    jobject jlistener;
    struct gapi_dataWriterListener listener;
    gapi_dataWriter dataWriter;

    jlistener = NULL;
    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
    listener = gapi_dataWriter_get_listener(dataWriter);

    jlistener = saj_read_java_listener_address(listener.listener_data);

    return jlistener;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetTopic
 * Signature: ()LDDS/Topic;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetTopic)(
    JNIEnv *env,
    jobject jdataWriter)
{
    jobject jtopic;
    gapi_topic topic;
    gapi_dataWriter dataWriter;

    jtopic = NULL;

    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
    topic = gapi_dataWriter_get_topic(dataWriter);

    if (topic != GAPI_OBJECT_NIL){
        jtopic = saj_read_java_address(topic);
    }
    return jtopic;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetPublisher
 * Signature: ()LDDS/Publisher;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetPublisher)(
    JNIEnv *env,
    jobject jdataWriter)
{
    jobject jpublisher;
    gapi_publisher publisher;
    gapi_dataWriter dataWriter;

    jpublisher = NULL;

    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
    publisher = gapi_dataWriter_get_publisher(dataWriter);

    if (publisher != GAPI_OBJECT_NIL){
        jpublisher = saj_read_java_address(publisher);
    }
    return jpublisher;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetLivelinessLostStatus
 * Signature: ()LDDS/LivelinessLostStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetLivelinessLostStatus)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jstatusHolder)
{
    gapi_dataWriter dataWriter;
    jobject jstatus;
    gapi_livelinessLostStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataWriter = (gapi_dataWriter) saj_read_gapi_address(env, jdataWriter);
        result = gapi_dataWriter_get_liveliness_lost_status(dataWriter, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutLivelinessLostStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(livelinessLostStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetOfferedDeadlineMissedStatus
 * Signature: ()LDDS/OfferedDeadlineMissedStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetOfferedDeadlineMissedStatus)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jstatusHolder)
{
    gapi_dataWriter dataWriter;
    jobject jstatus;
    gapi_offeredDeadlineMissedStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataWriter = (gapi_dataWriter) saj_read_gapi_address(env, jdataWriter);
        result = gapi_dataWriter_get_offered_deadline_missed_status(dataWriter, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutOfferedDeadlineMissedStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(offeredDeadlineMissedStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetOfferedIncompatibleQosStatus
 * Signature: ()LDDS/OfferedIncompatibleQosStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetOfferedIncompatibleQosStatus)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jstatusHolder)
{
    gapi_dataWriter dataWriter;
    jobject jstatus;
    gapi_offeredIncompatibleQosStatus* status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataWriter = (gapi_dataWriter) saj_read_gapi_address(env, jdataWriter);
        status = gapi_offeredIncompatibleQosStatus_alloc();
        result = gapi_dataWriter_get_offered_incompatible_qos_status(dataWriter, status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutOfferedIncompatibleQosStatus(env, status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(offeredIncompatibleQosStatusHolder_value_fid), jstatus);
                (*env)->DeleteLocalRef(env, jstatus);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
        gapi_free(status);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetPublicationMatchStatus
 * Signature: ()LDDS/PublicationMatchStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetPublicationMatchedStatus)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jstatusHolder)
{
    gapi_dataWriter dataWriter;
    jobject jstatus;
    gapi_publicationMatchedStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataWriter = (gapi_dataWriter) saj_read_gapi_address(env, jdataWriter);
        result = gapi_dataWriter_get_publication_matched_status(dataWriter, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutPublicationMatchStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(publicationMatchedStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniAssertLiveliness
 * Signature: (I)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniAssertLiveliness)(
    JNIEnv *env,
    jobject jdataWriter)
{
    gapi_dataWriter dataWriter;

    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
    return (jint)gapi_dataWriter_assert_liveliness(dataWriter);
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetMatchedSubscriptions
 * Signature: (LDDS/InstanceHandleSeqHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetMatchedSubscriptions)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jseqHolder)
{
    gapi_dataWriter dataWriter;
    gapi_instanceHandleSeq *subscription_handles;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jlongArray jarray;

    if(jseqHolder != NULL){
        dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
        subscription_handles = gapi_instanceHandleSeq__alloc();

        result = gapi_dataWriter_get_matched_subscriptions(
                                                dataWriter, subscription_handles);

        if(result == GAPI_RETCODE_OK){
            rc = saj_instanceHandleSequenceCopyOut(env, subscription_handles, &jarray);
            gapi_free(subscription_handles);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jseqHolder,
                        GET_CACHED(instanceHandleSeqHolder_value_fid), jarray);
                (*env)->DeleteLocalRef(env, jarray);
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
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetMatchedSubscriptionData
 * Signature: (LDDS/SubscriptionBuiltinTopicDataHolder;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetMatchedSubscriptionData)(
    JNIEnv *env,
    jobject jdataWriter,
    jobject jdataHolder,
    jlong jhandle)
{
    gapi_dataWriter dataWriter;
    gapi_subscriptionBuiltinTopicData *subscription_data;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jdata;

    if(jdataHolder != NULL){
        jdata = NULL;
        dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter);
        subscription_data = gapi_subscriptionBuiltinTopicData__alloc();

        result = gapi_dataWriter_get_matched_subscription_data(
                                        dataWriter, subscription_data,
                                        (const gapi_instanceHandle_t)jhandle);

        if(result == GAPI_RETCODE_OK){
            rc = saj_subscriptionBuiltinTopicDataCopyOut(env, subscription_data, &jdata);
            gapi_free(subscription_data);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jdataHolder,
                        GET_CACHED(subscriptionBuiltinTopicDataHolder_value_fid), jdata);
                (*env)->DeleteLocalRef(env, jdata);
            } else {
                result = GAPI_RETCODE_ERROR;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return (jint)result;
}

/*
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniWaitForAcknowledgments
 * Signature: (LDDS/Duration_t;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitForAcknowledgments)(
    JNIEnv *env,
    jobject jwriter,
    jobject jduration)
{
    gapi_dataWriter dataWriter;
    gapi_duration_t* duration;
    saj_returnCode rc;
    gapi_returnCode_t result;

    dataWriter = (gapi_dataWriter)saj_read_gapi_address(env, jwriter);

    if(jduration != NULL){
        duration = gapi_duration_t__alloc();
        rc = saj_durationCopyIn(env, jduration, duration);

        if(rc == SAJ_RETCODE_OK){
            result = gapi_dataWriter_wait_for_acknowledgments(dataWriter, duration);
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
