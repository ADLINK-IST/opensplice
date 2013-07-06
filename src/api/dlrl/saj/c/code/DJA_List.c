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
#include "DJA_List.h"
#include "DLRL_Report.h"
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"

JNIEXPORT jint JNICALL
Java_DDS_List_jniLength(
    JNIEnv * env,
    jobject ls_object)
{
    return 0;
}


JNIEXPORT void JNICALL
Java_DDS_List_jniClear(
    JNIEnv * env,
    jobject ls_object)
{

}


JNIEXPORT jintArray JNICALL
Java_DDS_List_jniAddedElements(
    JNIEnv * env,
    jobject ls_object)
{
    return (*env)->NewIntArray(env, 0);
}


JNIEXPORT jintArray JNICALL
Java_DDS_List_jniModifiedElements(
    JNIEnv * env,
    jobject ls_object)
{
      return (*env)->NewIntArray(env, 0);
}


JNIEXPORT void JNICALL
Java_DDS_List_jniRemove(
    JNIEnv * env,
    jobject ls_object)
{

}


JNIEXPORT void JNICALL
Java_DDS_List_jniDeleteList(
    JNIEnv * env,
    jobject ls_object)
{

}


JNIEXPORT jintArray JNICALL
Java_DDS_List_jniRemovedElements(
    JNIEnv * env,
    jobject ls_object)
{
    return (*env)->NewIntArray(env, 0);
}


JNIEXPORT jobjectArray JNICALL
Java_DDS_List_jniGetValues(
    JNIEnv * env,
    jobject ls_object)
{
    return NULL;
}


JNIEXPORT jobject JNICALL
Java_DDS_List_jniGet(
    JNIEnv * env,
    jobject ls_object,
    jint jindex)
{
    return NULL;
}


JNIEXPORT void JNICALL
Java_DDS_List_jniPut(
    JNIEnv * env,
    jobject ls_object,
    jint jindex,
    jobject jtargetObject)
{

}


JNIEXPORT void JNICALL
Java_DDS_List_jniAdd(
    JNIEnv * env,
    jobject ls_object,
    jobject jtargetObject)
{

}

