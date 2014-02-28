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
#include "saj_DataReader.h"
#include "saj_dataReaderListener.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_status.h"

#include "saj__fooDataReader.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DataReaderImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniCreateReadcondition
 * Signature: (III)LDDS/ReadCondition;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateReadcondition)(
    JNIEnv *env,
    jobject jdataReader,
    jint jsampleStates,
    jint jviewStates,
    jint jinstanceStates)
{
    jobject jcondition;
    gapi_dataReader dataReader;
    gapi_readCondition condition;

    jcondition = NULL;
    condition = GAPI_OBJECT_NIL;

    dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
    condition = gapi_dataReader_create_readcondition(
                                dataReader,
                                (const gapi_sampleStateMask)jsampleStates,
                                (const gapi_viewStateMask)jviewStates,
                                (const gapi_instanceStateMask)jinstanceStates);

    if (condition != GAPI_OBJECT_NIL){
        saj_construct_java_object(env,  PACKAGENAME "ReadConditionImpl",
                                        (PA_ADDRCAST)condition, &jcondition);
    }
    return jcondition;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniCreateQuerycondition
 * Signature: (IIILjava/lang/String;[Ljava/lang/String;)LDDS/QueryCondition;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateQuerycondition)(
    JNIEnv *env,
    jobject jdataReader,
    jint jsampleStates,
    jint jviewStates,
    jint jinstanceStates,
    jstring jexpression,
    jobjectArray jparams)
{
    jobject jcondition;
    gapi_dataReader dataReader;
    gapi_queryCondition condition;
    gapi_stringSeq* params;
    saj_returnCode rc;
    gapi_char* expression;

    jcondition = NULL;
    condition = GAPI_OBJECT_NIL;
    expression = NULL;

    dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
    rc = SAJ_RETCODE_OK;
    params = gapi_stringSeq__alloc();

    if(jparams != NULL){
        rc = saj_stringSequenceCopyIn(env, jparams, params);
    }

    if(rc == SAJ_RETCODE_OK){
        if(jexpression != NULL){
            expression = (gapi_char*)(*env)->GetStringUTFChars(env, jexpression, 0);
        }
        condition = gapi_dataReader_create_querycondition(
                                dataReader,
                                (const gapi_sampleStateMask)jsampleStates,
                                (const gapi_viewStateMask)jviewStates,
                                (const gapi_instanceStateMask)jinstanceStates,
                                expression,
                                params);

        if(jexpression != NULL){
            (*env)->ReleaseStringUTFChars(env, jexpression, expression);
        }
        if (condition != GAPI_OBJECT_NIL){
            rc = saj_construct_java_object(env,
                                        PACKAGENAME "QueryConditionImpl",
                                        (PA_ADDRCAST)condition, &jcondition);
        }
    }
    gapi_free(params);

    return jcondition;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniDeleteReadcondition
 * Signature: (LDDS/ReadCondition;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteReadcondition)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jcondition)
{
    gapi_readCondition condition;
    gapi_dataReader dataReader;
    gapi_returnCode_t grc;
    c_bool must_free;

    condition = (gapi_readCondition) saj_read_gapi_address(env, jcondition);
    dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);

    must_free = saj_setThreadEnv(env);
    grc = gapi_dataReader_delete_readcondition(dataReader, condition);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniDeleteContainedEntities
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContainedEntities)(
    JNIEnv *env,
    jobject jdataReader)
{
    gapi_dataReader dataReader;
    jint result;
    c_bool must_free;

    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);

    must_free = saj_setThreadEnv(env);
    result = (jint)gapi_dataReader_delete_contained_entities(dataReader);
    saj_delThreadEnv(must_free);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniSetQos
 * Signature: (LDDS/DataReaderQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jqos)
{
    gapi_dataReaderQos* qos;
    gapi_dataReader dataReader;
    saj_returnCode rc;
    jint result;

    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);

    qos = gapi_dataReaderQos__alloc();
    rc = saj_DataReaderQosCopyIn(env, jqos, qos);
    result = (jint)GAPI_RETCODE_ERROR;

    if(rc == SAJ_RETCODE_OK){
        result = (jint)gapi_dataReader_set_qos(dataReader, qos);
    }
    gapi_free(qos);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetQos
 * Signature: (LDDS/DataReaderQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jqosHolder)
{
    gapi_dataReaderQos* qos;
    gapi_returnCode_t result;
    saj_returnCode rc;
    jobject jqos;
    gapi_dataReader dataReader;

    if(jqosHolder != NULL){
        dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
        jqos = NULL;
        qos = gapi_dataReaderQos__alloc();
        result = gapi_dataReader_get_qos(dataReader, qos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DataReaderQosCopyOut(env, qos, &jqos);
            gapi_free(qos);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                        GET_CACHED(dataReaderQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniSetListener
 * Signature: (LDDS/DataReaderListener;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetListener)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jlistener,
    jint jmask)
{
    struct gapi_dataReaderListener *listener;
    gapi_dataReader dataReader;
    gapi_returnCode_t grc;

    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);

    listener = saj_dataReaderListenerNew(env, jlistener);
    if(listener != NULL){
        saj_write_java_listener_address(env, dataReader, listener->listener_data);
    }
    grc = gapi_dataReader_set_listener(dataReader, listener,
                                                    (unsigned long int)jmask);

    if((grc != GAPI_RETCODE_OK) && (listener != NULL)){
        saj_listenerDataFree(env, saj_listenerData(listener->listener_data));
    }
    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetListener
 * Signature: ()LDDS/DataReaderListener;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetListener)(
    JNIEnv *env,
    jobject jdataReader)
{
    jobject jlistener;
    struct gapi_dataReaderListener listener;
    gapi_dataReader dataReader;

    jlistener = NULL;
    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
    listener = gapi_dataReader_get_listener(dataReader);

    jlistener = saj_read_java_listener_address(listener.listener_data);

    return jlistener;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetTopicdescription
 * Signature: ()LDDS/TopicDescription;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetTopicdescription)(
    JNIEnv *env,
    jobject jdataReader)
{
    jobject jdescription;
    gapi_topicDescription description;
    gapi_dataReader dataReader;

    jdescription = NULL;
    description = GAPI_OBJECT_NIL;

    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
    description = gapi_dataReader_get_topicdescription(dataReader);

    if (description != GAPI_OBJECT_NIL){
        jdescription = saj_read_java_address(description);
    }
    return jdescription;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetSubscriber
 * Signature: ()LDDS/Subscriber;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetSubscriber)(
    JNIEnv *env,
    jobject jdataReader)
{
    jobject jsubscriber;
    gapi_subscriber subscriber;
    gapi_dataReader dataReader;

    jsubscriber = NULL;
    subscriber = GAPI_OBJECT_NIL;

    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
    subscriber = gapi_dataReader_get_subscriber(dataReader);

    if (subscriber != GAPI_OBJECT_NIL){
        jsubscriber = saj_read_java_address(subscriber);
    }
    return jsubscriber;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetSampleRejectedStatus
 * Signature: ()LDDS/SampleRejectedStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetSampleRejectedStatus)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jstatusHolder)
{
    gapi_dataReader dataReader;
    jobject jstatus;
    gapi_sampleRejectedStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
        result = gapi_dataReader_get_sample_rejected_status(dataReader, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutSampleRejectedStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(sampleRejectedStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetLivelinessChangedStatus
 * Signature: ()LDDS/LivelinessChangedStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetLivelinessChangedStatus)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jstatusHolder)
{
    gapi_dataReader dataReader;
    jobject jstatus;
    gapi_returnCode_t result;
    gapi_livelinessChangedStatus status;
    saj_returnCode rc;

    if(jstatusHolder){
        dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
        result = gapi_dataReader_get_liveliness_changed_status(dataReader, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutLivelinessChangedStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(livelinessChangedStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetRequestedDeadlineMissedStatus
 * Signature: ()LDDS/RequestedDeadlineMissedStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetRequestedDeadlineMissedStatus)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jstatusHolder)
{
    gapi_dataReader dataReader;
    jobject jstatus;
    gapi_requestedDeadlineMissedStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
        result = gapi_dataReader_get_requested_deadline_missed_status(dataReader, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutRequestedDeadlineMissedStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(requestedDeadlineMissedStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetRequestedIncompatibleQosStatus
 * Signature: ()LDDS/RequestedIncompatibleQosStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetRequestedIncompatibleQosStatus)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jstatusHolder)
{
    gapi_dataReader dataReader;
    jobject jstatus;
    gapi_requestedIncompatibleQosStatus* status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
        status = gapi_requestedIncompatibleQosStatus_alloc();
        result = gapi_dataReader_get_requested_incompatible_qos_status(dataReader, status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutRequestedIncompatibleQosStatus(env, status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(requestedIncompatibleQosStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetSubscriptionMatchStatus
 * Signature: ()LDDS/SubscriptionMatchStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetSubscriptionMatchedStatus)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jstatusHolder)
{
    gapi_dataReader dataReader;
    jobject jstatus;
    gapi_subscriptionMatchedStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
        result = gapi_dataReader_get_subscription_matched_status(dataReader, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutSubscriptionMatchStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(subscriptionMatchedStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetSampleLostStatus
 * Signature: ()LDDS/SampleLostStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetSampleLostStatus)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jstatusHolder)
{
    gapi_dataReader dataReader;
    jobject jstatus;
    gapi_sampleLostStatus status;
    saj_returnCode rc;
    gapi_returnCode_t result;

    if(jstatusHolder){
        dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdataReader);
        result = gapi_dataReader_get_sample_lost_status(dataReader, &status);

        if(result == GAPI_RETCODE_OK){
            rc = saj_statusCopyOutSampleLostStatus(env, &status, &jstatus);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jstatusHolder,
                            GET_CACHED(sampleLostStatusHolder_value_fid), jstatus);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniWaitForHistoricalData
 * Signature: (LDDS/Duration_t;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitForHistoricalData)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jduration)
{
    gapi_dataReader dataReader;
    gapi_duration_t* duration;
    saj_returnCode rc;
    jint jresult;

    jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);

    if(jduration != NULL){
        rc = SAJ_RETCODE_ERROR;
        duration = gapi_duration_t__alloc();
        rc = saj_durationCopyIn(env, jduration, duration);

        if(rc == SAJ_RETCODE_OK){
            jresult = (jint)gapi_dataReader_wait_for_historical_data(
                                                            dataReader, duration);
        }
        gapi_free(duration);
    }
    return jresult;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitForHistoricalDataWCondition)(
    JNIEnv *env,
    jobject jdataReader,
    jstring jfilterExpression,
    jobjectArray jfilterParameters,
    jobject jminSourceTimestamp,
    jobject jmaxSourceTimestamp,
    jobject jresourceLimits,
    jobject jduration)
{
    gapi_dataReader dataReader;
    gapi_duration_t duration;
    saj_returnCode rc;
    gapi_char* filterExpression;
    gapi_stringSeq* filterParameters;
    jint jresult = (jint)GAPI_RETCODE_ERROR;
    gapi_time_t minSourceTimestamp, maxSourceTimestamp;
    gapi_resourceLimitsQosPolicy resourceLimits;

    if(jduration && jminSourceTimestamp && jmaxSourceTimestamp && jresourceLimits){
        dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
        saj_timeCopyIn(env, jminSourceTimestamp, &minSourceTimestamp);
        saj_timeCopyIn(env, jmaxSourceTimestamp, &maxSourceTimestamp);
        saj_ResourceLimitsQosPolicyCopyIn(env, jresourceLimits, &resourceLimits);
        saj_durationCopyIn(env, jduration, &duration);

        if(jfilterParameters){
            filterParameters = gapi_stringSeq__alloc();

            if(filterParameters){
                rc = saj_stringSequenceCopyIn(env, jfilterParameters, filterParameters);
            } else {
                rc = SAJ_RETCODE_ERROR;
                jresult = (jint)GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            filterParameters = NULL;
            rc = SAJ_RETCODE_OK;
        }

        if(rc == SAJ_RETCODE_OK){
            if(jfilterExpression != NULL){
                filterExpression = (gapi_char*)
                    (*env)->GetStringUTFChars(env, jfilterExpression, 0);
            } else {
                filterExpression = NULL;
            }
            jresult = (jint)gapi_dataReader_wait_for_historical_data_w_condition(
                    dataReader, filterExpression, filterParameters,
                    &minSourceTimestamp, &maxSourceTimestamp,
                    &resourceLimits, &duration);

            if(jfilterExpression){
                (*env)->ReleaseStringUTFChars(env, jfilterExpression, filterExpression);
            }
            if(filterParameters){
                gapi_free(filterParameters);
            }
        }
    } else {
        jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return jresult;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetMatchedPublications
 * Signature: (LDDS/InstanceHandleSeqHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetMatchedPublications)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jseqHolder)
{
    gapi_dataReader dataReader;
    gapi_instanceHandleSeq *publication_handles;
    saj_returnCode rc;
    jint jresult;
    jintArray jarray;

    if(jseqHolder != NULL){
        dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
        publication_handles = gapi_instanceHandleSeq__alloc();

        jresult = (jint)gapi_dataReader_get_matched_publications(
                                                dataReader, publication_handles);
        rc = saj_instanceHandleSequenceCopyOut(env, publication_handles, &jarray);
        gapi_free(publication_handles);

        if(rc == SAJ_RETCODE_OK){
            (*env)->SetObjectField(env, jseqHolder,
                    GET_CACHED(instanceHandleSeqHolder_value_fid), jarray);
            (*env)->DeleteLocalRef(env, jarray);
        } else {
            jresult = (jint)GAPI_RETCODE_ERROR;
        }
    } else {
        jresult = (jint)GAPI_RETCODE_BAD_PARAMETER;
    }
    return jresult;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetMatchedPublicationData
 * Signature: (LDDS/PublicationBuiltinTopicDataHolder;I)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetMatchedPublicationData)(
    JNIEnv *env,
    jobject jdataReader,
    jobject jdataHolder,
    jlong jhandle)
{
    gapi_dataReader dataReader;
    gapi_publicationBuiltinTopicData *publication_data;
    saj_returnCode rc;
    gapi_returnCode_t result;
    jobject jdata;

    if(jdataHolder != NULL){
        jdata = NULL;
        dataReader = (gapi_dataReader)saj_read_gapi_address(env, jdataReader);
        publication_data = gapi_publicationBuiltinTopicData__alloc();

        result = gapi_dataReader_get_matched_publication_data(
                                        dataReader, publication_data,
                                        (const gapi_instanceHandle_t)jhandle);

        if(result == GAPI_RETCODE_OK){
            rc = saj_publicationBuiltinTopicDataCopyOut(env, publication_data, &jdata);
            gapi_free(publication_data);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jdataHolder,
                        GET_CACHED(publicationBuiltinTopicDataHolder_value_fid), jdata);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniCreateView
 * Signature: (LDDS/DataReaderViewQos;)LDDS/DataReaderView;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateView)(
    JNIEnv * env,
    jobject jdatareader,
    jobject jqos)
{
    jobject jreaderview = NULL;
    jobject jtypeSupport;
    gapi_subscriber subscriber;
    gapi_dataReader datareader;
    gapi_dataReaderView readerview;
    gapi_dataReaderViewQos* readerviewQos;
    gapi_domainParticipant participant;
    gapi_typeSupport typeSupport;
    gapi_string typeName;
    gapi_topicDescription description;
    gapi_char* dataReaderViewClassName;
    gapi_char* signature;
    saj_returnCode rc;

    readerview = GAPI_OBJECT_NIL;

    datareader = (gapi_dataReader) saj_read_gapi_address(env, jdatareader);
    subscriber = gapi_dataReader_get_subscriber(datareader);
    participant = gapi_subscriber_get_participant(subscriber);
    description = gapi_dataReader_get_topicdescription(datareader);

    typeName = gapi_topicDescription_get_type_name(description);
    typeSupport = gapi_domainParticipant_get_typesupport(participant, (const gapi_char*) typeName);
    gapi_free(typeName);

    jtypeSupport = saj_read_java_address((gapi_object)typeSupport);
    rc = saj_LookupTypeSupportDataReaderView(env, jtypeSupport, &dataReaderViewClassName);

    if(rc == SAJ_RETCODE_OK){
        if ((*env)->IsSameObject (env, jqos, GET_CACHED(DATAREADERVIEW_QOS_DEFAULT)) == JNI_TRUE) {
            readerviewQos = (gapi_dataReaderViewQos *)GAPI_DATAVIEW_QOS_DEFAULT;
            rc = SAJ_RETCODE_OK;
        } else {
            readerviewQos = gapi_dataReaderViewQos__alloc();
            rc = saj_DataReaderViewQosCopyIn(env, jqos, readerviewQos);
        }

        if(rc == SAJ_RETCODE_OK){
            readerview = gapi_dataReader_create_view(datareader, readerviewQos);

            if (readerview != GAPI_OBJECT_NIL){
                rc = saj_LookupTypeSupportConstructorSignature(env, jtypeSupport, &signature);

                if(rc == SAJ_RETCODE_OK){
                    rc = saj_construct_typed_java_object(env, dataReaderViewClassName,
                                                        (PA_ADDRCAST)readerview,
                                                        &jreaderview, signature,
                                                        jtypeSupport);
                    gapi_free(signature);
               }
            }
        }
        if ((readerviewQos != (gapi_dataReaderViewQos *)GAPI_DATAVIEW_QOS_DEFAULT)) {
            gapi_free(readerviewQos);
        }

        gapi_free(dataReaderViewClassName);
    }

    return jreaderview;
}

/*
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniDeleteView
 * Signature: (LDDS/DataReaderView;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteView)(
        JNIEnv *env,
        jobject jdatareader,
        jobject jdatareaderview)
{
    gapi_dataReader dataReader;
    gapi_dataReaderView dataReaderView;
    gapi_returnCode_t grc;

    dataReader = (gapi_dataReader) saj_read_gapi_address(env, jdatareader);
    dataReaderView = (gapi_dataReaderView) saj_read_gapi_address(env, jdatareaderview);

    grc = gapi_dataReader_delete_view(dataReader, dataReaderView);

    return (jint)grc;
}


/*
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetDefaultDataReaderViewQos
 * Signature: (LDDS/DataReaderViewQosHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDefaultDataReaderViewQos)(
    JNIEnv *env,
    jobject jdatareader,
    jobject jqosHolder)
{
    saj_returnCode rc;
    jobject jqos;
    gapi_dataReader datareader;
    gapi_returnCode_t result;
    gapi_dataReaderViewQos *qos;

    jqos = NULL;
    rc = SAJ_RETCODE_ERROR;

    if(jqosHolder != NULL){
        qos = gapi_dataReaderViewQos__alloc();

        datareader = (gapi_dataReader)saj_read_gapi_address(env, jdatareader);
        result = gapi_dataReader_get_default_datareaderview_qos(datareader, qos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DataReaderViewQosCopyOut(env, qos, &jqos);
            gapi_free(qos);

            if (rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                                       GET_CACHED(dataReaderViewQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniSetDefaultDataReaderViewQos
 * Signature: (LDDS/DataReaderViewQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetDefaultDataReaderViewQos)(
    JNIEnv *env,
    jobject jdatareader,
    jobject jqos)
{
    gapi_dataReaderViewQos* qos;
    gapi_dataReader datareader;
    saj_returnCode rc;
    jint result;

    result = (jint)GAPI_RETCODE_ERROR;
    qos = gapi_dataReaderViewQos__alloc();
    rc = saj_DataReaderViewQosCopyIn(env, jqos, qos);

    if (rc == SAJ_RETCODE_OK){
        datareader = (gapi_dataReader)saj_read_gapi_address(env, jdatareader);
        result = (jint)gapi_dataReader_set_default_datareaderview_qos(datareader, qos);
    }
    gapi_free(qos);

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniParallelDemarshallingMain
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniParallelDemarshallingMain) (
    JNIEnv * env,
    jobject jdatareader,
    jlong ctx)
{
    OS_UNUSED_ARG(jdatareader);
    return saj_fooDataReaderParallelDemarshallingMain(env, (sajParDemContext)((PA_ADDRCAST)ctx));
}

/*
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniSetProperty
 * Signature: (LDDS/Property;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetProperty) (
    JNIEnv * env,
    jobject jdatareader,
    jobject jprop)
{
    jint result = GAPI_RETCODE_OK;
    jstring jname, jvalue;
    char *name, *value;

    if(jprop == NULL){
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if((jname = (*env)->GetObjectField(env, jprop, GET_CACHED(property_name_fid))) != NULL){
        name = (char*)(*env)->GetStringUTFChars(env, jname, 0);
        if(strcmp("parallelReadThreadCount", name) == 0){
            int ival;

            if((jvalue = (*env)->GetObjectField(env, jprop, GET_CACHED(property_value_fid))) != NULL){
                char * end;

                value = (char*)(*env)->GetStringUTFChars(env, jvalue, 0);
                ival = strtol(value, &end, 10);
                if(*end == '\0' && ival >= 0){
                    /* The entire string was valid and contains a valid value */
                    if(saj_fooDataReaderSetParallelReadThreadCount(env, jdatareader, ival) < 0){
                        result = GAPI_RETCODE_ERROR;
                    }
                } else {
                    result = GAPI_RETCODE_BAD_PARAMETER;
                }
                (*env)->ReleaseStringUTFChars(env, jvalue, value);
                (*env)->DeleteLocalRef(env, jvalue);
            } else {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
        } else if(strcmp("CDRCopy", name) == 0){

            if((jvalue = (*env)->GetObjectField(env, jprop, GET_CACHED(property_value_fid))) != NULL){
                value = (char*)(*env)->GetStringUTFChars(env, jvalue, 0);
                if(strcmp("true", value) == 0){
                    if(saj_fooDataReaderSetCDRCopy(env, jdatareader, 1) < 0){
                        result = GAPI_RETCODE_ERROR;
                    }
                } else if (strcmp("false", value) == 0){
                    if(saj_fooDataReaderSetCDRCopy(env, jdatareader, 0) < 0){
                        result = GAPI_RETCODE_ERROR;
                    }
                } else {
                    result = GAPI_RETCODE_BAD_PARAMETER;
                }
                (*env)->ReleaseStringUTFChars(env, jvalue, value);
                (*env)->DeleteLocalRef(env, jvalue);
            } else {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
        } else {
            result = GAPI_RETCODE_UNSUPPORTED;
        }
        (*env)->ReleaseStringUTFChars(env, jname, name);
        (*env)->DeleteLocalRef(env, jname);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniGetProperty
 * Signature: (LDDS/PropertyHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetProperty) (
    JNIEnv * env,
    jobject jdatareader,
    jobject jpropHolder)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jdatareader);
    OS_UNUSED_ARG(jpropHolder);
    return (jint)GAPI_RETCODE_UNSUPPORTED;
}

#undef SAJ_FUNCTION
