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
#include "DJA_SelectionBridge.h"
#include "DJA_Initialisation.h"
#include "DLRL_Kernel_private.h"
#include "DLRL_Util.h"

/* important: Both the update and admin mutex of the owning object home are locked during execution of this */
/* operation. When we make the callback to Java we will unlock the admin mutex during the java callback so we  */
/* can allow the DLRL application to access as much data as possible during the callback, when we return from the  */
/* callback we immediately lock the admin mutex again. Because we will still have a lock on the update mutex it is not */
/* neccesary to check if anything was deleted because the deletion of the object home and the selection are protected by */
/* the update and admin mutex! (see deleteSelection operation of the object home) */
DK_ObjectAdmin**
DJA_SelectionBridge_us_checkObjects(
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    DLRL_LS_object filterCriterion,/* protected by uodate mutex of the home */
    DK_ObjectAdmin** objectArray,
    LOC_unsigned_long size,
    LOC_unsigned_long* passedAdminsArraySize)
{
    JNIEnv* env = (JNIEnv*)userData;
    DK_ObjectAdmin** passedAdmins = NULL;
    jint *jindexArray;
    jintArray indexes = NULL;
    DK_ObjectAdmin* anObjectAdmin;
    jobject janObjectAdmin;
    LOC_unsigned_long length;
    LOC_unsigned_long count;
    jobjectArray jobjects = NULL;
    jobject jselection = NULL;
    LOC_unsigned_long realLength = 0;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(env);
    assert(selection);
    assert(filterCriterion);
    assert(objectArray);
    assert(passedAdminsArraySize);

    /* first we have to copy the array of object admins into an array of java object admins */
    jobjects = (*env)->NewObjectArray(env, size, cachedJNI.objectRoot_class, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    for(count = 0; count < size; count++){
        anObjectAdmin = (DK_ObjectAdmin*)objectArray[count];

        janObjectAdmin = (jobject)DK_ObjectAdmin_us_getLSObject(anObjectAdmin);
        assert(janObjectAdmin);
        (*env)->SetObjectArrayElement(env, jobjects, count, janObjectAdmin);
    }
    /* now we can call the java callback routine that will return an array of indexes. The index indicate which elements */
    /* of the object admin array passed the criteria */
    jselection = (jobject)DK_SelectionAdmin_us_getLSSelection(selection);/* protected by uodate mutex of the home */
    /* before doing the callback, release the admin mutex, we still have the update mutex locked though! */
    DK_SelectionAdmin_unlockHome(selection);
    indexes = (*env)->CallObjectMethod(env, jselection, cachedJNI.selection_checkObjects_mid, (jobject)filterCriterion, jobjects);
    DK_SelectionAdmin_lockHome(selection);
    DLRL_JavaException_PROPAGATE(env, exception);
    length = (LOC_unsigned_long)(*env)->GetArrayLength(env, indexes);
    if(length > 0){
        jindexArray = (*env)->GetIntArrayElements(env, indexes, NULL);
        if (jindexArray == NULL){
            DLRL_JavaException_PROPAGATE(env, exception);
        }
        DLRL_ALLOC_WITH_SIZE(passedAdmins, (sizeof(DK_ObjectAdmin*)*length), exception,
                                            "Unable to allocate array container for ObjectAdmins!");
        for (count = 0; count < length; count++) {
            LOC_long anIndex = (LOC_long)jindexArray[(jint)count];
            /* a negative index marks the end of the values in the array. */
            if(anIndex >= 0){
                assert(anIndex < (LOC_long)size);
                passedAdmins[count] = objectArray[anIndex];
                realLength++;
            } else {
                count = length;/* aka exit the loop. */
            }
        }
        (*env)->ReleaseIntArrayElements(env, indexes, jindexArray, 0);
    }
    *passedAdminsArraySize = realLength;

    DLRL_Exception_EXIT(exception);
    if(jobjects){
        (*env)->DeleteLocalRef(env, jobjects);
    }
    if(indexes){
        (*env)->DeleteLocalRef(env, indexes);
    }
    DLRL_INFO(INF_EXIT);
    return passedAdmins;
}

void
DJA_SelectionBridge_us_triggerListenerInsertedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin)
{
    DJA_CachedJNITypedObject* typedCache;
    JNIEnv* env = (JNIEnv*)userData;
    jobject jlistener;
    jobject jobjectAdmin;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(userData);
    assert(listener);
    assert(objectAdmin);

    typedCache = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    jlistener = (jobject)listener;
    jobjectAdmin = (jobject)DK_ObjectAdmin_us_getLSObject(objectAdmin);
    (*env)->CallVoidMethod(
        env,
        jlistener,
        typedCache->typedSelectionListener_onObjectIn_mid,
        jobjectAdmin);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_SelectionBridge_us_triggerListenerModifiedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin)
{
    DJA_CachedJNITypedObject* typedCache;
    JNIEnv* env = (JNIEnv*)userData;
    jobject jlistener;
    jobject jobjectAdmin;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(userData);
    assert(listener);
    assert(objectAdmin);

    typedCache = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    jlistener = (jobject)listener;
    jobjectAdmin = (jobject)DK_ObjectAdmin_us_getLSObject(objectAdmin);
    (*env)->CallVoidMethod(
        env,
        jlistener,
        typedCache->typedSelectionListener_onObjectModified_mid,
        jobjectAdmin);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_SelectionBridge_us_triggerListenerRemovedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object listener,
    DK_ObjectAdmin* objectAdmin)
{
    DJA_CachedJNITypedObject* typedCache;
    JNIEnv* env = (JNIEnv*)userData;
    jobject jlistener;
    jobject jobjectAdmin;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(userData);
    assert(listener);
    assert(objectAdmin);

    typedCache = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    jlistener = (jobject)listener;
    jobjectAdmin = (jobject)DK_ObjectAdmin_us_getLSObject(objectAdmin);
    (*env)->CallVoidMethod(
        env,
        jlistener,
        typedCache->typedSelectionListener_onObjectOut_mid,
        jobjectAdmin);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
