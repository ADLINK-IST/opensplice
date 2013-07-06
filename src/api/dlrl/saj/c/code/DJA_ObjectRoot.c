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
#include "DJA_ObjectRoot.h"

#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
/* DLRL JNI API includes */
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"

JNIEXPORT jobject JNICALL
Java_DDS_ObjectRoot_jniOid(
    JNIEnv * env,
    jobject ls_object)
{
    jobject joid = NULL;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    DK_ObjectID oid;/* on stack def */
    jintArray oidArray = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->oid()");
    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    DK_ObjectAdmin_ts_getObjectID(objectAdmin, &exception, &oid);
    DLRL_Exception_PROPAGATE(&exception);

    /* create the oid object and set the correct values */
    joid = (*env)->NewObject(env, cachedJNI.oid_class, cachedJNI.oid_constructor_mid);
    DLRL_JavaException_PROPAGATE(env, &exception);
    assert(joid);
    oidArray = (*env)->GetObjectField(env, joid, cachedJNI.oid_value_fid);
    assert(oidArray);
    (*env)->SetIntArrayRegion(env, oidArray, 0, 3, (jint*)oid.oid);
    DLRL_JavaException_PROPAGATE(env, &exception);
    (*env)->SetIntField(env, joid, cachedJNI.oid_systemId_fid, (jint)oid.oid[0]);
    (*env)->SetIntField(env, joid, cachedJNI.oid_localId_fid, (jint)oid.oid[1]);
    (*env)->SetIntField(env, joid, cachedJNI.oid_serial_fid, (jint)oid.oid[2]);

    DLRL_Exception_EXIT(&exception);
    if(oidArray){
        (*env)->DeleteLocalRef(env, oidArray);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return joid;
}

JNIEXPORT jint JNICALL
Java_DDS_ObjectRoot_jniHomeIndex(
    JNIEnv * env,
    jobject ls_object)
{
    jint index;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->home_index()");
    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    index = (jint)DK_ObjectAdmin_ts_getHomeIndex(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return index;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectRoot_jniGetInvalidRelations(
    JNIEnv * env,
    jobject ls_object)
{
    jobject jrelationsArray = NULL;
    Coll_List* invalidRelations = NULL;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    jstring jname = NULL;
    LOC_string name = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long count = 0;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->get_invalid_relations()");
    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);

    DK_ObjectAdmin_lockHome(objectAdmin);
    DK_ObjectAdmin_us_checkAlive(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    invalidRelations = DK_ObjectAdmin_us_getInvalidRelations(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    assert(invalidRelations);
    size = Coll_List_getNrOfElements(invalidRelations);
    jrelationsArray = (*env)->NewObjectArray(env, size, cachedJNI.string_class, NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);
    for(count = 0; count < size; count++){
        name = (LOC_string)Coll_List_popBack(invalidRelations);
        /* dont free the 'name', it is not a duplicate, but a direct pointer to the relation name in the kernel! */
        jname = (*env)->NewStringUTF(env, name);
        DLRL_JavaException_PROPAGATE(env, &exception);
        (*env)->SetObjectArrayElement(env, jrelationsArray, count, jname);
        if(jname){
            (*env)->DeleteLocalRef(env, jname);
        }
        DLRL_JavaException_PROPAGATE(env, &exception);
    }
    Coll_List_delete(invalidRelations);
    invalidRelations = NULL;

    DLRL_Exception_EXIT(&exception);
    if(invalidRelations){
        while(Coll_List_getNrOfElements(invalidRelations) > 0){
            Coll_List_popBack(invalidRelations);
        }
        Coll_List_delete(invalidRelations);
        invalidRelations = NULL;
    }
    DK_ObjectAdmin_unlockHome(objectAdmin);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jrelationsArray;
}

JNIEXPORT jint JNICALL
Java_DDS_ObjectRoot_jniReadState(
    JNIEnv * env,
    jobject ls_object)
{
    jint readState = 0;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->read_state()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    readState = (jint)DK_ObjectAdmin_ts_getReadState(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return readState;
}

JNIEXPORT jint JNICALL
Java_DDS_ObjectRoot_jniWriteState(
    JNIEnv * env,
    jobject ls_object)
{
    jint writeState = 0;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->write_state()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    writeState = (jshort)DK_ObjectAdmin_ts_getWriteState(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return writeState;
}

JNIEXPORT jstring JNICALL
Java_DDS_ObjectRoot_jniMaintopicName(
    JNIEnv * env,
    jobject ls_object)
{
    LOC_string mainTopicName = NULL;
    jstring retVal = NULL;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    DK_ObjectAdmin_lockHome(objectAdmin);
    DK_ObjectAdmin_us_checkAlive(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    mainTopicName = DK_ObjectAdmin_us_getMainTopicName(objectAdmin);
    assert(mainTopicName);
    retVal = (*env)->NewStringUTF(env, mainTopicName);
    DLRL_JavaException_PROPAGATE(env, &exception);

    DLRL_Exception_EXIT(&exception);
    DK_ObjectAdmin_unlockHome(objectAdmin);

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectRoot_jniObjectHome(
    JNIEnv * env,
    jobject ls_object)
{
    jobject jhome = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->home()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    jhome = DK_ObjectAdmin_ts_getLSHome(objectAdmin, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jhome;
}

JNIEXPORT jstring JNICALL
Java_DDS_ObjectRoot_jniClassName(
    JNIEnv * env,
    jobject ls_object)
{
    jstring jname = NULL;
    LOC_string name = NULL;
    DK_ObjectHomeAdmin* home;
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->class_name()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    home = DK_ObjectAdmin_ts_getHome(objectAdmin, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    if(home){
        DK_ObjectHomeAdmin_lockAdmin(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);
        name = DK_ObjectHomeAdmin_us_getName(home);
        jname = name ? (*env)->NewStringUTF(env, name) : NULL;
        DLRL_JavaException_PROPAGATE(env, &exception);
    }
    DLRL_Exception_EXIT(&exception);
    /* exception check is done outside the lock/unlock. Note that an exception check must always be done after doing a  */
    /* JNI call on the env. */
    if(home){
        DK_ObjectHomeAdmin_unlockAdmin(home);
        DK_Entity_ts_release((DK_Entity*)home);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jname;
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectRoot_jniOwner(
    JNIEnv * env,
    jobject ls_object)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    DK_CacheBase* cacheBase;
    DK_CacheKind kind;
    jobject jcacheBase= NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->owner()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    cacheBase = DK_ObjectAdmin_ts_getOwner(objectAdmin, &exception);/* must release */
    DLRL_Exception_PROPAGATE(&exception);
    assert(cacheBase);
    kind = DK_CacheBase_ts_getKind(cacheBase, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    if(kind == DK_CACHE_KIND_CACHE){
        jcacheBase = (jobject)DK_CacheAdmin_ts_getLSCache((DK_CacheAdmin*)cacheBase, &exception, (void*)env);
        DLRL_Exception_PROPAGATE(&exception);
    } else {
        jcacheBase = (jobject)DK_CacheAccessAdmin_ts_getLSAccess((DK_CacheAccessAdmin*)cacheBase, &exception, (void*)env);
        DLRL_Exception_PROPAGATE(&exception);
    }

    DLRL_Exception_EXIT(&exception);
    if(cacheBase){
        DK_Entity_ts_release((DK_Entity*)cacheBase);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jcacheBase;
}

JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniDestroy(
    JNIEnv * env,
    jobject ls_object)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->destroy()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    DK_ObjectAdmin_ts_destroy(objectAdmin, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jboolean JNICALL
Java_DDS_ObjectRoot_jniIsModified(
    JNIEnv * env,
    jobject ls_object,
    jint scope)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    jboolean isModified = FALSE;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->is_modified()");

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    isModified = (jboolean)DK_ObjectAdmin_ts_isModified(objectAdmin, &exception, (DK_ObjectScope)scope);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return isModified;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectRoot_jniWhichContainedModified(
    JNIEnv * env,
    jobject ls_object)
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->which_contained_modified() NOT SUPPORTED");
    /* DK_ObjectAdmin* objectAdmin = (DK_ObjectAdmin*)objectPointer; */

    DLRL_INFO(INF_EXIT);
    return NULL;
}

JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniDeleteObjectRoot(
    JNIEnv * env,
    jobject ls_object)
{
    DK_Entity* objectAdminEntity;

    DLRL_INFO(INF_ENTER);

    objectAdminEntity = (DK_Entity*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    if(objectAdminEntity){
        DK_Entity_ts_release(objectAdminEntity);
    }

    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniStateHasChanged(
    JNIEnv * env,
    jobject ls_object,
    jboolean jisImmutable)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);

    DK_ObjectAdmin_ts_stateHasChanged(objectAdmin, &exception, (void*)env, jisImmutable);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniChangeRelationship(
    JNIEnv * env,
    jobject ls_object,
    jint jindex,
    jobject ls_relatedObject)
{
    DK_ObjectAdmin* objectAdmin = NULL;
    DK_ObjectAdmin* relatedObjectAdmin = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);
    if(ls_relatedObject){
        relatedObjectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env,ls_relatedObject,cachedJNI.objectRoot_admin_fid);
    }
    DK_ObjectAdmin_ts_changeRelation(
        objectAdmin,
        &exception,
        (void*)env,
        (LOC_unsigned_long)jindex,
        relatedObjectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}


JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniChangeCollection__ILDDS_StrMap_2(
    JNIEnv * env,
    jobject ls_object,
    jint jindex,
    jobject ls_collection)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    DK_Collection* collection = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, ls_collection, "val");

    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);
    collection = (DK_Collection*)(*env)->GetLongField(env, ls_collection, cachedJNI.strMap_admin_fid);
    assert(collection);

    DK_ObjectAdmin_ts_changeCollection(
        objectAdmin,
        &exception,
        (void*)env,
        (LOC_unsigned_long)jindex,
        collection);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniChangeCollection__ILDDS_IntMap_2(
    JNIEnv * env,
    jobject ls_object,
    jint jindex,
    jobject ls_collection)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    DK_Collection* collection = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, ls_collection, "val");

    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);
    collection = (DK_Collection*)(*env)->GetLongField(env, ls_collection, cachedJNI.intMap_admin_fid);
    assert(collection);

    DK_ObjectAdmin_ts_changeCollection(objectAdmin, &exception, (void*)env, (LOC_unsigned_long)jindex, collection);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}


JNIEXPORT void JNICALL
Java_DDS_ObjectRoot_jniChangeCollection__ILDDS_Set_2(
    JNIEnv * env,
    jobject ls_object,
    jint jindex,
    jobject ls_collection)
{
    DK_ObjectAdmin* objectAdmin;
    DLRL_Exception exception;
    DK_Collection* collection = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, ls_collection, "val");

    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);
    collection = (DK_Collection*)(*env)->GetLongField(env, ls_collection, cachedJNI.set_admin_fid);
    assert(collection);

    DK_ObjectAdmin_ts_changeCollection(objectAdmin, &exception, (void*)env, (LOC_unsigned_long)jindex, collection);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectRoot_jniGet(
    JNIEnv * env,
    jobject ls_object,
    jint relationIndex)
{
 /*   DLRL_LS_object targetRoot = NULL;*/

    DLRL_INFO(INF_ENTER);

 /*   DLRL_Exception_init(&exception);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(env, ls_object, cachedJNI.objectRoot_admin_fid);
    assert(objectAdmin);*/
#ifndef NDEBUG
    printf("NDEBUG - Not implemented\n");
#endif

/*    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);*/
    DLRL_INFO(INF_EXIT);
    return NULL;
}
