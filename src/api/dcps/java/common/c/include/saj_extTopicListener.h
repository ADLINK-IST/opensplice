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
#ifndef SAJ_EXTTOPICLISTENER_H
#define SAJ_EXTTOPICLISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_topicListener*
saj_extTopicListenerNew(
    JNIEnv *env,
    jobject listener);

void
saj_extTopicListenerOnAllDataDisposed(
    void* listenerData, 
    gapi_topic topic);

#ifdef __cplusplus
}
#endif
#endif
