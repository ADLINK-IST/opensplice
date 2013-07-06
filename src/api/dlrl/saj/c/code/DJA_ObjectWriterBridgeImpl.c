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
/* NOT IN DESIGN -  entire class */

/* C includes */
#include <stdio.h>
#include <assert.h>

/* SAJ includes */
#include "saj_copyIn.h"

#include "DLRL_Types.h"
/* DLRL includes */
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"
#include "gapi.h"
#include "saj_utilities.h"

#define ENTITY_NAME "DLRL Kernel ObjectWriterBridge"

u_instanceHandle
DJA_ObjectWriterBridge_us_registerInstance(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectWriter* objWriter,
    DK_ObjectAdmin* objectAdmin)
{
    DK_TopicInfo* topicInfo = NULL;
    struct saj_srcInfo_s srcInfo;
    const gapi_foo* src = NULL;
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    JNIEnv* env = (JNIEnv*)userData;
    u_instanceHandle handle = DK_DCPSUtility_ts_getNilHandle();
    u_writer writer = NULL;
    DLRL_LS_object ls_writer = NULL;
    DLRL_LS_object ls_objAdmin = NULL;
    DJA_CachedJNITypedObject* objectCachedData = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    gapi_fooDataWriter gapiWriter = NULL;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(objWriter);
    assert(objectAdmin);
    assert(exception);

    srcInfo.javaObject = NULL;
    writer = DK_ObjectWriter_us_getWriter(objWriter);
    topicInfo = DK_ObjectWriter_us_getTopicInfo(objWriter);
    ls_writer = DK_ObjectWriter_us_getLSWriter(objWriter);
    ls_objAdmin = DK_ObjectAdmin_us_getLSObject(objectAdmin);
    typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    assert(typedTopicCachedData);
    /* get the cached JNI data from the user data field stored in the java home */
    home = DK_TopicInfo_us_getOwner(topicInfo);
    objectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(objectCachedData);

    srcInfo.javaEnv = env;
    srcInfo.javaObject = (*env)->GetObjectField(env, ls_objAdmin, objectCachedData->typedRoot_currentTopic_fid);
    if(!srcInfo.javaObject){
        DLRL_Exception_THROW(exception, DLRL_ERROR, "A null pointer was encountered when reading the current topic "
                            "field of object '%p'.",ls_objAdmin);
    }
    srcInfo.copyProgram = (saj_copyCache)(*env)->GetLongField(env, (jobject)ls_writer,
                                                                    typedTopicCachedData->typedWriter_copyCache_fid);
    src = (const gapi_foo*)&srcInfo;
/* TODO ID: ?? might want to optimize getting the gapi data writer do this in a struct (IE 1 struct with 2 pointers as  */
/* stored LS_writer instead of the java writer */
    gapiWriter = (gapi_fooDataWriter)saj_read_gapi_address(env, ls_writer);
    handle = (gapi_instanceHandle_t)gapi_fooDataWriter_register_instance(gapiWriter, src);

    DLRL_Exception_EXIT(exception);
    if(srcInfo.javaObject){
        (*env)->DeleteLocalRef(env, srcInfo.javaObject);
    }
    DLRL_INFO(INF_EXIT);
    return handle;
}

/* may be optimized later by doing some prep work only once, but we'll see if thats benefitial later on. */
void
DJA_ObjectWriterBridge_us_write(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectWriter* writer,
    DK_ObjectAdmin* object)
{
    JNIEnv* env = (JNIEnv*)userData;
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo* src = NULL;
    DK_TopicInfo* topicInfo = NULL;
    DJA_CachedJNITypedTopic* topicUserData = NULL;
    DJA_CachedJNITypedObject* objectUserData = NULL;
    jobject jwriter = NULL;
    jobject ls_object = NULL;
    jobject jinstanceData = NULL;
    gapi_instanceHandle_t gapiHandle;
    gapi_returnCode_t returnCode = GAPI_RETCODE_OK;
    DK_ObjectHomeAdmin* home = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(writer);
    assert(object);

    topicInfo = DK_ObjectWriter_us_getTopicInfo(writer);
    topicUserData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    assert(topicUserData);
    home = DK_ObjectAdmin_us_getHome(object);
    objectUserData = DK_ObjectHomeAdmin_us_getUserData(home);
    jwriter = (jobject)DK_ObjectWriter_us_getLSWriter(writer);

    ls_object = (jobject)DK_ObjectAdmin_us_getLSObject(object);
    jinstanceData = (*env)->GetObjectField(env, ls_object, objectUserData->typedRoot_currentTopic_fid);
    gapiHandle  = DK_ObjectAdmin_us_getHandle(object);
    if (jinstanceData != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = jinstanceData;
        srcInfo.copyProgram=(saj_copyCache)(*env)->GetLongField(env, jwriter, topicUserData->typedWriter_copyCache_fid);
	    src = (const gapi_foo*)&srcInfo;
    }

    assert(DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_NEW ||
            DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_MODIFIED);
    returnCode = gapi_fooDataWriter_write ((gapi_fooDataWriter)saj_read_gapi_address (env, jwriter), src,
                                                                                                 gapiHandle);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, returnCode, "Unable to write object data");

    DLRL_Exception_EXIT(exception);
    if(jinstanceData){
        (*env)->DeleteLocalRef(env, jinstanceData);
    }
    DLRL_INFO(INF_EXIT);
}
/* may be optimized later by doing some prep work only once, but we'll see if thats benefitial later on. */
void
DJA_ObjectWriterBridge_us_destroy(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectWriter* writer,
    DK_ObjectAdmin* object)
{
    JNIEnv* env = (JNIEnv*)userData;
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo* src = NULL;
    DK_TopicInfo* topicInfo = NULL;
    DJA_CachedJNITypedTopic* topicUserData = NULL;
    DJA_CachedJNITypedObject* objectUserData = NULL;
    jobject jwriter = NULL;
    jobject ls_object = NULL;
    jobject jinstanceData = NULL;
    gapi_instanceHandle_t gapiHandle;
    gapi_returnCode_t returnCode = GAPI_RETCODE_OK;
    DK_ObjectHomeAdmin* home = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(writer);
    assert(object);

    topicInfo = DK_ObjectWriter_us_getTopicInfo(writer);
    topicUserData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    assert(topicUserData);
    home = DK_ObjectAdmin_us_getHome(object);
    objectUserData = DK_ObjectHomeAdmin_us_getUserData(home);
    jwriter = (jobject)DK_ObjectWriter_us_getLSWriter(writer);

    ls_object = (jobject)DK_ObjectAdmin_us_getLSObject(object);
    jinstanceData = (*env)->GetObjectField(env, ls_object, objectUserData->typedRoot_currentTopic_fid);
    gapiHandle  = DK_ObjectAdmin_us_getHandle(object);
    if (jinstanceData != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = jinstanceData;
        srcInfo.copyProgram=(saj_copyCache)(*env)->GetLongField(env, jwriter, topicUserData->typedWriter_copyCache_fid);
	    src = (const gapi_foo*)&srcInfo;
    }

    assert(DK_ObjectAdmin_us_getWriteState(object) == DK_OBJECT_STATE_OBJECT_DELETED);
    returnCode = gapi_fooDataWriter_dispose ((gapi_fooDataWriter)saj_read_gapi_address (env, jwriter), src,
                                                                                                        gapiHandle);
    DLRL_Exception_PROPAGATE_GAPI_RESULT(exception, returnCode, "Unable to dispose object data");

    DLRL_Exception_EXIT(exception);
    if(jinstanceData){
        (*env)->DeleteLocalRef(env, jinstanceData);
    }
    DLRL_INFO(INF_EXIT);
}
