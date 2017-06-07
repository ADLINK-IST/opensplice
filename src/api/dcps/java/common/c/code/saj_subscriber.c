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
#include "saj_Subscriber.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"

#include "saj__fooDataReader.h"
#include "u_observable.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_SubscriberImpl_##name

/*
 * Method: jniSubscriberNew
 * Param : domain participant
 * Param : subscriber name
 * Param : subscriberQos qos
 * Return: u_subscriber
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniSubscriberNew) (
    JNIEnv  *env,
    jobject this,
    jlong uParticipant,
    jstring jname,
    jobject qos)
{
    u_subscriber uSubscriber = NULL;
    u_subscriberQos uQos;
    saj_returnCode result;
    const os_char *name;
    jobject userData;

    name = GET_STRING_UTFCHAR(env, jname, 0);
    if (name != NULL){
        uQos = u_subscriberQosNew(NULL);
        if (uQos != NULL) {
            result = saj_subscriberQosCopyIn(env, qos, uQos);
            if (result == SAJ_RETCODE_OK){
                uSubscriber = u_subscriberNew(SAJ_VOIDP(uParticipant), name, uQos, FALSE);
                if (uSubscriber != NULL) {
                    userData = NEW_GLOBAL_REF(env, this);
                    u_observableSetUserData(u_observable(uSubscriber), userData);
                }
            }
            u_subscriberQosFree(uQos);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
    }

    return (jlong)(PA_ADDRCAST)uSubscriber;

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniSubscriberFree) (
    JNIEnv  *env,
    jobject this,
    jlong uSubscriber)
{
    saj_returnCode retcode;
    u_result uResult;
    jobject userData;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(this);

    if (uSubscriber) {
        userData = (jobject)u_observableSetUserData(u_observable(SAJ_VOIDP(uSubscriber)), NULL);
        DELETE_GLOBAL_REF(env, userData);
    }
    uResult = u_objectClose(SAJ_VOIDP(uSubscriber));
    retcode = saj_retcode_from_user_result(uResult);

    return retcode;
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
    jlong uSubscriber,
    jobject jqos)
{
    saj_returnCode retcode = SAJ_RETCODE_ERROR;
    u_subscriberQos uQos;

    assert(jqos != NULL);
    OS_UNUSED_ARG(jsubscriber);

    uQos = u_subscriberQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_subscriberQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK) {
            retcode = saj_retcode_from_user_result(u_subscriberSetQos(SAJ_VOIDP(uSubscriber), uQos));
        }
        u_subscriberQosFree(uQos);
    }

    return retcode;
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
    jlong uSubscriber,
    jobject jqosHolder)
{
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;
    u_subscriberQos uQos;
    jobject jqos = NULL;

    assert(jqosHolder != NULL);
    OS_UNUSED_ARG(jsubscriber);

    uResult = u_subscriberGetQos(SAJ_VOIDP(uSubscriber), &uQos);
    retcode = saj_retcode_from_user_result(uResult);
    if (retcode == SAJ_RETCODE_OK) {
        retcode = saj_subscriberQosCopyOut(env, uQos, &jqos);
        u_subscriberQosFree(uQos);
        if (retcode == SAJ_RETCODE_OK) {
            SET_OBJECT_FIELD(env, jqosHolder, subscriberQosHolder_value, jqos);
            DELETE_LOCAL_REF(env, jqos);
        }
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniBeginAccess
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniBeginAccess)(
    JNIEnv *env,
    jobject jsubscriber,
    jlong uSubscriber)
{
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jsubscriber);

    uResult = u_subscriberBeginAccess(SAJ_VOIDP(uSubscriber));
    retcode = saj_retcode_from_user_result(uResult);

    return (jint)retcode;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniEndAccess
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniEndAccess)(
    JNIEnv *env,
    jobject jsubscriber,
    jlong uSubscriber)
{
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jsubscriber);

    uResult = u_subscriberEndAccess(SAJ_VOIDP(uSubscriber));
    retcode = saj_retcode_from_user_result(uResult);

    return (jint)retcode;
}

/**
 * Class:     org_opensplice_dds_dcps_SubscriberImpl
 * Method:    jniGetDataReaders
 * Signature: (LDDS/DataReaderSeqHolder;III)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDataReaders)(
    JNIEnv *env,
    jobject jsubscriber,
    jlong uSubscriber,
    jobject readers,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    saj_returnCode retcode;
    u_result uResult;
    u_observable elem;
    u_sampleMask mask;
    c_iter ureaders = NULL;
    c_ulong iterLen;
    jobject readerSeq;
    jclass drClass;
    unsigned int i;

    OS_UNUSED_ARG(jsubscriber);

    if (readers == NULL) {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(retcode, "readers '<NULL>' is invalid.");
    } else {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x), view mask(0x%x) or instance mask(0x%x)",
                sample_states, view_states, instance_states);
        }
    }

    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_subscriberGetDataReaders(SAJ_VOIDP(uSubscriber), mask, &ureaders);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK) {
            iterLen = c_iterLength(ureaders);
            drClass = FIND_CLASS(env, "DDS/DataReader");
            assert(drClass);
            readerSeq = NEW_OBJECTARRAY(env, iterLen, drClass, NULL);
            for (i = 0; i < iterLen; i++) {
                elem = u_observable(c_iterTakeFirst(ureaders));
                assert(elem);
                SET_OBJECTARRAY_ELEMENT(env, readerSeq, i, u_observableGetUserData(elem));
            }
            SET_OBJECT_FIELD(env, readers, dataReaderSeqHolder_value, readerSeq);
        }
        c_iterFree(ureaders);
    }
    return (jint)retcode;

CATCH_EXCEPTION:
    c_iterFree(ureaders);
    return SAJ_RETCODE_ERROR;
}

#undef SAJ_FUNCTION
