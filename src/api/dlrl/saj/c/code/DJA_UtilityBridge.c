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
/* jni includes */
#include <jni.h>
/* DLRL SAJ includes */
#include "DJA_ExceptionHandler.h"
/* DLRL includes */
#include "DLRL_Report.h"
#include <assert.h>
#include "DJA_Initialisation.h"

#include "DJA_UtilityBridge.h"

/* NOT IN DESIGN - moved */
/* NOT IN DESIGNstruct OIDStructure_s{ */
/* NOT IN DESIGN    LOC_unsigned_long field1; */
/* NOT IN DESIGN    LOC_unsigned_long field2; */
/* NOT IN DESIGN}; */

LOC_boolean
DJA_UtilityBridge_us_AreSameLSObjects(
    void* userData,
    DLRL_LS_object obj1,
    DLRL_LS_object obj2)
{
    LOC_boolean isSame;
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(obj1);
    assert(obj2);

    isSame = (*env)->IsSameObject(env, (jobject)obj1, (jobject)obj2);

    /*  no Java exception can occur */
    DLRL_INFO(INF_EXIT);
    return isSame;
}

void
DJA_UtilityBridge_us_createIntegerSeq(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size)
{
    /*not yet implemented for SAJ */

}
void
DJA_UtilityBridge_us_createStringSeq(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size)
{
    /*not yet implemented for SAJ */

}

void
DJA_UtilityBridge_us_addElementToStringSeq(
    DLRL_Exception* exception,
    void* userData,
    void* arg,
    LOC_unsigned_long count,
    void* keyUserData)
{
    /*not yet implemented for SAJ */

}

void
DJA_UtilityBridge_us_addElementToIntegerSeq(
    DLRL_Exception* exception,
    void* userData,
    void* arg,
    LOC_unsigned_long count,
    void* keyUserData)
{
    /*not yet implemented for SAJ */

}

DLRL_LS_object
DJA_UtilityBridge_us_duplicateLSObject(
    void* userData,
    DLRL_LS_object object)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(object);

     DLRL_INFO(INF_EXIT);
    return (DLRL_LS_object)(*env)->NewGlobalRef (env, (jobject)object);
}

DLRL_LS_object
DJA_UtilityBridge_us_localDuplicateLSObject(
    void* userData,
    DLRL_LS_object object)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(object);

    DLRL_INFO(INF_EXIT);
    return (DLRL_LS_object)(*env)->NewLocalRef(env, (jobject)object);
}


void
DJA_UtilityBridge_us_releaseLSObject(
    void* userData,
    DLRL_LS_object object)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(object);

    (*env)->DeleteGlobalRef(env, (jobject)object);

    DLRL_INFO(INF_EXIT);
}

void
DJA_UtilityBridge_us_getThreadCreateUserData(
    DLRL_Exception* exception,
    void* userData,
    void** retVal)
{
    jint result;
    JavaVM *jvm = NULL;
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);
    assert(!*retVal);

    result = (*env)->GetJavaVM(env, &jvm);
    if(result != 0){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation. Out of resources.");
    }
    *retVal = (void*)jvm;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_UtilityBridge_us_doThreadAttach(
    DLRL_Exception* exception,
    void* userData)
{
    JavaVM *jvm;
    JNIEnv *env = NULL;
    void* threadData;
    jint jresult;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);

    jvm = (JavaVM*)userData;
    jresult = (*jvm)->AttachCurrentThread(jvm, (void**)&env, NULL);
    if(jresult != 0){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation. Out of resources.");
    }
    threadData = os_threadMemMalloc(OS_THREAD_JVM, sizeof(env));
    *(JNIEnv**)threadData = env;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_UtilityBridge_us_doThreadDetach(
    DLRL_Exception* exception,
    void* userData)
{
    JavaVM *jvm;
    JNIEnv* env;
    jint jresult;

    DLRL_INFO(INF_ENTER);

    assert(exception);

    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);
    jresult = (*env)->GetJavaVM(env, &jvm);
    if(jresult != 0){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation. Out of resources.");
    }
    jresult = (*jvm)->DetachCurrentThread(jvm);
    os_threadMemFree(OS_THREAD_JVM);
    if(jresult != 0){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation. Out of resources.");
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void*
DJA_UtilityBridge_us_getThreadSessionUserData()
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_EXIT);
    return (void*)(*(JNIEnv**)os_threadMemGet(OS_THREAD_JVM));
}
