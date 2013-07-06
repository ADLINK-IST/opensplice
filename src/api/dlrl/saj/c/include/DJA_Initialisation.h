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
#ifndef DJA_INITIALISATION_H
#define DJA_INITIALISATION_H

#include <jni.h>
#include "DLRL_Types.h"
#include "Coll_List.h"
#include "DLRL_Kernel.h"

typedef struct DJA_CachedJNI_t {
    /*ObjectHome */
    jclass objectHome_class;
    jfieldID objectHome_admin_fid;
    jmethodID objectHome_loadMetamodel_mid;
    jmethodID objectHome_registerType_mid;

    /* DCPSUtil */
    jclass dcpsUtil_class;
    jmethodID dcpsUtil_createTopic_mid;
    jmethodID dcpsUtil_createDataReader_mid;
    jmethodID dcpsUtil_createDataWriter_mid;

    jmethodID dcpsUtil_deleteDataReader_mid;
    jmethodID dcpsUtil_deleteDataWriter_mid;
    jmethodID dcpsUtil_deleteTopic_mid;

    /* StrMap */
    jclass strMap_class;
    jfieldID strMap_admin_fid;

    /* IntMap */
    jclass intMap_class;
    jfieldID intMap_admin_fid;

    /* set */
    jclass set_class;
    jfieldID set_admin_fid;

    /* string clas */
    jclass string_class;

    /* Object class */
    jclass object_class;

    /* ObjectListener class */
    jclass objectListener_class;

    /* CacheListener */
    jclass cacheListener_class;

    /* java/lang/throwable */
    jclass throw_class;
    jmethodID throw_getClass_mid;
    jmethodID throw_getMessage_mid;

    /* java/lang/class */
    jclass class_class;
    jmethodID class_getName_mid;

    /* Cache */
    jclass cache_class;
    jfieldID cache_admin_fid;
    jmethodID cache_createPublisher_mid;
    jmethodID cache_createSubscriber_mid;
    jmethodID cache_deletePublisher_mid;
    jmethodID cache_deleteSubscriber_mid;
    jmethodID cache_triggerListenersStartUpdates_mid;
    jmethodID cache_triggerListenersEndUpdates_mid;
    jmethodID cache_triggerListenersUpdatesEnabled_mid;
    jmethodID cache_triggerListenersUpdatesDisabled_mid;

    /* selection */
    jclass selection_class;
    jfieldID selection_admin_fid;
    jmethodID selection_checkObjects_mid;

    /* QueryCriterion */
    jclass queryCriterion_class;
    jfieldID queryCriterion_admin_fid;

    /* OID structure class */
    jclass oid_class;
    jmethodID oid_constructor_mid;
    jfieldID oid_value_fid;/* NOT IN DESIGN */
    jfieldID oid_systemId_fid;/* NOT IN DESIGN */
    jfieldID oid_localId_fid;/* NOT IN DESIGN */
    jfieldID oid_serial_fid;/* NOT IN DESIGN */

    /* ObjectRoot class */
    jclass objectRoot_class;
    jfieldID objectRoot_admin_fid;
    jfieldID objectRoot_isAlive_fid;/* NOT IN DESIGN */
    jfieldID objectRoot_isRegistered_fid;/* NOT IN DESIGN */
    jfieldID objectRoot_writeState_fid;/* NOT IN DESIGN */
    jfieldID objectRoot_prevTopicValid_fid;/* NOT IN DESIGN */

    /* CacheAccess class */
    jclass cacheAccess_class;/* NOT IN DESIGN */
    jfieldID cacheAccess_admin_fid;/* NOT IN DESIGN */

    /* DDS/DCPS Entity */
    jmethodID entity_enable_mid;

} DJA_CachedJNI;




typedef struct DJA_CachedJNITypedObject_s{
    /* Typed ObjectHome */
    jclass typedHome_class;
    jmethodID typedHome_triggerListeners_mid;

    /* Typed ObjectListener */
    jclass typedListener_class;

    /* Typed Selection */
    jclass typedSelection_class;

    jmethodID typedFilter_checkObject_mid;

    jmethodID typedSelectionListener_onObjectOut_mid;
    jmethodID typedSelectionListener_onObjectModified_mid;
    jmethodID typedSelectionListener_onObjectIn_mid;
    /* Typed ObjectRoot */
    jclass typedRoot_class;
    jmethodID typedRoot_constructor_mid;
    jfieldID typedRoot_currentTopic_fid;
    jfieldID typedRoot_previousTopic_fid;

    /* Main topic class */
    jclass typedTopic_class;

    Coll_List relationFieldIDs;
    Coll_List relationIsFoundFieldIDs;
    Coll_List collectionFieldIDs;

    /* Typed str map */
    jclass typedStrMap_class;
    jmethodID typedStrMap_constructor_mid;

    /* Typed int map */
    jclass typedIntMap_class;
    jmethodID typedIntMap_constructor_mid;

    /* Typed set */
    jclass typedSet_class;
    jmethodID typedSet_constructor_mid;

    /* base path info */
    LOC_string pathPrefixed;
    LOC_string pathPrefixedExceptLast;

} DJA_CachedJNITypedObject;

typedef struct DJA_CachedJNITypedTopic_s{
    /* Typed Topic */
    jclass typedTopic_class;
    jmethodID typedTopic_constructor_mid;
    jfieldID typedTopic_oidKey_fid;
    jfieldID typedTopic_nameKey_fid;

    /* Typed DataReader */
    jfieldID typedReader_copyCache_fid;

    /* Typed DataWriter */
    jfieldID typedWriter_copyCache_fid;
} DJA_CachedJNITypedTopic;

extern DJA_CachedJNI cachedJNI;

DJA_CachedJNITypedObject*
DJA_Initialisation_loadTypedObjectCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    jobject jhome,
    LOC_string pathPrefixed,
    LOC_string pathPrefixedExceptLast,
    LOC_string topicTypePrefixed,
    LOC_string targetImplClassName);

DJA_CachedJNITypedTopic*
DJA_Initialisation_loadTypedTopicCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    DJA_CachedJNITypedObject* typedObjectCachedData,
    DK_ObjectHomeAdmin* home,
    jobject jhome);

void
DJA_Initialisation_loadTypedReaderCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    DJA_CachedJNITypedTopic* typedTopicCachedData,
    jobject jreader);

void
DJA_Initialisation_loadTypedWriterCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    DJA_CachedJNITypedTopic* typedTopicCachedData,
    jobject jwriter);

/* assumes lock (admin) on the cache and admin locks on all required homes */
void
DJA_Initialisation_cacheRelationFields(
    DLRL_Exception* exception,
    JNIEnv* env,
    DK_CacheAdmin* cache);

void
DJA_Initialisation_us_destroyTypedTopicCache(
    JNIEnv* env,
    DJA_CachedJNITypedTopic* typedTopicCache);

void
DJA_Initialisation_us_destroyTypedObjectCache(
    JNIEnv* env,
    DJA_CachedJNITypedObject* typedObjectCache);

#endif
