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
#include "saj_domainParticipantListener.h"
#include "saj_subscriberListener.h"
#include "saj_dataWriterListener.h"
#include "saj_dataReaderListener.h"
#include "saj_topicListener.h"

struct gapi_domainParticipantListener*
saj_domainParticipantListenerNew(
    JNIEnv *env,
    jobject jlistener)
{
    struct gapi_domainParticipantListener* listener;
    saj_listenerData ld;

    listener = NULL;

    ld = saj_listenerDataNew(env, jlistener);

    if(ld != NULL){
        listener = gapi_domainParticipantListener__alloc();
        saj_listenerInit((struct gapi_listener*)listener);
        listener->listener_data = ld;

        listener->on_inconsistent_topic         = saj_topicListenerOnInconsistentTopic;
        listener->on_offered_deadline_missed    = saj_dataWriterListenerOnOfferedDeadlineMissed;
        listener->on_offered_incompatible_qos   = saj_dataWriterListenerOnOfferedIncompatibleQos;
        listener->on_liveliness_lost            = saj_dataWriterListenerOnLivelinessLost;
        listener->on_publication_match          = saj_dataWriterListenerOnPublicationMatch;
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
