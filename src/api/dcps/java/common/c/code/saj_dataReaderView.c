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
#include "saj_DataReaderView.h"
#include "saj_utilities.h"
#include "saj_qosUtils.h"
#include "u_dataView.h"
#include "u_dataViewQos.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_DataReaderViewImpl_##name

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniDataReaderViewNew)(
    JNIEnv *env,
    jobject jdataReader,
    jlong uReader,
    jstring jname,
    jobject qos)
{
    u_dataView uView = NULL;
    u_dataViewQos uQos;
    saj_returnCode retcode = SAJ_RETCODE_OK;
    const os_char *name;

    OS_UNUSED_ARG(jdataReader);

    name = GET_STRING_UTFCHAR(env, jname, 0);
    if (name != NULL){
        uQos = u_dataViewQosNew(NULL);
        if (uQos != NULL) {
            retcode = saj_dataReaderViewQosCopyIn(env, qos, uQos);
            if (retcode == SAJ_RETCODE_OK) {
                uView = u_dataViewNew(SAJ_VOIDP(uReader), name, uQos);
            }

            u_dataViewQosFree(uQos);
        }
        RELEASE_STRING_UTFCHAR(env, jname, name);
    } else {
        retcode = SAJ_RETCODE_ERROR;
    }

    return SAJ_JLONG(uView);

    CATCH_EXCEPTION:
    return 0;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDataReaderViewFree)(
    JNIEnv *env,
    jobject jdataReader,
    jlong uView)
{
    u_result uResult;
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jdataReader);
    uResult = u_objectClose(SAJ_VOIDP(uView));
    return saj_retcode_from_user_result(uResult);
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
    jlong uView,
    jobject jqos)
{
    u_dataViewQos uQos;
    u_result uResult;
    saj_returnCode retcode;

    OS_UNUSED_ARG(jdataReaderView);

    uQos = u_dataViewQosNew(NULL);
    if (uQos != NULL) {
        retcode = saj_dataReaderViewQosCopyIn(env, jqos, uQos);
        if (retcode == SAJ_RETCODE_OK) {
            uResult = u_dataViewSetQos(SAJ_VOIDP(uView), uQos);
            retcode = saj_retcode_from_user_result(uResult);
        }
        u_dataViewQosFree(uQos);
    } else {
        retcode = SAJ_RETCODE_ERROR;
    }

    return (jint)retcode;
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
    jlong uView,
    jobject jqosHolder)
{
    u_dataViewQos uQos;
    u_result uResult;
    saj_returnCode retcode;
    jobject jqos = NULL;

    OS_UNUSED_ARG(jdataReaderView);

    if (jqosHolder != NULL) {
        uResult = u_dataViewGetQos(SAJ_VOIDP(uView), &uQos);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK) {
            retcode = saj_dataReaderViewQosCopyOut(env, uQos, &jqos);
            u_dataViewQosFree(uQos);
            if (retcode == SAJ_RETCODE_OK) {
                SET_OBJECT_FIELD(env, jqosHolder, dataReaderViewQosHolder_value, jqos);
                DELETE_LOCAL_REF(env, jqos);
            }
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

#undef SAJ_FUNCTION
