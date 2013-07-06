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
#include "DJA_CacheAccessImpl.h"

/* DLRL includes */
#include "DLRL_Exception.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DLRL_Report.h"

/* DLRL JNI API includes */
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniOwner(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;
    jobject owner = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->owner()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    owner= (jobject)DK_CacheAccessAdmin_ts_getLSOwner(
        access,
        &exception,
        (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return owner;
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniWrite(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->write()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    DK_CacheAccessAdmin_ts_write(access, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniPurge(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->purge()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    DK_CacheAccessAdmin_ts_purge(access, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniDeleteCacheAccess(
    JNIEnv * env,
    jobject ls_access)
{
    DK_Entity* cacheAccessEntity;

    DLRL_INFO(INF_ENTER);

    cacheAccessEntity = (DK_Entity*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    if(cacheAccessEntity){
        DK_Entity_ts_release(cacheAccessEntity);
    }
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniObjects(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;
    jobjectArray objectsArray = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->objects()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    /* during the action callback the cache access of the element will be
     * locked.
     */
    DK_CacheAccessAdmin_ts_getObjects(
        access,
        &exception,
        (void*)env,
        (void**)&objectsArray);
    DLRL_Exception_PROPAGATE(&exception);
    if(!objectsArray){
        objectsArray = (*env)->NewObjectArray(
            env,
            0,
            cachedJNI.objectRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return objectsArray;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniGetInvalidObjects(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;
    jobjectArray jinvalidObjectsArray = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    DK_CacheAccessAdmin_ts_getInvalidObjects(
        access,
        &exception,
        (void*)env,
        (void**)&jinvalidObjectsArray);
    if(!jinvalidObjectsArray){
        jinvalidObjectsArray = (*env)->NewObjectArray(
            env,
            0,
            cachedJNI.objectRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jinvalidObjectsArray;
}

JNIEXPORT jintArray JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniContainedTypes(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;
    jintArray jintElements = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->contained_types()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    DK_CacheAccessAdmin_ts_getContainedTypes(
        access,
        &exception,
        (void*)env,
        (void**)(&jintElements));
    DLRL_Exception_PROPAGATE(&exception);
    if(!jintElements){
        jintElements = (*env)->NewIntArray(env, 0);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jintElements;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniTypeNames(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;
    jobjectArray array = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->type_names()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);

    DK_CacheAccessAdmin_ts_getContainedTypeNames(
        access,
        &exception,
        (void*)env,
        (void**)&array);
    DLRL_Exception_PROPAGATE(&exception);
    if(!array){
        array = (*env)->NewObjectArray(env, 0, cachedJNI.string_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}

/* following operations are not yet supported. */
JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniRefresh(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->refresh()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);
    DLRL_INFO(INF, "Refresh of a CacheAccess is not supported, no-op");

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniContracts(
    JNIEnv * env,
    jobject ls_access)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->contracts()");

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);
    DLRL_INFO(INF, "Querying the contracts of a CacheAccess is not supported, no-op");

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return NULL;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniCreateContract(
    JNIEnv * env,
    jobject ls_access,
    jobject ls_object,
    jint scope,
    jint jdepth)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;
    jobject ls_contract = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->create_contract()");
    assert(ls_object);

    DLRL_Exception_init(&exception);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(
        env,
        ls_object,
        cachedJNI.objectRoot_admin_fid);
    if (!objectAdmin) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER,
            "'object' parameter is corrupted.");
    }

    /*ls_contract = (jobject)DK_CacheAccessAdmin_ts_createLSContract(
        access,
        &exception,
        (void*)env,
        objectAdmin,
        (DK_ObjectScope)scope,
        (LOC_long)depth);*/
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return ls_contract;
}

/*DLRL_LS_object DK_CacheAccessAdmin_ts_createLSContract(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object,
    DK_ObjectScope scope,
    LOC_long depth)
{
    DK_Contract* contract;
    DLRL_LS_object lsContract = NULL;
    DK_Contract tmpContract;//on stack def, using for searching only
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    // userdata may be null
    assert(object);
    assert(scope < DK_ObjectScope_elements);
    assert(depth => -1);

    DK_CacheAccessAdmin_lock(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED, "TODO");
    }
    tmpContract.object = object;
    if(Coll_Set_contains(_this->contracts, &tmpContract))
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_EXISTING, "TODO");
    }
    contract = DK_Contract_new(exception, object, scope, depth);
    DLRL_Exception_PROPAGATE(exception);
    errorCode = Coll_Set_add(_this->contracts, contract);
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to allocate");
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return lsContract;
}*/

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheAccessImpl_jniDeleteContract(
    JNIEnv * env,
    jobject ls_access,
    jobject ls_contract)
{
    DLRL_Exception exception;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheAccess->delete_contract()");

    DLRL_Exception_init(&exception);
    DLRL_VERIFY_NOT_NULL(&exception, ls_contract,"contract");

    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    assert(access);
    /* check the contract if it is corrupted. */
    DLRL_INFO(INF, "Deleting a contract for a CacheAccess is not supported, no-op");

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}
