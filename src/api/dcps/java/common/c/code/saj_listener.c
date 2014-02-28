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
#include "saj_listener.h"
#include "saj_utilities.h"
#include "os_heap.h"


saj_listenerData
saj_listenerDataNew(
    JNIEnv* env,
    jobject jlistener)
{
    saj_listenerData ld;
    
    ld = NULL;
    
    if(jlistener != NULL){
        ld = saj_listenerData(os_malloc(C_SIZEOF(saj_listenerData)));
        ld->jlistener = (*env)->NewGlobalRef(env, jlistener);
        saj_exceptionCheck(env);
    }
    return ld;
}

void
saj_listenerInit(
    struct gapi_listener* listener)
{
    OS_UNUSED_ARG(listener);

    return;
}

void
saj_listenerDataFree(
    JNIEnv* env,
    saj_listenerData ld)
{
    if(ld != NULL){
        (*env)->DeleteGlobalRef(env, ld->jlistener);
        saj_exceptionCheck(env);
        os_free(ld);
    }
}

void
saj_listenerAttach(
    void* listener_data)
{
    JavaVM *jvm;
    JNIEnv *env;
    void* threadData;
    jint jresult;

    jvm = (JavaVM*)listener_data;
    jresult = (*jvm)->AttachCurrentThread(jvm, (void**)&env, NULL);
    assert(jresult == 0); OS_UNUSED_ARG(jresult);
    threadData = os_threadMemMalloc(OS_THREAD_JVM, sizeof(env));
    *(JNIEnv**)threadData = env;
}

void
saj_listenerDetach(
   void* listener_data)
{   
    JavaVM *jvm;
    JNIEnv* env;
    jint jresult;
    
    OS_UNUSED_ARG(listener_data);

    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);
    jresult = (*env)->GetJavaVM(env, &jvm);
    assert(jresult == 0); OS_UNUSED_ARG(jresult);
    jresult = (*jvm)->DetachCurrentThread(jvm);
    os_threadMemFree(OS_THREAD_JVM);
    assert(jresult == 0);
}
