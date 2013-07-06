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
#include <stdio.h>
#include <assert.h>

/* jni includes */
#include <jni.h>

/* DCPS SAJ includes */
#include "saj_utilities.h"

/* DCPS GAPI includes */
#include "gapi.h"
#include "gapi_common.h"
#include "gapi_object.h"
#include "gapi_publisher.h"
#include "gapi_subscriber.h"
#include "gapi_entity.h"
/* DLRL SAJ includes */
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"

/* DLRL kernel includes */
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_CacheBridge.h"

static void
DJA_CacheBridge_us_triggerCacheListeners(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData,
    jmethodID triggerMethod);

u_publisher
DJA_CacheBridge_us_createPublisher(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object* ls_publisher)
{
    /* JNI thread env */
    JNIEnv * env;
    /* Cached JNI data used within this call */
    jclass jcache = cachedJNI.cache_class;
    jmethodID createPublisherMid = cachedJNI.cache_createPublisher_mid;
    /* needed vars for java representations of participant of publisher */
    jobject jParticipant;
    jobject jpublisher = NULL;
    gapi_publisher gapiPublisher = NULL;
    _Publisher _publisher = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_publisher upublisher = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(participant);
    assert(ls_participant);

    env = (JNIEnv *)userData;
    jParticipant = (jobject)ls_participant;

    DLRL_INFO(INF_CALLBACK, "cache->createPublisher(participant)");
    jpublisher = (*env)->CallStaticObjectMethod(env, jcache, createPublisherMid, jParticipant);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_INFO(INF_DCPS, "saj_read_gapi_address(publisher)");
    gapiPublisher = jpublisher? (gapi_publisher)saj_read_gapi_address(env, jpublisher) : NULL;
    if(!gapiPublisher){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Unable to create a publisher within "
                             "DK_CacheAdmin due to a DCPS error."
                             "Check DCPS error log file for (possibly) "
                             "more information.");
    }
    _publisher = gapi_publisherClaim(gapiPublisher, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result,
                                         "Failed to claim the publisher handle");
    upublisher = _PublisherUpublisher(_publisher);
    if(upublisher)
    {
        /* now create a proxy to this user layer upublisher which can be used by the DLRL
         * in a safe manner, as the user layer upublisher returned by the _PublisherUpublisher
         * operation is owned by the gapi.
         */
        upublisher = u_publisher(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(upublisher)));
    }
    _EntityRelease(_publisher);/* before the propagate */
    DLRL_Exception_PROPAGATE(exception);/* after the release */
    if(!upublisher){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
            "Unable to create a publisher within DK_CacheAdmin due to a DCPS error. "
            "Check DCPS error log file for (possibly) more information.");
    }
    *ls_publisher = jpublisher ? (void*)(*(env))->NewGlobalRef (env, jpublisher) : NULL;
    if(!(*ls_publisher))
    {
        u_entityFree(u_entity(upublisher));
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to create a language specific publisher within DK_CacheAdmin");
    }
    DLRL_Exception_EXIT(exception);
    if(jpublisher){
        (*env)->DeleteLocalRef(env, jpublisher);
    }
    DLRL_INFO(INF_EXIT);
    return upublisher;
}

u_subscriber
DJA_CacheBridge_us_createSubscriber(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object* ls_subscriber)
{
    /* JNI thread env */
    JNIEnv * env;
    /* Cached JNI data used within this call */
    jclass jcache = cachedJNI.cache_class;
    jmethodID createSubscriberMid = cachedJNI.cache_createSubscriber_mid;
    /* needed vars for java representations of participant of subscriber */
    jobject jParticipant;
    jobject jsubscriber = NULL;
    gapi_subscriber gapiSubscriber = NULL;
    _Subscriber _subscriber = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_subscriber usubscriber = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(participant);

    env = (JNIEnv *)userData;
    /* retrieve the Java domain partcipant based upon the user data in the C domain participant. */
    jParticipant = (jobject)ls_participant;

    DLRL_INFO(INF_CALLBACK, "cache->createSubscriber(participant)");
    jsubscriber = jParticipant ? (*env)->CallStaticObjectMethod(env, jcache, createSubscriberMid, jParticipant) : NULL;
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_INFO(INF_DCPS, "saj_read_gapi_address(subscriber)");
    gapiSubscriber = jsubscriber? (gapi_subscriber)saj_read_gapi_address(env, jsubscriber) : NULL;
    if(!gapiSubscriber){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
            "Unable to create a subscriber within DK_CacheAdmin due to a DCPS error. "
            "Check DCPS error log file for (possibly) more information.");
    }
    _subscriber = gapi_subscriberClaim(gapiSubscriber, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result,
                                         "Failed to claim the subscriber handle");
    usubscriber = _SubscriberUsubscriber(_subscriber);
    if(usubscriber)
    {
        /* now create a proxy to this user layer usubscriber which can be used by the DLRL
         * in a safe manner, as the user layer topic returned by the _SubscriberUsubscriber
         * operation is owned by the gapi.
         */
        usubscriber = u_subscriber(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(usubscriber)));
    }
    _EntityRelease(_subscriber);/* before the propagate */
    DLRL_Exception_PROPAGATE(exception);/* after the release */
    if(!usubscriber){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
            "Unable to create a subscriber within DK_CacheAdmin due to a DCPS error. "
            "Check DCPS error log file for (possibly) more information.");
    }
    *ls_subscriber = jsubscriber ? (void*)(*(env))->NewGlobalRef (env, jsubscriber) : NULL;
    if(!(*ls_subscriber))
    {
        u_entityFree(u_entity(usubscriber));
        usubscriber = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to create a language specific subscriber within DK_CacheAdmin");
    }
    DLRL_Exception_EXIT(exception);
    if(jsubscriber){
        (*env)->DeleteLocalRef(env, jsubscriber);
    }
    DLRL_INFO(INF_EXIT);
    return usubscriber;
}

void
DJA_CacheBridge_us_deletePublisher(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object ls_publisher)
{
    /* JNI thread env */
    JNIEnv * env;
    /* Cached JNI data used within this call */
    jclass jcache = cachedJNI.cache_class;
    jmethodID deletePublisherMid = cachedJNI.cache_deletePublisher_mid;
    /* needed vars for java representations of participant of publisher */
    jobject jparticipant;
    LOC_ReturnCode_t returnCode = LOC_RETCODE_OK;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(participant);
    assert(ls_publisher);

    env = (JNIEnv *)userData;
    /* retrieve the Java domain partcipant & publisher based upon the user data in the C domain participant/publisher */
    jparticipant = (jobject)ls_participant;

    DLRL_INFO(INF_CALLBACK, "deletePublisher(participant, publisher)");
    returnCode = (LOC_ReturnCode_t)(*env)->CallStaticIntMethod(env, jcache, deletePublisherMid, jparticipant,
                                                                (jobject)ls_publisher);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode, "Delete of publisher failed");

    (*env)->DeleteGlobalRef(env, (jobject)ls_publisher);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_deleteSubscriber(
    DLRL_Exception* exception,
    void* userData, u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object ls_subscriber)
{
    /* JNI thread env */
    JNIEnv * env;
    /* Cached JNI data used within this call */
    jclass jcache = cachedJNI.cache_class;
    jmethodID deleteSubscriberMid = cachedJNI.cache_deleteSubscriber_mid;
    /* needed vars for java representations of participant of subscriber */
    jobject jparticipant;
    LOC_ReturnCode_t returnCode = LOC_RETCODE_OK;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(participant);
    assert(ls_subscriber);

    env = (JNIEnv *)userData;
    /* retrieve the Java domain partcipant based upon the user data in the C domain participant. */
    jparticipant = (jobject)ls_participant;

    DLRL_INFO(INF_CALLBACK, "deleteSubscriber(participant, subscriber)");
    returnCode = (LOC_ReturnCode_t)(*env)->CallStaticIntMethod(env, jcache, deleteSubscriberMid, jparticipant,
                                                                (jobject)ls_subscriber);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode, "Delete of subscriber failed");

    (*env)->DeleteGlobalRef(env, (jobject)ls_subscriber);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_triggerListenersWithStartOfUpdates(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData)
{
    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(relatedCache);
    assert(userData);

    DJA_CacheBridge_us_triggerCacheListeners(exception, relatedCache, userData,
                                             cachedJNI.cache_triggerListenersStartUpdates_mid);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_triggerListenersWithEndOfUpdates(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData)
{

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(relatedCache);
    assert(userData);

    DJA_CacheBridge_us_triggerCacheListeners(exception, relatedCache, userData,
                                             cachedJNI.cache_triggerListenersEndUpdates_mid);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_triggerListenersWithUpdatesEnabled(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData)
{
    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(relatedCache);
    assert(userData);

    DJA_CacheBridge_us_triggerCacheListeners(exception, relatedCache, userData,
                                             cachedJNI.cache_triggerListenersUpdatesEnabled_mid);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
void
DJA_CacheBridge_us_homesAction(
    DLRL_Exception* exception,
    void* userData,
    const Coll_List* homes,
    void** arg)
{
    JNIEnv *env = (JNIEnv *)userData;
    LOC_unsigned_long size = 0;
    jobjectArray jhomes = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* aHome = NULL;
    jobject jaHome = NULL;
    LOC_unsigned_long count = 0;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);
    assert(homes);
    assert(!(*arg));

    size = Coll_List_getNrOfElements(homes);
    *arg = (*env)->NewObjectArray(env, size, cachedJNI.objectHome_class, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    jhomes = (jobjectArray)(*arg);

    iterator = Coll_List_getFirstElement(homes);
    while(iterator){
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        /* locking is unneccesary, homes will always be alive if the cache is alive. */
        jaHome = DK_ObjectHomeAdmin_ts_getLSHome(aHome, exception, (void*)env);
        DLRL_Exception_PROPAGATE(exception);
        (*env)->SetObjectArrayElement(env, jhomes, count, jaHome);
        DLRL_JavaException_PROPAGATE(env, exception);
        (*env)->DeleteLocalRef(env, jaHome);
        count++;
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_accessesAction(
    DLRL_Exception* exception,
    void* userData,
    const Coll_Set* accesses,
    void** arg)
{
    JNIEnv *env = (JNIEnv *)userData;
    jobjectArray jaccesses = (jobjectArray)*arg;
    Coll_Iter* iterator = NULL;
    DK_CacheAccessAdmin* anAccess = NULL;
    LOC_unsigned_long count = 0;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);
    assert(accesses);
    assert(!(*arg));

    *arg = (*env)->NewObjectArray(env, Coll_Set_getNrOfElements(accesses), cachedJNI.cacheAccess_class, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    jaccesses = (jobjectArray)(*arg);

    iterator = Coll_Set_getFirstElement(accesses);
    while(iterator){
        anAccess = (DK_CacheAccessAdmin*)Coll_Iter_getObject(iterator);
        (*env)->SetObjectArrayElement(env, jaccesses, count,
                                                DK_CacheAccessAdmin_ts_getLSAccess(anAccess, exception, (void*)env));
        DLRL_JavaException_PROPAGATE(env, exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean DJA_CacheBridge_us_isDataAvailable(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_subscriber)
{
    gapi_statusMask mask;
    LOC_boolean retVal = FALSE;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);
    assert(ls_subscriber);

    mask = gapi_entity_get_status_changes((gapi_entity)saj_read_gapi_address((JNIEnv*)userData, ls_subscriber));
    if(mask & GAPI_DATA_ON_READERS_STATUS){
        retVal = TRUE;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DJA_CacheBridge_us_listenersAction(
    DLRL_Exception* exception,
    void* userData,
    const Coll_Set* listeners,
    void** arg)
{
    JNIEnv *env = (JNIEnv *)userData;
    jobjectArray jlisteners = NULL;
    Coll_Iter* iterator = NULL;
    jobject aListener = NULL;
    LOC_unsigned_long count = 0;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(userData);
    assert(listeners);
    assert(!(*arg));

    *arg = (void*)(*env)->NewObjectArray(env, Coll_Set_getNrOfElements(listeners), cachedJNI.cacheListener_class, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);
    jlisteners = (jobjectArray)(*arg);

    iterator = Coll_Set_getFirstElement(listeners);
    while(iterator){
        aListener = (jobject)Coll_Iter_getObject(iterator);
        (*env)->SetObjectArrayElement(env, jlisteners, count, aListener);
        DLRL_JavaException_PROPAGATE(env, exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_objectsAction(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long totalSize,
    LOC_unsigned_long* elementIndex,
    DK_ObjectArrayHolder* holder)
{
    JNIEnv *env = (JNIEnv *)userData;
    jobjectArray objects = NULL;
    LOC_unsigned_long elementCount = 0;
    DK_ObjectAdmin* element = NULL;

    DLRL_INFO(INF_ENTER);

    if(!(*arg)){
        *arg = (void*)(*env)->NewObjectArray(env, totalSize, cachedJNI.objectRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);
    }
    objects = (jobjectArray)(*arg);
    assert(objects);

    for(elementCount = 0; elementCount < holder->size; elementCount++){
        element = holder->objectArray[elementCount];
        assert(DK_ObjectAdmin_us_getLSObject(element));
        assert((*elementIndex) < totalSize);
        (*env)->SetObjectArrayElement(env, objects, *elementIndex, (jobject)DK_ObjectAdmin_us_getLSObject(element));
        DLRL_JavaException_PROPAGATE(env, exception);
        (*elementIndex)++;
    }

    DLRL_Exception_EXIT(exception);
    /* dont translate exception into java exception, this operation is called from the kernel... */
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_triggerListenersWithUpdatesDisabled(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData)
{
    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(relatedCache);
    assert(userData);

    DJA_CacheBridge_us_triggerCacheListeners(exception, relatedCache, userData,
                                             cachedJNI.cache_triggerListenersUpdatesDisabled_mid);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_CacheBridge_us_triggerCacheListeners(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData,
    jmethodID triggerMethod)
{
    jobjectArray jlisteners = NULL;
    JNIEnv* env;
    Coll_Set* listeners = NULL;
    LOC_unsigned_long listenerSize = 0;
    Coll_Iter* iterator;
    LOC_unsigned_long count = 0;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(relatedCache);
    assert(userData);

    env = (JNIEnv*)userData;
    listeners = DK_CacheAdmin_us_getListeners(relatedCache);
    listenerSize = Coll_Set_getNrOfElements(listeners);
    /* redudant check, but safe to avoid java context switch even if the kernel is changed =) */
    if(listenerSize > 0){
        jlisteners = (*env)->NewObjectArray(env, listenerSize, cachedJNI.cacheListener_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);

        iterator = Coll_Set_getFirstElement(listeners);

        while(iterator){
            void* aListener = Coll_Iter_getObject(iterator);
            (*env)->SetObjectArrayElement(env, jlisteners, count, (jobject)aListener);
            DLRL_JavaException_PROPAGATE(env, exception);

            iterator = Coll_Iter_getNext(iterator);
            count++;
        }
        DLRL_INFO(INF_CALLBACK, "triggerListeners(listeners)");
        (*env)->CallVoidMethod(env, (jobject)DK_CacheBase_us_getLSCache((DK_CacheBase*)relatedCache), triggerMethod,
                                                                                                        jlisteners);
        DLRL_JavaException_PROPAGATE(env, exception);

    }
    DLRL_Exception_EXIT(exception);
    if(jlisteners){
        (*env)->DeleteLocalRef(env, jlisteners);
    }
    DLRL_INFO(INF_EXIT);
}
