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

/* DLRL includes */
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
/* DCPS SAJ includes */
#include "saj_utilities.h"
#include "saj_copyOut.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"


#define ENTITY_NAME "DLRL Kernel ObjectReaderBridge"

/* assumes the owning home of this data reader is locked as well as any related homes needed to manage relations. */
void
DJA_ObjectReaderBridge_us_doLSReadPreProcessing(
    DK_ReadInfo* readInfo,
    DK_ObjectReader* objReader)
{

    JNIEnv* env = NULL;
    DLRL_LS_object ls_reader  = NULL;
    DJA_CachedJNITypedTopic*  typedTopicCachedData= NULL;
    DLRL_Exception* exception = NULL;
    DK_TopicInfo* topicInfo = NULL;
    /* allocate the readInfo on stack to prevent an alloc. */
    C_STRUCT(saj_dstInfo) dst;

    DLRL_INFO(INF_ENTER);
    assert(readInfo);
    assert(objReader);

    /* get the cached JNI data from the user data field of the associated cached topic jni info from the main topic of  */
    /* the object home */
    topicInfo = DK_ObjectReader_us_getTopicInfo(objReader);
    ls_reader = DK_ObjectReader_us_getLSReader(objReader);
    typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    assert(typedTopicCachedData);
    env = (JNIEnv*)readInfo->userData;
    exception = readInfo->exception;

    dst.javaEnv = env;
    /* Setting the java object to NIL means that the copy out function will construct a new object */
    dst.javaObject = NULL;
    dst.javaClass = typedTopicCachedData->typedTopic_class;
    /* can also be done once when entering the reader copy (getting the long value) */
    dst.copyProgram = (saj_copyCache)(*env)->GetLongField(env, (jobject)ls_reader,
                                                                    typedTopicCachedData->typedReader_copyCache_fid);
    if(!dst.copyProgram){
        DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR,
            "Failed to retrieve the copy algoritm from a DCPS DataReader. "
            "Check DCPS error log file for (possibly) more information.");
    }
    /* set java DCPS copy out routine user data */
    readInfo->dstInfo = &dst;
    readInfo->copyOut = saj_copyOutStruct;
    /* continue the read in the kernel. This ensure the stack remains intact (the dst was allocated on stack!) */
    DK_ObjectReader_us_doRead(objReader, exception, readInfo);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DLRL_LS_object
DJA_ObjectReaderBridge_us_createLSTopic(
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin,/* if this is null it means that you should create a new ls topic */
    void* dstInfo,
    void (*ls_copyOut)(void*, void*),
    void* sampleData)
{
    struct saj_dstInfo_s* dst;

    DLRL_INFO(INF_ENTER);
    assert(dstInfo);
    assert(ls_copyOut);

    dst = (struct saj_dstInfo_s*)dstInfo;
    dst->javaObject = NULL;

    /* If the object existed before, examine whether its previous topic can be recycled. */
    if (objectAdmin) {
        DK_ObjectHomeAdmin* home = NULL;
        DJA_CachedJNITypedObject* objectCachedData = NULL;
        DLRL_LS_object lsObject = NULL;
        JNIEnv* env = dst->javaEnv;

        /* get the cached Java info from the user data field stored in the ObjectHomeAdmin. */
        home = DK_ObjectAdmin_us_getHome(objectAdmin);
        objectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(objectCachedData);

        /* Attempt to access the previous topic field of the owning object. */
        lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);
        dst->javaObject = (*env)->GetObjectField(env, lsObject, objectCachedData->typedRoot_previousTopic_fid);
    }

    DLRL_INFO(INF_DCPS, "copy_out(...)");
    ls_copyOut(sampleData, dst);

    DLRL_INFO(INF_EXIT);
    return (DLRL_LS_object)dst->javaObject;
}

void
DJA_ObjectReaderBridge_us_setCollectionToLSObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin,
    DK_Collection* collection,
    LOC_unsigned_long collectionIndex)
{
    DJA_CachedJNITypedObject* ownerObjectCachedData = NULL;
    jobject ownerRoot = NULL;
    jobject collectionObject = NULL;
    jfieldID collectionFieldID = NULL;
    /* java thread env pointer */
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(exception);
    assert(home);
    assert(objectAdmin);
    assert(collection);

    ownerObjectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    if(env && ownerObjectCachedData){
        ownerRoot = (jobject)DK_ObjectAdmin_us_getLSObject(objectAdmin);
        collectionObject = (jobject)DK_Collection_us_getLSObject(collection);
        if(ownerRoot != NULL){
            assert(collectionObject);
            collectionFieldID = Coll_List_getObject(&(ownerObjectCachedData->collectionFieldIDs), collectionIndex);
            assert(collectionFieldID);
            (*env)->SetObjectField(env, ownerRoot, collectionFieldID, collectionObject);
        }/* else do nothing */
    }
    /*  Java exception cannot occur */
    DLRL_INFO(INF_EXIT);
}

void
DJA_ObjectReaderBridge_us_updateObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* object,
    DLRL_LS_object ls_topic)
{
    DJA_CachedJNITypedObject* objectCachedData = NULL;
    JNIEnv* env = (JNIEnv*)userData;
    jobject previousTopic = NULL;
    jobject ls_object = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(env);
    assert(home);
    assert(object);
    /* ls_topic may be NULL */

    objectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    ls_object = (jobject)DK_ObjectAdmin_us_getLSObject(object);
    assert(ls_object);

    /* Replace the previous topic field by the current topic field. */
    previousTopic = (*env)->GetObjectField(env, ls_object, objectCachedData->typedRoot_currentTopic_fid);
    (*env)->SetObjectField(env, ls_object, objectCachedData->typedRoot_previousTopic_fid, previousTopic);

    /* set new topic sample as current topic */
    (*env)->SetObjectField(env, ls_object, objectCachedData->typedRoot_currentTopic_fid, ls_topic);

     /* Set the validity field for the previous topic if it has been modified. */
    if (previousTopic) {
        /* Set the validity for the previousTopic field. */
        (*env)->SetBooleanField(env, ls_object, cachedJNI.objectRoot_prevTopicValid_fid, TRUE);
    }

 /*not (yet) used:   DLRL_Exception_EXIT(exception);*/
    DLRL_INFO(INF_EXIT);
}

void
DJA_ObjectReaderBridge_us_resetLSModificationInfo(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    JNIEnv* env = (JNIEnv*)userData;
    jobject ls_object = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(env);
    assert(objectAdmin);

    /* Obtain the Java representation of the object. */
    ls_object = (jobject)DK_ObjectAdmin_us_getLSObject(objectAdmin);
    assert(ls_object);

    /* Reset the validity for the previousTopic field. */
    (*env)->SetBooleanField(env, ls_object, cachedJNI.objectRoot_prevTopicValid_fid, FALSE);

    DLRL_INFO(INF_EXIT);
}
