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
#include "DLRL_Types.h"
/* JNI includes */
#include <jni.h>
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"
#include "DJA_CollectionBridge.h"
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

/* assumes the collections target home locked, as well as the collections owning home */
DLRL_LS_object
DJA_CollectionBridge_us_createLSCollection(
    DLRL_Exception* exception,
    void* userData,
    DK_Collection* collection,
    DK_RelationType relationType)
{
    DLRL_LS_object ls_collection = NULL;
    DJA_CachedJNITypedObject* targetObjectCachedData = NULL;
    JNIEnv* env = (JNIEnv*)userData;
    jobject tempObject = NULL;
    jclass typedCollectionClass = NULL;
    jmethodID typedCollectionConstructorMid = NULL;
    jfieldID collectionAdminFid = NULL;
    DK_ObjectHomeAdmin* targetHome = NULL;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(userData);
    assert(collection);
    assert(relationType < DK_RelationType_elements);

    targetHome = DK_Collection_us_getTargetHome(collection);
    targetObjectCachedData =(DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(targetHome);
    assert(targetObjectCachedData);
    if(relationType == DK_RELATION_TYPE_STR_MAP){
        collectionAdminFid = cachedJNI.strMap_admin_fid;
        typedCollectionClass = targetObjectCachedData->typedStrMap_class;
        typedCollectionConstructorMid = targetObjectCachedData->typedStrMap_constructor_mid;
    } else if(relationType == DK_RELATION_TYPE_INT_MAP){
        collectionAdminFid = cachedJNI.intMap_admin_fid;
        typedCollectionClass = targetObjectCachedData->typedIntMap_class;
        typedCollectionConstructorMid = targetObjectCachedData->typedIntMap_constructor_mid;
    } else {
        assert(relationType == DK_RELATION_TYPE_SET);
        collectionAdminFid = cachedJNI.set_admin_fid;
        typedCollectionClass = targetObjectCachedData->typedSet_class;
        typedCollectionConstructorMid = targetObjectCachedData->typedSet_constructor_mid;
    }
    tempObject = (*env)->NewObject(env, typedCollectionClass, typedCollectionConstructorMid);
    DLRL_JavaException_PROPAGATE(env, exception);
    (*env)->SetLongField(env, tempObject, collectionAdminFid, (jlong)DK_Entity_ts_duplicate((DK_Entity*)collection));
    ls_collection = (*env)->NewGlobalRef(env, tempObject);
    if (!ls_collection) {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Not enough memory to complete operation.");
    }

    DLRL_Exception_EXIT(exception);
    if(tempObject){
        (*env)->DeleteLocalRef(env, tempObject);
    }
    DLRL_INFO(INF_EXIT);

    return ls_collection;
}
