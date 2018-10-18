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

#include "saj_Publisher.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_PublisherImpl_##name

/*
 * Method: jniPublisherNew
 * Param : domain participant
 * Param : publisher name
 * Param : publisherQos qos
 * Return: u_publisher
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniPublisherNew) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant,
    jstring jname,
    jobject qos)
{
    u_publisher uPublisher = NULL;
    u_publisherQos uQos;
    saj_returnCode result;
    const os_char *name;
    jobject userData;

    name = GET_STRING_UTFCHAR(env, jname, 0);
    if (name != NULL){
        uQos = u_publisherQosNew(NULL);
        if (uQos != NULL) {
            result = saj_publisherQosCopyIn(env, qos, uQos);
            if (result == SAJ_RETCODE_OK){
                uPublisher = u_publisherNew(SAJ_VOIDP(uParticipant), name, uQos, FALSE);
                if (uPublisher != NULL) {
                    userData = NEW_GLOBAL_REF(env, this);
                    u_observableSetUserData(SAJ_VOIDP(uPublisher), userData);
                }
            }

            u_publisherQosFree(uQos);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
    }

    return (jlong)(PA_ADDRCAST)uPublisher;

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniPublisherFree) (
    JNIEnv  *env,
    jobject this,
    jlong uPublisher)
{
    saj_returnCode result;
    u_result uResult;
    jobject userData;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (uPublisher) {
        userData = u_observableSetUserData(u_observable(SAJ_VOIDP(uPublisher)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }

    uResult = u_objectClose(SAJ_VOIDP(uPublisher));
    result = saj_retcode_from_user_result(uResult);

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
    jlong uPublisher,
    jobject jqos)
{
    saj_returnCode retcode = SAJ_RETCODE_ERROR;
    u_publisherQos uQos;
    u_result uResult;

    assert(jqos != NULL);
    OS_UNUSED_ARG(jpublisher);

    uQos = u_publisherQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_publisherQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK) {
            uResult = u_publisherSetQos(SAJ_VOIDP(uPublisher), uQos);
            retcode = saj_retcode_from_user_result(uResult);
        }
        u_publisherQosFree(uQos);
    }

    return retcode;
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
    jlong uPublisher,
    jobject jqosHolder)
{
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;
    u_publisherQos uQos;
    u_result uResult;
    jobject jqos = NULL;

    assert(jqosHolder != NULL);
    OS_UNUSED_ARG(jpublisher);

    uResult = u_publisherGetQos(SAJ_VOIDP(uPublisher), &uQos);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK) {
        retcode = saj_publisherQosCopyOut(env, uQos, &jqos);
        u_publisherQosFree(uQos);
        if (retcode == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, jqosHolder, publisherQosHolder_value, jqos);
            DELETE_LOCAL_REF(env, jqos);
        }
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniSuspendPublications
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSuspendPublications)(
    JNIEnv *env,
    jobject jpublisher,
    jlong uPublisher)
{
    saj_returnCode result;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jpublisher);

    uResult = u_publisherSuspend(SAJ_VOIDP(uPublisher));
    result = saj_retcode_from_user_result(uResult);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniResumePublications
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniResumePublications)(
    JNIEnv *env,
    jobject jpublisher,
    jlong uPublisher)
{
    saj_returnCode result;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jpublisher);

    uResult = u_publisherResume(SAJ_VOIDP(uPublisher));
    result = saj_retcode_from_user_result(uResult);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniBeginCoherentChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniBeginCoherentChanges)(
    JNIEnv *env,
    jobject jpublisher,
    jlong uPublisher)
{
    saj_returnCode result;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jpublisher);

    uResult = u_publisherCoherentBegin(SAJ_VOIDP(uPublisher));
    result = saj_retcode_from_user_result(uResult);

    return result;
}

/**
 * Class:     org_opensplice_dds_dcps_PublisherImpl
 * Method:    jniEndCoherentChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniEndCoherentChanges)(
    JNIEnv *env,
    jobject jpublisher,
    jlong uPublisher)
{
    saj_returnCode result;
    u_result uResult;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jpublisher);

    uResult = u_publisherCoherentEnd(SAJ_VOIDP(uPublisher));
    result = saj_retcode_from_user_result(uResult);

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
    jlong uPublisher,
    jobject jduration)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jpublisher);
    OS_UNUSED_ARG(uPublisher);
    OS_UNUSED_ARG(jduration);
    SAJ_REPORT(SAJ_RETCODE_UNSUPPORTED, "WaitForAcknowledgments not supported");
    return SAJ_RETCODE_UNSUPPORTED;
}
#undef SAJ_FUNCTION
