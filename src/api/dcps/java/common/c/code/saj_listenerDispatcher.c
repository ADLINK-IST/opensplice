/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "saj_ListenerDispatcher.h"
#include "saj_utilities.h"
#include "u_listener.h"
#include "v_status.h"
#include "os_report.h"
#include "cmn_listenerDispatcher.h"

#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_ListenerDispatcher_##name

C_CLASS(eventHandlerArg);
C_STRUCT(eventHandlerArg)
{
    JNIEnv *env;
    c_iter eventList;
    os_boolean dataAvailable;
};

static void
eventHandler (
    v_listenerEvent event,
    c_voidp arg)
{
    c_iter eventList = NULL;
    eventHandlerArg a = (eventHandlerArg)arg;
    if (event->kind & V_EVENT_TRIGGER) {
        /* Nothing to deliver so ignore. */
        return;
    }
    eventList = saj_eventListNew(a->env, event);
    a->eventList = c_iterConcat(a->eventList, eventList);
    /* no need to free eventList because c_iterConcat frees it */
}

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniListenerNew)(
    JNIEnv *env,
    jobject jentity,
    jlong uParticipant)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);
    return SAJ_JLONG(u_listenerNew(SAJ_VOIDP(uParticipant), OS_TRUE));
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniListenerFree)(
    JNIEnv *env,
    jobject jentity,
    jlong uListener)
{
    int retcode;
    u_result uResult = U_RESULT_OK;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);

    uResult = u_objectClose(SAJ_VOIDP(uListener));
    retcode = saj_retcode_from_user_result(uResult);
    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniListenerInterrupt)(
    JNIEnv *env,
    jobject jentity,
    jlong uListener)
{
    int retcode;
    u_result uResult = U_RESULT_OK;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);

    uResult = u_listenerTrigger(SAJ_VOIDP(uListener));
    retcode = saj_retcode_from_user_result(uResult);
    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWait)(
    JNIEnv *env,
    jobject jentity,
    jlong uListener,
    jobject eventListHolder)
{
    jobjectArray eventList;
    C_STRUCT(eventHandlerArg) arg;
    u_result uResult;
    int retcode;
    int length = 0;
    jobject jevent;
    int i;

    OS_UNUSED_ARG(jentity);

    arg.env = env;
    arg.eventList = NULL;
    arg.dataAvailable = FALSE;

    uResult = u_listenerWait(SAJ_VOIDP(uListener), eventHandler, &arg, OS_DURATION_INFINITE);
    retcode = saj_retcode_from_user_result(uResult);
    if ((retcode == SAJ_RETCODE_OK) && (arg.eventList != NULL)) {
        length = c_iterLength(arg.eventList);
        eventList = NEW_OBJECTARRAY(env, length, GET_CACHED(event_class), NULL);
        for (i = 0; i < length; i++)
        {
            jevent = c_iterTakeFirst(arg.eventList);
            assert(jevent != NULL);
            SET_OBJECTARRAY_ELEMENT(env, eventList, i, jevent);
            DELETE_LOCAL_REF(env, jevent);
        }
        SET_OBJECT_FIELD(env, eventListHolder, eventList_value, eventList);
        DELETE_LOCAL_REF(env, eventList);
        c_iterFree(arg.eventList);
    }

    return (jint)retcode;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

/**
 * Class:     org_opensplice_dds_dcps_ListenerDispatcherImpl
 * Method:    jniGetStatusChanges
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetStatusChanges)(
    JNIEnv *env,
    jobject jentity,
    jlong uListenerDispatcher)
{
    u_result uResult;
    u_eventMask mask;

    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);

    uResult = u_entityGetEventState(SAJ_VOIDP(uListenerDispatcher), &mask);
    if (uResult != U_RESULT_OK) {
        OS_REPORT(OS_ERROR, "ListenerDispatcher::get_status_changes", 0,
                  "u_entityGetEventState failed. result = %s",
                   u_resultImage(uResult));
    }
    return (jint)mask;
}

JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniGetInstanceHandle)(
    JNIEnv *env,
    jobject jentity,
    jlong uListenerDispatcher)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jentity);
    return SAJ_JLONG(u_entityGetInstanceHandle(SAJ_VOIDP(uListenerDispatcher)));
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniStackSize)(
    JNIEnv *env,
    jobject jListenerDispatcher,
    jlong uParticipant)
{
    OS_UNUSED_ARG (env);
    OS_UNUSED_ARG (jListenerDispatcher);

    return cmn_listenerDispatcher_stack_size (SAJ_VOIDP(uParticipant));
}

#undef SAJ_FUNCTION
