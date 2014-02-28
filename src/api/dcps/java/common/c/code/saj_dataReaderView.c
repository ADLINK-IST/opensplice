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
#include "saj_DataReaderView.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj_status.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DataReaderViewImpl_##name

/**
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniCreateReadcondition
 * Signature: (III)LDDS/ReadCondition;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateReadcondition)(
    JNIEnv *env,
    jobject jdataReaderView,
    jint jsampleStates,
    jint jviewStates,
    jint jinstanceStates)
{
    jobject jcondition;
    gapi_dataReaderView dataReaderView;
    gapi_readCondition condition;

    jcondition = NULL;
    condition = GAPI_OBJECT_NIL;

    dataReaderView = (gapi_dataReaderView) saj_read_gapi_address(env, jdataReaderView);
    condition = gapi_dataReaderView_create_readcondition(
                                dataReaderView,
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
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniCreateQuerycondition
 * Signature: (IIILjava/lang/String;[Ljava/lang/String;)LDDS/QueryCondition;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniCreateQuerycondition)(
    JNIEnv *env,
    jobject jdataReaderView,
    jint jsampleStates,
    jint jviewStates,
    jint jinstanceStates,
    jstring jexpression,
    jobjectArray jparams)
{
    jobject jcondition;
    gapi_dataReaderView dataReaderView;
    gapi_queryCondition condition;
    gapi_stringSeq* params;
    saj_returnCode rc;
    gapi_char* expression;

    jcondition = NULL;
    condition = GAPI_OBJECT_NIL;
    expression = NULL;

    dataReaderView = (gapi_dataReaderView) saj_read_gapi_address(env, jdataReaderView);
    rc = SAJ_RETCODE_OK;
    params = gapi_stringSeq__alloc();

    if(jparams != NULL){
        rc = saj_stringSequenceCopyIn(env, jparams, params);
    }

    if(rc == SAJ_RETCODE_OK){
        if(jexpression != NULL){
            expression = (gapi_char*)(*env)->GetStringUTFChars(env, jexpression, 0);
        }
        condition = gapi_dataReaderView_create_querycondition(
                                dataReaderView,
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
    jobject jdataReaderView,
    jobject jcondition)
{
    gapi_readCondition condition;
    gapi_dataReader dataReaderView;
    gapi_returnCode_t grc;
    c_bool must_free;

    condition = (gapi_readCondition) saj_read_gapi_address(env, jcondition);
    dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);

    must_free = saj_setThreadEnv(env);
    grc = gapi_dataReaderView_delete_readcondition(dataReaderView,
                                                   condition);
    saj_delThreadEnv(must_free);

    return (jint)grc;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniDeleteContainedEntities
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDeleteContainedEntities)(
    JNIEnv *env,
    jobject jdataReaderView)
{
    gapi_dataReaderView dataReaderView;
    jint result;
    c_bool must_free;

    dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);

    must_free = saj_setThreadEnv(env);
    result = (jint)gapi_dataReaderView_delete_contained_entities(dataReaderView);
    saj_delThreadEnv(must_free);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniSetQos
 * Signature: (LDDS/DataReaderViewQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jdataReaderView,
    jobject jqos)
{
    gapi_dataReaderViewQos* qos;
    gapi_dataReaderView dataReaderView;
    saj_returnCode rc;
    jint result;

    dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);

    qos = gapi_dataReaderViewQos__alloc();
    rc = saj_DataReaderViewQosCopyIn(env, jqos, qos);
    result = (jint)GAPI_RETCODE_ERROR;

    if(rc == SAJ_RETCODE_OK){
        result = (jint)gapi_dataReaderView_set_qos(dataReaderView, qos);
    }
    gapi_free(qos);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniGetQos
 * Signature: (LDDS/DataReaderViewQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jdataReaderView,
    jobject jqosHolder)
{
    gapi_dataReaderViewQos* qos;
    gapi_returnCode_t result;
    saj_returnCode rc;
    jobject jqos;
    gapi_dataReaderView dataReaderView;

    if(jqosHolder != NULL){
        dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);
        jqos = NULL;
        qos = gapi_dataReaderViewQos__alloc();
        result = gapi_dataReaderView_get_qos(dataReaderView, qos);

        if(result == GAPI_RETCODE_OK){
            rc = saj_DataReaderViewQosCopyOut(env, qos, &jqos);
            gapi_free(qos);

            if(rc == SAJ_RETCODE_OK){
                (*env)->SetObjectField(env, jqosHolder,
                        GET_CACHED(dataReaderViewQosHolder_value_fid), jqos);
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
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniGetDataReader
 * Signature: ()LDDS/DataReader;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetDataReader)(
    JNIEnv *env,
    jobject jdataReaderView)
{
    jobject jdatareader;
    gapi_dataReader dataReader;
    gapi_dataReaderView dataReaderView;

    jdatareader = NULL;
    dataReader = GAPI_OBJECT_NIL;

    dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);
    dataReader = gapi_dataReaderView_get_datareader(dataReaderView);

    if (dataReader != GAPI_OBJECT_NIL){
        jdatareader = saj_read_java_address(dataReader);
    }
    return jdatareader;
}

/*
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniGetStatusCondition
 * Signature: ()LDDS/StatusCondition;
 */
JNIEXPORT jobject JNICALL
SAJ_FUNCTION(jniGetStatusCondition)(
    JNIEnv *env,
    jobject jdataReaderView)
{
    jobject jstatusCondition;
    gapi_statusCondition statusCondition;
    gapi_dataReaderView dataReaderView;

    jstatusCondition = NULL;
    statusCondition = GAPI_OBJECT_NIL;

    dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);
    statusCondition = gapi_dataReaderView_get_statuscondition(dataReaderView);

    if (statusCondition != GAPI_OBJECT_NIL){
        jstatusCondition = saj_read_java_address(statusCondition);
    }
    return jstatusCondition;
}

/*
 * Class:     org_opensplice_dds_dcps_DataReaderViewImpl
 * Method:    jniGetStatusChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetStatusChanges)(
        JNIEnv * env,
        jobject jdataReaderView)
{
    gapi_dataReaderView dataReaderView;

    dataReaderView = (gapi_dataReaderView)saj_read_gapi_address(env, jdataReaderView);
    return gapi_dataReaderView_get_status_changes(dataReaderView);
}


#undef SAJ_FUNCTION
