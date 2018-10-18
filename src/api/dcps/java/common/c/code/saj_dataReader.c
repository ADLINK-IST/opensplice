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
#include "saj_DataReader.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "saj__fooDataReader.h"
#include "saj_dataReaderParDem.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DataReaderImpl_##name

/*
 * Method: jniDataReaderNew
 * Param : domain participant
 * Param : reader name
 * Param : readerQos qos
 * Return: u_reader
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniDataReaderNew) (
    JNIEnv  *env,
    jobject this,
    jlong uSubscriber,
    jstring jname,
    jstring jexpression,
    jobjectArray jparameters,
    jobject qos)
{
    u_dataReader uReader = NULL;
    u_readerQos uQos;
    saj_returnCode retcode;
    const os_char *name;
    const os_char *expression;
    c_value *params = NULL;
    jobject userData;
    int nrOfParams = 0;
    int i;
    jobject jParam;

    name = GET_STRING_UTFCHAR(env, jname, 0);
    if (name != NULL){
        uQos = u_readerQosNew(NULL);
        if (uQos != NULL) {
            retcode = saj_dataReaderQosCopyIn(env, qos, uQos);
            if (retcode == SAJ_RETCODE_OK) {
                expression = GET_STRING_UTFCHAR(env, jexpression, 0);
                if (expression != NULL) {
                    jParam = NULL;
                    if (jparameters){
                        nrOfParams = GET_ARRAY_LENGTH(env, jparameters);
                        if(nrOfParams > 0) {
                            params = os_malloc(nrOfParams * sizeof(c_value));
                            for (i =0; i<nrOfParams; i++) {
                                jParam = GET_OBJECTARRAY_ELEMENT(env, jparameters, i);
                                params[i].is.String = (os_char *)GET_STRING_UTFCHAR(env, jParam, 0);
                                params[i].kind = V_STRING;
                            }
                        }
                    }
                    uReader = u_dataReaderNew(SAJ_VOIDP(uSubscriber), name,
                                                   expression, params, nrOfParams, uQos);
                    RELEASE_STRING_UTFCHAR(env, jexpression, expression);
                    if (params != NULL) {
                        for (i =0; i<nrOfParams; i++) {
                            RELEASE_STRING_UTFCHAR(env, jParam, params[i].is.String);
                        }
                        os_free(params);
                    }

                    if (uReader != NULL) {
                        userData = NEW_GLOBAL_REF(env, this);
                        u_observableSetUserData(SAJ_VOIDP(uReader), userData);
                    } else {
                        retcode = SAJ_RETCODE_ERROR;
                    }
                } else {
                    retcode = SAJ_RETCODE_ERROR;
                    SAJ_REPORT(retcode, "query_expression 'null' is invalid.");
                }
            }

            u_readerQosFree(uQos);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
    }

    return SAJ_JLONG(uReader);

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDataReaderFree) (
    JNIEnv  *env,
    jobject this,
    jlong uReader)
{
    saj_returnCode result = SAJ_RETCODE_OK;
    u_result uResult;
    jobject userData;

    if (uReader) {
        userData = u_observableSetUserData(u_observable(SAJ_VOIDP(uReader)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }
    /* set thread count back to 0, so threads are deleted if they are present. */
    (void) saj_fooDataReaderSetParallelReadThreadCount(env, this, 0);
    uResult = u_objectClose(SAJ_VOIDP(uReader));
    result = saj_retcode_from_user_result(uResult);

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
    jlong uReader,
    jobject jqos)
{
    u_readerQos uQos;
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_OK;

    assert(jqos);
    OS_UNUSED_ARG(jdataReader);

    uQos = u_readerQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_dataReaderQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK) {
            uResult = u_dataReaderSetQos(SAJ_VOIDP(uReader), uQos);
            retcode = saj_retcode_from_user_result(uResult);
        }
        u_readerQosFree(uQos);
    } else {
        retcode = SAJ_RETCODE_ERROR;
    }

    return (jint)retcode;
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
    jlong uReader,
    jobject jqosHolder)
{
    u_readerQos uQos;
    u_result uResult;
    jobject jqos = NULL;
    saj_returnCode retcode = SAJ_RETCODE_OK;

    assert(jqosHolder != NULL);
    OS_UNUSED_ARG(jdataReader);

    uResult = u_dataReaderGetQos(SAJ_VOIDP(uReader), &uQos);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        retcode = saj_dataReaderQosCopyOut(env, uQos, &jqos);
        u_readerQosFree(uQos);
        if (retcode == SAJ_RETCODE_OK){
            SET_OBJECT_FIELD(env, jqosHolder, dataReaderQosHolder_value, jqos);
            DELETE_LOCAL_REF(env, jqos);
        }
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

struct copyArg {
    JNIEnv *env;
    jobject object;
};

static v_result
copy_sample_rejected_status (
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_sampleRejectedInfo *from;
    struct copyArg *copyArg;

    from = (struct v_sampleRejectedInfo *)info;
    copyArg = (struct copyArg *)arg;

    copyArg->object = saj_sampleRejectedStatus_new(copyArg->env, from);
    if (copyArg->object != NULL) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uReader,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_OK;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_sample_rejected_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetSampleRejectedStatus(SAJ_VOIDP(uReader), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, sampleRejectedStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

static v_result
copy_liveliness_changed_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_livelinessChangedInfo *from = (struct v_livelinessChangedInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = saj_livelinessChangedStatus_new(copyArg->env, from);
    if (copyArg->object != NULL) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uReader,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_liveliness_changed_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetLivelinessChangedStatus(SAJ_VOIDP(uReader), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, livelinessChangedStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

static v_result
copy_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_deadlineMissedInfo *from = (struct v_deadlineMissedInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = saj_requestedDeadlineMissedStatus_new(copyArg->env, from);
    if (copyArg->object != NULL) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uReader,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_deadline_missed_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetDeadlineMissedStatus(SAJ_VOIDP(uReader), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, requestedDeadlineMissedStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

static v_result
copy_incompatible_qos_status (
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_incompatibleQosInfo *from = (struct v_incompatibleQosInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;
    copyArg->object = saj_requestedIncompatibleQosStatus_new(copyArg->env, from);
    if (copyArg->object != NULL) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uReader,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_incompatible_qos_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetIncompatibleQosStatus(SAJ_VOIDP(uReader), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, requestedIncompatibleQosStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

static v_result
copy_subscription_matched_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_topicMatchInfo *from = (struct v_topicMatchInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = saj_subscriptionMatchStatus_new(copyArg->env, from);
    if (copyArg->object != NULL) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uReader,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_subscription_matched_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetSubscriptionMatchStatus(SAJ_VOIDP(uReader), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, subscriptionMatchedStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

static v_result
copy_sample_lost_status (
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_sampleLostInfo *from = (struct v_sampleLostInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = saj_sampleLostStatus_new(copyArg->env, from);
    if (copyArg->object != NULL) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uReader,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_sample_lost_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetSampleLostStatus(SAJ_VOIDP(uReader), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, sampleLostStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/****************************************************************************************/

/**
 * Class:     org_opensplice_dds_dcps_DataReaderImpl
 * Method:    jniWaitForHistoricalData
 * Signature: (LDDS/Duration_t;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitForHistoricalData)(
    JNIEnv *env,
    jobject jdataReader,
    jlong uReader,
    jobject jduration)
{
    os_duration timeout;
    u_result uResult;
    saj_returnCode retcode;

    assert(jduration != NULL);
    OS_UNUSED_ARG(jdataReader);

    retcode = saj_durationCopyIn(env, jduration, &timeout);
    if (retcode == SAJ_RETCODE_OK){
        uResult = u_dataReaderWaitForHistoricalData(SAJ_VOIDP(uReader), timeout);
        retcode = saj_retcode_from_user_result(uResult);
    }

    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWaitForHistoricalDataWCondition)(
    JNIEnv *env,
    jobject jdataReader,
    jlong uReader,
    jstring jfilterExpression,
    jobjectArray jfilterParameters,
    jobject jminSourceTimestamp,
    jobject jmaxSourceTimestamp,
    jobject jresourceLimits,
    jobject jduration)
{
    os_duration duration;
    u_result uResult;
    saj_returnCode retcode;
    os_timeW minSourceTimestamp, maxSourceTimestamp;
    os_char* filterExpression;
    const os_char* filterParameter;
    os_char** filterParameters;
    os_int32 length, i;
    os_int32 maxSamples, maxInstances, maxSamplesPerInstance;
    jstring jParam;

    length = 0;
    filterParameters = NULL;

    assert(jduration && jminSourceTimestamp && jmaxSourceTimestamp && jresourceLimits);
    OS_UNUSED_ARG(jdataReader);

    retcode = saj_timeCopyInAcceptInvalidTime(env, jminSourceTimestamp, &minSourceTimestamp);
    if (retcode == SAJ_RETCODE_OK) {
        retcode = saj_timeCopyInAcceptInvalidTime(env, jmaxSourceTimestamp, &maxSourceTimestamp);
    }
    if (retcode == SAJ_RETCODE_OK) {
        retcode = saj_durationCopyIn(env, jduration, &duration);
    }
    if (retcode == SAJ_RETCODE_OK) {
        if (jfilterParameters){
            length = GET_ARRAY_LENGTH(env, jfilterParameters);
            if(length > 0) {
                filterParameters = os_malloc(length * sizeof(os_char *));
                for (i =0; i<length; i++) {
                    jParam = GET_OBJECTARRAY_ELEMENT(env, jfilterParameters, i);
                    filterParameter = GET_STRING_UTFCHAR(env, jParam, 0);
                    filterParameters[i] = os_strdup(filterParameter);
                    RELEASE_STRING_UTFCHAR(env, jParam, filterParameter);
                }
            }
        }
        if (jfilterExpression != NULL){
            filterExpression = (os_char*)GET_STRING_UTFCHAR(env, jfilterExpression, 0);
        } else {
            filterExpression = NULL;
        }

        maxSamples = GET_INT_FIELD(env, jresourceLimits,
                                   resourceLimitsQosPolicy_maxSamples);
        maxInstances = GET_INT_FIELD(env, jresourceLimits,
                                   resourceLimitsQosPolicy_maxInstances);
        maxSamplesPerInstance = GET_INT_FIELD(env, jresourceLimits,
                                   resourceLimitsQosPolicy_maxSamplesPerInstance);

        uResult = u_dataReaderWaitForHistoricalDataWithCondition(
                        SAJ_VOIDP(uReader),
                        filterExpression, (const os_char**)filterParameters, length,
                        minSourceTimestamp, maxSourceTimestamp,
                        maxSamples, maxInstances, maxSamplesPerInstance,
                        duration);
        retcode = saj_retcode_from_user_result(uResult);

        if (jfilterExpression){
            RELEASE_STRING_UTFCHAR(env, jfilterExpression, filterExpression);
        }
        if (filterParameters){
            for (i =0; i<length; i++) {
                os_free(filterParameters[i]);
            }
            os_free(filterParameters);
        }
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

#define handleBufferSize (32)

C_CLASS(handleBuffer);
C_STRUCT(handleBuffer)
{
    u_instanceHandle buf[handleBufferSize];
    handleBuffer next;
};

C_CLASS(copy_matched_publications_arg);
C_STRUCT(copy_matched_publications_arg)
{
    os_uint32 length;
    C_STRUCT(handleBuffer) first;
    handleBuffer handles;
};

static v_result
copy_matched_publication(
    u_publicationInfo *info,
    c_voidp arg)
{
    copy_matched_publications_arg a = (copy_matched_publications_arg)arg;
    os_uint32 bufpos = a->length % handleBufferSize;

    if ((bufpos == 0) && (a->length > 0)) {
        a->handles->next = os_malloc(sizeof(C_STRUCT(handleBuffer)));
        a->handles = a->handles->next;
    }
    a->handles->buf[bufpos] = u_instanceHandleFromGID(info->key);
    a->length++;

    return V_RESULT_OK;
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
    jlong uReader,
    jobject jseqHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    jlongArray jarray;
    C_STRUCT(copy_matched_publications_arg) arg;

    assert(jseqHolder);
    OS_UNUSED_ARG(jdataReader);

    arg.length = 0;
    arg.handles = &arg.first;

    uResult = u_readerGetMatchedPublications(SAJ_VOIDP(uReader), copy_matched_publication, &arg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        jarray = NEW_LONGARRAY(env, arg.length);
        if (arg.length > 0) {
            jlong *jhandles;
            os_uint32 i, bufpos;
            handleBuffer prevbuf;

            jhandles = GET_LONG_ARRAY_ELEMENTS(env, jarray, NULL);
            prevbuf = NULL;
            for (i=0; i<arg.length; i++) {
                bufpos = i % handleBufferSize;
                if ((bufpos == 0) && (i > 0)) {
                    if (prevbuf != NULL) {
                        os_free(prevbuf);
                    }
                    prevbuf = arg.handles;
                    arg.handles = arg.handles->next;
                };
                jhandles[i] = (jlong)arg.handles->buf[bufpos];
            }
            RELEASE_LONG_ARRAY_ELEMENTS(env, jarray, jhandles, 0);
        }
        SET_OBJECT_FIELD(env, jseqHolder, instanceHandleSeqHolder_value, jarray);
        DELETE_LOCAL_REF(env, jarray);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

static v_result
copyPublicationMatchedData(
    u_publicationInfo *info,
    void *arg)
{
    struct copyArg *copyArg = (struct copyArg *)arg;
    v_result rc = V_RESULT_OK;
    saj_returnCode retcode;

    retcode = saj_publicationBuiltinTopicDataCopyOut(copyArg->env, info, &copyArg->object);
    if (retcode != SAJ_RETCODE_OK) {
        rc = V_RESULT_INTERNAL_ERROR;
    }

    return rc;
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
    jlong uReader,
    jobject jdataHolder,
    jlong jhandle)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;

    assert(jdataHolder != NULL);
    OS_UNUSED_ARG(jdataReader);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_readerGetMatchedPublicationData(SAJ_VOIDP(uReader), jhandle, copyPublicationMatchedData, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jdataHolder, publicationBuiltinTopicDataHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
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
    jlong uReader,
    jobject jprop)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    jstring jname, jvalue;
    const char *name, *value;

    assert(jprop != NULL);
    OS_UNUSED_ARG(uReader);

    jname = GET_OBJECT_FIELD(env, jprop, property_name);
    if (jname != NULL) {
        name = GET_STRING_UTFCHAR(env, jname, 0);
        if (strcmp("parallelReadThreadCount", name) == 0) {
            int ival;
            jvalue = GET_OBJECT_FIELD(env, jprop, property_value);
            if (jvalue != NULL) {
                char * end;
                value = GET_STRING_UTFCHAR(env, jvalue, 0);
                ival = strtol(value, &end, 10);
                if (*end == '\0' && ival >= 0) {
                    /* The entire string was valid and contains a valid value */
                    retcode = saj_fooDataReaderSetParallelReadThreadCount(env, jdatareader, ival);
                } else {
                    retcode = SAJ_RETCODE_BAD_PARAMETER;
                }
                RELEASE_STRING_UTFCHAR(env, jvalue, value);
                DELETE_LOCAL_REF(env, jvalue);
            } else {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
            }
        } else if (strcmp("CDRCopy", name) == 0) {
            jvalue = GET_OBJECT_FIELD(env, jprop, property_value);
            if (jvalue != NULL) {
                value = GET_STRING_UTFCHAR(env, jvalue, 0);
                if(strcmp("true", value) == 0){
                    if (saj_fooDataReaderSetCDRCopy(env, jdatareader, 1) < 0) {
                        retcode = SAJ_RETCODE_ERROR;
                    }
                } else if (strcmp("false", value) == 0) {
                    if (saj_fooDataReaderSetCDRCopy(env, jdatareader, 0) < 0) {
                        retcode = SAJ_RETCODE_ERROR;
                    }
                } else {
                    retcode = SAJ_RETCODE_BAD_PARAMETER;
                }
                RELEASE_STRING_UTFCHAR(env, jvalue, value);
                DELETE_LOCAL_REF(env, jvalue);
            } else {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
            }
        } else if (strcmp("ignoreLoansOnDeletion", name) == 0) {
            jvalue = GET_OBJECT_FIELD(env, jprop, property_value);
            if (jvalue != NULL) {
                value = GET_STRING_UTFCHAR(env, jvalue, 0);
                if(strcmp("true", value) == 0){
                    /* delete datareader is currently not checking for outstanding
                     * loans. If it is doing it in the future, the value set here must
                     * be cached in the datareader and used if the datareader is deleted.
                     */
                } else if (strcmp("false", value) == 0) {
                    /* delete datareader is currently not checking for outstanding
                     * loans. If it is doing it in the future, the value set here must
                     * be cached in the datareader and used if the datareader is deleted.
                     */
                } else {
                    retcode = SAJ_RETCODE_BAD_PARAMETER;
                }
                RELEASE_STRING_UTFCHAR(env, jvalue, value);
                DELETE_LOCAL_REF(env, jvalue);
             } else {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
             }
        } else {
            /* Properties not parsed at this level may be passed on to below API's */
            retcode = SAJ_RETCODE_UNSUPPORTED;
            SAJ_REPORT(retcode, "Set Property %s not supported",name);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
        DELETE_LOCAL_REF(env, jname);
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
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
    jlong uReader,
    jobject jpropHolder)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jdatareader);
    OS_UNUSED_ARG(uReader);
    OS_UNUSED_ARG(jpropHolder);
    SAJ_REPORT(SAJ_RETCODE_UNSUPPORTED, "Get Property not supported");
    return (jint)SAJ_RETCODE_UNSUPPORTED;
}

struct copyInstanceHandle {
    JNIEnv *env;
    jobject holder;
};

static u_result
copyInstanceHandles(
    u_instanceHandle *list,
    os_uint32 length,
    c_voidp arg)
{
    u_result result = U_RESULT_OK;
    struct copyInstanceHandle *a = (struct copyInstanceHandle *)arg;
    jlongArray buffer;

    if (a != NULL) {
        buffer = NEW_LONGARRAY(a->env, length);
        if (buffer != NULL) {
            assert (sizeof (jlong) == sizeof (u_instanceHandle));
            SET_LONG_ARRAY_REGION(a->env, buffer, 0, length, (jlong *) list);
        } else {
            result = U_RESULT_OUT_OF_MEMORY;
        }
        SET_OBJECT_FIELD(a->env, a->holder, instanceHandleSeqHolder_value, buffer);
    }
    return result;
    CATCH_EXCEPTION: return U_RESULT_INTERNAL_ERROR;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadInstanceHandles) (
    JNIEnv * env,
    jobject jdatareader,
    jlong uReader,
    jobject jhandles)
{
    struct copyInstanceHandle copyArg;
    saj_returnCode retcode;
    u_result uResult;

    assert(uReader != 0);
    OS_UNUSED_ARG(jdatareader);

    copyArg.env = env;
    copyArg.holder = jhandles;
    uResult = u_dataReaderGetInstanceHandles(SAJ_VOIDP(uReader), copyInstanceHandles, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);

    return (jint)retcode;
}

#undef SAJ_FUNCTION
