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
#include "saj_Topic.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "u_observable.h"
#include "v_kernelParser.h"

#include "saj__report.h"

struct copyArg {
    JNIEnv *env;
    jobject object;
};

static v_result
copy_inconsistent_topic_status (
    c_voidp info,
    c_voidp arg)
{
    v_result result = V_RESULT_OK;
    struct v_inconsistentTopicInfo *from = (struct v_inconsistentTopicInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;
    copyArg->object = saj_inconsistentTopicStatus_new(copyArg->env, from);
    if (copyArg->object == NULL) {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
}

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_TopicImpl_##name

/*
 * Method: jniTopicNew
 * Param : domain participant
 * Param : topic name
 * Param : topicQos qos
 * Return: u_topic
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniTopicNew) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant,
    jstring jtopicName,
    jstring jtypeName,
    jstring jkeyList,
    jobject qos)
{
    u_topic uTopic = NULL;
    u_topicQos uQos;
    saj_returnCode result;
    const os_char *topicName;
    const os_char *typeName;
    const os_char *keyList;
    jobject userData;

    assert(jtopicName != NULL);
    assert(jtypeName != NULL);
    assert(jkeyList != NULL);
    assert(qos != NULL);

    topicName = GET_STRING_UTFCHAR(env, jtopicName, 0);
    if (topicName != NULL){
        typeName = GET_STRING_UTFCHAR(env, jtypeName, 0);
        if (typeName != NULL){
            keyList = GET_STRING_UTFCHAR(env, jkeyList, 0);
            if (keyList != NULL){
                uQos = u_topicQosNew(NULL);
                if (uQos != NULL) {
                    result = saj_topicQosCopyIn(env, qos, uQos);
                    if (result == SAJ_RETCODE_OK){
                        uTopic = u_topicNew(SAJ_VOIDP(uParticipant), topicName, typeName, keyList, uQos);
                        if (uTopic != NULL) {
                            userData = NEW_GLOBAL_REF(env, this);
                            u_observableSetUserData(SAJ_VOIDP(uTopic), userData);
                        }
                    }
                    u_topicQosFree(uQos);
                }
                RELEASE_STRING_UTFCHAR(env, jkeyList, keyList);
            }
            RELEASE_STRING_UTFCHAR(env, jtypeName, typeName);
        }
        RELEASE_STRING_UTFCHAR(env, jtopicName, topicName);
    }

    return (jlong)(PA_ADDRCAST)uTopic;

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTopicFree) (
    JNIEnv  *env,
    jobject this,
    jlong uTopic)
{
    saj_returnCode result;
    u_result uResult;
    jobject userData;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (uTopic) {
        userData = u_observableSetUserData(u_observable(SAJ_VOIDP(uTopic)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }
    uResult = u_objectClose(SAJ_VOIDP(uTopic));
    result = saj_retcode_from_user_result(uResult);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetInconsistentTopicStatus
 * Signature: ()LDDS/InconsistentTopicStatus;
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetInconsistentTopicStatus)(
    JNIEnv *env,
    jobject jtopic,
    jlong uTopic,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jtopic);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_topicGetInconsistentTopicStatus(SAJ_VOIDP(uTopic), TRUE, copy_inconsistent_topic_status, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK) {
        SET_FIELD(env, Object, jstatusHolder, inconsistentTopicStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

static v_result
copy_all_data_disposed_topic_status (
    c_voidp info,
    c_voidp arg)
{
    struct v_allDataDisposedInfo *from = (struct v_allDataDisposedInfo *)info;
    struct copyArg *copyArg = (struct copyArg *)arg;

    copyArg->object = NEW_OBJECT(copyArg->env, GET_CACHED(allDataDisposedTopicStatus_class),
                                 GET_CACHED(allDataDisposedTopicStatus_constructor_mid),
                                 (jint)from->totalCount, (jint)from->totalChanged);

    return V_RESULT_OK;
    CATCH_EXCEPTION: return U_RESULT_INTERNAL_ERROR;
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
    jlong uTopic,
    jobject jstatusHolder)
{
    u_result uResult;
    saj_returnCode retcode;
    struct copyArg copyArg;

    assert(jstatusHolder);
    OS_UNUSED_ARG(jtopic);

    copyArg.env = env;
    copyArg.object = NULL;

    uResult = u_topicGetAllDataDisposedStatus(SAJ_VOIDP(uTopic), TRUE, copy_all_data_disposed_topic_status, &copyArg);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK) {
        SET_FIELD(env, Object, jstatusHolder, allDataDisposedTopicStatusHolder_value, copyArg.object);
        DELETE_LOCAL_REF(env, copyArg.object);
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
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
    jlong uTopic,
    jobject jqosHolder)
{
    saj_returnCode retcode;
    u_topicQos uQos;
    jobject jqos = NULL;

    assert(jqosHolder != NULL);
    OS_UNUSED_ARG(jtopic);

    retcode = saj_retcode_from_user_result(u_topicGetQos(SAJ_VOIDP(uTopic), &uQos));
    if (retcode == SAJ_RETCODE_OK) {
        retcode = saj_topicQosCopyOut(env, uQos, &jqos);
        u_topicQosFree(uQos);
        if (retcode == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, jqosHolder, topicQosHolder_value, jqos);
            DELETE_LOCAL_REF(env, jqos);
        }
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
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
    jlong uTopic,
    jobject jqos)
{
    saj_returnCode retcode = SAJ_RETCODE_ERROR;
    u_topicQos uQos;
    u_result uResult;

    assert(jqos != NULL);
    OS_UNUSED_ARG(jtopic);

    uQos = u_topicQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_topicQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK) {
            uResult = u_topicSetQos(SAJ_VOIDP(uTopic), uQos);
            retcode = saj_retcode_from_user_result(uResult);
        }
        u_topicQosFree(uQos);
    }

    return retcode;
}

/**
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetName)(
    JNIEnv *env,
    jobject jtopic,
    jlong uTopic)
{
    jstring jname;
    os_char *name;

    OS_UNUSED_ARG(jtopic);
    jname = NULL;

    name = u_topicName(SAJ_VOIDP(uTopic));
    if (name != NULL) {
        jname = NEW_STRING_UTF(env, name);
        os_free(name);
    }

    return jname;

    CATCH_EXCEPTION:
    os_free(name);
    return 0;
}

JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetTypeName)(
    JNIEnv *env,
    jobject jtopic,
    jlong uTopic)
{
    jstring jname;
    os_char *name;

    OS_UNUSED_ARG(jtopic);
    jname = NULL;

    name = u_topicTypeName(SAJ_VOIDP(uTopic));
    if (name != NULL) {
        jname = NEW_STRING_UTF(env, name);
        os_free(name);
    }

    return jname;

    CATCH_EXCEPTION:
    os_free(name);
    return 0;
}

/*
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniDisposeAllData
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDisposeAllData)(
    JNIEnv *env,
    jobject jtopic,
    jlong uTopic)
{
    u_result uResult;
    saj_returnCode retcode;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jtopic);

    uResult = u_topicDisposeAllData(SAJ_VOIDP(uTopic));
    retcode = saj_retcode_from_user_result(uResult);

    return (jint)retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniValidateFilter
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniValidateFilter)(
    JNIEnv *env,
    jobject jtopic,
    jlong uTopic,
    jstring jexpression,
    jobjectArray jparameters)
{
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;
    jobject *jParam = NULL;
    const os_char *predicate = NULL;
    const char **strings = NULL; /* Const because it will hold const char *elements. */
    c_value *params = NULL;
    q_expr expr;
    os_uint32 nrOfParams = 0;
    os_uint32 i;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jtopic);

    if ((uTopic != 0) && (jexpression != NULL)) {
        if (jparameters) {
            nrOfParams = GET_ARRAY_LENGTH(env, jparameters);
        }
        if (nrOfParams < 100) {
            predicate = GET_STRING_UTFCHAR(env, jexpression, 0);
            if (predicate != NULL) {
                expr = v_parser_parse(predicate);
                if (expr) {
                    if(nrOfParams > 0) {
                        jParam = os_malloc(nrOfParams * sizeof(jobject));
                        strings = os_malloc(nrOfParams * sizeof(char *));
                        params = os_malloc(nrOfParams * sizeof(struct c_value));
                        for (i =0; i<nrOfParams; i++) {
                            jParam[i] = GET_OBJECTARRAY_ELEMENT(env, jparameters, i);
                            strings[i] = GET_STRING_UTFCHAR(env, jParam[i], 0);
                            params[i] = c_stringValue((const c_string) strings[i]);
                        }
                    }
                    if (u_topicContentFilterValidate2(SAJ_VOIDP(uTopic), expr, params, nrOfParams)) {
                        retcode = SAJ_RETCODE_OK;
                    } else {
                        retcode = SAJ_RETCODE_BAD_PARAMETER;
                        SAJ_REPORT(retcode,
                            "filter_expression '%s' is invalid.", predicate);
                    }
                    q_dispose(expr);
                    for (i =0; i<nrOfParams; i++) {
                        RELEASE_STRING_UTFCHAR(env, jParam[i], strings[i]);
                        DELETE_LOCAL_REF(env, jParam[i]);
                    }
                    if (params != NULL) {
                        os_free((void *) params);
                    }
                    if (strings != NULL) {
                        os_free((void *) strings);
                    }
                    if (jParam != NULL) {
                        os_free(jParam);
                    }
                } else {
                    retcode = SAJ_RETCODE_BAD_PARAMETER;
                    SAJ_REPORT(retcode,
                        "filter_expression '%s' is invalid.", predicate);
                }
                RELEASE_STRING_UTFCHAR(env, jexpression, predicate);
            } else {
                retcode = SAJ_RETCODE_BAD_PARAMETER;
                SAJ_REPORT(retcode,"filter_expression is invalid.");
            }
        } else {
            retcode = SAJ_RETCODE_BAD_PARAMETER;
            SAJ_REPORT(retcode,
                "Invalid number of filter_parameters '%u', maximum is 99.",
                nrOfParams);
        }
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/*
 * Class:     org_opensplice_dds_dcps_TopicImpl
 * Method:    jniGetKeyExpr
 * Signature: ()I
 */
JNIEXPORT jstring JNICALL
SAJ_FUNCTION(jniGetKeyExpr)(
    JNIEnv *env,
    jobject jtopic,
    jlong uTopic)
{
    jstring jkeys;
    os_char *keys;

    OS_UNUSED_ARG(jtopic);
    jkeys = NULL;

    keys = u_topicKeyExpr(SAJ_VOIDP(uTopic));
    if (keys != NULL) {
        jkeys = NEW_STRING_UTF(env, keys);
        os_free(keys);
    }
    return jkeys;
    CATCH_EXCEPTION:
    os_free(keys);
    return 0;
}


#undef SAJ_FUNCTION
