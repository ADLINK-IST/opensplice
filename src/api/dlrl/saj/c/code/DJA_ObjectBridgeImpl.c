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
#include <jni.h>

/* C includes */
#include <stdio.h>
#include <assert.h>

/* DLRL includes */
#include "DLRL_Report.h"
#include "DLRL_Types.h"
#include "DJA_Initialisation.h"
#include "DJA_ObjectBridge.h"

void
DJA_ObjectBridge_us_setIsAlive(
    void* userData,
    DLRL_LS_object ls_object,
    LOC_boolean val)
{
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(ls_object);

    (*env)->SetBooleanField(env, (jobject)ls_object, cachedJNI.objectRoot_isAlive_fid, val);

    DLRL_INFO(INF_EXIT);
}

void
DJA_ObjectBridge_us_setIsRegistered(
    void* userData,
    DLRL_LS_object ls_object,
    LOC_boolean val)
{
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(ls_object);

    (*env)->SetBooleanField(env, (jobject)ls_object, cachedJNI.objectRoot_isRegistered_fid, val);

    DLRL_INFO(INF_EXIT);
}

void
DJA_ObjectBridge_us_notifyWriteStateChange(
    void* userData,
    DLRL_LS_object ls_object,
    DK_ObjectState val)
{
    JNIEnv* env = (JNIEnv*)userData;

    DLRL_INFO(INF_ENTER);
    assert(userData);
    assert(ls_object);

    (*env)->SetIntField(env, (jobject)ls_object, cachedJNI.objectRoot_writeState_fid, (jint)val);
    DLRL_INFO(INF_EXIT);
}
