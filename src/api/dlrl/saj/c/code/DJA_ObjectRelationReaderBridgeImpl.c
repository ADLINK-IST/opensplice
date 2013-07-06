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
#include "DLRL_Kernel_private.h"
#include <jni.h>
#include "DJA_Initialisation.h"
#include "DJA_ExceptionHandler.h"

/* may set the relation field to NULL */
/* NOT IN DESIGN - name changed */
void
DJA_ObjectRelationReaderBridge_us_setRelatedObjectForObject(
    void* userData,
    DK_ObjectHomeAdmin* ownerObjectHome,
    DK_ObjectAdmin* owner,
    LOC_unsigned_long relationIndex,
    DK_ObjectAdmin* relationObjectAdmin,
    LOC_boolean isValid)
{
    DJA_CachedJNITypedObject* ownerObjectCachedData = NULL;
    jobject ownerRoot = NULL;
    jobject targetRoot = NULL;
    jfieldID relationFieldID = NULL;
    jfieldID relationIsFoundFieldID = NULL;
    /* java thread env pointer */
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(owner);
    assert(ownerObjectHome);
/*   relationObjectAdmin and userData may be null */

    ownerObjectCachedData = (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(ownerObjectHome);
    assert(ownerObjectCachedData);
    ownerRoot = (jobject)DK_ObjectAdmin_us_getLSObject(owner);
    if(relationObjectAdmin){
        /* TODO: Measures need to be taken in case target is not dereferenced yet. */
        targetRoot = (jobject)DK_ObjectAdmin_us_getLSObject(relationObjectAdmin);
    }
    
    /* If Java object is dereferenced, set relation to the target pointer. */
    if(ownerRoot != NULL){
        relationFieldID = Coll_List_getObject(&(ownerObjectCachedData->relationFieldIDs), relationIndex);
        assert(relationFieldID);
        relationIsFoundFieldID = Coll_List_getObject(&(ownerObjectCachedData->relationIsFoundFieldIDs), relationIndex);
        assert(relationIsFoundFieldID);
        (*env)->SetObjectField(env, ownerRoot, relationFieldID, targetRoot);
        if(targetRoot || !isValid){
            (*env)->SetBooleanField(env, ownerRoot, relationIsFoundFieldID, TRUE);
        } else {
            (*env)->SetBooleanField(env, ownerRoot, relationIsFoundFieldID, FALSE);
        }
    }/* else do nothing */
    /*  Java exception cannot occur */
    DLRL_INFO(INF_EXIT);
}
