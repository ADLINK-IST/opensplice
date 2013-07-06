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
#include "saj_listener.h"
#include "saj_dataWriterListener.h"
#include "os_heap.h"
#include "gapi.h"

struct gapi_dataWriterListener*
saj_dataWriterListenerNew(
    JNIEnv *env,
    jobject jlistener)
{
    struct gapi_dataWriterListener* listener;
    saj_listenerData ld;

    listener = NULL;

    ld = saj_listenerDataNew(env, jlistener);

    if(ld != NULL){
        listener = gapi_dataWriterListener__alloc();
        saj_listenerInit((struct gapi_listener*)listener);
        listener->listener_data = ld;

        listener->on_offered_deadline_missed    = saj_dataWriterListenerOnOfferedDeadlineMissed;
        listener->on_offered_incompatible_qos   = saj_dataWriterListenerOnOfferedIncompatibleQos;
        listener->on_liveliness_lost            = saj_dataWriterListenerOnLivelinessLost;
        listener->on_publication_match          = saj_dataWriterListenerOnPublicationMatch;
    }
    return listener;
}

void
saj_dataWriterListenerOnOfferedDeadlineMissed(
    void* listenerData,
    gapi_dataWriter dataWriter,
    const gapi_offeredDeadlineMissedStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataWriter;
    saj_returnCode rc;

    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutOfferedDeadlineMissedStatus(env,
                        (gapi_offeredDeadlineMissedStatus *)status,
                        &jstatus);

    if(rc == SAJ_RETCODE_OK){
        jdataWriter = saj_read_java_address(dataWriter);
        (*env)->CallVoidMethod(env, ld->jlistener,
                        GET_CACHED(listener_onOfferedDeadlineMissed_mid),
                        jdataWriter, jstatus);
    }
}

void
saj_dataWriterListenerOnOfferedIncompatibleQos(
    void* listenerData,
    gapi_dataWriter dataWriter,
    const gapi_offeredIncompatibleQosStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataWriter;
    saj_returnCode rc;

    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutOfferedIncompatibleQosStatus(env,
                        (gapi_offeredIncompatibleQosStatus *)status,
                        &jstatus);

    if(rc == SAJ_RETCODE_OK){
        jdataWriter = saj_read_java_address(dataWriter);
        (*env)->CallVoidMethod(env, ld->jlistener,
                        GET_CACHED(listener_onOfferedIncompatibleQos_mid),
                        jdataWriter, jstatus);
    }
}

void
saj_dataWriterListenerOnLivelinessLost(
    void* listenerData,
    gapi_dataWriter dataWriter,
    const gapi_livelinessLostStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataWriter;
    saj_returnCode rc;

    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutLivelinessLostStatus(env,
                        (gapi_livelinessLostStatus *)status,
                        &jstatus);

    if(rc == SAJ_RETCODE_OK){
        jdataWriter = saj_read_java_address(dataWriter);
        (*env)->CallVoidMethod(env, ld->jlistener,
                        GET_CACHED(listener_onLivelinessLost_mid),
                        jdataWriter, jstatus);
    }
}

void
saj_dataWriterListenerOnPublicationMatch(
    void* listenerData,
    gapi_dataWriter dataWriter,
    const gapi_publicationMatchedStatus* status)
{
    saj_listenerData ld;
    JNIEnv *env;
    jobject jstatus;
    jobject jdataWriter;
    saj_returnCode rc;

    ld = saj_listenerData(listenerData);
    env = *(JNIEnv**)os_threadMemGet(OS_THREAD_JVM);

    rc = saj_statusCopyOutPublicationMatchStatus(env,
                        (gapi_publicationMatchedStatus *)status,
                        &jstatus);

    if(rc == SAJ_RETCODE_OK){
        jdataWriter = saj_read_java_address(dataWriter);
        (*env)->CallVoidMethod(env, ld->jlistener,
                        GET_CACHED(listener_onPublicationMatch_mid),
                        jdataWriter, jstatus);
    }
}
