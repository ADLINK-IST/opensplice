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

#include "DJA_SelectionImpl.h"

/* DLRL Kernel */
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

/* DLRL includes */
#include "DLRL_Types.h"
#include "DLRL_Exception.h"

/* DLRL JNI API includes */
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"

/* collection */
#include "Coll_List.h"

JNIEXPORT jboolean JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniAutoRefresh(
    JNIEnv * env,
    jobject ls_selection)
{
    jboolean jautoRefresh = FALSE;
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->auto_refresh()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    jautoRefresh = DK_SelectionAdmin_ts_getAutoRefresh(selection, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jautoRefresh;
}

/* called by the finalize in the java class to clean up the reference it holds to the C object */
JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniDeleteSelection(
    JNIEnv * env,
    jobject ls_selection)
{
    DK_Entity* selectionEntity;

    DLRL_INFO(INF_ENTER);

    selectionEntity = (DK_Entity*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    if(selectionEntity){
        DK_Entity_ts_release(selectionEntity);
    }
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniListener(
    JNIEnv * env,
    jobject ls_selection)
{
    jobject jlistener;
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->listener()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    jlistener = (jobject)DK_SelectionAdmin_ts_getListener(selection, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jlistener;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniSetListener(
    JNIEnv * env,
    jobject ls_selection,
    jobject jlistener)
{
    jobject retVal;
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->listener()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    retVal = (jobject)DK_SelectionAdmin_ts_setListener(
        selection,
        &exception,
        (void*)env,
        (DLRL_LS_object)jlistener);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT jboolean JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniConcernsContained(
    JNIEnv * env,
    jobject ls_selection)
{
    jboolean jconcernsContained = FALSE;
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->concerns_contained()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    jconcernsContained = DK_SelectionAdmin_ts_getConcernsContained(selection, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jconcernsContained;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniCriterion(
    JNIEnv * env,
    jobject ls_selection)
{
    jobject jcriterion = NULL;
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->criterion()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    jcriterion = (jobject)DK_SelectionAdmin_ts_getCriterion(
        selection,
        &exception,
        (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jcriterion;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniMembers(
    JNIEnv * env,
    jobject ls_selection)
{
    jobjectArray jmembers = NULL;
    LOC_unsigned_long count = 0;
    jclass elementClass = NULL;
    Coll_Set* members = NULL;
    Coll_Iter* iterator = NULL;
    jobject jaMember = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_SelectionAdmin* selection = NULL;
    DK_ObjectAdmin* aMember = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->members()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    DK_SelectionAdmin_lockHome(selection);

    DK_SelectionAdmin_us_checkAlive(selection, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    members = DK_SelectionAdmin_us_getMembers(selection);
    ownerHome = DK_SelectionAdmin_us_getOwnerHome(selection);

    DK_ObjectHomeAdmin_us_checkAlive(ownerHome, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(ownerHome);
    elementClass = typedObjectCachedData->typedRoot_class;
    jmembers = (*env)->NewObjectArray(env, Coll_Set_getNrOfElements(members), elementClass, NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);
    iterator = Coll_Set_getFirstElement(members);
    count = 0;
    while(iterator){
        aMember = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        assert(DK_ObjectAdmin_us_getLSObject(aMember));
        jaMember = (jobject)DK_ObjectAdmin_us_getLSObject(aMember);
        (*env)->SetObjectArrayElement(env, jmembers, count, jaMember);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(&exception);
    DK_SelectionAdmin_unlockHome(selection);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jmembers;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniInsertedMembers(
    JNIEnv * env,
    jobject ls_selection)
{
    jobjectArray jmembers = NULL;
    LOC_unsigned_long count = 0;
    jclass elementClass = NULL;
    Coll_List* members = NULL;
    Coll_Iter* iterator = NULL;
    jobject jaMember = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_SelectionAdmin* selection = NULL;
    DK_ObjectAdmin* aMember = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->members()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);
    DK_SelectionAdmin_lockHome(selection);
    DK_SelectionAdmin_us_checkAlive(selection, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    ownerHome = DK_SelectionAdmin_us_getOwnerHome(selection);

    DK_ObjectHomeAdmin_us_checkAlive(ownerHome, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    members = DK_SelectionAdmin_us_getInsertedMembers(selection);

    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(ownerHome);
    elementClass = typedObjectCachedData->typedRoot_class;
    jmembers = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(members), elementClass, NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);
    iterator = Coll_List_getFirstElement(members);
    count = 0;
    while(iterator){
        aMember = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        assert(DK_ObjectAdmin_us_getLSObject(aMember));
        jaMember = (jobject)DK_ObjectAdmin_us_getLSObject(aMember);
        (*env)->SetObjectArrayElement(env, jmembers, count, jaMember);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(&exception);
    DK_SelectionAdmin_unlockHome(selection);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jmembers;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniModifiedMembers(
    JNIEnv * env,
    jobject ls_selection)
{
    jobjectArray jmembers = NULL;
    LOC_unsigned_long count = 0;
    jclass elementClass = NULL;
    Coll_List* members = NULL;
    Coll_Iter* iterator = NULL;
    jobject jaMember = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_SelectionAdmin* selection = NULL;
    DK_ObjectAdmin* aMember = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->members()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    DK_SelectionAdmin_lockHome(selection);

    DK_SelectionAdmin_us_checkAlive(selection, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    ownerHome = DK_SelectionAdmin_us_getOwnerHome(selection);

    DK_ObjectHomeAdmin_us_checkAlive(ownerHome, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    members = DK_SelectionAdmin_us_getModifiedMembers(selection);

    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(ownerHome);
    elementClass = typedObjectCachedData->typedRoot_class;
    jmembers = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(members), elementClass, NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);
    iterator = Coll_List_getFirstElement(members);
    count = 0;
    while(iterator){
        aMember = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        assert(DK_ObjectAdmin_us_getLSObject(aMember));
        jaMember = (jobject)DK_ObjectAdmin_us_getLSObject(aMember);
        (*env)->SetObjectArrayElement(env, jmembers, count, jaMember);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(&exception);
    DK_SelectionAdmin_unlockHome(selection);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jmembers;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniRemovedMembers(
    JNIEnv * env,
    jobject ls_selection)
{
    jobjectArray jmembers = NULL;
    LOC_unsigned_long count = 0;
    jclass elementClass = NULL;
    Coll_List* members = NULL;
    Coll_Iter* iterator = NULL;
    jobject jaMember = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DK_SelectionAdmin* selection = NULL;
    DK_ObjectAdmin* aMember = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->members()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);
    assert(selection);

    DK_SelectionAdmin_lockHome(selection);

    DK_SelectionAdmin_us_checkAlive(selection, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    ownerHome = DK_SelectionAdmin_us_getOwnerHome(selection);

    DK_ObjectHomeAdmin_us_checkAlive(ownerHome, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    members = DK_SelectionAdmin_us_getRemovedMembers(selection);

    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(ownerHome);
    elementClass = typedObjectCachedData->typedRoot_class;
    jmembers = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(members), elementClass, NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);
    iterator = Coll_List_getFirstElement(members);
    count = 0;
    while(iterator){
        aMember = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        assert(DK_ObjectAdmin_us_getLSObject(aMember));
        jaMember = (jobject)DK_ObjectAdmin_us_getLSObject(aMember);
        (*env)->SetObjectArrayElement(env, jmembers, count, jaMember);
        DLRL_JavaException_PROPAGATE(env, &exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(&exception);
    DK_SelectionAdmin_unlockHome(selection);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jmembers;
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_SelectionImpl_jniRefresh(
    JNIEnv * env,
    jobject ls_selection)
{
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "selection->refresh()");

    DLRL_Exception_init(&exception);
    selection = (DK_SelectionAdmin*)(*env)->GetLongField(env, ls_selection, cachedJNI.selection_admin_fid);

    DK_SelectionAdmin_ts_refresh(selection, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

