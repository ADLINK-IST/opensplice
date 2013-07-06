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
#include "saj_listener.h"
#include "saj_dataReaderListener.h"
#include "saj_subscriberListener.h"
#include "gapi.h"

struct gapi_subscriberListener *
saj_subscriberListenerNew(
    JNIEnv *env,
    jobject jlistener)
{
    struct gapi_subscriberListener* listener;
    saj_listenerData ld;
    
    listener = NULL;
    
    ld = saj_listenerDataNew(env, jlistener);
    
    if(ld != NULL){
        listener = gapi_subscriberListener__alloc();
        saj_listenerInit((struct gapi_listener*)listener);
        listener->listener_data = ld;
        
        listener->on_requested_deadline_missed  = saj_dataReaderListenerOnRequestedDeadlineMissed;
        listener->on_requested_incompatible_qos = saj_dataReaderListenerOnRequestedIncompatibleQos;
        listener->on_sample_rejected            = saj_dataReaderListenerOnSampleRejected;
        listener->on_liveliness_changed         = saj_dataReaderListenerOnLivelinessChanged;
        listener->on_data_available             = saj_dataReaderListenerOnDataAvailable;
        listener->on_subscription_match         = saj_dataReaderListenerOnSubscriptionMatch;
        listener->on_sample_lost                = saj_dataReaderListenerOnSampleLost;
        listener->on_data_on_readers            = saj_subscriberListenerOnDataOnReaders;
    }
    return listener;
}


void
saj_subscriberListenerOnDataOnReaders(
    void* listenerData, 
    gapi_subscriber subscriber)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jsubscriber;
    
    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    jsubscriber = saj_read_java_address(subscriber);
    (*env)->CallVoidMethod(env, ld->jlistener, 
                                GET_CACHED(listener_onDataOnReaders_mid), 
                                jsubscriber);
}

