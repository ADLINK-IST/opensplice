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
/* C includes */
#include <assert.h>

/* jni includes */
#include <jni.h>

/* DLRL kernel includes */
#include "DLRL_Report.h"
#include "DLRL_Exception.h"
#include "DLRL_Types.h"
#include "DJA_CacheAccessBridge.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

/* collection includes */
#include "Coll_Iter.h"

void
DJA_CacheAccessBridge_us_containedTypesAction(
    DLRL_Exception* exception,
    void* userData,
    LOC_long* indexes,
    LOC_unsigned_long totalSize,
    void** arg)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;
    jintArray jintElements = NULL;
    jint* jintArrayElements = NULL;
    LOC_unsigned_long count;

    DLRL_INFO(INF_ENTER);

    jintElements = (*env)->NewIntArray(env, totalSize);
    DLRL_JavaException_PROPAGATE(env, exception);
    jintArrayElements = (*env)->GetIntArrayElements(env, jintElements, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    for(count = 0; count < totalSize; count++){
        jintArrayElements[count] = (jint)indexes[count];
    }
    (*env)->ReleaseIntArrayElements(env, jintElements, jintArrayElements, 0);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheAccessBridge_us_containedTypeNamesAction(
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long totalSize,
    LOC_unsigned_long index,
    LOC_string name,
    void** arg)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;
    jstring jname = NULL;

    DLRL_INFO(INF_ENTER);

    if(!(*arg)){
        *arg = (void*)(*env)->NewObjectArray(env, totalSize, cachedJNI.string_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);
    }

    jname = (*env)->NewStringUTF(env, name);
    DLRL_JavaException_PROPAGATE(env, exception);

    (*env)->SetObjectArrayElement(env, (jobjectArray)(*arg), index, jname);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    if(jname){
        (*env)->DeleteLocalRef(env, jname);
    }
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
void
DJA_CacheAccessBridge_us_objectsAction(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size,
    LOC_unsigned_long*
    elementIndex,
    Coll_Set* objects)
{
    JNIEnv* env = (JNIEnv*)userData;
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* object = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);
    assert(objects);
    /* arg may be NULL */

    /* if the arg is still null we have to allocate a new object array for each element. */
    if(!(*arg)){
        *arg = (void*)(*env)->NewObjectArray(env, size, cachedJNI.objectRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);
    }
    iterator = Coll_Set_getFirstElement(objects);
    while(iterator){
        object = Coll_Iter_getObject(iterator);

        assert(DK_ObjectAdmin_us_getLSObject(object));
        (*env)->SetObjectArrayElement(env, (jobjectArray)*arg, (*elementIndex), (jobject)DK_ObjectAdmin_us_getLSObject(object));
        DLRL_JavaException_PROPAGATE(env, exception);
        iterator = Coll_Iter_getNext(iterator);
        (*elementIndex)++;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* invalidObjects may be cleared by using popBack when iterating it (dont have to lower ref counts), if not it's
 * ok as well may not use ts operation for any entity that requires the home admin lock
 */
void
DJA_CacheAccessBridge_us_invalidObjectsAction(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    Coll_List* invalidObjects)
{
    JNIEnv* env = (JNIEnv*)userData;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long count = 0;
    DK_ObjectAdmin* object = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);
    assert(invalidObjects);

    size = Coll_List_getNrOfElements(invalidObjects);
    *arg = (void*)(*env)->NewObjectArray(env, size, cachedJNI.objectRoot_class, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    for(count = 0; count < size; count++){
        object = (DK_ObjectAdmin*)Coll_List_popBack(invalidObjects);
        /* dont release the 'object', it is not a duplicate */
        (*env)->SetObjectArrayElement(env, ((jobjectArray)*arg), count, (jobject)DK_ObjectAdmin_us_getLSObject(object));
        DLRL_JavaException_PROPAGATE(env, exception);
    }

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
}
