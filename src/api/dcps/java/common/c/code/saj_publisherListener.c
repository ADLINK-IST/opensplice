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
#include "saj_publisherListener.h"
#include "saj_dataWriterListener.h"

struct gapi_publisherListener*
saj_publisherListenerNew(
    JNIEnv *env,
    jobject jlistener)
{
    struct gapi_publisherListener* listener;
    saj_listenerData ld;
    
    listener = NULL;
    
    ld = saj_listenerDataNew(env, jlistener);
    
    if(ld != NULL){
        listener = gapi_publisherListener__alloc();
        saj_listenerInit((struct gapi_listener*)listener);
        listener->listener_data = ld;
        
        listener->on_offered_deadline_missed    = saj_dataWriterListenerOnOfferedDeadlineMissed;
        listener->on_offered_incompatible_qos   = saj_dataWriterListenerOnOfferedIncompatibleQos;
        listener->on_liveliness_lost            = saj_dataWriterListenerOnLivelinessLost;
        listener->on_publication_match          = saj_dataWriterListenerOnPublicationMatch;
    }
    return listener;
}
