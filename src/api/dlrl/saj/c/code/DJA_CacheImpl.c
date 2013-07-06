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

#include "DJA_CacheImpl.h"

/* DLRL Kernel */
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

/* DLRL includes */
#include "DLRL_Types.h"
#include "DLRL_Exception.h"

/* collection includes */
#include "Coll_Set.h"

/* DLRL JNI API includes */
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"

/* include DCPS SAJ code */
#include "saj_utilities.h"

JNIEXPORT jint JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniCacheUsage(
    JNIEnv * env,
    jobject ls_cache)
{
    jint jusage = 0;
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->get_cache_usage()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);

    jusage = (jint)DK_CacheAdmin_ts_getCacheUsage(cache, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jusage;
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniDeleteCache(
    JNIEnv * env,
    jobject ls_cache)
{
    DK_Entity* cacheEntity;

    DLRL_INFO(INF_ENTER);

    cacheEntity = (DK_Entity*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    if(cacheEntity){
        DK_Entity_ts_release(cacheEntity);
    }
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jint JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniPubsubState(
    JNIEnv * env,
    jobject ls_cache)
{
    jint jpubSubState;
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->get_pub_sub_state()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);

    jpubSubState = (jint)DK_CacheAdmin_ts_getPubSubState(cache, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jpubSubState;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniTheParticipant(
    JNIEnv * env,
    jobject ls_cache)
{
    jobject jparticipant = NULL;
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->the_participant()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jparticipant = (jobject)DK_CacheAdmin_ts_getLSParticipant(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jparticipant;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniThePublisher(
    JNIEnv * env,
    jobject ls_cache)
{
    jobject jpublisher = NULL;
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->the_publisher()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jpublisher = (jobject)DK_CacheAdmin_ts_getLSPublisher(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jpublisher;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniTheSubscriber(
    JNIEnv * env,
    jobject ls_cache)
{
    jobject jsubscriber = NULL;
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->the_subscriber()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jsubscriber = (jobject)DK_CacheAdmin_ts_getLSSubscriber(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jsubscriber;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniHomes(
    JNIEnv * env,
    jobject ls_cache)
{
    jobjectArray jhomes = NULL;
    DLRL_Exception exception;
    DK_CacheAdmin* cache = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->get_homes()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    DK_CacheAdmin_ts_getLSHomes(cache, &exception, (void*)env, (void**)&jhomes);
    DLRL_Exception_PROPAGATE(&exception);
    if(!jhomes){
        jhomes = (*env)->NewObjectArray(env, 0, cachedJNI.objectHome_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }
    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jhomes;
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniRegisterAllForPubsub(
    JNIEnv * env,
    jobject ls_cache)
{
    DK_CacheAdmin* cache = NULL;
    Coll_List* homes = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* aHome = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->register_all_for_pub_sub()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);

    DK_CacheAdmin_ts_registerAllForPubSub(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);
    /* need to cache some java specific JNI information. Can only be done after the register all for */
    /* pub sub has been completed. */
    DK_CacheAdmin_lockAdministrative(cache);
    DK_CacheAdmin_us_checkAlive(cache, &exception);
    if(exception.exceptionID == DLRL_NO_EXCEPTION){
        homes = DK_CacheAdmin_us_getHomes(cache);
        iterator = Coll_List_getFirstElement(homes);
        while(iterator){/* lock everything, ignore the exception in the alive check. */
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_lockAdmin(aHome);
            DK_ObjectHomeAdmin_us_checkAlive(aHome, &exception);
            iterator = Coll_Iter_getNext(iterator);
        }
        if(exception.exceptionID == DLRL_NO_EXCEPTION){
            /* this method requires all homes to be locked. */
            DJA_Initialisation_cacheRelationFields(&exception, env, cache);
        }

        iterator = Coll_List_getFirstElement(homes);
        while(iterator){
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_unlockAdmin(aHome);
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    DK_CacheAdmin_unlockAdministrative(cache);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniEnableAllForPubsub(
    JNIEnv * env,
    jobject ls_cache)
{
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->enable_all_for_pub_sub()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);

    DK_CacheAdmin_ts_enableAllForPubSub(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jint JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniRegisterHome(
    JNIEnv * env,
    jobject ls_cache,
    jobject jhome)
{
    DLRL_LS_object oldVal = NULL;
    jint registeredIndex = -1;
    jobject jglobalizedHome= NULL;
    DK_ObjectHomeAdmin* home;
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->register_home(home)");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jhome, "a_home");

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(env, jhome, cachedJNI.objectHome_admin_fid);
    if (!home) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "a_home parameter is corrupted.");
    }

    jglobalizedHome = (*env)->NewGlobalRef(env, jhome);
    if (!jglobalizedHome) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY, "Not enough memory to complete operation.");
    }

    registeredIndex = (jint) DK_CacheAdmin_ts_registerHome(cache, &exception, home);
    DLRL_Exception_PROPAGATE(&exception);

    /* if the home was registered successfully the weak global ref we made during object home creation can be */
    /* replaced by a normal global ref as the DLRL cache now maintains a reference to the kernel object home and will */
    /* call it's delete function which will correctly clean the normal global ref. For more details see function: */
    /* Java_DDS_ObjectHome_jniConstructObjectHome */
    oldVal = DK_ObjectHomeAdmin_ts_registerLSObjectHome(home, &exception, (DLRL_LS_object)jglobalizedHome);
    DLRL_Exception_PROPAGATE(&exception);

    assert(oldVal);
    (*env)->DeleteWeakGlobalRef(env, (jobject)oldVal);

    DLRL_Exception_EXIT(&exception);
    if (!oldVal && jglobalizedHome) {
        (*env)->DeleteGlobalRef(env, jglobalizedHome);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return (jint) registeredIndex;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniFindHomeByName(
    JNIEnv * env,
    jobject ls_cache,
    jstring jhomeName)
{
    jobject jhome = NULL;
    DK_CacheAdmin* cache;
    DLRL_Exception exception;
    LOC_const_string homeName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->find_home_by_name(...)");

    DLRL_Exception_init(&exception);

    homeName = jhomeName ? (LOC_const_string)(*env)->GetStringUTFChars(env, jhomeName, 0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jhomeName, "class_name");

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jhome = (jobject)DK_CacheAdmin_ts_findLSHomeByName(cache, &exception, (void*)env, homeName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(homeName){
        (*env)->ReleaseStringUTFChars(env, jhomeName, homeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jhome;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniFindHomeByIndex(
    JNIEnv * env,
    jobject ls_cache,
    jint jindex)
{
    jobject jhome = NULL;
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->find_home_by_index(%d)", (LOC_long)jindex);

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jhome = (jobject)DK_CacheAdmin_ts_findLSHomeByIndex(cache, &exception, (void*)env, (LOC_long)jindex);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jhome;
}

JNIEXPORT jboolean JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniUpdatesEnabled(
    JNIEnv *env,
    jobject ls_cache)
{
    DK_CacheAdmin* cache;
    jboolean updatesEnabled = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->updates_enabled()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    updatesEnabled = (jboolean)DK_CacheAdmin_ts_getUpdatesEnabled(cache, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return updatesEnabled;
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniListeners(
    JNIEnv *env,
    jobject ls_cache)
{
    DK_CacheAdmin* cache;
    jobjectArray jlisteners = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->listeners()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    DK_CacheAdmin_ts_getListeners(cache, &exception, (void*)env, (void**)&jlisteners);
    DLRL_Exception_PROPAGATE(&exception);
    if(!jlisteners){
        jlisteners = (*env)->NewObjectArray(env, 0, cachedJNI.cacheListener_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jlisteners;
}

JNIEXPORT jboolean JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniAttachListener(
    JNIEnv *env,
    jobject ls_cache,
    jobject jlistener)
{
    jobject jlistenerGlobalRef;
    DK_CacheAdmin* cache;
    jboolean succeeded = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->attach_listener(listener)");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jlistener, "listener");

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jlistenerGlobalRef = (*env)->NewGlobalRef (env, jlistener);
    /*  Ensure jlistenerGlobalRef is created (NULL pointer if not enough memory). */
    if (!jlistenerGlobalRef) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY, "Not enough memory to complete operation.");
    }

    succeeded = (jboolean)DK_CacheAdmin_ts_attachListener(cache, &exception, (void*)env,
                                                            (DLRL_LS_object)jlistenerGlobalRef);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return succeeded;
}

JNIEXPORT jboolean JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniDetachListener(
    JNIEnv *env,
    jobject ls_cache,
    jobject jlistener)
{
    DK_CacheAdmin* cache;
    jboolean succeeded = FALSE;
    jobject jlistenerGlobalRef = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->detach_listener(listener)");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jlistener, "listener");

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    jlistenerGlobalRef = DK_CacheAdmin_ts_detachListener(cache, &exception, (void*)env,
                                                                (DLRL_LS_object)jlistener);
    DLRL_Exception_PROPAGATE(&exception);

    if(jlistenerGlobalRef){
        (*env)->DeleteGlobalRef(env, jlistenerGlobalRef);
        succeeded = TRUE;
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return succeeded;
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniEnableUpdates(
    JNIEnv *env,
    jobject ls_cache)
{
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->enable_updates()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    DK_CacheAdmin_ts_enableUpdates(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniDisableUpdates(
    JNIEnv *env,
    jobject ls_cache)
{
    DLRL_Exception exception;
    DK_CacheAdmin* cache;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->disable_updates()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    DK_CacheAdmin_ts_disableUpdates(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniSubAccesses(
    JNIEnv *env,
    jobject ls_cache)
{
    DK_CacheAdmin* cache;
    DLRL_Exception exception;
    jobjectArray jaccesses = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->sub_accesses()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    DK_CacheAdmin_ts_getLSAccesses(cache, &exception, (void*)env, (void**)&jaccesses);
    DLRL_Exception_PROPAGATE(&exception);
    if(!jaccesses){
        jaccesses = (*env)->NewObjectArray(env, 0, cachedJNI.cacheAccess_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jaccesses;
}

JNIEXPORT jobject JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniCreateAccess(
    JNIEnv * env,
    jobject ls_cache,
    jobject jcacheAccess,
    jint usage)
{
    DLRL_Exception exception;
    DK_CacheAdmin* cache = NULL;
    DK_CacheAccessAdmin* access = NULL;
    jobject globalCacheAccess = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->create_access()");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    assert(jcacheAccess);

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    globalCacheAccess = (*env)->NewGlobalRef(env, jcacheAccess);
   /*  Ensure globalCacheAccess is created (NULL pointer if not enough memory). */
    if (!globalCacheAccess) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY, "Not enough memory to complete operation.");
    }

    access = DK_CacheAdmin_ts_createAccess(cache, &exception, (void*)env, globalCacheAccess, (DK_Usage)usage);
    DLRL_Exception_PROPAGATE(&exception);
    (*env)->SetLongField(env, jcacheAccess, cachedJNI.cacheAccess_admin_fid, (jlong)access);

    DLRL_Exception_EXIT(&exception);
    if((exception.exceptionID != DLRL_NO_EXCEPTION) && globalCacheAccess){
        (*env)->DeleteGlobalRef(env, globalCacheAccess);
        globalCacheAccess = NULL;
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return globalCacheAccess;
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniDeleteAccess(
    JNIEnv * env,
    jobject ls_cache,
    jobject cacheAccess)
{
    DLRL_Exception exception;
    DK_CacheAdmin* cache;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->delete_access()");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, cacheAccess, "access");

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(env, cacheAccess, cachedJNI.cacheAccess_admin_fid);
    if (!access) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER, "access parameter is corrupted.");
    }

    DK_CacheAdmin_ts_deleteAccess(cache, &exception, (void*)env, access);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniRefresh(
    JNIEnv *env,
    jobject ls_cache)
{
    DLRL_Exception exception;
    DK_CacheAdmin* cache;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "cache->refresh()");

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);

    DK_CacheAdmin_ts_refresh(cache, &exception, (void*)env);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobjectArray JNICALL
Java_org_opensplice_dds_dlrl_CacheImpl_jniObjects(
    JNIEnv *env,
    jobject ls_cache)
{
    DLRL_Exception exception;
    DK_CacheAdmin* cache;
    jobjectArray array = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, ls_cache, cachedJNI.cache_admin_fid);
    assert(cache);
    DK_CacheAdmin_ts_getObjects(cache, &exception, (void*)env, (void**)&array);
    if(!array){
        array = (*env)->NewObjectArray(env, 0, cachedJNI.objectRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}
