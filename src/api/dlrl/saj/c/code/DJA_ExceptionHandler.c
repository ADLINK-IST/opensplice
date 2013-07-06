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
#include "DLRL_Report.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"
#include "DLRL_Types.h"
#include <string.h>
#include <assert.h>

/* DLRL includes */
#include "DLRL_Types.h"

static void
DJA_ExceptionHandler_ThrowByName(
    JNIEnv *env,
    const char *name,
    const char *msg);

void
DJA_ExceptionHandler_handleException(
    JNIEnv * env,
    DLRL_Exception* exception)
{
    LOC_long exceptionID;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);

    exceptionID = exception->exceptionID;
    switch (exceptionID){
        case DLRL_NO_EXCEPTION:
            /* do nothing */
            break;
        case DLRL_DCPS_ERROR:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/DCPSError", exception->exceptionMessage);
            break;
        case DLRL_BAD_HOME_DEFINITION:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/BadHomeDefinition", exception->exceptionMessage);
            break;
        case DLRL_BAD_PARAMETER:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/BadParameter", exception->exceptionMessage);
            break;
        case DLRL_SQL_ERROR:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/SQLError", exception->exceptionMessage);
            break;
        case DLRL_NOT_FOUND:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/NotFound", exception->exceptionMessage);
            break;
        case DLRL_ALREADY_EXISTING:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/AlreadyExisting", exception->exceptionMessage);
            break;
        case DLRL_INVALID_OBJECTS:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/InvalidObjects", exception->exceptionMessage);
            break;
        case DLRL_PRECONDITION_NOT_MET:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/PreconditionNotMet", exception->exceptionMessage);
            break;
        case DLRL_OUT_OF_MEMORY:
            DJA_ExceptionHandler_ThrowByName(env, "java/lang/OutOfMemoryError", exception->exceptionMessage);
            break;
         case DLRL_ALREADY_DELETED:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/AlreadyDeleted", exception->exceptionMessage);
            break;
         case DLRL_NO_SUCH_ELEMENT:
            DJA_ExceptionHandler_ThrowByName(env, "DDS/NoSuchElement", exception->exceptionMessage);
            break;
         case DLRL_ERROR:
             /* throw a run time exception. */
            DJA_ExceptionHandler_ThrowByName(env, "java/lang/IllegalStateException", exception->exceptionMessage);
            break;
        default:
            assert(FALSE);/* unrecognized exception, not allowed. */
            break;


    }
    DLRL_INFO(INF_EXIT);
}


void
DJA_ExceptionHandler_ThrowByName(
    JNIEnv *env,
    const char *name,
    const char *msg)
{
    jclass cls;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(name);
    assert(msg);
    cls = (*env)->FindClass(env, name);
    /* if cls is NULL, an exception has already been thrown */
    if (cls != NULL) {
        (*env)->ThrowNew(env, cls, msg);
    }
    /* free the local ref */
    (*env)->DeleteLocalRef(env, cls);
    DLRL_INFO(INF_EXIT);
}


void
DJA_ExceptionHandler_forwardJavaExceptionToKernel(
    JNIEnv *env,
    DLRL_Exception* exception)
{

    /* Cached JNI data used within this call */
    jmethodID getClassMid = cachedJNI.throw_getClass_mid;
    jmethodID getMessageMid = cachedJNI.throw_getMessage_mid;
    jmethodID getNameMid = cachedJNI.class_getName_mid;
    jobject jclass = NULL;
    jstring jname = NULL;
    jstring jmessage = NULL;
    jthrowable jexception = NULL;
    LOC_string message= NULL;
    LOC_string name= NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);

    jexception = (*env)->ExceptionOccurred(env);

    if (jexception){
        /*  clear the exception */
        (*env)->ExceptionClear(env);
        /* ignore exception */
        jclass = (*env)->CallObjectMethod(env, jexception, getClassMid);

        DLRL_INFO(INF_CALLBACK, "Class->getName()");
        /* ignore exception */
        jname = jclass ? (*env)->CallObjectMethod(env, jclass, getNameMid) : NULL;
        /* ignore exception */

        name = jname ? (LOC_string)(*env)->GetStringUTFChars(env, jname, 0) : "no name";

        DLRL_INFO(INF_CALLBACK, "Exception->getMessage()");

        /* ignore exception */
        jmessage = name ? (*env)->CallObjectMethod(env, jexception, getMessageMid) : NULL;
        /* ignore exception */
        message = jmessage ? (LOC_string)(*env)->GetStringUTFChars(env, jmessage, 0) : "no message";

        if(DJA_ExceptionHandler_hasJavaExceptionOccurred(env)){
            (*env)->ExceptionClear(env);
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "%s", message);
        }
        /*  all processing succesfull so far => translate the exception */
        if(0 == strcmp(name, "java.lang.OutOfMemoryError")) {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "%s", message);
        } else if (0 == strncmp(name, "java.lang.", strlen("java.lang."))) {
            DLRL_Exception_THROW(exception, DLRL_ERROR, "%s %s", name, message);
        } else if (0 == strcmp(name, "DDS.DCPSError")) {
            DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, "%s", message);
        } else if (0 == strcmp(name, "DDS.BadHomeDefinition")) {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION, "%s", message);
        } else if (0 == strcmp(name, "DDS.BadParameter")) {
            DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER, "%s", message);
        } else if (0 == strcmp(name, "DDS.NotFound")) {
            DLRL_Exception_THROW(exception, DLRL_NOT_FOUND, "%s", message);
        } else if (0 == strcmp(name, "DDS.SQLError")) {
            DLRL_Exception_THROW(exception, DLRL_SQL_ERROR, "%s", message);
        } else if (0 == strcmp(name, "DDS.AlreadyExisting")) {
            DLRL_Exception_THROW(exception, DLRL_ALREADY_EXISTING, "%s", message);
        } else if (0 == strcmp(name, "DDS.InvalidObjects")) {
            DLRL_Exception_THROW(exception, DLRL_INVALID_OBJECTS, "%s", message);
        } else if (0 == strcmp(name, "DDS.PreconditionNotMet")) {
            DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "%s", message);
        } else if (0 == strcmp(name, "DDS.AlreadyDeleted")) {
            DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED, "%s", message);
        } else if (0 == strcmp(name, "DDS.NoSuchElement")) {
            DLRL_Exception_THROW(exception, DLRL_NO_SUCH_ELEMENT, "%s", message);
        } else {
            /*  Hmm, unknown exception thrown, propagate it as a general error. */
            DLRL_Exception_THROW(exception, DLRL_ERROR, "Unable to identify java exception '%s'. It's of an unknown"
                                                        " type, %s", name, message);
        }

        DLRL_Exception_EXIT(exception);
        /*  cleanup, check the java and c versions of the messages and names, as we use default values for the  */
        /* c version of the message and name if the java name or message was null. */
        if(jclass){
            if(jname&& name){
                (*env)->ReleaseStringUTFChars(env, jname, name);
            }
            if (jmessage && message){
                (*env)->ReleaseStringUTFChars(env, jmessage, message);
            }
        }
        (*env)->DeleteLocalRef(env, jexception);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DJA_ExceptionHandler_hasJavaExceptionOccurred(
    JNIEnv *env)
{

    DLRL_INFO(INF_ENTER);
    assert(env);

    DLRL_INFO(INF_EXIT);
    return (*env)->ExceptionCheck(env);
}

char *
DJA_ExceptionHandler_returnCodeDCPSToString(
    LOC_ReturnCode_t returnCode)
{
    char *returnString = NULL;

    DLRL_INFO(INF_ENTER);

    switch (returnCode)
    {
        case LOC_RETCODE_ERROR:
            returnString = "RETCODE_ERROR";
            break;
        case LOC_RETCODE_UNSUPPORTED:
            returnString = "RETCODE_UNSUPPORTED";
            break;
        case LOC_RETCODE_BAD_PARAMETER:
            returnString = "RETCODE_BAD_PARAMETER";
            break;
        case LOC_RETCODE_PRECONDITION_NOT_MET:
            returnString = "RETCODE_PRECONDITION_NOT_MET";
            break;
        case LOC_RETCODE_OUT_OF_RESOURCES:
            returnString = "RETCODE_OUT_OF_RESOURCES";
            break;
        case LOC_RETCODE_NOT_ENABLED:
            returnString = "RETCODE_NOT_ENABLED";
            break;
        case LOC_RETCODE_IMMUTABLE_POLICY:
            returnString = "RETCODE_IMMUTABLE_POLICY";
            break;
        case LOC_RETCODE_INCONSISTENT_POLICY:
            returnString = "RETCODE_INCONSISTENT_POLICY";
            break;
        case LOC_RETCODE_ALREADY_DELETED:
            returnString = "RETCODE_ALREADY_DELETED";
            break;
        case LOC_RETCODE_TIMEOUT:
            returnString = "RETCODE_TIMEOUT";
            break;
        case LOC_RETCODE_NO_DATA:
            returnString = "RETCODE_NO_DATA";
            break;
        case LOC_RETCODE_OK:
            returnString = "RETCODE_OK";
            break;
        default:
            returnString = "Unknown DCPS returncode";
            break;
    }
    return returnString;

    DLRL_INFO(INF_EXIT);
}


/* NOT IN DESIGN */
char *
DJA_ExceptionHandler_returnCodeGAPIToString(
    gapi_returnCode_t returnCode)
{
    char *returnString = NULL;

    DLRL_INFO(INF_ENTER);

    switch (returnCode)
    {
        case GAPI_RETCODE_ERROR:
            returnString = "RETCODE_ERROR";
            break;
        case GAPI_RETCODE_UNSUPPORTED:
            returnString = "RETCODE_UNSUPPORTED";
            break;
        case GAPI_RETCODE_BAD_PARAMETER:
            returnString = "RETCODE_BAD_PARAMETER";
            break;
        case GAPI_RETCODE_PRECONDITION_NOT_MET:
            returnString = "RETCODE_PRECONDITION_NOT_MET";
            break;
        case GAPI_RETCODE_OUT_OF_RESOURCES:
            returnString = "RETCODE_OUT_OF_RESOURCES";
            break;
        case GAPI_RETCODE_NOT_ENABLED:
            returnString = "RETCODE_NOT_ENABLED";
            break;
        case GAPI_RETCODE_IMMUTABLE_POLICY:
            returnString = "RETCODE_IMMUTABLE_POLICY";
            break;
        case GAPI_RETCODE_INCONSISTENT_POLICY:
            returnString = "RETCODE_INCONSISTENT_POLICY";
            break;
        case GAPI_RETCODE_ALREADY_DELETED:
            returnString = "RETCODE_ALREADY_DELETED";
            break;
        case GAPI_RETCODE_TIMEOUT:
            returnString = "RETCODE_TIMEOUT";
            break;
        case GAPI_RETCODE_NO_DATA:
            returnString = "RETCODE_NO_DATA";
            break;
        case GAPI_RETCODE_OK:
            returnString = "RETCODE_OK";
            break;
        default:
            returnString = "Unknown DCPS returncode";
            break;
    }
    return returnString;

    DLRL_INFO(INF_EXIT);
}
