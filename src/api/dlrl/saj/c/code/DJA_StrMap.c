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
#include "DJA_StrMap.h"
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"
#include "DJA_MapCommon.h"
#include "DLRL_Util.h"

JNIEXPORT jint JNICALL
Java_DDS_StrMap_jniLength(
    JNIEnv * env,
    jobject ls_map)
{
    DK_MapAdmin* map = NULL;
    jint length = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    length = (jint)DK_MapAdmin_ts_getLength(map, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return length;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_StrMap_jniAddedElements(
    JNIEnv * env,
    jobject ls_map)
{
    jobjectArray returnValue = NULL;
    DK_Collection* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    returnValue = (jobjectArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_ADDED,
                                        DJA_MAP_KEY_TYPE_STRING);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_StrMap_jniModifiedElements(
    JNIEnv * env,
    jobject ls_map)
{
    jobjectArray returnValue = NULL;
    DK_Collection* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    returnValue = (jobjectArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_MODIFIED,
                                                    DJA_MAP_KEY_TYPE_STRING);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_StrMap_jniGetKeys(
    JNIEnv * env,
    jobject ls_map)
{
    DK_Collection* map = NULL;
    DLRL_Exception exception;
    jobjectArray returnValue = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    returnValue = (jobjectArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_MAIN,
                                                DJA_MAP_KEY_TYPE_STRING);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_StrMap_jniRemovedElements(
    JNIEnv * env,
    jobject ls_map)
{
    jobjectArray returnValue = NULL;
    DK_Collection* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    returnValue = (jobjectArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_REMOVED,
                                                    DJA_MAP_KEY_TYPE_STRING);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_StrMap_jniGetValues(
    JNIEnv * env,
    jobject ls_map)
{
    DK_Collection* map = NULL;
    DLRL_Exception exception;
    jobject jtypedIterator = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    jtypedIterator = DJA_MapCommon_ts_getValues(env, map, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jtypedIterator;
}

JNIEXPORT jobject JNICALL
Java_DDS_StrMap_jniGet(
    JNIEnv * env,
    jobject ls_map,
    jstring jkey)
{
    jobject retVal = NULL;
    DK_MapAdmin* collection = NULL;
    DLRL_Exception exception;
    LOC_string key = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, jkey, "key");

    key = (LOC_string)(*env)->GetStringUTFChars(env, jkey, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    collection = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(collection);
    retVal = (jobject)DK_MapAdmin_ts_get(collection, &exception, (void*)env, (void*)key);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(key){
        (*env)->ReleaseStringUTFChars(env, jkey, key);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT void JNICALL
Java_DDS_StrMap_jniDeleteStrMap(
    JNIEnv * env,
    jobject ls_strMap)
{
    DK_Entity* strMapEntity;

    DLRL_INFO(INF_ENTER);

    strMapEntity = (DK_Entity*)(*env)->GetLongField(env, ls_strMap, cachedJNI.strMap_admin_fid);
    if(strMapEntity){
        DK_Entity_ts_release(strMapEntity);
    }

    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_StrMap_jniPut(
    JNIEnv * env,
    jobject ls_map,
    jstring jkey,
    jobject value)
{
    DK_MapAdmin* map = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    DLRL_Exception exception;
    LOC_string key = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, jkey, "key");
    DLRL_VERIFY_NOT_NULL(&exception, value, "value");

    key = (LOC_string)(*env)->GetStringUTFChars(env, jkey, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, value, cachedJNI.objectRoot_admin_fid);
    if(!objectAdmin){
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "value parameter is corrupted");
    }
    DK_MapAdmin_ts_put(map, &exception, (void*)env, (void*)key, objectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(key){
        (*env)->ReleaseStringUTFChars(env, jkey, key);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_StrMap_jniRemove(
    JNIEnv * env,
    jobject ls_map,
    jstring jkey)
{
    DK_MapAdmin* map = NULL;
    DLRL_Exception exception;
    LOC_const_string key = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, jkey, "key");

    key = (LOC_const_string)(*env)->GetStringUTFChars(env, jkey, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    DK_MapAdmin_ts_remove(map, &exception, (void*)env, (const void*)key);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(key){
        (*env)->ReleaseStringUTFChars(env, jkey, key);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_StrMap_jniClear(
    JNIEnv* env,
    jobject ls_map)
{
    DK_MapAdmin* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.strMap_admin_fid);
    assert(map);

    DK_MapAdmin_ts_clear(map, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}
