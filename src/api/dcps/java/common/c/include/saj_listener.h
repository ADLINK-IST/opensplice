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
#ifndef SAJ_LISTENER_H
#define SAJ_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>
#include "gapi.h"

C_CLASS(saj_listenerData);

C_STRUCT(saj_listenerData){
    jobject jlistener;
};

#define saj_listenerData(a) ((saj_listenerData)(a))

saj_listenerData
saj_listenerDataNew(
    JNIEnv* env,
    jobject jlistener);

void
saj_listenerInit(
    struct gapi_listener* listener);

void
saj_listenerDataFree(
    JNIEnv* env,
    saj_listenerData ld);

void
saj_listenerAttach(
    void* listener);

void
saj_listenerDetach(
    void* listener);

#ifdef __cplusplus
}
#endif
#endif
