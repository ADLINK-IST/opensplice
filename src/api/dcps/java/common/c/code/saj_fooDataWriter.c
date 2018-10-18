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

#include "saj_FooDataWriter.h"
#include "saj_copyIn.h"
#include "saj_copyOut.h"
#include "saj_copyCache.h"
#include "saj_utilities.h"
#include "u_writer.h"
#include "saj__report.h"

/* Defines the package of the java implementation classes */
#define SAJ_PACKAGENAME "org/opensplice/dds/dcps/"
#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_FooDataWriterImpl_##name

static v_copyin_result
copyAction(
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;
    saj_srcInfo srcInfo = (saj_srcInfo)data;

    if (srcInfo->javaObject != NULL) {
        os_int32 copyResult = saj_copyInStruct(c_getBase(type), data, to);
        switch (copyResult) {
        case OS_RETCODE_OK:
            result = V_COPYIN_RESULT_OK;
            break;
        case OS_RETCODE_BAD_PARAMETER:
            result = V_COPYIN_RESULT_INVALID;
            break;
        case OS_RETCODE_OUT_OF_RESOURCES:
            result = V_COPYIN_RESULT_OUT_OF_MEMORY;
            break;
        default:
            break;
        }
    }
    /* Compiler expects a return when building release. */
    return result;
}

static u_bool
copyKeyAction(
    void *data,
    void *to)
{
    saj_copyOutStruct(data, to);
    return TRUE;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniRegisterInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/Time_t;)I
 */
/*
    private native static long jniRegisterInstance (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniRegisterInstance) (
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject instance_data,
    jobject source_timestamp)
{
    int result = SAJ_RETCODE_OK;
    os_timeW timestamp;
    u_result uResult;
    u_instanceHandle uHandle = 0;
    C_STRUCT(saj_srcInfo) srcInfo;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        if (saj_timeCopyIn (env, source_timestamp, &timestamp) == SAJ_RETCODE_OK) {
            uResult = u_writerRegisterInstance(SAJ_VOIDP(uWriter), copyAction, &srcInfo,
                                               timestamp, &uHandle);
            result = saj_retcode_from_user_result(uResult);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(result, "instance_data 'null' is invalid.");
    }

    return (jlong)uHandle;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniUnregisterInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniUnregisterInstance (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUnregisterInstance) (
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle,
    jobject source_timestamp)
{
    saj_returnCode retcode;
    u_result uResult;
    os_timeW timestamp;
    C_STRUCT(saj_srcInfo) srcInfo;

    OS_UNUSED_ARG(object);

    retcode = saj_timeCopyIn (env, source_timestamp, &timestamp);
    if (retcode == SAJ_RETCODE_OK) {
        if (instance_data != NULL) {
            srcInfo.javaEnv = env;
            srcInfo.javaObject = instance_data;
            srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
            uResult = u_writerUnregisterInstance(SAJ_VOIDP(uWriter), copyAction, &srcInfo,
                                                 timestamp, handle);
        } else {
            uResult = u_writerUnregisterInstance(SAJ_VOIDP(uWriter), copyAction, NULL,
                                                 timestamp, handle);
        }
        retcode = saj_retcode_from_user_result(uResult);
    }

    return (jint)retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniWrite
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniWrite (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWrite) (
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle,
    jobject source_timestamp)
{
    u_result uResult;
    saj_returnCode retcode = SAJ_RETCODE_OK;
    os_timeW timestamp;
    C_STRUCT(saj_srcInfo) srcInfo;

    assert (copyCache != 0);
    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        retcode = saj_timeCopyIn (env, source_timestamp, &timestamp);
        if (retcode == SAJ_RETCODE_OK) {
            uResult = u_writerWrite(SAJ_VOIDP(uWriter), copyAction, &srcInfo,
                                    timestamp, (u_instanceHandle)handle);
            retcode = saj_retcode_from_user_result(uResult);
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(retcode, "instance_data 'null' is invalid.");
    }

    return (jint)retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniDispose
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniDispose (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long instance_handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDispose) (
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject instance_data,
    jlong instance_handle,
    jobject source_timestamp)
{
    saj_returnCode retcode = SAJ_RETCODE_OK;
    u_result uResult;
    os_timeW timestamp;
    C_STRUCT(saj_srcInfo) srcInfo;

    assert (copyCache != 0);
    OS_UNUSED_ARG(object);

    retcode = saj_timeCopyIn (env, source_timestamp, &timestamp);

    if (retcode == SAJ_RETCODE_OK) {
        if(instance_data != NULL){
            srcInfo.javaEnv = env;
            srcInfo.javaObject = instance_data;
            srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
            uResult = u_writerDispose(SAJ_VOIDP(uWriter), copyAction, &srcInfo,
                              timestamp, (u_instanceHandle)instance_handle);
        } else {
            uResult = u_writerDispose(SAJ_VOIDP(uWriter), copyAction, NULL,
                              timestamp, (u_instanceHandle)instance_handle);
        }
        retcode = saj_retcode_from_user_result(uResult);
    }
    return (jint)retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniWritedispose
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniWritedispose (
        Object DataWriter,
        long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWritedispose) (
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle,
    jobject source_timestamp)
{
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;
    u_result uResult;
    os_timeW timestamp;
    C_STRUCT(saj_srcInfo) srcInfo;

    assert (copyCache != 0);
    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        retcode = saj_timeCopyIn (env, source_timestamp, &timestamp);
        if (retcode == SAJ_RETCODE_OK) {
            uResult = u_writerWriteDispose(SAJ_VOIDP(uWriter), copyAction, &srcInfo,
                                           timestamp, (u_instanceHandle)handle);
            retcode = saj_retcode_from_user_result(uResult);
        }
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(retcode, "instance_data 'null' is invalid.");
    }

    return (jint)retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniGetKeyValue
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniGetKeyValue (
        Object DataWriter,
	long copyCache,
        Object key_holder,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetKeyValue) (
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject key_holder,
    jlong handle)
{
    saj_returnCode retcode = SAJ_RETCODE_BAD_PARAMETER;
    C_STRUCT(saj_dstInfo) dstInfo;
    void *dst = NULL;
    u_result uResult;
    jobject element;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache ((saj_copyCache)(PA_ADDRCAST)copyCache);

    OS_UNUSED_ARG(object);
    assert(key_holder != NULL);

    element = (*env)->GetObjectField(env, key_holder, rc->dataHolder_value_fid);
    CHECK_EXCEPTION(env);
    dstInfo.javaEnv = env;
    dstInfo.javaObject = element;
    dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
    dst = (void *)&dstInfo;

    uResult = u_writerCopyKeysFromInstanceHandle(SAJ_VOIDP(uWriter),
                                                (u_instanceHandle)handle,
                                                (u_writerCopyKeyAction)copyKeyAction, dst);
    retcode = saj_retcode_from_user_result(uResult);
    if (dstInfo.javaObject != element) {
        (*env)->SetObjectField(env, key_holder, rc->dataHolder_value_fid, dstInfo.javaObject);
        CHECK_EXCEPTION(env);
    }

    return retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniLookupInstance
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniLookupInstance)(
    JNIEnv *env,
    jclass object,
    jlong uWriter,
    jlong copyCache,
    jobject instanceData)
{
    u_instanceHandle uHandle = U_INSTANCEHANDLE_NIL;
    C_STRUCT(saj_srcInfo) srcInfo;

    OS_UNUSED_ARG(object);

    if (instanceData != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instanceData;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        (void)u_writerLookupInstance(SAJ_VOIDP(uWriter), copyAction, &srcInfo, &uHandle);
    }

    return (jlong)uHandle;
}
