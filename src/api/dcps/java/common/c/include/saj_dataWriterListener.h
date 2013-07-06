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
#ifndef SAJ_DATAWRITER_LISTENER
#define SAJ_DATAWRITER_LISTENER

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_dataWriterListener*
saj_dataWriterListenerNew(
    JNIEnv *env,
    jobject jlistener);

void
saj_dataWriterListenerOnOfferedDeadlineMissed(
    void* listenerData, 
    gapi_dataWriter dataWriter,
    const gapi_offeredDeadlineMissedStatus* status);


void
saj_dataWriterListenerOnOfferedIncompatibleQos(
    void* listenerData, 
    gapi_dataWriter dataWriter,
    const gapi_offeredIncompatibleQosStatus* status);

void
saj_dataWriterListenerOnLivelinessLost(
    void* listenerData, 
    gapi_dataWriter dataWriter,
    const gapi_livelinessLostStatus* status);

void
saj_dataWriterListenerOnPublicationMatch(
    void* listenerData, 
    gapi_dataWriter dataWriter,
    const gapi_publicationMatchedStatus* status);

#ifdef __cplusplus
}
#endif
#endif
