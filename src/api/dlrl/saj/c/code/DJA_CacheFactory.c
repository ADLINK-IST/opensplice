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
#include "DJA_CacheFactory.h"

/* DLRL includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
/* include DLRL SAJ code */
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"
#include "gapi_common.h"
#include "gapi_object.h"
#include "gapi_domainParticipant.h"
#include "gapi_entity.h"

/* include DCPS SAJ code (to be replaced with facade) */
#include "saj_utilities.h"

JNIEXPORT jlong
JNICALL Java_DDS_CacheFactory_jniCreateCache(
    JNIEnv * env,
    jobject ls_cacheFactory,
    jobject ls_cache,
    jint purpose,
    jstring jcacheName,
    jobject jparticipant)
{
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    DLRL_LS_object ls_participant;
    jobject globalizedCache;
    gapi_domainParticipant gapiParticipant = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DomainParticipant _participant = NULL;
    u_participant uparticipant= NULL;
    LOC_const_string cacheName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheFactory->create_cache(...)");
    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    assert(ls_cache);
    DLRL_VERIFY_NOT_NULL(&exception, jcacheName, "name");
    DLRL_VERIFY_NOT_NULL(&exception, jparticipant, "domain");

    cacheName = (LOC_const_string)(*env)->GetStringUTFChars(env, jcacheName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    globalizedCache = (*env)->NewGlobalRef(env, ls_cache);
    /*  Ensure globalizedCache is created (NULL pointer if not enough memory). */
    if (!globalizedCache) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
                             "Not enough memory to complete operation.");
    }

    ls_participant = (*env)->NewGlobalRef(env, jparticipant);
    /*  Ensure ls_participant is created (NULL pointer if not enough memory). */
    if (!ls_participant) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
                             "Not enough memory to complete operation.");
    }

    gapiParticipant = (gapi_domainParticipant)saj_read_gapi_address(env, jparticipant);
    _participant = gapi_domainParticipantClaim(gapiParticipant, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(&exception, result,
                                         "Failed to claim the domainparticipant handle");
    uparticipant = _DomainParticipantUparticipant(_participant);
    if(uparticipant)
    {
        /* now create a proxy to this user layer participant which can be used by the DLRL
         * in a safe manner, as the user layer participant returned by the _DomainParticipantUparticipant
         * operation is owned by the gapi.
         */
        uparticipant = u_participant(DK_DCPSUtility_ts_createProxyUserEntity(&exception, u_entity(uparticipant)));
    }
    _EntityRelease(_participant);/* before the propagate */
    DLRL_Exception_PROPAGATE(&exception);/* after the release */
    cache = DK_CacheFactoryAdmin_ts_createCache(&exception,
                                                (DLRL_LS_object)globalizedCache,
                                                (void*)env, purpose,
                                                cacheName,
                                                uparticipant,
                                                ls_participant);
    DLRL_Exception_PROPAGATE(&exception);
    ls_participant = NULL;
    globalizedCache = NULL;

    (*env)->SetLongField(env, ls_cache, cachedJNI.cache_admin_fid, (jlong)cache);

    DLRL_Exception_EXIT(&exception);

    if(globalizedCache){
        (*env)->DeleteGlobalRef(env, globalizedCache);
    }
    if(ls_participant){
     /*TODO this is not correct, kernel doesn;t yet create it;s own global ref   (*env)->DeleteGlobalRef(env, ls_participant);*/
    }
    if (cacheName) {
        (*env)->ReleaseStringUTFChars(env, jcacheName, cacheName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return (jlong)cache;
}

JNIEXPORT jobject
JNICALL Java_DDS_CacheFactory_jniFindCacheByName (
    JNIEnv * env,
    jobject ls_cacheFactory,
    jstring jcacheName)
{
    jobject jCache = NULL;
    DK_CacheAdmin* cache = NULL;
    DLRL_Exception exception;
    LOC_const_string cacheName = NULL;

    DLRL_Exception_init(&exception);

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheFactory->find_cache_by_name(...)");

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jcacheName, "name");

    cacheName = (LOC_const_string)(*env)->GetStringUTFChars(env, jcacheName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    cache = DK_CacheFactoryAdmin_ts_findCachebyName(cacheName);
    if(cache){
        jCache = (jobject)DK_CacheAdmin_ts_getLSCache(cache, &exception, (void*)env);
        DLRL_Exception_PROPAGATE(&exception);
    }

    DLRL_Exception_EXIT(&exception);
    if(cache){
        DK_Entity_ts_release((DK_Entity*)cache);
    }
    if (cacheName) {
        (*env)->ReleaseStringUTFChars(env, jcacheName, cacheName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jCache;
}

JNIEXPORT void
JNICALL Java_DDS_CacheFactory_jniDeleteCache (
    JNIEnv * env,
    jobject ls_cacheFactory,
    jobject jcache)
{
    DK_CacheAdmin* cache;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheFactory->delete_cache(cache)");

    DLRL_Exception_init(&exception);
    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jcache, "a_cache");

    cache = (DK_CacheAdmin*)(*env)->GetLongField(env, jcache, cachedJNI.cache_admin_fid);
    if (!cache) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER,
                             "a_cache parameter is corrupted.");
    }
    DK_CacheFactoryAdmin_ts_deleteCache(&exception, (void*)env, cache);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jlong
JNICALL Java_DDS_CacheFactory_jniGetInstance (
    JNIEnv * env,
    jclass ls_cacheFactory)
{
    jlong returnVal = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "CacheFactory->get_instance()");

    DLRL_Exception_init(&exception);
    returnVal = (jlong)DK_CacheFactoryAdmin_ts_getInstance(&exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return returnVal;
}

JNIEXPORT void
JNICALL Java_DDS_CacheFactory_jniRegisterLSRepresentative (
    JNIEnv * env,
    jobject ls_cacheFactory)
{
    DLRL_Exception exception;
    jobject globalizedFactory;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    globalizedFactory = (*env)->NewGlobalRef(env, ls_cacheFactory);
    if (!globalizedFactory) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
                             "Not enough memory to complete operation.");
    }

    DK_CacheFactoryAdmin_ts_registerLSCacheFactory((DLRL_LS_object)globalizedFactory);

    DLRL_Exception_EXIT(exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}
