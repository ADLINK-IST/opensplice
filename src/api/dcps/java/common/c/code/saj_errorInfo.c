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
#include "saj_ErrorInfo.h"
#include "saj_utilities.h"

#define SAJ_FUNCTION(name) Java_DDS_ErrorInfo_##name

/**
 * Class:     DDS_ErrorInfo
 * Method:    jniErrorInfoAlloc
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniErrorInfoAlloc)(
    JNIEnv *env,
    jobject jerrorInfo)
{
    gapi_errorInfo errorInfo;
    jboolean jresult;

    jresult = JNI_FALSE;
    errorInfo = gapi_errorInfo__alloc();

    if(errorInfo != NULL){
        saj_register_weak_java_object(env, (PA_ADDRCAST)errorInfo, jerrorInfo);
        jresult = JNI_TRUE;
    }
    return jresult;
}

/**
 * Class:     DDS_ErrorInfo
 * Method:    jniErrorInfoFree
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniErrorInfoFree)(
    JNIEnv *env,
    jobject jerrorInfo)
{
    gapi_errorInfo errorInfo;
    saj_userData ud;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    ud = gapi_object_get_user_data(errorInfo);

    if(ud != NULL){
        saj_destroy_weak_user_data(env, ud);
        gapi_free(errorInfo);
    }
    return;
}

/**
 * Class:     DDS_ErrorInfo
 * Method:    jniErrorInfoUpdate
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUpdate)(
    JNIEnv *env,
    jobject jerrorInfo)
{
    gapi_errorInfo errorInfo;
    gapi_returnCode_t result;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    result = gapi_errorInfo_update(errorInfo);
    return (jint)result;
}

/**
 * Class:     DDS_ErrorInfo
 * Method:    jniErrorInfoGetErrorCode
 * Signature: (LDDS/ErrorCodeHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetCode)(
    JNIEnv *env,
    jobject jerrorInfo,
    jobject jerrorCodeHolder)
{
    gapi_errorInfo errorInfo;
    gapi_returnCode_t result;
    gapi_errorCode_t errorcode;
    jint jcode;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    result = gapi_errorInfo_get_code(errorInfo, &errorcode);

    if (result == GAPI_RETCODE_OK) {
        jcode = (jint)errorcode;
        (*env)->SetIntField(
            env,
            jerrorCodeHolder,
            GET_CACHED(errorCodeHolder_value_fid),
            jcode
        );
    }
    return (jint)result;
}

/*
 * Class:     DDS_ErrorInfo
 * Method:    jniGetLocation
 * Signature: (LDDS/StringHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetLocation)(
    JNIEnv *env,
    jobject jerrorInfo,
    jobject jstringHolder)
{
    gapi_errorInfo errorInfo;
    gapi_returnCode_t result;
    gapi_string location = NULL;
    jstring jlocation;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    result = gapi_errorInfo_get_location(errorInfo, &location);

    if ((result == GAPI_RETCODE_OK) && (location!=NULL)) {
        jlocation = (*env)->NewStringUTF(env, location);
        gapi_free(location);
        if (jlocation == NULL){
            return result;
        }
        (*env)->SetObjectField(
            env,
            jstringHolder,
            GET_CACHED(stringHolder_value_fid),
            jlocation
        );
    }
    return (jint)result;
}

/*
 * Class:     DDS_ErrorInfo
 * Method:    jniGetSourceLine
 * Signature: (LDDS/StringHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetSourceLine)(
    JNIEnv *env,
    jobject jerrorInfo,
    jobject jstringHolder)
{
    gapi_errorInfo errorInfo;
    gapi_returnCode_t result;
    gapi_string sourceline = NULL;
    jstring jsourceline;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    result = gapi_errorInfo_get_source_line(errorInfo, &sourceline);

    if ((result == GAPI_RETCODE_OK) && (sourceline!=NULL)) {
        jsourceline = (*env)->NewStringUTF(env, sourceline);
        gapi_free(sourceline);
        if (jsourceline == NULL){
            return result;
        }
        (*env)->SetObjectField(
            env,
            jstringHolder,
            GET_CACHED(stringHolder_value_fid),
            jsourceline
        );
    }
    return (jint)result;
}

/*
 * Class:     DDS_ErrorInfo
 * Method:    jniGetStackTrace
 * Signature: (LDDS/StringHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetStackTrace)(
    JNIEnv *env,
    jobject jerrorInfo,
    jobject jstringHolder)
{
    gapi_errorInfo errorInfo;
    gapi_returnCode_t result;
    gapi_string stacktrace = NULL;
    jstring jstacktrace;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    result = gapi_errorInfo_get_stack_trace(errorInfo, &stacktrace);

    if ((result == GAPI_RETCODE_OK) && (stacktrace!=NULL)) {
        jstacktrace = (*env)->NewStringUTF(env, stacktrace);
        gapi_free(stacktrace);
        if (jstacktrace == NULL){
            return result;
        }
        (*env)->SetObjectField(
            env,
            jstringHolder,
            GET_CACHED(stringHolder_value_fid),
            jstacktrace
        );
    }
    return (jint)result;
}

/*
 * Class:     DDS_ErrorInfo
 * Method:    jniGetMessage
 * Signature: (LDDS/StringHolder;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetMessage)(
    JNIEnv *env,
    jobject jerrorInfo,
    jobject jstringHolder)
{
    gapi_errorInfo errorInfo;
    gapi_returnCode_t result;
    gapi_string message = NULL;
    jstring jmessage;

    errorInfo = (gapi_errorInfo) saj_read_gapi_address(env, jerrorInfo);
    result = gapi_errorInfo_get_message(errorInfo, &message);

    if ((result == GAPI_RETCODE_OK) && (message!=NULL)) {
        jmessage = (*env)->NewStringUTF(env, message);
        gapi_free(message);
        if (jmessage == NULL){
            return result;
        }
        (*env)->SetObjectField(
            env,
            jstringHolder,
            GET_CACHED(stringHolder_value_fid),
            jmessage
        );
    }
    return (jint)result;
}


