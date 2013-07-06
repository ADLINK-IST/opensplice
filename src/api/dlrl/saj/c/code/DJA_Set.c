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
#include "DJA_Set.h"
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

typedef enum DDS_Set_ElementCollectionType_e{
    DDS_SET_ELEMENT_COLLECTION_TYPE_MAIN,
    DDS_SET_ELEMENT_COLLECTION_TYPE_ADDED,
    DDS_SET_ELEMENT_COLLECTION_TYPE_REMOVED,
    DDS_SET_ELEMENT_COLLECTION_TYPE_elements
} DDS_Set_ElementCollectionType;

static jobject
DDS_Set_getObjectArrayForCollectionType(
    JNIEnv * env,
    jobject ls_set,
    DDS_Set_ElementCollectionType type);

JNIEXPORT jint JNICALL
Java_DDS_Set_jniLength(
    JNIEnv * env,
    jobject ls_set)
{
    DK_SetAdmin* set = NULL;
    jint length = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    set = (DK_SetAdmin*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    assert(set);

    length = (jint)DK_SetAdmin_ts_getLength(set, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return length;
}

JNIEXPORT void JNICALL
Java_DDS_Set_jniAdd(
    JNIEnv * env,
    jobject ls_set,
    jobject ls_value)
{
    DK_ObjectAdmin* objectAdmin = NULL;
    DK_SetAdmin* set = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    DLRL_VERIFY_NOT_NULL(&exception, ls_value, "value");
    set = (DK_SetAdmin*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    assert(set);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_value, cachedJNI.objectRoot_admin_fid) ;
    if(!objectAdmin){
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "value parameter is corrupted");
    }

    DK_SetAdmin_ts_add(set, &exception, (void*)env, objectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jboolean JNICALL
Java_DDS_Set_jniContains(
    JNIEnv * env,
    jobject ls_set,
    jobject ls_value)
{
    DK_SetAdmin* set = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    LOC_boolean retVal = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    DLRL_VERIFY_NOT_NULL(&exception, ls_value, "value");
    set = (DK_SetAdmin*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    assert(set);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_value, cachedJNI.objectRoot_admin_fid);
    if(!objectAdmin){
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "value parameter is corrupted");
    }

    retVal = DK_SetAdmin_ts_contains(set, &exception, objectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT void JNICALL
Java_DDS_Set_jniRemove(
    JNIEnv * env,
    jobject ls_set,
    jobject value)
{
    DK_SetAdmin* set = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    DLRL_VERIFY_NOT_NULL(&exception, value, "value");
    set = (DK_SetAdmin*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    assert(set);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, value, cachedJNI.objectRoot_admin_fid);
    if(!objectAdmin){
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "value parameter is corrupted");
    }
    DK_SetAdmin_ts_remove(set, &exception, (void*)env, objectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_Set_jniDeleteSet(
    JNIEnv * env,
    jobject ls_set)
{
    DK_Entity* setEntity;

    DLRL_INFO(INF_ENTER);

    setEntity = (DK_Entity*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    if(setEntity){
        DK_Entity_ts_release(setEntity);
    }

    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_Set_jniAddedElements(
    JNIEnv * env,
    jobject ls_set)
{
    jobjectArray jelements;

    DLRL_INFO(INF_ENTER);

    jelements = DDS_Set_getObjectArrayForCollectionType(env, ls_set, DDS_SET_ELEMENT_COLLECTION_TYPE_ADDED);

    DLRL_INFO(INF_EXIT);
    return jelements;
}

JNIEXPORT jobject JNICALL
Java_DDS_Set_jniRemovedElements(
    JNIEnv * env,
    jobject ls_set)
{
    jobjectArray jelements;

    DLRL_INFO(INF_ENTER);

    jelements = DDS_Set_getObjectArrayForCollectionType(env, ls_set, DDS_SET_ELEMENT_COLLECTION_TYPE_REMOVED);

    DLRL_INFO(INF_EXIT);
    return jelements;
}

JNIEXPORT jobject JNICALL
Java_DDS_Set_jniGetValues(
    JNIEnv * env,
    jobject ls_set)
{
    jobjectArray jelements;

    DLRL_INFO(INF_ENTER);

    jelements = DDS_Set_getObjectArrayForCollectionType(env, ls_set, DDS_SET_ELEMENT_COLLECTION_TYPE_MAIN);

    DLRL_INFO(INF_EXIT);
    return jelements;
}

JNIEXPORT void JNICALL
Java_DDS_Set_jniClear(
    JNIEnv * env,
    jobject ls_set)
{
    DK_SetAdmin* set = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    set = (DK_SetAdmin*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    assert(set);

    DK_SetAdmin_ts_clear(set, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

jobjectArray
DDS_Set_getObjectArrayForCollectionType(
    JNIEnv * env,
    jobject ls_set,
    DDS_Set_ElementCollectionType type)
{
    DK_Collection* collection = NULL;
    Coll_Set* valuesSet = NULL;
    Coll_List* valuesList = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHolder* holder = NULL;
    DK_ObjectAdmin* target = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;
    jobjectArray jelements = NULL;
    LOC_unsigned_long count = 0;
    DJA_CachedJNITypedObject* typedObjectCachedData= NULL;
    jclass elementClass = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    collection = (DK_Collection*)(*env)->GetLongField(env, ls_set, cachedJNI.set_admin_fid);
    assert(collection);

    /* locks owner and target home, and next operation checks if collection, owner home and target homes are alive */
    DK_Collection_lockAll(collection);
    DK_Collection_us_checkAliveAll(collection, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    targetHome = DK_Collection_us_getTargetHome(collection);
    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(targetHome);
    elementClass = typedObjectCachedData->typedRoot_class;
    if(type == DDS_SET_ELEMENT_COLLECTION_TYPE_MAIN){
        valuesSet = DK_SetAdmin_us_getHolders((DK_SetAdmin*)collection);
        /* create & set the array */
        jelements = (*env)->NewObjectArray(env, Coll_Set_getNrOfElements(valuesSet), elementClass, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_Set_getFirstElement(valuesSet);
    } else if (type == DDS_SET_ELEMENT_COLLECTION_TYPE_ADDED){
        valuesList = DK_SetAdmin_us_getAddedElements((DK_SetAdmin*)collection, &exception);
        DLRL_Exception_PROPAGATE(&exception);
        /* create & set the array */
        jelements = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(valuesList), elementClass, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_List_getFirstElement(valuesList);
    } else {
        assert(type == DDS_SET_ELEMENT_COLLECTION_TYPE_REMOVED);
        valuesList = DK_Collection_us_getRemovedElements(collection);
        /* create & set the array */
        jelements = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(valuesList), elementClass, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_List_getFirstElement(valuesList);
    }
    while(iterator){
        holder = (DK_ObjectHolder*)Coll_Iter_getObject(iterator);
        /* holder is protected by the home mutex (of the owner home) */
        assert(DK_ObjectHolder_us_isResolved(holder));
        /* no duplicate needed, must be valid, protected by the home mutex */
        target = DK_ObjectHolder_us_getTarget(holder);
        /* must lock target object, if its no longer alive then treat it as a not found. otherwise */
        /* get the ls object (which is always valid on an alive object admin) and set it as an array  */
        /* element. Note that the object admin may never lock the collection */
        if(DK_ObjectAdmin_us_isAlive(target)){
            jobject jtarget = (jobject)DK_ObjectAdmin_us_getLSObject(target);
            (*env)->SetObjectArrayElement(env, jelements, count, jtarget);
            DLRL_JavaException_PROPAGATE(env, &exception);
        } else {/* an already deleted object becomes a not found exception */
            (*env)->SetObjectArrayElement(env, jelements, count, NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
        }
        /* TODO ID: 132 */
        count++;
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(&exception);
    DK_Collection_unlockAll(collection);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jelements;
}
