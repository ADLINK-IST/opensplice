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

#include "saj_utilities.h"
#include "saj_status.h"
#include "saj_topicListener.h"
#include "saj_extTopicListener.h"

struct gapi_topicListener*
saj_extTopicListenerNew(
    JNIEnv *env,
    jobject jlistener)
{
    struct gapi_topicListener* listener;
    saj_listenerData ld;
    
    listener = NULL;
    
    ld = saj_listenerDataNew(env, jlistener);
    
    if(ld != NULL){
        listener = gapi_topicListener__alloc();
        saj_listenerInit((struct gapi_listener*)listener);
        listener->listener_data = ld;
        
        listener->on_inconsistent_topic = saj_topicListenerOnInconsistentTopic;
        listener->on_all_data_disposed  = saj_extTopicListenerOnAllDataDisposed;
    }
    return listener;
}

void
saj_extTopicListenerOnAllDataDisposed(
    void* listenerData, 
    gapi_topic topic)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jtopic;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);
    
    jtopic = saj_read_java_address(topic);
    (*env)->CallVoidMethod(env, ld->jlistener, 
                                GET_CACHED(listener_onAllDataDisposed_mid), 
                                jtopic);
}
