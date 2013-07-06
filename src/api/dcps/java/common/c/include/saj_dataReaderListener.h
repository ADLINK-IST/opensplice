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
#ifndef SAJ_DATAREADER_LISTENER
#define SAJ_DATAREADER_LISTENER

#ifdef __cplusplus
extern "C" {
#endif

#include "saj_listener.h"

struct gapi_dataReaderListener*
saj_dataReaderListenerNew(
    JNIEnv *env,
    jobject listener);

void
saj_dataReaderListenerOnRequestedDeadlineMissed(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_requestedDeadlineMissedStatus* status);

void
saj_dataReaderListenerOnRequestedIncompatibleQos(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_requestedIncompatibleQosStatus* status);

void
saj_dataReaderListenerOnSampleRejected(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_sampleRejectedStatus* status);

void
saj_dataReaderListenerOnLivelinessChanged(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_livelinessChangedStatus* status);

void
saj_dataReaderListenerOnDataAvailable(
    void* listenerData, 
    gapi_dataReader dataReader);

void
saj_dataReaderListenerOnSubscriptionMatch(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_subscriptionMatchedStatus* status);

void
saj_dataReaderListenerOnSampleLost(
    void* listenerData, 
    gapi_dataReader dataReader,
    const gapi_sampleLostStatus* status);

#ifdef __cplusplus
}
#endif
#endif
