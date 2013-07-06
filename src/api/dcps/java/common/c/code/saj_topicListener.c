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

struct gapi_topicListener*
saj_topicListenerNew(
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
    }
    return listener;
}

void
saj_topicListenerOnInconsistentTopic(
    void* listenerData, 
    gapi_topic topic,
    const gapi_inconsistentTopicStatus *status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jtopic;
    saj_returnCode rc;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);
    
    rc = saj_statusCopyOutInconsistentTopicStatus(env, 
                        (gapi_inconsistentTopicStatus *)status, &jstatus);
    
    if(rc == SAJ_RETCODE_OK){
        jtopic = saj_read_java_address(topic);
        (*env)->CallVoidMethod(
                            env, ld->jlistener, 
                            GET_CACHED(listener_onInconsistentTopic_mid), 
                            jtopic, jstatus);
    }

}
