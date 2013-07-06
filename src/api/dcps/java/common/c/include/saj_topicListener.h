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
#ifndef SAJ_TOPICLISTENER_H
#define SAJ_TOPICLISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_topicListener*
saj_topicListenerNew(
    JNIEnv *env,
    jobject listener);

void
saj_topicListenerOnInconsistentTopic(
    void* listenerData, 
    gapi_topic topic,
    const gapi_inconsistentTopicStatus *status);

#ifdef __cplusplus
}
#endif
#endif
