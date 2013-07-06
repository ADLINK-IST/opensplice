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
#include "DJA_IntMap.h"
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"
#include "DJA_MapCommon.h"

JNIEXPORT jint JNICALL
Java_DDS_IntMap_jniLength(
    JNIEnv * env,
    jobject ls_map)
{
    DK_MapAdmin* map = NULL;
    DLRL_Exception exception;
    jint length = 0;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    length = (jint)DK_MapAdmin_ts_getLength(map, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return length;
}

JNIEXPORT jintArray JNICALL
Java_DDS_IntMap_jniAddedElements(
    JNIEnv * env,
    jobject ls_map)
{
    jintArray returnValue = NULL;
    DK_Collection* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    returnValue = (jintArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_ADDED,
                                                    DJA_MAP_KEY_TYPE_INT);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jintArray JNICALL
Java_DDS_IntMap_jniModifiedElements(
    JNIEnv * env,
    jobject ls_map)
{
    jintArray returnValue = NULL;
    DK_Collection* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    returnValue = (jintArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception , DJA_MAP_ELEMENT_TYPE_MODIFIED,
                                                    DJA_MAP_KEY_TYPE_INT);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jintArray JNICALL
Java_DDS_IntMap_jniGetKeys(
    JNIEnv * env,
    jobject ls_map)
{
    DK_Collection* map = NULL;
    DLRL_Exception exception;
    jintArray returnValue = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    returnValue = (jintArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_MAIN,
                                                    DJA_MAP_KEY_TYPE_INT);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jintArray JNICALL
Java_DDS_IntMap_jniRemovedElements(
    JNIEnv * env,
    jobject ls_map)
{
    jintArray returnValue = NULL;
    DK_Collection* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    returnValue = (jintArray)DJA_MapCommon_ts_fillElementsArray(env, map, &exception, DJA_MAP_ELEMENT_TYPE_REMOVED,
                                                    DJA_MAP_KEY_TYPE_INT);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_IntMap_jniGetValues(
    JNIEnv * env,
    jobject ls_map)
{
    DK_Collection* map = NULL;
    DLRL_Exception exception;
    jobject jtypedIterator = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_Collection*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    jtypedIterator = DJA_MapCommon_ts_getValues(env, map, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jtypedIterator;
}

JNIEXPORT jobject JNICALL
Java_DDS_IntMap_jniGet(
    JNIEnv * env,
    jobject ls_map,
    jint key)
{
    jobject retVal = NULL;
    DK_MapAdmin* collection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    collection = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(collection);

    retVal = (jobject)DK_MapAdmin_ts_get(collection, &exception, (void*)env, (void*)&key);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT void JNICALL
Java_DDS_IntMap_jniDeleteIntMap(
    JNIEnv * env,
    jobject ls_intMap)
{
    DK_Entity* intMapEntity;

    DLRL_INFO(INF_ENTER);

    intMapEntity = (DK_Entity*)(*env)->GetLongField(env, ls_intMap, cachedJNI.intMap_admin_fid);
    if(intMapEntity){
        DK_Entity_ts_release(intMapEntity);
    }

    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_IntMap_jniPut(
    JNIEnv * env,
    jobject ls_map,
    jint key,
    jobject value)
{
    DK_MapAdmin* map = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, value, "value");

    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, value, cachedJNI.objectRoot_admin_fid);
    if(!objectAdmin){
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "value parameter is corrupted");
    }

    DK_MapAdmin_ts_put(map, &exception, (void*)env, (void*)&key, objectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_IntMap_jniRemove(
    JNIEnv * env,
    jobject ls_map,
    jint key)
{
    DK_MapAdmin* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    DK_MapAdmin_ts_remove(map, &exception, (void*)env, (const void*)&key);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_IntMap_jniClear(
    JNIEnv* env,
    jobject ls_map)
{
    DK_MapAdmin* map = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    map = (DK_MapAdmin*)(*env)->GetLongField(env, ls_map, cachedJNI.intMap_admin_fid);
    assert(map);

    DK_MapAdmin_ts_clear(map, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}
