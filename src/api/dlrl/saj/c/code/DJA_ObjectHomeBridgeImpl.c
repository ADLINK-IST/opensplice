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
#include "saj_copyIn.h"

#include "DLRL_Types.h"

#include "os_heap.h"

/* DLRL includes */
#include "DLRL_Report.h"
#include "DLRL_Kernel.h"

#include "DLRL_Kernel_private.h"
#include "DJA_ObjectHomeBridge.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

/* JNI includes */
#include <jni.h>


#define ENTITY_NAME "DLRL Java API ObjectHomeBridge"

void
DJA_ObjectHomeBridge_us_loadMetamodel(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    void* userData)
{
    /* JNI thread env */
    JNIEnv* env;
    /* Cached JNI data used within this call */
    jmethodID loadMetamodelMid = cachedJNI.objectHome_loadMetamodel_mid;
    /* the java object home */
    jobject ls_objectHome;

    DLRL_INFO(INF_ENTER);
    assert(home);
    assert(exception);
    assert(userData);

    env = (JNIEnv*)userData;
    /* get java object home */
    ls_objectHome = (jobject)DK_ObjectHomeAdmin_us_getLSHome(home);

    if(ls_objectHome){
        DLRL_INFO(INF_CALLBACK, "objectHome->loadMetamodel()");
        (*env)->CallVoidMethod(env, ls_objectHome, loadMetamodelMid);
        DLRL_JavaException_PROPAGATE(env, exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DJA_ObjectHomeBridge_us_unregisterAdminWithLSHome(
    void* userData,
    DLRL_LS_object ls_home,
    LOC_boolean isRegistered)
{
    JNIEnv * env = (JNIEnv *)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(ls_home);

    DLRL_INFO(INF_ENTITY, "DeleteGlobalRef on ls_home");
    if(isRegistered){
        (*env)->DeleteGlobalRef(env, (jobject)ls_home);
    } else {
        (*env)->DeleteWeakGlobalRef(env, (jobject)ls_home);
    }
    DLRL_INFO(INF_EXIT);
}

/* this operation already takes inheritance structures into account when triggering listeners */
void
DJA_ObjectHomeBridge_us_triggerListeners(
    DLRL_Exception* exception,
    void *userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* newSamples,
    Coll_List* modifiedSamples,
    Coll_List* deletedSamples)
{
    jobjectArray jlisteners2D = NULL;
    jobjectArray jnewObjects = NULL;
    jobjectArray jmodifiedObjects = NULL;
    jobjectArray jdeletedObjects = NULL;
    jobjectArray jlisteners = NULL;
    JNIEnv* env;
    LOC_boolean listenersFound = FALSE;
    DJA_CachedJNITypedObject* typedObjectCachedData;
    DK_ObjectHomeAdmin* currentHome;
    LOC_unsigned_long homeCounter = 0;/* starts at 0 always */
    LOC_unsigned_long count = 0;
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long size = 1;/* home for which this is called is element number 1 */
    DK_ObjectHomeAdmin* parent;
    Coll_Set* listeners;
    void* aListener;
    jobject anObject;
    DK_ObjectAdmin* anObjectAdmin;


    DLRL_INFO(INF_ENTER);

    assert(userData);
    assert(exception);
    assert(home);
    assert(newSamples);
    assert(modifiedSamples);
    assert(deletedSamples);

    env = (JNIEnv*)userData;
    typedObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(typedObjectCachedData);

    /* first determine the number of parent homes we might need to propagate events to. */
    parent = DK_ObjectHomeAdmin_us_getParent(home);
    while(parent){
         size++;
         parent = DK_ObjectHomeAdmin_us_getParent(home);
    }
    /* allocate a 2d array of listeners, it will contain arrays of listeners */
    jlisteners2D = (*env)->NewObjectArray(env, size, cachedJNI.object_class, NULL);
    DLRL_JavaException_PROPAGATE(env, exception);

    /* now we are going to fill the 2d array of listeners. */
    currentHome = home;
    while(currentHome){
        listeners = DK_ObjectHomeAdmin_us_getListeners(currentHome);
        if(Coll_Set_getNrOfElements(listeners) > 0){
            jlisteners = (*env)->NewObjectArray(env, Coll_Set_getNrOfElements(listeners),
                                                                    cachedJNI.objectListener_class, NULL);
            DLRL_JavaException_PROPAGATE(env, exception);
            iterator = Coll_Set_getFirstElement(listeners);
            while(iterator){
                aListener = Coll_Iter_getObject(iterator);
                (*env)->SetObjectArrayElement(env, jlisteners, count, (jobject)aListener);
                iterator = Coll_Iter_getNext(iterator);
                DLRL_JavaException_PROPAGATE(env, exception);
                count++;
                listenersFound = TRUE;
            }
            (*env)->SetObjectArrayElement(env, jlisteners2D, homeCounter, jlisteners);
            DLRL_JavaException_PROPAGATE(env, exception);
            (*env)->DeleteLocalRef(env, jlisteners);
            jlisteners = NULL;
        }
        currentHome = DK_ObjectHomeAdmin_us_getParent(currentHome);
        homeCounter++;
    }
    if (listenersFound){
        LOC_unsigned_long count = 0;
        jmodifiedObjects = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(modifiedSamples),
                                                            typedObjectCachedData->typedRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);

        jdeletedObjects = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(deletedSamples),
                                        typedObjectCachedData->typedRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);

        jnewObjects = (*env)->NewObjectArray(env, Coll_List_getNrOfElements(newSamples),
                                        typedObjectCachedData->typedRoot_class, NULL);
        DLRL_JavaException_PROPAGATE(env, exception);

        iterator = Coll_List_getFirstElement(newSamples);
        count = 0;
        while(iterator){
            anObjectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
            anObject = (jobject)DK_ObjectAdmin_us_getLSObject(anObjectAdmin);
            (*env)->SetObjectArrayElement(env, jnewObjects, count, anObject);
            DLRL_JavaException_PROPAGATE(env, exception);
            iterator = Coll_Iter_getNext(iterator);
            count++;
        }
        iterator = Coll_List_getFirstElement(modifiedSamples);
        count = 0;
        while(iterator){
            anObjectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
            anObject = (jobject)DK_ObjectAdmin_us_getLSObject(anObjectAdmin);
            (*env)->SetObjectArrayElement(env, jmodifiedObjects, count, anObject);
            DLRL_JavaException_PROPAGATE(env, exception);
            iterator = Coll_Iter_getNext(iterator);
            count++;
        }
        iterator = Coll_List_getFirstElement(deletedSamples);
        count = 0;
        while(iterator){
            anObjectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
            anObject = (jobject)DK_ObjectAdmin_us_getLSObject(anObjectAdmin);
            (*env)->SetObjectArrayElement(env, jdeletedObjects, count, anObject);
            DLRL_JavaException_PROPAGATE(env, exception);
            iterator = Coll_Iter_getNext(iterator);
            count++;
        }
        DLRL_INFO(INF_CALLBACK,"triggerListeners(listeners[][], newSamples[], "
                                "modifiedSamples[], deletedSamples[])");
        (*env)->CallVoidMethod(env, (jobject)DK_ObjectHomeAdmin_us_getLSHome(home),
                                typedObjectCachedData->typedHome_triggerListeners_mid, jlisteners2D, jnewObjects,
                                jmodifiedObjects, jdeletedObjects);
        DLRL_JavaException_PROPAGATE(env, exception);
    }

    DLRL_Exception_EXIT(exception);
    if(jnewObjects){
        (*env)->DeleteLocalRef(env, jnewObjects);
    }
    if(jdeletedObjects){
        (*env)->DeleteLocalRef(env, jdeletedObjects);
    }
    if(jmodifiedObjects){
        (*env)->DeleteLocalRef(env, jmodifiedObjects);
    }
    if(jlisteners){
        (*env)->DeleteLocalRef(env, jlisteners);
    }
    if(jlisteners2D){
        (*env)->DeleteLocalRef(env, jlisteners2D);
    }
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DJA_ObjectHomeBridge_us_checkObjectForSelection(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    DK_ObjectAdmin* objectAdmin)
{
    JNIEnv* env = (JNIEnv*)userData;
    LOC_boolean retVal = FALSE;
    DJA_CachedJNITypedObject* typedCache;
    DK_CriterionKind kind;
    jobject lsFilter;
    jobject jobjectAdmin;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);
    assert(env);
    assert(selection);
    assert(objectAdmin);

    typedCache = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(typedCache);
    kind = DK_SelectionAdmin_us_getCriterionKind(selection);
    if(kind == DK_CRITERION_KIND_FILTER)
    {
        lsFilter = (jobject)DK_SelectionAdmin_us_getLSFilter(selection);
        assert(lsFilter);
        jobjectAdmin = (jobject)DK_ObjectAdmin_us_getLSObject(objectAdmin);
        assert(jobjectAdmin);
        /* unlock admin, to allow max access during the callback.
         * update mutex remains locked
         */
        DK_ObjectHomeAdmin_unlockAdmin(home);
        retVal = (LOC_boolean)(*env)->CallBooleanMethod(
            env,
            lsFilter,
            typedCache->typedFilter_checkObject_mid,
            jobjectAdmin,
            DK_MEMBERSHIPSTATE_UNDEFINED_MEMBERSHIP);
        /* relock, then propagate */
        DK_ObjectHomeAdmin_lockAdmin(home);
        DLRL_JavaException_PROPAGATE(env, exception);
    } else
    {
        /*we dont support that so....*/
        assert(FALSE);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DJA_ObjectHomeBridge_us_deleteUserData(
    void* userData,
    void* homeUserData)
{

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(homeUserData);

    DJA_Initialisation_us_destroyTypedObjectCache((JNIEnv *)userData, (DJA_CachedJNITypedObject*)homeUserData);

    DLRL_INFO(INF_EXIT);
}

DLRL_LS_object
DJA_ObjectHomeBridge_us_createTypedObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_TopicInfo* topicInfo,
    DLRL_LS_object ls_topic,
    DK_ObjectAdmin* objectAdmin)
{
    JNIEnv* env = (JNIEnv*)userData;
    jobject typedObject = NULL;
    jobject tempObject = NULL;
    DJA_CachedJNITypedObject* objectCachedData = NULL;
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    LOC_boolean clearLocalRef = FALSE;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(objectAdmin);
    assert(env);
    /* ls_topic may be null */

    /* get the cached JNI data from the user data field stored in the java home */
    objectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(objectCachedData);

    DLRL_INFO(INF_ENTITY, "creating object of type %s", DK_ObjectHomeAdmin_us_getName(home));
    tempObject = (*env)->NewObject(env, objectCachedData->typedRoot_class, objectCachedData->typedRoot_constructor_mid);
    DLRL_JavaException_PROPAGATE(env, exception);

    if(!ls_topic){
        typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
        assert(typedTopicCachedData);
        ls_topic = (*env)->NewObject(env, typedTopicCachedData->typedTopic_class,
                                    typedTopicCachedData->typedTopic_constructor_mid);
        DLRL_JavaException_PROPAGATE(env, exception);
        clearLocalRef = TRUE;
    }
    (*env)->SetObjectField(env, tempObject, objectCachedData->typedRoot_currentTopic_fid, ls_topic);
    (*env)->SetLongField(env, tempObject, cachedJNI.objectRoot_admin_fid,
                                                        (jlong)DK_Entity_ts_duplicate((DK_Entity*)objectAdmin));
    typedObject = (*(env))->NewGlobalRef (env, tempObject);
    if (!typedObject) {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Not enough memory to complete operation.");
    }

    DLRL_Exception_EXIT(exception);
    if(tempObject){
        (*env)->DeleteLocalRef(env, tempObject);
    }
    if(ls_topic && clearLocalRef){
        (*env)->DeleteLocalRef(env, ls_topic);
    }
    DLRL_INFO(INF_EXIT);
    return (DLRL_LS_object)typedObject;
}

void
DJA_ObjectHomeBridge_us_doCopyInForTopicOfObject(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectWriter* objWriter,
    DK_ObjectAdmin* objectAdmin,
    void* message,
    void* dataSample)
{
    JNIEnv* env = (JNIEnv*)userData;
    DK_TopicInfo* topicInfo = NULL;
    struct saj_srcInfo_s srcInfo;
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    DJA_CachedJNITypedObject* objectCachedData = NULL;
    DLRL_LS_object ls_writer = NULL;
    DLRL_LS_object ls_objAdmin = NULL;
    c_base base;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(home);
    assert(objWriter);
    assert(objectAdmin);
    assert(message);
    assert(dataSample);

    srcInfo.javaObject = NULL;
    topicInfo = DK_ObjectWriter_us_getTopicInfo(objWriter);
    ls_writer = DK_ObjectWriter_us_getLSWriter(objWriter);
    ls_objAdmin = DK_ObjectAdmin_us_getLSObject(objectAdmin);
    typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    assert(typedTopicCachedData);
    /* get the cached JNI data from the user data field stored in the java home */
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
    base = c_getBase(c_object(message));
    saj_copyInStruct(base, (void*)&srcInfo, dataSample);

    DLRL_Exception_EXIT(exception);
    if(srcInfo.javaObject){
        (*env)->DeleteLocalRef(env, srcInfo.javaObject);
    }
    DLRL_INFO(INF_EXIT);
}

void DJA_ObjectHomeBridge_us_createTypedObjectSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size)
{
#ifndef NDEBUG
    printf("NDEBUG - not implemented\n");
#endif
}


void DJA_ObjectHomeBridge_us_addElementToTypedObjectSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count)
{
#ifndef NDEBUG
    printf("NDEBUG - not implemented\n");
#endif
}

void DJA_ObjectHomeBridge_us_createTypedSelectionSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size)
{
#ifndef NDEBUG
    printf("NDEBUG - not implemented\n");
#endif
}


void DJA_ObjectHomeBridge_us_addElementToTypedSelectionSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count)
{
#ifndef NDEBUG
    printf("NDEBUG - not implemented\n");
#endif
}

void DJA_ObjectHomeBridge_us_createTypedListenerSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size)
{
#ifndef NDEBUG
    printf("NDEBUG - not implemented\n");
#endif
}


void DJA_ObjectHomeBridge_us_addElementToTypedListenerSeq(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count)
{
#ifndef NDEBUG
    printf("NDEBUG - not implemented\n");
#endif
}

/* className may be NIL */
void
DJA_ObjectHomeBridge_us_setDefaultTopicKeys(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_TopicInfo* topicInfo,
    DLRL_LS_object ls_object,
    DK_ObjectID* oid,
    LOC_string className)
{
    DJA_CachedJNITypedTopic* typedTopicCachedData = NULL;
    DJA_CachedJNITypedObject* objectCachedData = NULL;
    jobject ls_topic;
    jobject ls_oid;
    jintArray oidArray;
    jstring jname;
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(home);
    assert(topicInfo);
    assert(ls_object);
    assert(oid);
    /* className may be null */

    typedTopicCachedData = (DJA_CachedJNITypedTopic*)DK_TopicInfo_us_getTopicUserData(topicInfo);
    assert(typedTopicCachedData);
    /* get the cached JNI data from the user data field stored in the java home */
    objectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(objectCachedData);

    ls_topic = (*env)->GetObjectField(env, ls_object, objectCachedData->typedRoot_currentTopic_fid);
    assert(ls_topic);
    ls_oid = (*env)->GetObjectField(env, ls_topic, typedTopicCachedData->typedTopic_oidKey_fid);
    if(!ls_oid){
        ls_oid = (*env)->NewObject(env, cachedJNI.oid_class, cachedJNI.oid_constructor_mid);
       (*env)->SetObjectField(env, ls_topic, typedTopicCachedData->typedTopic_oidKey_fid, ls_oid);
    }
    oidArray = (*env)->GetObjectField(env, ls_oid, cachedJNI.oid_value_fid);
    assert(oidArray);
    (*env)->SetIntArrayRegion(env, oidArray, 0, 3, (jint*)oid->oid);
    DLRL_JavaException_PROPAGATE(env, exception);
    (*env)->SetIntField(env, ls_oid, cachedJNI.oid_systemId_fid, (jint)oid->oid[0]);
    (*env)->SetIntField(env, ls_oid, cachedJNI.oid_localId_fid, (jint)oid->oid[1]);
    (*env)->SetIntField(env, ls_oid, cachedJNI.oid_serial_fid, (jint)oid->oid[2]);

    if(className){
        assert(typedTopicCachedData->typedTopic_nameKey_fid);
        jname = (*env)->NewStringUTF(env, className);
        DLRL_JavaException_PROPAGATE(env, exception);
        (*env)->SetObjectField(env, ls_topic, typedTopicCachedData->typedTopic_nameKey_fid, jname);
        (*env)->DeleteLocalRef(env, jname);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
