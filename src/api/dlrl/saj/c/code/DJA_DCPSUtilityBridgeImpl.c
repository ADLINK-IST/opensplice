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

#include "saj_utilities.h"
#include "saj_copyOut.h"
#include "gapi_common.h"
#include "gapi_object.h"
#include "gapi_entity.h"
#include "gapi_topic.h"
#include "gapi_dataReader.h"
#include "gapi_dataWriter.h"
#include "DLRL_Types.h"

#include "os_heap.h"

/* DLRL includes */
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"

#include "DLRL_Kernel_private.h"
#include "DJA_DCPSUtilityBridge.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

/* JNI includes */
#include <jni.h>

#define ENTITY_NAME "DLRL Java API DCPSUtilityBridge"

u_topic
DJA_DCPSUtilityBridge_us_createTopic(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    LOC_char* topicName,
    LOC_char* typeName,
    void** topicUserData,
    void** ls_topic,
    LOC_boolean isMainTopic)
{
    /* JNI thread env */
    JNIEnv* env = (JNIEnv*)userData;
    /* Cached JNI data used within this call */
    jmethodID createTopicMid = cachedJNI.dcpsUtil_createTopic_mid;
    /* java topic entity description */
    jobject jtopic = NULL;
    /* needed string representations of topic name and type */
    jstring jtypeName = NULL;
    jstring jtopicName = NULL;
    /* the java object home */
    jobject ls_objectHome = NULL;
    jobject ls_participant = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    gapi_topic gapiTopic = NULL;
    _Topic _topic = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_topic utopic = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(home);
    assert(topicName);
    assert(typeName);

    /* get java objects */
    /* the cache getter on the object home doesnt require a release on the returned pointer */
    ls_participant = (jobject)DK_CacheAdmin_us_getLSParticipant(DK_ObjectHomeAdmin_us_getCache(home));
    ls_objectHome = (jobject)DK_ObjectHomeAdmin_us_getLSHome(home);

    typedObjectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    if(isMainTopic)
    {
        typedTopicCachedData = DJA_Initialisation_loadTypedTopicCache(exception, env, typedObjectCachedData, home, ls_objectHome);
        DLRL_Exception_PROPAGATE(exception);
        *topicUserData = (void*)typedTopicCachedData;
    }
    jtypeName = (*env)->NewStringUTF(env, typeName);
    DLRL_JavaException_PROPAGATE(env, exception);
    jtopicName = (*env)->NewStringUTF(env, topicName);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_INFO(INF_CALLBACK, "objectHome->createTopic(participant, topicName, typeName)");
    jtopic = (*env)->CallStaticObjectMethod(env, cachedJNI.dcpsUtil_class, createTopicMid, ls_participant, jtopicName, jtypeName);
    DLRL_JavaException_PROPAGATE(env, exception);
    if(!jtopic){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Unable to create the DCPS topic with name '%s' and type '%s' "
                             "Check DCPS error log file for (possibly) more information.",
                             topicName, typeName);
    }
    DLRL_INFO(INF_DCPS, "saj_read_gapi_address(topic)");
    gapiTopic = jtopic? (gapi_topic)saj_read_gapi_address(env, jtopic) : NULL;
    if(!gapiTopic){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Unable to create the DCPS Topic entity for "
                             "topic %s with type %s of %s '%p'. "
                             "Check DCPS error log file for (possibly) more "
                             "information.",
                             DLRL_VALID_NAME(topicName),
                             DLRL_VALID_NAME(typeName),
                             "DK_ObjectHomeAdmin",
                             home);
    }
    _topic = gapi_topicClaim(gapiTopic, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result,
                                         "Failed to claim the topic handle");
    utopic = _TopicUtopic(_topic);
    if(utopic)
    {
        /* now create a proxy to this user layer topic which can be used by the DLRL
         * in a safe manner, as the user layer topic returned by the _TopicUtopic
         * operation is owned by the gapi.
         */
        utopic = u_topic(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(utopic)));
    }
    _EntityRelease(_topic);/* before the propagate */
    DLRL_Exception_PROPAGATE(exception);/* after the release */
    if(!utopic){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Unable to create the DCPS Topic entity for "
                             "topic %s with type %s of %s '%p'. "
                             "Check DCPS error log file for (possibly) more "
                             "information.",
                             DLRL_VALID_NAME(topicName),
                             DLRL_VALID_NAME(typeName),
                             "DK_ObjectHomeAdmin",
                             home);
    }
    *ls_topic = (void*)(*(env))->NewGlobalRef (env, jtopic);
    if(!(*ls_topic))
    {
        u_entityFree(u_entity(utopic));
        utopic = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                             "Unable to create a global ref for the "
                             "type specific topic class.");
    }

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && typedTopicCachedData){
        DJA_Initialisation_us_destroyTypedTopicCache(env, typedTopicCachedData);
        *topicUserData = NULL;
    }
    if(jtopic){
        (*env)->DeleteLocalRef(env, jtopic);
    }
    if(jtopicName){
        (*env)->DeleteLocalRef(env, jtopicName);
    }
    if(jtypeName){
        (*env)->DeleteLocalRef(env, jtypeName);
    }
    /* returning NULL means an error has occured */
    DLRL_INFO(INF_EXIT);
    return utopic;
}

u_reader
DJA_DCPSUtilityBridge_us_createDataReader(
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    void** ls_reader)
{
    /* JNI thread env */
    JNIEnv* env = NULL;
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    jobject jtopic = NULL;
    jobject ls_participant = NULL;
    jstring jtopicName = NULL;
    DK_CacheAdmin* cache = NULL;
    jobject ls_subscriber = NULL;
    jobject jdataReader= NULL;
    DK_ObjectHomeAdmin* home = NULL;
    LOC_char* topicName = NULL;
    gapi_dataReader gapiReader = NULL;
    _DataReader _reader = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_reader ureader = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(topicInfo);

    env = (JNIEnv*)userData;
    home = DK_TopicInfo_us_getOwner(topicInfo);/* no duplicate done */
    topicName = (LOC_char*)DK_TopicInfo_us_getTopicName(topicInfo);
    assert(topicName);
    typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    /* typedTopicCachedData may be null*/

    /* get java object */
    jtopic = (jobject)DK_TopicInfo_us_getLSTopic(topicInfo);

    /* the cache getter on the object home doesnt require a release on the returned pointer */
    cache = DK_ObjectHomeAdmin_us_getCache(home);
    ls_participant = (jobject)DK_CacheAdmin_us_getLSParticipant(cache);
    ls_subscriber = (jobject)DK_CacheAdmin_us_getLSSubscriber(cache);
    jtopicName = (*env)->NewStringUTF(env, topicName);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_INFO(INF_CALLBACK, "objectHome->createDataReader(participant, subscriber, topic, topicName)");
    jdataReader = (*env)->CallStaticObjectMethod(env, cachedJNI.dcpsUtil_class,
                                                 cachedJNI.dcpsUtil_createDataReader_mid,
                                                 ls_participant,
                                                 ls_subscriber,
                                                 jtopic,
                                                 jtopicName);
    DLRL_JavaException_PROPAGATE(env, exception);
    if(!jdataReader){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Creation of data reader for topic %s failed! "
                             "Check DCPS error log file for (possibly) more information.",
                             topicName);
    }

    if(typedTopicCachedData)
    {
        /* no need to undo the following action in case of an exception later on, as the typedTopicCachedData is already  */
        /* managed by the DLRL whom will ensure its cleaned properly. */
        DJA_Initialisation_loadTypedReaderCache(exception, env, typedTopicCachedData, jdataReader);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_INFO(INF_DCPS, "saj_read_gapi_address(datareader)");
    gapiReader = jdataReader? (gapi_dataReader)saj_read_gapi_address(env, jdataReader) : NULL;
    if(!gapiReader){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
            "Unable to create the DCPS DataReader entity for topic %s of DLRL Kernel ObjectHomeAdmin '%s'. "
            "Check DCPS error log file for (possibly) more information.",
            DLRL_VALID_NAME(topicName), DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(home)));
    }
    _reader = gapi_dataReaderClaim(gapiReader, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result, "Failed to claim the data reader handle");
    ureader = u_reader(_DataReaderUreader(_reader));
    if(ureader)
    {
        /* now create a proxy to this user layer ureader which can be used by the DLRL
         * in a safe manner, as the user layer ureader returned by the _DataReaderUreader
         * operation is owned by the gapi.
         */
        ureader = u_reader(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(ureader)));
    }
    _EntityRelease(_reader);/* before the propagate */
    DLRL_Exception_PROPAGATE(exception);/* after the release */
    if(!ureader){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
            "Unable to create the DCPS DataReader entity for topic %s of DLRL Kernel ObjectHomeAdmin '%s'. "
            "Check DCPS error log file for (possibly) more information.",
            DLRL_VALID_NAME(topicName), DLRL_VALID_NAME(DK_ObjectHomeAdmin_us_getName(home)));
    }

    *ls_reader = (void*)(*(env))->NewGlobalRef (env, jdataReader);
    if(!(*ls_reader))
    {
        u_entityFree(u_entity(ureader));
        ureader = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to create a global ref for the type specific DataReader class.");
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        /* return null if an exception occured, clean the data reader if it existed (later an exception will be set when  */
        /* JNI raises an exception as well */
        ureader = NULL;
    }
    if(jtopicName){
        (*env)->DeleteLocalRef(env, jtopicName);
    }
    if(jdataReader){
        (*env)->DeleteLocalRef(env, jdataReader);
    }

    DLRL_INFO(INF_EXIT);
    return ureader;
}

u_writer
DJA_DCPSUtilityBridge_us_createDataWriter(
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo,
    void** ls_writer)
{
    /* JNI thread env */
    JNIEnv* env = NULL;

    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    /* java data writer & topic entities */
    jobject jdataWriter = NULL;
    jobject jtopic = NULL;
    jobject ls_publisher = NULL;
    DK_CacheAdmin* cache = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    gapi_dataWriter gapiWriter = NULL;
    _DataWriter _writer = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_writer uwriter = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(topicInfo);

    home = DK_TopicInfo_us_getOwner(topicInfo);/* no duplicate done */
    env = (JNIEnv*)userData;
    typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    /* typedTopicCachedData may be NULL */
    cache = DK_ObjectHomeAdmin_us_getCache(home);

    /* get java object */
    jtopic = (jobject)DK_TopicInfo_us_getLSTopic(topicInfo);
    ls_publisher = jtopic ? (jobject)DK_CacheAdmin_us_getLSPublisher(cache) : NULL;

    DLRL_INFO(INF_CALLBACK, "objectHome->createDataWriter(publisher, topic)");
    jdataWriter = jtopic ? (*env)->CallStaticObjectMethod(env, cachedJNI.dcpsUtil_class,
                                                cachedJNI.dcpsUtil_createDataWriter_mid, ls_publisher, jtopic) : NULL;
    DLRL_JavaException_PROPAGATE(env, exception);
    if(!jdataWriter){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Creation of data writer failed! "
                             "Check DCPS error log file for (possibly) more "
                             "information.");
    }
    if(typedTopicCachedData)
    {
        DJA_Initialisation_loadTypedWriterCache(exception, env, typedTopicCachedData, jdataWriter);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_INFO(INF_DCPS, "saj_read_gapi_address(datawriter)");
    gapiWriter = jdataWriter? (gapi_dataWriter)saj_read_gapi_address(env, jdataWriter) : NULL;
    if(!gapiWriter){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Unable to create the DCPS DataWriter entity. "
                             "Check DCPS error log file for (possibly) more "
                             "information.");
    }
    _writer = gapi_dataWriterClaim(gapiWriter, &result);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, result,
                                         "Unable to claim data writer handle");
    uwriter = U_WRITER_GET(_writer);
    if(uwriter)
    {
        /* now create a proxy to this user layer topic which can be used by the DLRL
         * in a safe manner, as the user layer topic returned by the _TopicUtopic
         * operation is owned by the gapi.
         */
        uwriter = u_writer(DK_DCPSUtility_ts_createProxyUserEntity(exception, u_entity(uwriter)));
    }
    _EntityRelease(_writer);/* before the propagate */
    DLRL_Exception_PROPAGATE(exception);/* after the release */
    if(!uwriter){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
                             "Unable to create the DCPS DataWriter entity. "
                             "Check DCPS error log file for (possibly) more "
                             "information.");
    }
    *ls_writer = (void*)(*(env))->NewGlobalRef (env, jdataWriter);
    if (!(*ls_writer))
    {
        u_entityFree(u_entity(uwriter));
        uwriter = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                             "Not enough memory to complete operation.");
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION){
        /* return null if an exception occured,
           clean the data reader if it existed
           (later an exception will be set when
            JNI raises an exception as well */
        uwriter = NULL;
    }
    if(jdataWriter){
        (*env)->DeleteLocalRef(env, jdataWriter);
    }
    DLRL_INFO(INF_EXIT);
    return uwriter;
}

void
DJA_DCPSUtilityBridge_us_registerType(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_CacheAdmin* cache,
    LOC_char* topicName,
    LOC_char* typeName)
{
    /* JNI thread env */
    JNIEnv* env = (JNIEnv*)userData;
    /* return value */
    LOC_ReturnCode_t returnCode = LOC_RETCODE_OK;
    /* Cached JNI data used within this call */
    jmethodID registerTypeMid = cachedJNI.objectHome_registerType_mid;
    /* the java object particpant */
    jobject ls_participant = NULL;
    /* used JNI variables */
    jstring jtopicName = NULL;
    jstring jtopicType = NULL;
    DLRL_LS_object ls_home = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(cache);
    assert(home);
    assert(topicName);
    assert(typeName);

    /* get the java objects */
    ls_participant = (jobject)DK_CacheAdmin_us_getLSParticipant(cache);
    ls_home = DK_ObjectHomeAdmin_us_getLSHome(home);
    jtopicName = (*env)->NewStringUTF(env, topicName);
    DLRL_JavaException_PROPAGATE(env, exception);

    jtopicType = (*env)->NewStringUTF(env, typeName);
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_INFO(INF_CALLBACK, "objectHome->registerType(participant, typeName, topicName)");
    returnCode = (LOC_ReturnCode_t)(*env)->CallIntMethod(env, ls_home, registerTypeMid,
                                                   (jobject)ls_participant, jtopicType, jtopicName);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode,
                                 "Unable to register type to DCPS TypeSupport entity for "
                                 "topic %s of %s '%p'.",
                                 DLRL_VALID_NAME(topicName),
                                 "DK_ObjectHomeAdmin",
                                 home);

    DLRL_Exception_EXIT(exception);
    if(jtopicName){
        (*env)->DeleteLocalRef(env, jtopicName);
    }
    if(jtopicType){
        (*env)->DeleteLocalRef(env, jtopicType);
    }
    DLRL_INFO(INF_EXIT);
}

void
DJA_DCPSUtilityBridge_us_deleteDataReader(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    u_reader reader,
    DLRL_LS_object ls_reader)
{
    /* JNI thread env */
    JNIEnv* env = (JNIEnv *)userData;
    /* Java subscriber entity representation */
    jobject jsubscriber;
    /* Java data reader entity representation */
    jobject jdatareader;
    /* Cached JNI data used within this call */
    jmethodID deleteDataReaderMid = cachedJNI.dcpsUtil_deleteDataReader_mid;
    LOC_ReturnCode_t returnCode = LOC_RETCODE_OK;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(cache);
    assert(reader);
    assert(ls_reader);
    assert(exception);

    jdatareader = (jobject)ls_reader;
    jsubscriber = (jobject)DK_CacheAdmin_us_getLSSubscriber(cache);

    DLRL_INFO(INF_CALLBACK, "objectHome->deleteDataReader(subscriber, datareader)");
    returnCode = (LOC_ReturnCode_t)(*env)->CallStaticIntMethod(env, cachedJNI.dcpsUtil_class, deleteDataReaderMid,
                                                                jsubscriber, jdatareader);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode,
                                 "Delete of datareader failed");

    (*env)->DeleteGlobalRef(env, jdatareader);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_DCPSUtilityBridge_us_deleteDataWriter(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    u_writer writer,
    DLRL_LS_object ls_writer)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;
    /* Java publisher entity representation */
    jobject jpublisher;
    /* Java data writer entity representation */
    jobject jdatawriter;
    /* Cached JNI data used within this call */
    jmethodID deleteDataWriterMid = cachedJNI.dcpsUtil_deleteDataWriter_mid;
    LOC_ReturnCode_t returnCode = LOC_RETCODE_OK;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(cache);
    assert(writer);
    assert(ls_writer);
    assert(exception);

    /* retrieve the Java objects */
    jpublisher = (jobject)DK_CacheAdmin_us_getLSPublisher(cache);
    jdatawriter = (jobject)ls_writer;

    DLRL_INFO(INF_CALLBACK, "objectHome->deleteDataWriter(publisher, datawriter)");
    returnCode = (LOC_ReturnCode_t)(*env)->CallStaticIntMethod(env, cachedJNI.dcpsUtil_class, deleteDataWriterMid, jpublisher,
                                                        jdatawriter);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode,
                                 "Delete of datawriter failed");
    (*env)->DeleteGlobalRef(env, jdatawriter);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_DCPSUtilityBridge_us_deleteTopic(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache,
    DK_TopicInfo* topicInfo)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;
    /* Java participant entity representation */
    jobject jparticipant;
    /* Java topic entity representation */
    jobject jtopic;
    /* Cached JNI data used within this call */
    jmethodID deleteTopicMid = cachedJNI.dcpsUtil_deleteTopic_mid;
    /* the java object home */
    LOC_ReturnCode_t returnCode = LOC_RETCODE_OK;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(cache);
    assert(topicInfo);
    assert(exception);

    /* retrieve the Java objects */
    jparticipant = (jobject)DK_CacheAdmin_us_getLSParticipant(cache);
    jtopic = (jobject)DK_TopicInfo_us_getLSTopic(topicInfo);

    DLRL_INFO(INF_CALLBACK, "objectHome->deleteTopic(participant, topic)");
    returnCode = (LOC_ReturnCode_t)(*env)->CallStaticIntMethod(env, cachedJNI.dcpsUtil_class, deleteTopicMid,
                                                                jparticipant, jtopic);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode,
                                 "Delete of topic failed");
    (*env)->DeleteGlobalRef(env, jtopic);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_DCPSUtilityBridge_us_releaseTopicUserData(
    void* userData,
    void* topicUserData)
{

    DLRL_INFO(INF_ENTER);
    assert(userData);

    if(topicUserData)
    {
        DJA_Initialisation_us_destroyTypedTopicCache((JNIEnv*)userData, (DJA_CachedJNITypedTopic*) topicUserData);
    }

    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
void
DJA_DCPSUtilityBridge_us_enableEntity(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_entity)
{
    /* JNI thread env */
    JNIEnv * env = (JNIEnv *)userData;
    LOC_ReturnCode_t returnCode;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(ls_entity);

    returnCode = (LOC_ReturnCode_t)(*env)->CallIntMethod(env, ls_entity, cachedJNI.entity_enable_mid);
    DLRL_JavaException_PROPAGATE(env, exception);
    DLRL_DcpsException_PROPAGATE(exception, returnCode,
                                 "Enable of entity failed");

    DLRL_INFO(INF_EXIT);
    DLRL_Exception_EXIT(exception);
}
