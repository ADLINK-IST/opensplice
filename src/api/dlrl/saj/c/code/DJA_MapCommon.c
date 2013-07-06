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
#include "DJA_MapCommon.h"
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"
#include "DLRL_Util.h"

jobject
DJA_MapCommon_ts_getValues(
    JNIEnv* env,
    DK_Collection* collection,
    DLRL_Exception* exception)
{
    DK_ObjectAdmin* target = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    jobjectArray jelements = NULL;
    LOC_unsigned_long count = 0;
    DJA_CachedJNITypedObject* typedObjectCachedData= NULL;
    jclass elementClass = NULL;
    Coll_Set* objectHolders = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(collection);
    assert(exception);

    /* locks owner and target home, and next operation checks if collection, owner home and target homes are alive */
    DK_Collection_lockAll(collection);
    DK_Collection_us_checkAliveAll(collection, exception);
    DLRL_Exception_PROPAGATE(exception);

    targetHome = DK_Collection_us_getTargetHome(collection);
    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(targetHome);
    elementClass = (*env)->NewLocalRef(env, typedObjectCachedData->typedRoot_class);
    if(!elementClass){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Not enough memory to complete operation.");
    }

    objectHolders = DK_MapAdmin_us_getObjectHolders((DK_MapAdmin*)collection);
    /* create & set the array */
    jelements = (*env)->NewObjectArray(env, Coll_Set_getNrOfElements(objectHolders), elementClass, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    iterator = Coll_Set_getFirstElement(objectHolders);
    /* DJA_ExceptionHandler_hasJavaExceptionOccurred checks if the array set element went wrong */
    /* and checks if the jelements array was allocated correctly initially */
    while(iterator){
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        /* holder is protected by the home mutex (of the owner home), which was claimed during the lockAll op. */
        assert(DK_ObjectHolder_us_isResolved(holder));
        /* no duplicate needed, must be valid, protected by the owner home mutex */
        target = DK_ObjectHolder_us_getTarget(holder);
        /* the target object is protected by the lock on the target object home. No updates can be processed. */
        /* if target object no longer alive then treat it as a not found. otherwise */
        /* get the ls object (which is always valid on an alive object admin) and set it as an array */
        /* element. */
        if(DK_ObjectAdmin_us_isAlive(target)){
            jobject jtarget = (jobject)DK_ObjectAdmin_us_getLSObject(target);
            (*env)->SetObjectArrayElement(env, jelements, count, jtarget);
            DLRL_JavaException_PROPAGATE(env, exception);
        } else {/* an already deleted object becomes a NULL pointer for the moment */
            (*env)->SetObjectArrayElement(env, jelements, count, NULL);
            DLRL_JavaException_PROPAGATE(env, exception);
        }
        /* TODO ID: 132 */
        count++;
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAll(collection);
    DLRL_INFO(INF_EXIT);
    return (jobject)jelements;
}

jobject
DJA_MapCommon_ts_fillElementsArray(
    JNIEnv* env,
    DK_Collection* collection,
    DLRL_Exception* exception,
    DJA_MapElementType elementType,
    DJA_MapKeyType collectionBase)
{
    void* keyUserData  = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long count = 0;
    Coll_Set* elementsSet = NULL;
    Coll_List* elementsList = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    jobjectArray jobjectElements = NULL;
    jintArray jintElements = NULL;
    jint* jintArrayElements = NULL;
    jstring aKey = NULL;
    jobject returnValue = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(collection);
    assert(exception);
    assert(elementType < DJA_MapElementType_elements);
    assert(collectionBase < DJA_MapKeyType_elements);

    /* locks owner and target home, and next operation checks if collection, owner home and target homes are alive */
    DK_Collection_lockAll(collection);
    DK_Collection_us_checkAliveAll(collection, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(elementType == DJA_MAP_ELEMENT_TYPE_ADDED){
        elementsList = DK_MapAdmin_us_getAddedElements((DK_MapAdmin*)collection, exception);
        DLRL_Exception_PROPAGATE(exception);
        size = Coll_List_getNrOfElements(elementsList);
        iterator = Coll_List_getFirstElement(elementsList);
    } else if(elementType == DJA_MAP_ELEMENT_TYPE_REMOVED){
        elementsList = DK_Collection_us_getRemovedElements(collection);
        size = Coll_List_getNrOfElements(elementsList);
        iterator = Coll_List_getFirstElement(elementsList);
    } else if(elementType == DJA_MAP_ELEMENT_TYPE_MODIFIED){
        elementsList = DK_MapAdmin_us_getModifiedElements((DK_MapAdmin*)collection);
        size = Coll_List_getNrOfElements(elementsList);
        iterator = Coll_List_getFirstElement(elementsList);
    } else {
        assert(elementType == DJA_MAP_ELEMENT_TYPE_MAIN);
        elementsSet = DK_MapAdmin_us_getObjectHolders((DK_MapAdmin*)collection);
        size = Coll_Set_getNrOfElements(elementsSet);
        iterator = Coll_Set_getFirstElement(elementsSet);
    }
    if(collectionBase == DJA_MAP_KEY_TYPE_STRING){
        jobjectElements = (*env)->NewObjectArray(env, size, cachedJNI.string_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);
        returnValue = (jobject)jobjectElements;
    } else {
        assert(collectionBase == DJA_MAP_KEY_TYPE_INT);
        jintElements = (*env)->NewIntArray(env, size);
        DLRL_JavaException_PROPAGATE(env, exception);
        jintArrayElements = (*env)->GetIntArrayElements(env, jintElements, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);
        returnValue = (jobject)jintElements;
    }

    while(iterator){
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        keyUserData = DK_ObjectHolder_us_getUserData(holder);
        assert(keyUserData);
        if(collectionBase == DJA_MAP_KEY_TYPE_STRING){
            aKey = keyUserData ? (*env)->NewStringUTF(env, keyUserData) : NULL;
            DLRL_JavaException_PROPAGATE(env, exception);
            if(aKey){
                (*env)->SetObjectArrayElement(env, jobjectElements, count, aKey);
                (*env)->DeleteLocalRef(env, aKey);
                DLRL_JavaException_PROPAGATE(env, exception);
            }
        } else {
            assert(collectionBase == DJA_MAP_KEY_TYPE_INT);
            jintArrayElements[count] = (jint)(*(LOC_long*)keyUserData);
        }
        count++;
        iterator = Coll_Iter_getNext(iterator);
    }
    if(collectionBase == DJA_MAP_KEY_TYPE_INT){
        /* copy back the array and release the resources */
        (*env)->ReleaseIntArrayElements(env, jintElements, jintArrayElements, 0);
    }

    DLRL_Exception_EXIT(exception);
    DK_Collection_unlockAll(collection);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}
