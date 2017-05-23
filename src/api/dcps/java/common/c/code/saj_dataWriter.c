/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "saj_DataWriter.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "u_object.h"
#include "u_observable.h"
#include "u_publisher.h"
#include "u_topic.h"
#include "u_writer.h"
#include "u_writerQos.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DataWriterImpl_##name

/*
 * Method: jniDataWriterNew
 * Param : domain participant
 * Param : writer name
 * Param : writerQos qos
 * Return: u_writer
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniDataWriterNew) (
    JNIEnv  *env,
    jobject this,
    jlong uPublisher,
    jstring jname,
    jlong uTopic,
    jobject qos)
{
    u_writer uWriter = NULL;
    u_writerQos uQos;
    saj_returnCode result;
    const os_char *name;
    jobject userData;

    name = GET_STRING_UTFCHAR(env, jname, 0);
    if (name != NULL){
        uQos = u_writerQosNew(NULL);
        if (uQos != NULL) {
            result = saj_dataWriterQosCopyIn(env, qos, uQos);
            if (result == SAJ_RETCODE_OK){
                uWriter = u_writerNew(SAJ_VOIDP(uPublisher), name,
                                      SAJ_VOIDP(uTopic), uQos);
                if (uWriter != NULL) {
                    userData = NEW_GLOBAL_REF(env, this);
                    u_observableSetUserData(SAJ_VOIDP(uWriter), userData);
                }

            }

            u_writerQosFree(uQos);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
    } else {
        result = SAJ_RETCODE_ERROR;
    }

    return SAJ_JLONG(uWriter);

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDataWriterFree) (
    JNIEnv  *env,
    jobject this,
    jlong uWriter)
{
    u_result uResult;
    jobject userData;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (uWriter) {
        userData = u_observableSetUserData(u_observable(SAJ_VOIDP(uWriter)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }
    uResult = u_objectClose(SAJ_VOIDP(uWriter));
    return saj_retcode_from_user_result(uResult);
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniSetQos
 * Signature: (LDDS/DataWriterQos;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSetQos)(
    JNIEnv *env,
    jobject jwriter,
    jlong uWriter,
    jobject jqos)
{
    saj_returnCode retcode = SAJ_RETCODE_ERROR;
    u_writerQos uQos;

    OS_UNUSED_ARG(jwriter);

    assert(jqos != NULL);
    uQos = u_writerQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_dataWriterQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK) {
            retcode = saj_retcode_from_user_result(u_writerSetQos(SAJ_VOIDP(uWriter), uQos));
        }
        u_writerQosFree(uQos);
    }

    return retcode;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetQos
 * Signature: (LDDS/DataWriterQosHolder;)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetQos)(
    JNIEnv *env,
    jobject jwriter,
    jlong uWriter,
    jobject jqosHolder)
{
    saj_returnCode retcode;
    u_writerQos uQos;
    jobject jqos = NULL;

    assert(jqosHolder != NULL);
    OS_UNUSED_ARG(jwriter);

    retcode = saj_retcode_from_user_result(u_writerGetQos(SAJ_VOIDP(uWriter), &uQos));
    if (retcode == SAJ_RETCODE_OK) {
        retcode = saj_dataWriterQosCopyOut(env, uQos, &jqos);
        u_writerQosFree(uQos);
        if (retcode == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, jqosHolder, dataWriterQosHolder_value, jqos);
            DELETE_LOCAL_REF(env, jqos);
        }
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

struct copyArg {
    JNIEnv *env;
    jobject object;
};

static v_result
copy_liveliness_lost_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_livelinessLostInfo *from = (struct v_livelinessLostInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = saj_livelinessLostStatus_new(copyArg->env, from);
    if (copyArg->object) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uWriter,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_liveliness_lost_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataWriter);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_writerGetLivelinessLostStatus(SAJ_VOIDP(uWriter), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, livelinessLostStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

static v_result
copy_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_deadlineMissedInfo *from = (struct v_deadlineMissedInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;
    v_result result;

    copyArg->object = saj_offeredDeadlineMissedStatus_new(copyArg->env, from);
    if (copyArg->object) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uWriter,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_deadline_missed_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataWriter);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_writerGetDeadlineMissedStatus(SAJ_VOIDP(uWriter), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, offeredDeadlineMissedStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

static v_result
copy_IncompatibleQosStatus(
    c_voidp info,
    c_voidp arg)
{
    struct v_incompatibleQosInfo *from;
    struct copyArg *copyArg = (struct copyArg *)arg;
    v_result result = V_RESULT_OK;

    from = (struct v_incompatibleQosInfo *)info;
    copyArg->object = saj_offeredIncompatibleQosStatus_new(copyArg->env, from);
    if (copyArg->object) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uWriter,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_IncompatibleQosStatus;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataWriter);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_writerGetIncompatibleQosStatus(SAJ_VOIDP(uWriter), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, offeredIncompatibleQosStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

static v_result
copy_publication_matched_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result;
    struct v_topicMatchInfo *from = (struct v_topicMatchInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = saj_publicationMatchStatus_new(copyArg->env, from);
    if (copyArg->object) {
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
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
    jlong uWriter,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;
    u_statusAction copyAction = copy_publication_matched_status;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jdataWriter);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_writerGetPublicationMatchStatus(SAJ_VOIDP(uWriter), TRUE, copyAction, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jstatusHolder, publicationMatchedStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/**
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniAssertLiveliness
 * Signature: (I)V
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniAssertLiveliness)(
    JNIEnv *env,
    jobject jdataWriter,
    jlong uWriter)
{
    u_result uResult;
    saj_returnCode retcode;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jdataWriter);

    uResult = u_writerAssertLiveliness(SAJ_VOIDP(uWriter));
    retcode = saj_retcode_from_user_result(uResult);

    return (jint)retcode;
}

#define handleBufferSize (32)

C_CLASS(handleBuffer);
C_STRUCT(handleBuffer)
{
    u_instanceHandle buf[handleBufferSize];
    handleBuffer next;
};

C_CLASS(copy_matched_subscriptions_arg);
C_STRUCT(copy_matched_subscriptions_arg)
{
    os_uint32 length;
    C_STRUCT(handleBuffer) first;
    handleBuffer handles;
};

static v_result
copy_matched_subscriptions(
    u_subscriptionInfo *info,
    void *arg)
{
    copy_matched_subscriptions_arg a = (copy_matched_subscriptions_arg)arg;
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
 * Class:     org_opensplice_dds_dcps_DataWriterImpl
 * Method:    jniGetMatchedSubscriptions
 * Signature: (LDDS/InstanceHandleSeqHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetMatchedSubscriptions)(
    JNIEnv *env,
    jobject jdataWriter,
    jlong uWriter,
    jobject jseqHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    jlongArray jarray;
    C_STRUCT(copy_matched_subscriptions_arg) arg;

    assert(jseqHolder);
    OS_UNUSED_ARG(jdataWriter);

    arg.length = 0;
    arg.handles = &arg.first;
    uResult = u_writerGetMatchedSubscriptions(SAJ_VOIDP(uWriter),
                                              copy_matched_subscriptions, &arg);
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
copySubscriptionMatchedData(
    u_subscriptionInfo *info,
    void *arg)
{
    struct copyArg *copyArg = (struct copyArg *)arg;
    v_result rc;

    rc = saj_subscriptionBuiltinTopicDataCopyOut(copyArg->env, info, &copyArg->object);

    return rc;
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
    jlong uWriter,
    jobject jdataHolder,
    jlong jhandle)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;

    assert(jdataHolder);
    OS_UNUSED_ARG(jdataWriter);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_writerGetMatchedSubscriptionData(SAJ_VOIDP(uWriter), jhandle,
                                                 copySubscriptionMatchedData,
                                                 &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK){
        SET_OBJECT_FIELD(env, jdataHolder, subscriptionBuiltinTopicDataHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
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
    jlong uWriter,
    jobject jduration)
{
    os_duration duration;
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;

    OS_UNUSED_ARG(jwriter);

    if (jduration != NULL){
        retcode = saj_durationCopyIn(env, jduration, &duration);
        if (retcode == SAJ_RETCODE_OK){
            uResult = u_writerWaitForAcknowledgments(SAJ_VOIDP(uWriter), duration);
            retcode = saj_retcode_from_user_result(uResult);
        }
    }

    return (jint)retcode;
}

#undef SAJ_FUNCTION
