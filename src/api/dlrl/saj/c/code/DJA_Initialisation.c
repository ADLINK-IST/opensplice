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
#include <string.h>
#include <assert.h>
#include "os_stdlib.h"
#include "os_heap.h"

#include "DJA_ExceptionHandler.h"

/* dlrl includes */
#include "DLRL_Report.h"
#include "DLRL_Kernel_private.h"
#include "DJA_Initialisation.h"
#include "DJA_Initializer.h"
#include "DLRL_Util.h"
#include "DJA_UtilityBridge.h"
#include "DK_UtilityBridge.h"

#include "DK_CacheBridge.h"
#include "DK_CacheAccessBridge.h"
#include "DK_CollectionBridge.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_ObjectBridge.h"
#include "DK_ObjectHomeBridge.h"
#include "DK_ObjectReaderBridge.h"
#include "DK_ObjectRelationReaderBridge.h"
#include "DK_ObjectWriterBridge.h"
#include "DK_SelectionBridge.h"

#include "DJA_CacheBridge.h"
#include "DJA_CacheAccessBridge.h"
#include "DJA_CollectionBridge.h"
#include "DJA_DCPSUtilityBridge.h"
#include "DJA_ObjectBridge.h"
#include "DJA_ObjectHomeBridge.h"
#include "DJA_ObjectReaderBridge.h"
#include "DJA_ObjectRelationReaderBridge.h"
#include "DJA_ObjectWriterBridge.h"
#include "DJA_SelectionBridge.h"

DJA_CachedJNI cachedJNI;

/* assumes lock (admin) on the cache and admin locks on all required homes */
static void
DJA_Initialisation_processRelations(
    DLRL_Exception* exception,
    JNIEnv* env,
    DK_CacheAdmin* adminLockedCache,
    DK_ObjectHomeAdmin* lockedOwnerHome,
    Coll_List* relationNames,
    Coll_List* targetFieldList,
    Coll_List* targetFieldIsFoundList,
    DJA_CachedJNITypedObject* ownerObjectCachedData);

/*NOT IN DESIGN - moved*/
static void
DJA_Initialisation_loadGenericCache(
    DLRL_Exception* exception,
    JNIEnv* env);

/*NOT IN DESIGN*/
static void
DJA_Initialisation_initializeDLRLKernel();

JNIEXPORT void JNICALL
Java_org_opensplice_dds_dlrl_Initializer_jniInitializeAll(
    JNIEnv * env,
    jclass theClass)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    DJA_Initialisation_loadGenericCache(&exception, env);
    DLRL_Exception_PROPAGATE(&exception);
    DJA_Initialisation_initializeDLRLKernel();

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_Initialisation_initializeDLRLKernel()
{
    DK_CacheBridge* cacheBridge;
    DK_CacheAccessBridge* cacheAccessBridge;
    DK_CollectionBridge* collectionBridge;
    DK_DCPSUtilityBridge* dcpsUtilityBridge;
    DK_ObjectBridge* objectBridge;
    DK_ObjectHomeBridge* objectHomeBridge;
    DK_ObjectReaderBridge* objectReaderBridge;
    DK_ObjectRelationReaderBridge* objectRelationReaderBridge;
    DK_ObjectWriterBridge* objectWriterBridge;
    DK_SelectionBridge* selectionBridge;
    DK_UtilityBridge* utilityBridge;

    DLRL_INFO(INF_ENTER);
    cacheBridge = DK_CacheFactoryAdmin_us_getCacheBridge();
    cacheAccessBridge = DK_CacheFactoryAdmin_us_getCacheAccessBridge();
    collectionBridge = DK_CacheFactoryAdmin_us_getCollectionBridge();
    dcpsUtilityBridge = DK_CacheFactoryAdmin_us_getDcpsUtilityBridge();
    objectBridge = DK_CacheFactoryAdmin_us_getObjectBridge();
    objectHomeBridge = DK_CacheFactoryAdmin_us_getObjectHomeBridge();
    objectReaderBridge = DK_CacheFactoryAdmin_us_getObjectReaderBridge();
    objectRelationReaderBridge = DK_CacheFactoryAdmin_us_getObjectRelationReaderBridge();
    objectWriterBridge = DK_CacheFactoryAdmin_us_getObjectWriterBridge();
    selectionBridge = DK_CacheFactoryAdmin_us_getSelectionBridge();
    utilityBridge = DK_CacheFactoryAdmin_us_getUtilityBridge();

    cacheBridge->createPublisher = DJA_CacheBridge_us_createPublisher;
    cacheBridge->createSubscriber = DJA_CacheBridge_us_createSubscriber;
    cacheBridge->deletePublisher = DJA_CacheBridge_us_deletePublisher;
    cacheBridge->deleteSubscriber = DJA_CacheBridge_us_deleteSubscriber;
    cacheBridge->triggerListenersWithStartOfUpdates = DJA_CacheBridge_us_triggerListenersWithStartOfUpdates;
    cacheBridge->triggerListenersWithEndOfUpdates = DJA_CacheBridge_us_triggerListenersWithEndOfUpdates;
    cacheBridge->triggerListenersWithUpdatesEnabled = DJA_CacheBridge_us_triggerListenersWithUpdatesEnabled;
    cacheBridge->triggerListenersWithUpdatesDisabled = DJA_CacheBridge_us_triggerListenersWithUpdatesDisabled;
    cacheBridge->homesAction = DJA_CacheBridge_us_homesAction;
    cacheBridge->listenersAction = DJA_CacheBridge_us_listenersAction;
    cacheBridge->accessesAction = DJA_CacheBridge_us_accessesAction;/* TODO actually use this operation as with ccpp */
    cacheBridge->objectsAction = DJA_CacheBridge_us_objectsAction;
    cacheBridge->isDataAvailable = DJA_CacheBridge_us_isDataAvailable;

    cacheAccessBridge->containedTypesAction = DJA_CacheAccessBridge_us_containedTypesAction;
    cacheAccessBridge->containedTypeNamesAction = DJA_CacheAccessBridge_us_containedTypeNamesAction;
    cacheAccessBridge->objectsAction = DJA_CacheAccessBridge_us_objectsAction;
    cacheAccessBridge->invalidObjectsAction = DJA_CacheAccessBridge_us_invalidObjectsAction;

    collectionBridge->createLSCollection = DJA_CollectionBridge_us_createLSCollection;

    dcpsUtilityBridge->registerType = DJA_DCPSUtilityBridge_us_registerType;
    dcpsUtilityBridge->createTopic = DJA_DCPSUtilityBridge_us_createTopic;
    dcpsUtilityBridge->createDataReader = DJA_DCPSUtilityBridge_us_createDataReader;
    dcpsUtilityBridge->createDataWriter = DJA_DCPSUtilityBridge_us_createDataWriter;
    dcpsUtilityBridge->deleteDataReader = DJA_DCPSUtilityBridge_us_deleteDataReader;
    dcpsUtilityBridge->deleteDataWriter = DJA_DCPSUtilityBridge_us_deleteDataWriter;
    dcpsUtilityBridge->deleteTopic = DJA_DCPSUtilityBridge_us_deleteTopic;
    dcpsUtilityBridge->releaseTopicUserData = DJA_DCPSUtilityBridge_us_releaseTopicUserData;
    dcpsUtilityBridge->enableEntity = DJA_DCPSUtilityBridge_us_enableEntity;

    objectBridge->setIsAlive = DJA_ObjectBridge_us_setIsAlive;
    objectBridge->setIsRegistered = DJA_ObjectBridge_us_setIsRegistered;
    objectBridge->notifyWriteStateChange = DJA_ObjectBridge_us_notifyWriteStateChange;
    objectBridge->clearLSObjectAdministration = NULL;/* Not used by Java */

    objectHomeBridge->loadMetamodel = DJA_ObjectHomeBridge_us_loadMetamodel;
    objectHomeBridge->unregisterAdminWithLSHome = DJA_ObjectHomeBridge_us_unregisterAdminWithLSHome;
    objectHomeBridge->deleteUserData = DJA_ObjectHomeBridge_us_deleteUserData;
    objectHomeBridge->triggerListeners = DJA_ObjectHomeBridge_us_triggerListeners;
    objectHomeBridge->createTypedObject = DJA_ObjectHomeBridge_us_createTypedObject;
    objectHomeBridge->doCopyInForTopicOfObject = DJA_ObjectHomeBridge_us_doCopyInForTopicOfObject;
    objectHomeBridge->setDefaultTopicKeys = DJA_ObjectHomeBridge_us_setDefaultTopicKeys;
    objectHomeBridge->createTypedObjectSeq = DJA_ObjectHomeBridge_us_createTypedObjectSeq;
    objectHomeBridge->addElementToTypedObjectSeq = DJA_ObjectHomeBridge_us_addElementToTypedObjectSeq;
    objectHomeBridge->createTypedSelectionSeq = DJA_ObjectHomeBridge_us_createTypedSelectionSeq;
    objectHomeBridge->addElementToTypedSelectionSeq = DJA_ObjectHomeBridge_us_addElementToTypedSelectionSeq;
    objectHomeBridge->createTypedListenerSeq = DJA_ObjectHomeBridge_us_createTypedListenerSeq;
    objectHomeBridge->addElementToTypedListenerSeq = DJA_ObjectHomeBridge_us_addElementToTypedListenerSeq;
    objectHomeBridge->checkObjectForSelection = DJA_ObjectHomeBridge_us_checkObjectForSelection;

    objectReaderBridge->updateObject = DJA_ObjectReaderBridge_us_updateObject;
    objectReaderBridge->doLSReadPreProcessing = DJA_ObjectReaderBridge_us_doLSReadPreProcessing;
    objectReaderBridge->setCollectionToLSObject = DJA_ObjectReaderBridge_us_setCollectionToLSObject;
    objectReaderBridge->createLSTopic = DJA_ObjectReaderBridge_us_createLSTopic;
    objectReaderBridge->resetLSModificationInfo = DJA_ObjectReaderBridge_us_resetLSModificationInfo;

    objectRelationReaderBridge->setRelatedObjectForObject = DJA_ObjectRelationReaderBridge_us_setRelatedObjectForObject;

    objectWriterBridge->registerInstance = DJA_ObjectWriterBridge_us_registerInstance;
    objectWriterBridge->write = DJA_ObjectWriterBridge_us_write;
    objectWriterBridge->destroy = DJA_ObjectWriterBridge_us_destroy;

    selectionBridge->checkObjects = DJA_SelectionBridge_us_checkObjects;
    selectionBridge->triggerListenerInsertedObject = DJA_SelectionBridge_us_triggerListenerInsertedObject;
    selectionBridge->triggerListenerModifiedObject = DJA_SelectionBridge_us_triggerListenerModifiedObject;
    selectionBridge->triggerListenerRemovedObject = DJA_SelectionBridge_us_triggerListenerRemovedObject;

    utilityBridge->areSameLSObjects = DJA_UtilityBridge_us_AreSameLSObjects;
    utilityBridge->createIntegerSeq = DJA_UtilityBridge_us_createIntegerSeq;
    utilityBridge->createStringSeq = DJA_UtilityBridge_us_createStringSeq;
    utilityBridge->addElementToStringSeq = DJA_UtilityBridge_us_addElementToStringSeq;
    utilityBridge->addElementToIntegerSeq = DJA_UtilityBridge_us_addElementToIntegerSeq;
    utilityBridge->duplicateLSValuetypeObject = DJA_UtilityBridge_us_duplicateLSObject;
    utilityBridge->releaseLSValuetypeObject = DJA_UtilityBridge_us_releaseLSObject;
    utilityBridge->duplicateLSInterfaceObject = DJA_UtilityBridge_us_duplicateLSObject;
    utilityBridge->releaseLSInterfaceObject = DJA_UtilityBridge_us_releaseLSObject;
    utilityBridge->localDuplicateLSInterfaceObject = DJA_UtilityBridge_us_localDuplicateLSObject;
    utilityBridge->localDuplicateLSValuetypeObject = DJA_UtilityBridge_us_localDuplicateLSObject;
    utilityBridge->getThreadCreateUserData = DJA_UtilityBridge_us_getThreadCreateUserData;
    utilityBridge->doThreadAttach = DJA_UtilityBridge_us_doThreadAttach;
    utilityBridge->doThreadDetach = DJA_UtilityBridge_us_doThreadDetach;
    utilityBridge->getThreadSessionUserData = DJA_UtilityBridge_us_getThreadSessionUserData;

    DLRL_INFO(INF_EXIT);
}

void
DJA_Initialisation_loadGenericCache(
    DLRL_Exception* exception,
    JNIEnv* env)
{
    jclass tempClass = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);

    /* initialize everything to NULL first, should use a memset, bla bla */
    cachedJNI.objectHome_class = NULL;
    cachedJNI.objectHome_admin_fid = NULL;
    cachedJNI.objectHome_loadMetamodel_mid = NULL;
    cachedJNI.objectHome_registerType_mid = NULL;
    cachedJNI.dcpsUtil_class = NULL;
    cachedJNI.dcpsUtil_createTopic_mid = NULL;
    cachedJNI.dcpsUtil_createDataReader_mid = NULL;
    cachedJNI.dcpsUtil_createDataWriter_mid = NULL;
    cachedJNI.dcpsUtil_deleteDataReader_mid = NULL;
    cachedJNI.dcpsUtil_deleteDataWriter_mid = NULL;
    cachedJNI.dcpsUtil_deleteTopic_mid = NULL;
    cachedJNI.strMap_class = NULL;
    cachedJNI.strMap_admin_fid = NULL;
    cachedJNI.intMap_class = NULL;
    cachedJNI.intMap_admin_fid = NULL;
    cachedJNI.set_class = NULL;
    cachedJNI.set_admin_fid = NULL;
    cachedJNI.string_class = NULL;
    cachedJNI.object_class = NULL;
    cachedJNI.objectListener_class = NULL;
    cachedJNI.cacheListener_class = NULL;
    cachedJNI.throw_class = NULL;
    cachedJNI.throw_getClass_mid = NULL;
    cachedJNI.throw_getMessage_mid = NULL;
    cachedJNI.class_class = NULL;
    cachedJNI.class_getName_mid = NULL;
    cachedJNI.cache_class = NULL;
    cachedJNI.cache_admin_fid = NULL;
    cachedJNI.cache_createPublisher_mid = NULL;
    cachedJNI.cache_createSubscriber_mid = NULL;
    cachedJNI.cache_deletePublisher_mid = NULL;
    cachedJNI.cache_deleteSubscriber_mid = NULL;
    cachedJNI.cache_triggerListenersStartUpdates_mid = NULL;
    cachedJNI.cache_triggerListenersEndUpdates_mid = NULL;
    cachedJNI.cache_triggerListenersUpdatesEnabled_mid = NULL;
    cachedJNI.cache_triggerListenersUpdatesDisabled_mid = NULL;
    cachedJNI.selection_class = NULL;
    cachedJNI.selection_admin_fid = NULL;
    cachedJNI.selection_checkObjects_mid = NULL;
    cachedJNI.queryCriterion_class = NULL;
    cachedJNI.queryCriterion_admin_fid = NULL;
    cachedJNI.oid_class = NULL;
    cachedJNI.oid_constructor_mid = NULL;
    cachedJNI.oid_value_fid = NULL;
    cachedJNI.oid_systemId_fid = NULL;
    cachedJNI.oid_localId_fid = NULL;
    cachedJNI.oid_serial_fid = NULL;
    cachedJNI.objectRoot_class = NULL;
    cachedJNI.objectRoot_admin_fid = NULL;
    cachedJNI.objectRoot_isAlive_fid = NULL;
    cachedJNI.objectRoot_isRegistered_fid = NULL;
    cachedJNI.objectRoot_writeState_fid = NULL;
    cachedJNI.objectRoot_prevTopicValid_fid = NULL;

    cachedJNI.cacheAccess_admin_fid = NULL;
    cachedJNI.cacheAccess_class = NULL;
    cachedJNI.entity_enable_mid = NULL;
    /* ObjectHome */
    tempClass = (*env)->FindClass(env, "DDS/ObjectHome");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.objectHome_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.objectHome_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/ObjectHome class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    cachedJNI.objectHome_admin_fid = (*env)->GetFieldID(env, cachedJNI.objectHome_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.objectHome_loadMetamodel_mid = (*env)->GetMethodID(env, cachedJNI.objectHome_class, "buildMetaModel",  "()V");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.objectHome_registerType_mid = (*env)->GetMethodID(env, cachedJNI.objectHome_class, "registerType",
                                                    "(LDDS/DomainParticipant;Ljava/lang/String;Ljava/lang/String;)I");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* StrMap */
    tempClass = (*env)->FindClass(env, "DDS/StrMap");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.strMap_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.strMap_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/StrMap class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.strMap_admin_fid = (*env)->GetFieldID(env, cachedJNI.strMap_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* IntMap */
    tempClass = (*env)->FindClass(env, "DDS/IntMap");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.intMap_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.intMap_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/IntMap class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.intMap_admin_fid = (*env)->GetFieldID(env, cachedJNI.intMap_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* set */
    tempClass = (*env)->FindClass(env, "DDS/Set");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.set_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.set_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/Set class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.set_admin_fid = (*env)->GetFieldID(env, cachedJNI.set_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);


    tempClass = (*env)->FindClass(env, "org/opensplice/dds/dlrl/DCPSUtil");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.dcpsUtil_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.dcpsUtil_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the org/opensplice/dds/dlrl/DCPSUtil class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.dcpsUtil_createTopic_mid = (*env)->GetStaticMethodID(env, cachedJNI.dcpsUtil_class, "createTopic",
                                            "(LDDS/DomainParticipant;Ljava/lang/String;Ljava/lang/String;)LDDS/Topic;");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.dcpsUtil_createDataReader_mid = (*env)->GetStaticMethodID(env, cachedJNI.dcpsUtil_class, "createDataReader",
                               "(LDDS/DomainParticipant;LDDS/Subscriber;LDDS/Topic;Ljava/lang/String;)LDDS/DataReader;");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.dcpsUtil_createDataWriter_mid = (*env)->GetStaticMethodID(env, cachedJNI.dcpsUtil_class, "createDataWriter",
                                                                        "(LDDS/Publisher;LDDS/Topic;)LDDS/DataWriter;");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.dcpsUtil_deleteDataReader_mid = (*env)->GetStaticMethodID(env, cachedJNI.dcpsUtil_class, "deleteDataReader",
                                                                             "(LDDS/Subscriber;LDDS/DataReader;)I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.dcpsUtil_deleteDataWriter_mid = (*env)->GetStaticMethodID(env, cachedJNI.dcpsUtil_class, "deleteDataWriter",
                                                                                "(LDDS/Publisher;LDDS/DataWriter;)I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.dcpsUtil_deleteTopic_mid = (*env)->GetStaticMethodID(env, cachedJNI.dcpsUtil_class, "deleteTopic",
                                                                                "(LDDS/DomainParticipant;LDDS/Topic;)I");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* String class */
    tempClass = (*env)->FindClass(env, "java/lang/String");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.string_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.string_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the java/lang/String class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    /* Object class */
    tempClass = (*env)->FindClass(env, "java/lang/Object");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.object_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.object_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the java/lang/Object class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    /* ObjectListener class */
    tempClass = (*env)->FindClass(env, "DDS/ObjectListener");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.objectListener_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.objectListener_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/ObjectListener class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    /* CacheListener class */
    tempClass = (*env)->FindClass(env, "DDS/CacheListener");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cacheListener_class =  (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.cacheListener_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/ObjectListener class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    /* java/lang/throwable */
    tempClass = (*env)->FindClass(env, "java/lang/Throwable");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.throw_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.throw_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the java/lang/Throwable class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.throw_getClass_mid = (*env)->GetMethodID(env, cachedJNI.throw_class, "getClass",  "()Ljava/lang/Class;");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.throw_getMessage_mid = (*env)->GetMethodID(env, cachedJNI.throw_class, "getMessage",  "()Ljava/lang/String;");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* Java/lang/Class */
    tempClass = (*env)->FindClass(env, "java/lang/Class");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.class_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.class_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the java/lang/Class class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.class_getName_mid = (*env)->GetMethodID(env, cachedJNI.class_class, "getName",  "()Ljava/lang/String;");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* Selection */
    tempClass = (*env)->FindClass(env, "org/opensplice/dds/dlrl/SelectionImpl");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.selection_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.selection_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the org/opensplice/dds/dlrl/SelectionImpl class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.selection_admin_fid = (*env)->GetFieldID(env, cachedJNI.selection_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.selection_checkObjects_mid = (*env)->GetMethodID(env, cachedJNI.selection_class, "checkObjects",
                                        "(LDDS/FilterCriterion;[LDDS/ObjectRoot;)[I");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* QueryCriterion */
    tempClass = (*env)->FindClass(env, "org/opensplice/dds/dlrl/QueryCriterionImpl");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.queryCriterion_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.queryCriterion_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the org/opensplice/dds/dlrl/QueryCriterionImpl class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.queryCriterion_admin_fid = (*env)->GetFieldID(env, cachedJNI.queryCriterion_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* Cache */
    tempClass = (*env)->FindClass(env, "org/opensplice/dds/dlrl/CacheImpl");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.cache_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the org/opensplice/dds/dlrl/CacheImpl class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.cache_createPublisher_mid = (*env)->GetStaticMethodID(env, cachedJNI.cache_class, "createPublisher",
                                    "(LDDS/DomainParticipant;)LDDS/Publisher;");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_createSubscriber_mid = (*env)->GetStaticMethodID(env, cachedJNI.cache_class, "createSubscriber",
                                    "(LDDS/DomainParticipant;)LDDS/Subscriber;");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_deletePublisher_mid = (*env)->GetStaticMethodID(env, cachedJNI.cache_class, "deletePublisher",
                                    "(LDDS/DomainParticipant;LDDS/Publisher;)I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_deleteSubscriber_mid = (*env)->GetStaticMethodID(env, cachedJNI.cache_class, "deleteSubscriber",
                                    "(LDDS/DomainParticipant;LDDS/Subscriber;)I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_triggerListenersStartUpdates_mid = (*env)->GetMethodID(env, cachedJNI.cache_class, "triggerListenersStartUpdates",
                                    "([LDDS/CacheListener;)V");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_triggerListenersEndUpdates_mid = (*env)->GetMethodID(env, cachedJNI.cache_class, "triggerListenersEndUpdates",
                                    "([LDDS/CacheListener;)V");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_admin_fid = (*env)->GetFieldID(env, cachedJNI.cache_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_triggerListenersUpdatesEnabled_mid = (*env)->GetMethodID(env, cachedJNI.cache_class, "triggerListenersUpdatesEnabled",
                                    "([LDDS/CacheListener;)V");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cache_triggerListenersUpdatesDisabled_mid = (*env)->GetMethodID(env, cachedJNI.cache_class, "triggerListenersUpdatesDisabled",
                                    "([LDDS/CacheListener;)V");
    DLRL_JavaException_PROPAGATE(env, exception);


    /* OID class */
    tempClass = (*env)->FindClass(env, "DDS/DLRLOid");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.oid_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.oid_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/DLRLOid class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.oid_constructor_mid = (*env)->GetMethodID(env, cachedJNI.oid_class,"<init>", "()V");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.oid_value_fid = (*env)->GetFieldID(env, cachedJNI.oid_class, "value","[I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.oid_systemId_fid = (*env)->GetFieldID(env, cachedJNI.oid_class, "systemId","I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.oid_localId_fid = (*env)->GetFieldID(env, cachedJNI.oid_class, "localId","I");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.oid_serial_fid = (*env)->GetFieldID(env, cachedJNI.oid_class, "serial","I");
    DLRL_JavaException_PROPAGATE(env, exception);
    /* ObjectRoot */
    tempClass = (*env)->FindClass(env, "DDS/ObjectRoot");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.objectRoot_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.objectRoot_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the DDS/ObjectRoot class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.objectRoot_admin_fid = (*env)->GetFieldID(env, cachedJNI.objectRoot_class, "admin","J");
    DLRL_JavaException_PROPAGATE(env, exception);

    cachedJNI.objectRoot_isAlive_fid = (*env)->GetFieldID(env, cachedJNI.objectRoot_class, "isAlive","Z");
    DLRL_JavaException_PROPAGATE(env, exception);

    cachedJNI.objectRoot_isRegistered_fid = (*env)->GetFieldID(env, cachedJNI.objectRoot_class, "isRegistered","Z");
    DLRL_JavaException_PROPAGATE(env, exception);

    cachedJNI.objectRoot_writeState_fid = (*env)->GetFieldID(env, cachedJNI.objectRoot_class, "writeState","I");
    DLRL_JavaException_PROPAGATE(env, exception);

    cachedJNI.objectRoot_prevTopicValid_fid = (*env)->GetFieldID(env, cachedJNI.objectRoot_class, "prevTopicValid","Z");
    DLRL_JavaException_PROPAGATE(env, exception);

    tempClass = (*env)->FindClass(env, "org/opensplice/dds/dlrl/CacheAccessImpl");
    DLRL_JavaException_PROPAGATE(env, exception);
    cachedJNI.cacheAccess_class = (*(env))->NewGlobalRef (env, tempClass);
    if(!cachedJNI.cacheAccess_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the "
                            "org/opensplice/dds/dlrl/CacheAccessImpl class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    cachedJNI.cacheAccess_admin_fid = (*env)->GetFieldID(env, cachedJNI.cacheAccess_class, "admin", "J");
    DLRL_JavaException_PROPAGATE(env, exception);

    tempClass = (*env)->FindClass(env, "org/opensplice/dds/dcps/EntityImpl");
    cachedJNI.entity_enable_mid = (*env)->GetMethodID(env, tempClass, "enable", "()I");
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    DLRL_Exception_EXIT(exception);
    if(tempClass){
        (*env)->DeleteLocalRef(env, tempClass);
    }
    DLRL_INFO(INF_EXIT);
}

/* assumes lock (admin) on the cache and admin locks on all required homes */
void
DJA_Initialisation_cacheRelationFields(
    DLRL_Exception* exception,
    JNIEnv* env,
    DK_CacheAdmin* cache)
{
    Coll_List* homes = NULL;
    DK_ObjectHomeAdmin* ownerHome = NULL;
    DJA_CachedJNITypedObject* ownerObjectCachedData;
    Coll_List* relationNames = NULL;
    Coll_List* multiRelationNames = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(env);
    assert(cache);

    homes = DK_CacheAdmin_us_getHomes(cache);
    iterator = Coll_List_getFirstElement(homes);
    while(iterator && !DJA_ExceptionHandler_hasJavaExceptionOccurred(env)){
        ownerHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        /* fetch the cached object data and start processing the single relations*/
        ownerObjectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(ownerHome);
        /* need to free the return value! */
        relationNames = DK_MMFacade_us_getSingleRelationNames(ownerHome, exception);
        DLRL_Exception_PROPAGATE(exception);
        DJA_Initialisation_processRelations(exception, env, cache, ownerHome, relationNames,
                                            &(ownerObjectCachedData->relationFieldIDs),
                                            &(ownerObjectCachedData->relationIsFoundFieldIDs), ownerObjectCachedData);
        DLRL_Exception_PROPAGATE(exception);
        /* need to free the return value! */
        multiRelationNames = DK_MMFacade_us_getMultiRelationNames(ownerHome, exception);
        DJA_Initialisation_processRelations(exception, env, cache, ownerHome, multiRelationNames,
                                            &(ownerObjectCachedData->collectionFieldIDs), NULL, ownerObjectCachedData);
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
        /* do clean up */
        while(Coll_List_getNrOfElements(relationNames) > 0){
            Coll_List_popBack(relationNames);
        }
        Coll_List_delete(relationNames);
        while(Coll_List_getNrOfElements(multiRelationNames) > 0){
            Coll_List_popBack(multiRelationNames);
        }
        Coll_List_delete(multiRelationNames);
        relationNames = NULL;/* avoid double clean up */
        multiRelationNames = NULL;/* avoid double clean up */
    }
    DLRL_Exception_EXIT(exception);
    /* might have to do some clean up! */
    if(relationNames){
        /* free the names list, do not free the name strings as they arent owned by the list */
        while(Coll_List_getNrOfElements(relationNames) > 0){
            Coll_List_popBack(relationNames);
        }
        Coll_List_delete(relationNames);
    }
    if(multiRelationNames){
        /* free the names list, do not free the name strings as they arent owned by the list */
        while(Coll_List_getNrOfElements(multiRelationNames) > 0){
            Coll_List_popBack(multiRelationNames);
        }
        Coll_List_delete(multiRelationNames);
    }
    DLRL_INFO(INF_EXIT);
}

DJA_CachedJNITypedObject*
DJA_Initialisation_loadTypedObjectCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    jobject jhome,
    LOC_string pathPrefixed,
    LOC_string pathPrefixedExceptLast,
    LOC_string topicTypePrefixed,
    LOC_string targetImplClassName)
{
    jclass tempClass = NULL;
    LOC_string stringHolder = NULL;
    LOC_long count = 0;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(jhome);
    assert(exception);

    DLRL_ALLOC(
        typedObjectCachedData,
        DJA_CachedJNITypedObject,
        exception,
        "Unable to alloc JNI typed object cache");
/* init everything to NULL */
    memset(typedObjectCachedData, 0, sizeof(DJA_CachedJNITypedObject));
    Coll_List_init(&(typedObjectCachedData->relationFieldIDs));
    Coll_List_init(&(typedObjectCachedData->relationIsFoundFieldIDs));
    Coll_List_init(&(typedObjectCachedData->collectionFieldIDs));

/* load the typed ObjectHome class*/
    tempClass = (*env)->GetObjectClass(env, jhome);
    typedObjectCachedData->typedHome_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedHome_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object home class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

/* load the 'triggerListeners' operation */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen("([[LDDS/ObjectListener;")+((strlen(pathPrefixed)+strlen("[L")+ strlen(";"))*3)+strlen(")V")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, "([[LDDS/ObjectListener;");
    for (count = 0; count < 3; count++){
        stringHolder = os_strncat(
            stringHolder,
            "[L",
            strlen("[L"));
        stringHolder = os_strncat(
            stringHolder,
            pathPrefixed,
            strlen(pathPrefixed));
        stringHolder = os_strncat(
            stringHolder,
            ";",
            strlen(";"));
    }
    stringHolder = os_strncat(stringHolder, ")V", strlen(")V"));

    typedObjectCachedData->typedHome_triggerListeners_mid = (*env)->GetMethodID(
        env,
        typedObjectCachedData->typedHome_class,
        "triggerListeners",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

/* load the typed ObjectRoot Impl class */
    tempClass = (*env)->FindClass(env, targetImplClassName);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedRoot_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedRoot_class)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object root "
            "implementation class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    /* free the stringHolder so it can be re-used */
    /*os_free(stringHolder);
    stringHolder = NULL;*/

/* load the typed ObjectRoot constructor*/
    typedObjectCachedData->typedRoot_constructor_mid = (*env)->GetMethodID(
        env,
        typedObjectCachedData->typedRoot_class,
        "<init>", "()V");
    DLRL_JavaException_PROPAGATE(env, exception);

/* cache previous and current topic field IDs */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(topicTypePrefixed) + strlen("L") + strlen(";")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, "L");
    stringHolder = os_strncat(
        stringHolder,
        topicTypePrefixed,
        strlen(topicTypePrefixed));
    stringHolder = os_strncat(
        stringHolder,
        ";",
        strlen(";"));

    typedObjectCachedData->typedRoot_currentTopic_fid = (*env)->GetFieldID(
        env,
        typedObjectCachedData->typedRoot_class,
        "currentTopic",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);
    typedObjectCachedData->typedRoot_previousTopic_fid = (*env)->GetFieldID(
        env,
        typedObjectCachedData->typedRoot_class,
        "previousTopic",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    tempClass = (*env)->FindClass(env, topicTypePrefixed);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedTopic_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedTopic_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed topic class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

/* typed listener class */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("Listener")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "Listener", strlen("Listener"));
    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedListener_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedListener_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object listener class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

/* typed Selection class */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("Selection")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "Selection", strlen("Selection"));
    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedSelection_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedListener_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object selection class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

    /* locate the object filter class */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("Filter")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "Filter", strlen("Filter"));
    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

    /* now locate the check_object operation */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen("(L")+strlen(pathPrefixed)+strlen(";")+strlen("LDDS/MembershipState;)Z")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, "(L");
    stringHolder = os_strncat(stringHolder, pathPrefixed, strlen(pathPrefixed));
    stringHolder = os_strncat(stringHolder, ";", strlen(";"));
    stringHolder = os_strncat(stringHolder, "LDDS/MembershipState;)Z", strlen("LDDS/MembershipState;)Z"));

    typedObjectCachedData->typedFilter_checkObject_mid = (*env)->GetMethodID(
        env,
        tempClass,
        "check_object",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;

    /* typed selection listener cache info. */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("SelectionListener")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "SelectionListener", strlen("SelectionListener"));
    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("(L;)V")+1),
        exception,
        "Unable to create a string, out of resources.");
    /* now cache each operation for listener callbacks */
    os_strcpy(stringHolder, "(L");
    stringHolder = os_strncat(stringHolder, pathPrefixedExceptLast, strlen(pathPrefixedExceptLast));
    stringHolder = os_strncat(stringHolder, ";)V", strlen(";)V"));
    typedObjectCachedData->typedSelectionListener_onObjectOut_mid = (*env)->GetMethodID(
        env,
        tempClass,
        "on_object_out",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);
    typedObjectCachedData->typedSelectionListener_onObjectModified_mid = (*env)->GetMethodID(
        env,
        tempClass,
        "on_object_modified",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);
    typedObjectCachedData->typedSelectionListener_onObjectIn_mid = (*env)->GetMethodID(
        env,
        tempClass,
        "on_object_in",
        stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);
    os_free(stringHolder);
    stringHolder = NULL;
/* typed str map class & constructor */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("StrMap")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "StrMap", strlen("StrMap"));

    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedStrMap_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedStrMap_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object StrMap class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

    typedObjectCachedData->typedStrMap_constructor_mid = (*env)->GetMethodID(
        env,
        typedObjectCachedData->typedStrMap_class,
        "<init>",
        "()V");
    DLRL_JavaException_PROPAGATE(env, exception);

/* typed int map class & constructor */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("IntMap")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "IntMap", strlen("IntMap"));

    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedIntMap_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedIntMap_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object IntMap class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

    typedObjectCachedData->typedIntMap_constructor_mid = (*env)->GetMethodID(
        env,
        typedObjectCachedData->typedIntMap_class,
        "<init>",
        "()V");
    DLRL_JavaException_PROPAGATE(env, exception);
/* typed set class & constructor */
    assert(!stringHolder);
    DLRL_ALLOC_WITH_SIZE(
        stringHolder,
        (strlen(pathPrefixedExceptLast) + strlen("Set")+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(stringHolder, pathPrefixedExceptLast);
    stringHolder = os_strncat(stringHolder, "Set", strlen("Set"));
    tempClass = (*env)->FindClass(env, stringHolder);
    DLRL_JavaException_PROPAGATE(env, exception);

    typedObjectCachedData->typedSet_class = (*(env))->NewGlobalRef (
        env,
        tempClass);
    if(!typedObjectCachedData->typedSet_class){
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a global ref for the typed object Set class.");
    }
    (*env)->DeleteLocalRef(env, tempClass);
    tempClass = NULL;
    /* free the stringHolder so it can be re-used */
    os_free(stringHolder);
    stringHolder = NULL;

    typedObjectCachedData->typedSet_constructor_mid = (*env)->GetMethodID(
        env,
        typedObjectCachedData->typedSet_class,
        "<init>",
        "()V");
    DLRL_JavaException_PROPAGATE(env, exception);

/* the path string */
    DLRL_ALLOC_WITH_SIZE(
        typedObjectCachedData->pathPrefixed,
        (strlen(pathPrefixed)+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(typedObjectCachedData->pathPrefixed, pathPrefixed);
/* the path string (prefix except last) */
    DLRL_ALLOC_WITH_SIZE(
        typedObjectCachedData->pathPrefixedExceptLast,
        (strlen(pathPrefixedExceptLast)+1),
        exception,
        "Unable to create a string, out of resources.");
    os_strcpy(
        typedObjectCachedData->pathPrefixedExceptLast,
        pathPrefixedExceptLast);

    DLRL_Exception_EXIT(exception);
    if(stringHolder){
        os_free(stringHolder);
    }
    if(tempClass){
        (*env)->DeleteLocalRef(env, tempClass);
    }
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        os_free(typedObjectCachedData);
        typedObjectCachedData = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return typedObjectCachedData;
}

void
DJA_Initialisation_us_destroyTypedObjectCache(
    JNIEnv* env,
    DJA_CachedJNITypedObject* typedObjectCache)
{

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(typedObjectCache);

    if(typedObjectCache->typedHome_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedHome_class);
        typedObjectCache->typedHome_class = NULL;
    }
    if(typedObjectCache->typedListener_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedListener_class);
        typedObjectCache->typedListener_class = NULL;
    }
    if(typedObjectCache->typedSelection_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedSelection_class);
        typedObjectCache->typedSelection_class = NULL;
    }
    if(typedObjectCache->typedTopic_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedTopic_class);
        typedObjectCache->typedTopic_class = NULL;
    }

    if(typedObjectCache->typedRoot_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedRoot_class);
        typedObjectCache->typedRoot_class = NULL;
    }
    if(typedObjectCache->typedStrMap_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedStrMap_class);
        typedObjectCache->typedStrMap_class = NULL;
    }
    if(typedObjectCache->typedIntMap_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedIntMap_class);
        typedObjectCache->typedIntMap_class = NULL;
    }
    if(typedObjectCache->typedSet_class){
        (*env)->DeleteGlobalRef(env, typedObjectCache->typedSet_class);
        typedObjectCache->typedSet_class = NULL;
    }
    if(typedObjectCache->pathPrefixed){
        os_free(typedObjectCache->pathPrefixed);
        typedObjectCache->pathPrefixed = NULL;
    }
    if(typedObjectCache->pathPrefixedExceptLast){
        os_free(typedObjectCache->pathPrefixedExceptLast);
        typedObjectCache->pathPrefixedExceptLast = NULL;
    }
    while(Coll_List_getNrOfElements(&(typedObjectCache->relationFieldIDs)) > 0){
        Coll_List_popBack(&(typedObjectCache->relationFieldIDs));
    }
    while(Coll_List_getNrOfElements(&(typedObjectCache->relationIsFoundFieldIDs)) > 0){
        Coll_List_popBack(&(typedObjectCache->relationIsFoundFieldIDs));
    }
    while(Coll_List_getNrOfElements(&(typedObjectCache->collectionFieldIDs)) > 0){
        Coll_List_popBack(&(typedObjectCache->collectionFieldIDs));
    }

    os_free(typedObjectCache);
    DLRL_INFO(INF_EXIT);
}

DJA_CachedJNITypedTopic*
DJA_Initialisation_loadTypedTopicCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    DJA_CachedJNITypedObject* typedObjectCachedData,
    DK_ObjectHomeAdmin* home,
    jobject jhome)
{
    jclass tempClass = NULL;
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    LOC_string oidField = NULL;
    LOC_string nameField = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(env);
    assert(home);
    assert(typedObjectCachedData);
    assert(jhome);

    DLRL_ALLOC(typedTopicCachedData, DJA_CachedJNITypedTopic, exception, "Unable to alloc JNI typed topic cache");
	memset(typedTopicCachedData, 0, sizeof(DJA_CachedJNITypedTopic));

    /* Load the typed main topic class */
    /* TODO just copying the typeTopic_class over, left over from older code. Not fixed as it's something that needs to
     * be evaluated when DLRL starts supporting multiple topics, in which case this the way the topic class are
     * maintained needs to be evaluated and possibly changed */
    typedTopicCachedData->typedTopic_class =  (*(env))->NewGlobalRef (env, typedObjectCachedData->typedTopic_class);
    if(!typedTopicCachedData->typedTopic_class){
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for a specific topic class.");
    }
    typedTopicCachedData->typedTopic_constructor_mid = (*env)->GetMethodID(env, typedTopicCachedData->typedTopic_class,
                                                                                                    "<init>", "()V");
    DLRL_JavaException_PROPAGATE(env, exception);

    /* must free out param values if set ! */
    DK_MMFacade_us_getKeyFieldNamesForDefaultMappedObject(exception, home, &oidField, &nameField);
    DLRL_Exception_PROPAGATE(exception);

    if(oidField){
        /* cache topic key fields in case the object this topic belongs to is default*/
        typedTopicCachedData->typedTopic_oidKey_fid = (*env)->GetFieldID(env, typedTopicCachedData->typedTopic_class,
                                                                                            oidField, "LDDS/DLRLOid;");
        DLRL_JavaException_PROPAGATE(env, exception);
    }
    if(nameField){
        typedTopicCachedData->typedTopic_nameKey_fid = (*env)->GetFieldID(env, typedTopicCachedData->typedTopic_class,
                                                                                    nameField, "Ljava/lang/String;");
        DLRL_JavaException_PROPAGATE(env, exception);
    }

    DLRL_Exception_EXIT(exception);
    if(oidField){
        os_free(oidField);
    }
    if(nameField){
        os_free(nameField);
    }
    if(tempClass){
        (*env)->DeleteLocalRef(env, tempClass);
    }

    DLRL_INFO(INF_EXIT);
    return typedTopicCachedData;
}

void
DJA_Initialisation_us_destroyTypedTopicCache(
    JNIEnv* env,
    DJA_CachedJNITypedTopic* typedTopicCache)
{

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(typedTopicCache);

    if(typedTopicCache->typedTopic_class){
        (*env)->DeleteGlobalRef(env, typedTopicCache->typedTopic_class);
        typedTopicCache->typedTopic_class = NULL;
    }

    os_free(typedTopicCache);
    DLRL_INFO(INF_EXIT);
}

void
DJA_Initialisation_loadTypedReaderCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    DJA_CachedJNITypedTopic* typedTopicCachedData,
    jobject jreader)
{
    jclass tempClass = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);
    assert(typedTopicCachedData);
    assert(jreader);

    /* Load the typed reader class */
    tempClass = (*env)->GetObjectClass(env, jreader);
    assert(tempClass);

    /* Load the copy cache field identifier */
    typedTopicCachedData->typedReader_copyCache_fid = (*env)->GetFieldID(env, tempClass, "copyCache", "J");
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    if(tempClass){
        (*env)->DeleteLocalRef(env, tempClass);
    }
    DLRL_INFO(INF_EXIT);
}

void
DJA_Initialisation_loadTypedWriterCache(
    DLRL_Exception* exception,
    JNIEnv* env,
    DJA_CachedJNITypedTopic* typedTopicCachedData,
    jobject jwriter)
{
    jclass tempClass = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);
    assert(typedTopicCachedData);
    assert(jwriter);

    /* Load the typed writer class */
    tempClass = (*env)->GetObjectClass(env, jwriter);
    assert(tempClass);

    /* Load the copy cache field identifier */
    typedTopicCachedData->typedWriter_copyCache_fid = (*env)->GetFieldID(env, tempClass, "copyCache", "J");
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    if(tempClass){
        (*env)->DeleteLocalRef(env, tempClass);
    }
    DLRL_INFO(INF_EXIT);
}

/* assumes lock (admin) on the cache and admin locks on all required homes */
void
DJA_Initialisation_processRelations(
    DLRL_Exception* exception,
    JNIEnv* env,
    DK_CacheAdmin* adminLockedCache,
    DK_ObjectHomeAdmin* lockedOwnerHome,
    Coll_List* relationNames,
    Coll_List* targetFieldList,
    Coll_List* targetFieldIsFoundList,
    DJA_CachedJNITypedObject* ownerObjectCachedData)
{
    LOC_string relationName = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;
    DJA_CachedJNITypedObject* targetObjectCachedData = NULL;
    LOC_string targetRelationName = NULL;
    LOC_string jniObjectRelationName = NULL;
    DK_RelationType relationType = DK_RELATION_TYPE_REF;/* default */
    Coll_Iter* iterator = NULL;
    jfieldID relationField = NULL;
    LOC_long returnCode = COLL_OK;
    LOC_string jniObjectRelationIsFoundName = NULL;
    jfieldID relationIsFoundField = NULL;
    LOC_string basePath = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(env);
    assert(adminLockedCache);
    assert(lockedOwnerHome);
    assert(relationNames);
    assert(targetFieldList);
    /* targetFieldIsFoundList may be null */
    assert(ownerObjectCachedData);

    /* dependant on the sequence of elements within the relation names. This sequence is leading in terms of indexes. */
    /* Always begin at index 0 and push back at the end to ensure no problems arise from index mismatches */
    iterator = Coll_List_getFirstElement(relationNames);

    while(iterator){
        relationName = (LOC_string)Coll_Iter_getObject(iterator);
        targetRelationName = DK_MMFacade_us_getTargetTypeNameForRelation(lockedOwnerHome, relationName);
        if(!targetRelationName){
             DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to retrieve the relation type name for relation %s of DLRL Kernel ObjectHome '%p'. "
                "The relation type name is not known within the DLRL.", DLRL_VALID_NAME(relationName),
                 lockedOwnerHome);
        }
        targetHome = DK_CacheAdmin_us_findHomeByName(adminLockedCache, targetRelationName);
        if(!targetHome){
             DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to retrieve the relation type '%s' for relation %s of DLRL Kernel ObjectHome '%p'. "
                "There is no ObjectHome known that manages the specified type.",
                 DLRL_VALID_NAME(targetRelationName), DLRL_VALID_NAME(relationName), lockedOwnerHome);
        }
        targetObjectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(targetHome);
        relationType = DK_MMFacade_us_getRelationType(lockedOwnerHome, exception, relationName);
        DLRL_Exception_PROPAGATE(exception);

        if(relationType == DK_RELATION_TYPE_REF){
            basePath = targetObjectCachedData->pathPrefixed;
            DLRL_ALLOC_WITH_SIZE(jniObjectRelationName, (strlen(basePath)+strlen("L")+strlen(";")+1),
                                                        exception, "Failed to allocate string, out of resources.");
            os_strcpy(jniObjectRelationName, "L");
            jniObjectRelationName = os_strncat(jniObjectRelationName, basePath, strlen(basePath));
            jniObjectRelationName = os_strncat(jniObjectRelationName, ";", strlen(";"));
        } else if(relationType == DK_RELATION_TYPE_STR_MAP){
            basePath = targetObjectCachedData->pathPrefixedExceptLast;
            DLRL_ALLOC_WITH_SIZE(jniObjectRelationName, (strlen(basePath)+strlen("L")+strlen(";")+
                                    strlen("StrMap")+1),exception, "Failed to allocate string, out of resources.");
            os_strcpy(jniObjectRelationName, "L");
            jniObjectRelationName = os_strncat(jniObjectRelationName, basePath, strlen(basePath));
            jniObjectRelationName = os_strncat(jniObjectRelationName, "StrMap", strlen("StrMap"));
            jniObjectRelationName = os_strncat(jniObjectRelationName, ";", strlen(";"));

        } else if(relationType == DK_RELATION_TYPE_INT_MAP){
            basePath = targetObjectCachedData->pathPrefixedExceptLast;
            DLRL_ALLOC_WITH_SIZE(jniObjectRelationName, (strlen(basePath)+strlen("L")+strlen(";")+
                                    strlen("IntMap")+1),exception, "Failed to allocate string, out of resources.");
            os_strcpy(jniObjectRelationName, "L");
            jniObjectRelationName = os_strncat(jniObjectRelationName, basePath, strlen(basePath));
            jniObjectRelationName = os_strncat(jniObjectRelationName, "IntMap", strlen("IntMap"));
            jniObjectRelationName = os_strncat(jniObjectRelationName, ";", strlen(";"));
        } else {
            assert(relationType == DK_RELATION_TYPE_SET);
            basePath = targetObjectCachedData->pathPrefixedExceptLast;
            DLRL_ALLOC_WITH_SIZE(jniObjectRelationName, (strlen(basePath)+strlen("L")+strlen(";")+
                                    strlen("Set")+1),exception, "Failed to allocate string, out of resources.");
            os_strcpy(jniObjectRelationName, "L");
            jniObjectRelationName = os_strncat(jniObjectRelationName, basePath, strlen(basePath));
            jniObjectRelationName = os_strncat(jniObjectRelationName, "Set", strlen("Set"));
            jniObjectRelationName = os_strncat(jniObjectRelationName, ";", strlen(";"));
        }

        relationField = (*env)->GetFieldID(env, ownerObjectCachedData->typedRoot_class, relationName, jniObjectRelationName);
        DLRL_JavaException_PROPAGATE(env, exception);

        returnCode = Coll_List_pushBack(targetFieldList, relationField);
        if(returnCode != COLL_OK){
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to push a relation field of a relation into the list of relations fields.");
        }

        if(targetFieldIsFoundList){
            jniObjectRelationIsFoundName = (LOC_string)os_malloc(strlen(relationName)+strlen("IsFound")+1);
            if(!jniObjectRelationIsFoundName){
                DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a object relation IsFound name string, out of resources.");
            }
            os_strcpy(jniObjectRelationIsFoundName, relationName);
            jniObjectRelationIsFoundName = os_strncat(jniObjectRelationIsFoundName, "IsFound", strlen("IsFound"));
            relationIsFoundField = (*env)->GetFieldID(env, ownerObjectCachedData->typedRoot_class, jniObjectRelationIsFoundName, "Z");
            DLRL_JavaException_PROPAGATE(env, exception);

            returnCode = Coll_List_pushBack(targetFieldIsFoundList, relationIsFoundField);
            if(returnCode != COLL_OK){
                DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to push a relationIsFound field of a relation into the list of relationIsFound fields.");
            }
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    if(jniObjectRelationName){
        os_free(jniObjectRelationName);
    }
    if(jniObjectRelationIsFoundName){
        os_free(jniObjectRelationIsFoundName);
    }
    DLRL_INFO(INF_EXIT);
}
