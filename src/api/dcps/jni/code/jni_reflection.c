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
#include "jni_reflection.h"

void
jni_initHashSet(
    JNIEnv *env,
    jobject object,
    const char* fieldName)
{
    jclass hashSet_class, fieldClass;
    jmethodID hashSetId;
    jfieldID fieldId;
    jobject hashSet;
    
    fieldClass = (*env)->GetObjectClass(env, object);
    
    assert(fieldClass != NULL);
    
    hashSet_class = (*env)->FindClass(env, "java/util/HashSet");
    hashSetId = (*env)->GetMethodID(env, hashSet_class, "<init>", "()V");
    fieldId = (*env)->GetFieldID(env, fieldClass, fieldName, "Ljava/util/HashSet;");
    hashSet = (*env)->NewObject(env, hashSet_class, hashSetId);
    (*env)->SetObjectField(env, object, fieldId, hashSet);
}

