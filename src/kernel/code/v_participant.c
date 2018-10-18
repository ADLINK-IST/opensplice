/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "v__kernel.h"
#include "v__participant.h"
#include "v__participantQos.h"
#include "v__kernel.h"
#include "v__leaseManager.h"
#include "v__publisher.h"
#include "v__writer.h"
#include "v__subscriber.h"
#include "v__dataReader.h"
#include "v__reader.h"
#include "v__observer.h"
#include "v__builtin.h"
#include "v__lease.h"
#include "v__waitset.h"
#include "v__observable.h"
#include "v__listener.h"
#include "v__processInfo.h"
#include "v__topicAdapter.h"
#include "v__typeRepresentation.h"
#include "v__spliced.h"

#include "v__entity.h"
#include "v_event.h"
#include "v_public.h"
#include "v_proxy.h"
#include "v_service.h"

#include "v_topic.h"
#include "v_subscriberQos.h"
#include "v_readerQos.h"
#include "v_dataReader.h"
#include "v_groupSet.h"
#include "v_group.h"
#include "v_event.h"

#include "os_report.h"
#include "os_process.h"
#include "q_expr.h"

#define V_BUILTINSUBSCRIBER_NAME "__BUILTIN SUBSCRIBER__"

static v_result
v_participantMonitorSpliceDeamonLiveliness(
    v_kernel kernel,
    const c_char *name,
    v_participant participant);

v_participant
v_participantNew(
    v_kernel kernel,
    const c_char *name,
    v_participantQos qos,
    c_bool enable)
{
    v_participant p = NULL;
    v_participantQos q;
    v_result result;

    assert(C_TYPECHECK(kernel,v_kernel));
    /* Do not use C_TYPECHECK on qos parameter,
     * since it might be allocated on heap!
     */

    /* do no use cast method on qos parameter,
     * it is most likely allocated on heap!
     */
    if (v_participantQosCheck(qos) == V_RESULT_OK) {
        q = v_participantQosNew(kernel, qos);

        if (q == NULL) {
            OS_REPORT(OS_ERROR, "v_participantNew", V_RESULT_INTERNAL_ERROR,
                        "Creation of participant <%s> failed. Cannot create participant QoS.",
                        name);
            p = NULL;
        } else {
            p = v_participant(v_objectNew(kernel,K_PARTICIPANT));
            v_participantInit(p, name, q);
            if(enable) {
                (void)v_entityEnable(v_entity(p));
            }
            result = v_participantMonitorSpliceDeamonLiveliness(kernel, name, p);
            if(result != V_RESULT_OK) {
                OS_REPORT(OS_CRITICAL, "v_participant", result,
                    "Unable to monitor the splice deamon's liveliness. It is possible no splice deamon "
                    "was available to monitor.");
            } /* else do nothing, it went ok */

            c_free(q);
        }
    }

    return p;
}

v_result
v_participantEnable (
    _In_ v_participant p)
{
    v_kernel kernel = v_objectKernel(v_object(p));
    v_message builtinMsg;

    /* Here the Builtin Topic of the participant is published.
     * This call maybe a noop in case builtin is disabled on kernel level.
     */
    builtinMsg = v_builtinCreateParticipantInfo(kernel->builtin,p);
    v_writeBuiltinTopic(kernel, V_PARTICIPANTINFO_ID, builtinMsg);
    c_free(builtinMsg);

    /* publish Builtin Topic CMParticipant */
    builtinMsg = v_builtinCreateCMParticipantInfo(kernel->builtin,p);
    v_writeBuiltinTopic(kernel, V_CMPARTICIPANTINFO_ID, builtinMsg);
    c_free(builtinMsg);

    return V_RESULT_OK;
}

v_result
v_participantMonitorSpliceDeamonLiveliness(
    v_kernel kernel,
    const c_char *name,
    v_participant participant)
{
    v_result result;
    c_iter participants;
    v_participant splicedParticipant;

    assert(C_TYPECHECK(participant,v_participant));
    assert(C_TYPECHECK(kernel,v_kernel));

    if(!name || (0 != strcmp(name, V_SPLICED_NAME) && 0 != strcmp(name, V_BUILT_IN_PARTICIPANT_NAME)))
    {
        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        if(c_iterLength(participants) == 1)
        {
            splicedParticipant = v_participant(c_iterTakeFirst(participants));
            assert(splicedParticipant);
            result = v_leaseManagerRegister(
                         participant->leaseManager,
                         v_service(splicedParticipant)->lease,
                         V_LEASEACTION_SPLICED_DEATH_DETECTED,
                         v_public(kernel),
                         FALSE /* only observing, do not repeat */);
            if(result != V_RESULT_OK) {
                OS_REPORT(OS_CRITICAL, "v_participant", result,
                    "A fatal error was detected when trying to register the spliced's liveliness lease "
                    "to the lease manager of participant %p (%s). "
                    "The result code was %d.", (void*)participant, name, result);
            }
            c_free(splicedParticipant);
        } else {
            v_participant p;
            result = V_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_CRITICAL, "v_participant", result,
                "A fatal error was detected when trying to register the spliced's liveliness lease "
                "to the lease manager of participant %p (%s). Found %u splice deamon(s), but expected to find 1!. "
                "The result code was %d.", (void*)participant, name, c_iterLength(participants), result);
            /* empty the iterator to avoid memory leaks, in cases where it is bigger then 1 */
            while ((p = c_iterTakeFirst(participants)) != NULL) {
                c_free(p);
            }
        }
        c_iterFree(participants);
    } else
    {
        /* else do nothing, exception for splice deamon monitoring are made for
         * the splice deamon participant and the built in participant
         */
        result = V_RESULT_OK;
    }
    return result;
}

void
v_participantInit(
    _Inout_ v_participant p,
    _In_opt_z_ const c_char *name,
    _In_ v_participantQos qos)
{
    v_kernel kernel;
    c_base base;
    v_message builtinMsg;
    c_type writerProxyType;
    char procName[128];

    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(qos, v_participantQos));

    kernel = v_objectKernel(p);
    base = c_getBase(p);
    v_entityInit(v_entity(p), name);

    p->entities = c_setNew(c_resolve(base,"kernelModuleI::v_entity"));
    p->qos = c_keep(qos);
    /* Currently default LIVELINESS policy is used: kind=AUTOMATIC,
     * duration=INFINITE This setting implies no lease registration.
     */

    p->leaseManager = v_leaseManagerNew(kernel);
    p->resendQuit = FALSE;
    (void)c_mutexInit(base, &p->resendMutex);
    c_condInit(base, &p->resendCond, &p->resendMutex);
    writerProxyType = v_kernelType(kernel,K_PROXY);
    p->resendWriters = c_tableNew(writerProxyType, "source.index,source.serial");
    p->resendIteration = 0;
    p->processId = (c_longlong)os_procIdSelf();
    (void) os_procGetProcessName (procName, sizeof (procName));
    p->processName = c_stringNew (base, procName);
    p->processIsZombie = FALSE;
    p->ignore = NULL; // ignore Admin is initially NULL and lazy created when the user ignores entities.

    p->typeRepresentations = c_tableNew(v_kernelType(kernel,K_TYPEREPRESENTATION),"typeName");

    v_addParticipant(kernel, p);

    if (v_entityEnabled(v_entity(p))) {
        /* Here the Builtin Topic of the participant is published.
         * This call maybe a noop in case builtin is disabled on kernel level.
         */
        builtinMsg = v_builtinCreateParticipantInfo(kernel->builtin,p);
        v_writeBuiltinTopic(kernel, V_PARTICIPANTINFO_ID, builtinMsg);
        c_free(builtinMsg);

        /* publish Builtin Topic CMParticipant */
        builtinMsg = v_builtinCreateCMParticipantInfo(kernel->builtin,p);
        v_writeBuiltinTopic(kernel, V_CMPARTICIPANTINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
}

void
v_participantFree(
    v_participant p)
{
    v_message builtinMsg;
    v_participant found;
    v_entity e;
    v_kernel kernel;

    /* Not clear yet why builtin subscriber lock and participant lock are not taken now?
     * Also not clear why observer lock is not taken but is freed at the end!
     */
    if (p != NULL) {
        assert(C_TYPECHECK(p,v_participant));

        kernel = v_objectKernel(p);
        (void)OSPL_REMOVE_OBSERVER(kernel,p, V_EVENTMASK_ALL, NULL);
        if (v_entityEnabled(v_entity(p))) {
            builtinMsg = v_builtinCreateParticipantInfo(kernel->builtin,p);
            v_writeDisposeBuiltinTopic(kernel, V_PARTICIPANTINFO_ID, builtinMsg);
            c_free(builtinMsg);

            builtinMsg = v_builtinCreateParticipantInfo(kernel->builtin,p);
            v_unregisterBuiltinTopic(kernel, V_PARTICIPANTINFO_ID, builtinMsg);
            c_free(builtinMsg);

            builtinMsg = v_builtinCreateCMParticipantInfo(kernel->builtin,p);
            v_writeDisposeBuiltinTopic(kernel, V_CMPARTICIPANTINFO_ID, builtinMsg);
            c_free(builtinMsg);

            builtinMsg = v_builtinCreateCMParticipantInfo(kernel->builtin,p);
            v_unregisterBuiltinTopic(kernel, V_CMPARTICIPANTINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }

        while ((e = c_take(p->entities)) != NULL) {
            switch (v_objectKind(e)) {
            case K_PUBLISHER:
                v_publisherFree(v_publisher(e));
            break;
            case K_SUBSCRIBER:
                v_subscriberFree(v_subscriber(e));
            break;
            case K_WAITSET:
                v_waitsetFree(v_waitset(e));
            break;
            case K_LISTENER:
                v_listenerFree(v_listener(e));
            break;
            case K_TOPIC_ADAPTER:
                v_topicAdapterFree(v_topicAdapter(e));
            break;
            default:
                OS_REPORT(OS_WARNING,"Kernel Participant",V_RESULT_ILL_PARAM,
                            "Illegal contained object (%s)",
                            v_participantName(p));
            break;
            }
            c_free(e); /* deref o since c_take will not */
        }

        found = v_removeParticipant(kernel,p);
        if (found) {
            assert(found == p);
            c_free(found);
        } else {
            OS_REPORT(OS_WARNING,"Spliced",V_RESULT_ILL_PARAM,
                        "Garbage collecting an already removed participant (%s), probably it crashed but was able to cleanup.",
                        v_participantName(p));
        }

        v_entityFree(v_entity(p));
    }
}

void
v_participantDeinit(
    v_participant p)
{
    assert(C_TYPECHECK(p,v_participant));

    v_leaseManagerFree(p->leaseManager);
    p->leaseManager = NULL;

    v_entityDeinit(v_entity(p));
}

void
v_participantAdd(
    v_participant p,
    v_object e)
{
    v_object found;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_object));

    OSPL_LOCK(p);
    found = c_setInsert(p->entities,e);
    OSPL_UNLOCK(p);
    assert(found == e);
    OS_UNUSED_ARG(found);
}

void
v_participantRemove(
    v_participant p,
    v_object e)
{
    v_object found;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_object));

    OSPL_LOCK(p);
    found = c_remove(p->entities,e,NULL,NULL);
    OSPL_UNLOCK(p);
    c_free(found);
}

void
v_participantConnectGroup (
    v_participant _this,
    v_group g)
{
    v_entity entity;
    c_iter entities;

    OSPL_LOCK(_this);
    entities = ospl_c_select(_this->entities, 0);
    OSPL_UNLOCK(_this);
    while ((entity = c_iterTakeFirst(entities)) != NULL) {
        switch (v_objectKind(entity)) {
        case K_PUBLISHER:
            (void)v_publisherConnectNewGroup(v_publisher(entity), g);
        break;
        case K_SUBSCRIBER:
            (void)v_subscriberConnectNewGroup(v_subscriber(entity), g);
        break;
        default:
            /* Other entities do not connect */
        break;
        }
        c_free(entity);
    }
    c_iterFree(entities);
}

static c_bool
notifyGroupCoherentPublication(
    c_object o,
    c_voidp arg)
{
    if (v_objectKind(o) == K_SUBSCRIBER) {
        v_subscriberNotifyGroupCoherentPublication(v_subscriber(o), v_message(arg));
    }
    return TRUE;
}

void
v_participantNotifyGroupCoherentPublication(
    v_participant _this,
    v_message msg)
{
    OSPL_LOCK(_this);
    (void)c_walk(_this->entities, notifyGroupCoherentPublication, msg);
    OSPL_UNLOCK(_this);
}

static c_bool
assertLivelinessPublisher(
    c_object o,
    c_voidp arg)
{
    v_object vObject = v_object(o);

    if (v_objectKind(vObject) == K_PUBLISHER) {
        v_publisherAssertLiveliness(v_publisher(vObject), (v_event)arg);
    }
    return TRUE;
}

#define v_historicalDeleteRequest(o) (C_CAST(o,v_historicalDeleteRequest))

void
v_participantDeleteHistoricalData(
    v_participant participant,
    const c_char* partitionExpr,
    const c_char* topicExpr)
{
    c_iter matchingGroups;
    v_group group;
    os_timeE t;
    c_value params[2];
    C_STRUCT(v_event) event;
    v_historicalDeleteRequest hde;
    c_base base;

    assert(participant != NULL);
    assert(C_TYPECHECK(participant, v_participant));
    assert(partitionExpr);
    assert(topicExpr);

    if(partitionExpr && topicExpr){
        params[0]  = c_stringValue((c_string)partitionExpr);
        params[1]  = c_stringValue((c_string)topicExpr);

        OSPL_LOCK(participant);
        t = os_timeEGet();
        matchingGroups = v_groupSetSelect(
                                v_objectKernel(participant)->groupSet,
                                "partition.name like %0 AND topic.name like %1",
                                params);
        OSPL_UNLOCK(participant);

        group = v_group(c_iterTakeFirst(matchingGroups));
        while(group){
            (void)v_groupDeleteHistoricalData(group, t);
            c_free(group);
            group = v_group(c_iterTakeFirst(matchingGroups));
        }
        c_iterFree(matchingGroups);

        hde = v_historicalDeleteRequest(v_objectNew(v_objectKernel(participant), K_HISTORICALDELETEREQUEST));
        if (hde) {
            base = c_getBase(participant);
            hde->partitionExpr = c_stringNew(base, (c_char *)partitionExpr);
            hde->topicExpr = c_stringNew(base, (c_char *)topicExpr);
            hde->deleteTime = t;

            event.kind = V_EVENT_HISTORY_DELETE;
            event.source = v_observable(participant);
            event.data = hde;
            event.handled = TRUE;

            OSPL_THROW_EVENT(v_objectKernel(participant), &event);
            c_free(hde);
        }
    }
    return;
}

void
v_participantNotify(
    v_participant _this,
    v_event event,
    c_voidp userData)
{
    /* This Notify method is part of the observer-observable pattern.
     * It is designed to be invoked when _this object as observer receives
     * an event from an observable object.
     * It must be possible to pass the event to the subclass of itself by
     * calling <subclass>Notify(_this, event, userData).
     * This implies that _this as observer cannot be locked within any
     * Notify method to avoid deadlocks.
     * For consistency _this must be locked by OSPL_LOCK(_this) before
     * calling this method.
     */
    OS_UNUSED_ARG(userData);
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_participant));
    assert(event != NULL);

    switch (event->kind) {
    case V_EVENT_LIVELINESS_ASSERT:
        (void)c_walk(_this->entities, assertLivelinessPublisher, event);
    break;
    case V_EVENT_NEW_GROUP:
    case V_EVENT_SERVICESTATE_CHANGED:
    case V_EVENT_DATA_AVAILABLE:
    case V_EVENT_HISTORY_DELETE:
    case V_EVENT_HISTORY_REQUEST:
    case V_EVENT_PERSISTENT_SNAPSHOT:
    case V_EVENT_CONNECT_WRITER:
        /*Do nothing here.*/
    break;
    case V_EVENT_ALL_DATA_DISPOSED:
    case V_EVENT_INCONSISTENT_TOPIC:
        (void)v_entityNotifyListener(v_entity(_this), event);
    break;
    default:
        OS_REPORT(OS_WARNING,"v_participantNotify",V_RESULT_ILL_PARAM,
                  "Notify encountered unknown event kind (%d)", event->kind);
    break;
    }
}

void
v_participantAssertLiveliness(
    v_participant p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));

    /* Walk over all entities and assert liveliness on all writers */
    OSPL_LOCK(p);
    (void)c_walk(p->entities, assertLivelinessPublisher, NULL);
    OSPL_UNLOCK(p);
}

v_participantQos
v_participantGetQos(
    v_participant _this)
{
    v_participantQos qos;

    assert(C_TYPECHECK(_this,v_participant));

    OSPL_LOCK(_this);
    qos = c_keep(_this->qos);
    OSPL_UNLOCK(_this);

    return qos;
}

v_result
v_participantSetQos(
    v_participant p,
    v_participantQos tmpl)
{
    v_message builtinMsg = NULL;
    v_participantQos qos;
    v_kernel kernel;
    v_qosChangeMask cm;
    v_result result;

    assert(C_TYPECHECK(p,v_participant));
    /* Do not use C_TYPECHECK on the tmpl qos parameter, since it might be allocated on heap! */

    result = v_participantQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        OSPL_LOCK(p);
        kernel = v_objectKernel(p);
        qos = v_participantQosNew(kernel, tmpl);

        if (qos) {
            result = v_participantQosCompare(p->qos, qos, &cm);
            if ((result == V_RESULT_OK) && (cm != 0)) {
                c_free(p->qos);
                p->qos = c_keep(qos);
                if (v__entityEnabled_nl(v_entity(p))) {
                    builtinMsg = v_builtinCreateParticipantInfo(kernel->builtin,p);
                }
            }
            c_free(qos);
        } else {
            result = V_RESULT_OUT_OF_MEMORY;
        }
        OSPL_UNLOCK(p);
    }

    if (builtinMsg != NULL) {
        v_writeBuiltinTopic(kernel, V_PARTICIPANTINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
    return result;
}

v_leaseManager
v_participantGetLeaseManager(
    v_participant p)
{
    assert(C_TYPECHECK(p,v_participant));

    return c_keep(p->leaseManager);
}

#define RESEND_SECS         (0U)
#define RESEND_NANOSECS     (2000000U) /* 2 ms */
#define RESEND_SECS_OOM     (1U)
#define RESEND_NANOSECS_OOM (0U)
void
v_participantResendManagerMain(
    v_participant p)
{
    c_iter writerProxies;
    v_proxy wp;
    v_writer w;
    v_writer *wPtr;
    v_handleResult r;
    os_duration waitTime = OS_DURATION_INIT(RESEND_SECS, RESEND_NANOSECS);
    v_result waitResult;
    c_bool outOfMemReported = FALSE;

    assert(C_TYPECHECK(p,v_participant));

    c_mutexLock(&p->resendMutex);

    while (!p->resendQuit) {
        writerProxies = ospl_c_select(p->resendWriters, 0);
        c_mutexUnlock(&p->resendMutex);
        wp = v_proxy(c_iterTakeFirst(writerProxies));
        while (wp != NULL) {
            wPtr = &w;
            r = v_handleClaim(wp->source,(v_object *)wPtr);
            if (r == V_HANDLE_OK) {
                assert(C_TYPECHECK(w,v_writer));
                if (!v_writerResend(w)) {
                    if (!outOfMemReported) {
                        outOfMemReported = TRUE;
                        OS_REPORT(OS_CRITICAL, "resendManager", V_RESULT_OUT_OF_MEMORY,
                                  "Out of resources: not enough memory available");
                    }
                    waitTime = OS_DURATION_INIT(RESEND_SECS_OOM, RESEND_NANOSECS_OOM);;
                } else {
                    waitTime = OS_DURATION_INIT(RESEND_SECS, RESEND_NANOSECS);
                }
                v_handleRelease(wp->source);
            } else {
              assert (0);
            }
            c_free(wp);
            wp = v_proxy(c_iterTakeFirst(writerProxies));
        }
        c_iterFree(writerProxies);
        c_mutexLock(&p->resendMutex);
        p->resendIteration++;
        c_condBroadcast (&p->resendCond); /* usually, no-one's listening */
        if (!p->resendQuit) {
            if (c_count(p->resendWriters) == 0) {
                waitResult = v_condWait(&p->resendCond, &p->resendMutex, OS_DURATION_INFINITE);
            } else {
                waitResult = v_condWait(&p->resendCond, &p->resendMutex, waitTime);
            }
            if (waitResult == V_RESULT_INTERNAL_ERROR)
            {
                OS_REPORT(OS_CRITICAL, "v_participantResendManagerMain", waitResult,
                          "v_condWait failed - thread will terminate");
                p->resendQuit = TRUE;
            }
        }
    }
    p->resendIteration++;
    c_condBroadcast (&p->resendCond);
    c_mutexUnlock(&p->resendMutex);
}

void
v_participantResendManagerQuit(
    v_participant p)
{
    assert(C_TYPECHECK(p,v_participant));

    c_mutexLock(&p->resendMutex);
    p->resendQuit = TRUE;
    c_condBroadcast(&p->resendCond);
    c_mutexUnlock(&p->resendMutex);
}

void
v_participantResendManagerAddWriter(
    v_participant p,
    v_writer w)
{
    v_proxy wp, found;
    assert(C_TYPECHECK(p,v_participant));

    wp = v_proxyNew(v_objectKernel(w), v_publicHandle(v_public(w)), NULL);

    c_mutexLock(&p->resendMutex);
    found = ospl_c_insert(p->resendWriters, wp);
    assert((found->source.index == wp->source.index) && (found->source.serial == wp->source.serial));
    OS_UNUSED_ARG(found);
    c_condBroadcast(&p->resendCond);
    c_mutexUnlock(&p->resendMutex);

    c_free(wp);
}

void
v_participantResendManagerRemoveWriter(
    v_participant p,
    v_writer w)
{
    C_STRUCT(v_proxy) wp;
    v_proxy found;

    wp.source = v_publicHandle(v_public(w));
    wp.userData = NULL;
    c_mutexLock(&p->resendMutex);
    found = c_remove(p->resendWriters, &wp, NULL, NULL);
    c_free(found); /* remove local reference transferred from collection */
    c_mutexUnlock(&p->resendMutex);
}

void
v_participantResendManagerRemoveWriterBlocking(
    v_participant p,
    v_writer w)
{
    v_result result = V_RESULT_OK;
    C_STRUCT(v_proxy) wp;
    v_proxy found;
    c_ulong resendIteration;

    wp.source = v_publicHandle(v_public(w));
    wp.userData = NULL;
    c_mutexLock(&p->resendMutex);
    resendIteration = p->resendIteration;
    found = c_remove(p->resendWriters, &wp, NULL, NULL);
    if (found)
    {
        c_free(found); /* remove local reference transferred from collection */
        if ((p->processId == (c_longlong)os_procIdSelf()) && !p->resendQuit)
        {
            while ((resendIteration == p->resendIteration) && (result == V_RESULT_OK)) {
                result = v_condWait (&p->resendCond, &p->resendMutex, OS_DURATION_INFINITE);
            }
            if (result != V_RESULT_OK) {
                OS_REPORT(OS_CRITICAL,"v_participantResendManagerRemoveWriterBlocking",result,
                            "v_condWait failed with result %d",result);
            }
        }
    }
    c_mutexUnlock(&p->resendMutex);
}


v_message
v_participantCreateCandMCommand(
    v_participant participant)
{
    v_message msg;
    v_kernel kernel;
    v_topic topic;

    assert(participant != NULL);
    assert(C_TYPECHECK(participant,v_participant));

    kernel = v_objectKernel(participant);
    topic = v_builtinTopicLookup(kernel->builtin, V_C_AND_M_COMMAND_ID);
    msg = v_topicMessageNew(topic);
    return msg;
}

v_result
v_participantCandMCommandSetDisposeAllData(
    v_participant participant,
    v_message msg,
    char *topicExpr,
    char *partitionExpr)
{
    v_kernel kernel;
    v_topic topic;
    c_base base;
    v_controlAndMonitoringCommand *command;
    struct v_commandDisposeAllData *disposeCmd;

    assert(participant != NULL);
    assert(C_TYPECHECK(participant,v_participant));
    assert(msg != NULL );
    assert(C_TYPECHECK(msg,v_message));

    kernel = v_objectKernel(participant);
    topic = v_builtinTopicLookup(kernel->builtin, V_C_AND_M_COMMAND_ID);
    command = (v_controlAndMonitoringCommand *) (msg + 1);
    command->u._d = V_COMMAND_DISPOSE_ALL_DATA;
    base = c_getBase(c_object(topic));

    disposeCmd = &command->u._u.dispose_all_data_info;
    disposeCmd->topicExpr = c_stringNew(base, topicExpr);
    disposeCmd->partitionExpr = c_stringNew(base, partitionExpr);

    return ( ( disposeCmd->topicExpr != NULL
               && disposeCmd->partitionExpr != NULL )
             ? V_RESULT_OK
             : V_RESULT_OUT_OF_MEMORY );
}

v_result
v_participantWriteCandMCommand(
    v_participant participant,
    v_message msg)
{
   v_writeResult wres;
   v_writer builtinWriter;

   assert(participant != NULL);
   assert(C_TYPECHECK(participant,v_participant));
   assert(msg != NULL);
   assert(C_TYPECHECK(msg,v_message));

   builtinWriter = v_builtinWriterLookup(v_objectKernel(participant)->builtin,
                                         V_C_AND_M_COMMAND_ID);
   wres = v_writerWrite(builtinWriter, msg, os_timeWGet(), NULL);
   return ( wres == V_WRITE_SUCCESS
            ? V_RESULT_OK
              : V_RESULT_INTERNAL_ERROR );
}

c_iter
v_participantGetEntityList(v_participant participant)
{
    v_object object;
    c_iter entities;
    c_ulong index;

    assert(participant != NULL);

    /* Copy the current entity set into an iter list. */
    OSPL_LOCK(participant);
    entities = ospl_c_select(participant->entities, 0);
    OSPL_UNLOCK(participant);

    /* Unfortunately, the entities list of the participant can
     * contain waitsets as well.
     * This loop makes sure that only entities are returned.
     */
    index = 0;
    while (index < c_iterLength(entities)) {
        object = v_object(c_iterObject(entities, index));
        assert(object);

        /* Remove this object from the list when it's not an entity. */
        if (c_instanceOf(object, "v_entity") == FALSE) {
            /* This is not an entity.
             *
             * Taking this object means that the remaining objects will shift a
             * place down in the list. Or when this is the last object, then this
             * place becomes invalid.
             * Either way, we shouldn't increase the index after removing an object.
             *
             * By taking the object, the c_iterLength is reduced by 1 as well.
             * That will be detected with the next loop iteration (no pun intended).
             * So, we won't get out of bounds, even when the removed object was the
             * last one (tail) in the list.
             */
            object = v_object(c_iterTake(entities, object));
            assert(object);
            c_free(object);
        } else {
            /* This is an entity: keep it and get next object. */
            index++;
        }
    }

    return entities;
}

v_typeRepresentation
v__participantAddTypeRepresentation (
    v_participant p,
    v_typeRepresentation tr)
{
    v_typeRepresentation found;

    assert(p != NULL);
    assert(C_TYPECHECK(p, v_participant));
    assert(tr != NULL);

    OSPL_LOCK(p);
    found = ospl_c_insert(p->typeRepresentations, tr);
    OSPL_UNLOCK(p);

    return found;
}

v_typeRepresentation
v__participantRemoveTypeRepresentation (
    v_participant p,
    v_typeRepresentation tr)
{
    v_typeRepresentation found;

    assert(p != NULL);
    assert(C_TYPECHECK(p, v_participant));
    assert(tr != NULL);

    OSPL_LOCK(p);
    found = c_remove(p->typeRepresentations, tr, NULL, NULL);
    OSPL_UNLOCK(p);

    return found;
}

static c_bool
transactionsPurge (
    c_object o,
    c_voidp arg)
{
    c_bool result = TRUE;

    OS_UNUSED_ARG(arg);

    switch(v_objectKind(o)) {
    case K_SUBSCRIBER:
        /* A transaction flush purges and flushes the obsolete groups,
         * flushing is required in case the obsolete group contains samples
         * which cannot be discarded. */
        v_subscriberGroupTransactionFlush(v_subscriber(o));
        break;
    default:
        /* No need to purge other entities */
        break;
    }

    return result;
}

void
v_participantTransactionsPurge (
    v_participant _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_participant));

    OSPL_LOCK(_this);
    (void)c_walk(_this->entities, transactionsPurge, NULL);
    OSPL_UNLOCK(_this);
}

static v_ignoreAdmin
getIgnoreAdmin(
    v_participant _this)
{
    if (_this->ignore == NULL) {
        c_type type = c_resolve(c_getBase(_this), "kernelModuleI::v_ignoreAdmin");
        _this->ignore = c_new(type);
        c_free(type);
    }
    return _this->ignore;
}

static os_boolean
inList(
    c_array list,
    v_gid gid)
{
    os_uint32 i, size;
    v_gid *gidlist = (v_gid *)list;
    size = c_arraySize(list);
    for (i=0; i<size; i++) {
        if (gidlist[i].systemId == gid.systemId && gidlist[i].localId == gid.localId)
        {
            return OS_TRUE;
        }
    }
    return OS_FALSE;
}

static void
ignoreGID(
    v_ignoreAdmin _this,
    c_array *ignoreList,
    v_gid gid)
{
    c_type type;
    os_uint32 i, size;

    type = c_resolve(c_getBase(_this),"kernelModule::v_gid");
    if (*ignoreList == NULL) {
        *ignoreList = c_arrayNew(type, 1);
        ((v_gid *)(*ignoreList))[0] = gid;
    } else if (!inList(*ignoreList, gid)) {
        v_gid *old = (v_gid *)(*ignoreList);
        size = c_arraySize(*ignoreList);
        *ignoreList = c_arrayNew(type, size+1);
        for (i=0; i<size; i++) {
            ((v_gid *)(*ignoreList))[i] = old[i];
        }
        ((v_gid *)(*ignoreList))[i] = gid;
        c_free(old);
    }
    c_free(type);

}

static void
disposeGID(
    v_ignoreAdmin _this,
    c_array *ignoreList,
    v_gid gid)
{
    c_type type;
    os_uint32 i, n, size;

    if (inList(*ignoreList, gid)) {
        v_gid *old = (v_gid *)(*ignoreList);
        size = c_arraySize(*ignoreList);
        type = c_resolve(c_getBase(_this),"kernelModule::v_gid");
        *ignoreList = c_arrayNew(type, size-1);
        c_free(type);
        n = 0;
        for (i=0; i<size; i++) {
            if (old[i].systemId != gid.systemId || old[i].localId != gid.localId)
            {
                ((v_gid *)(*ignoreList))[n++] = old[i];
            }
        }
        c_free(old);
    }
}

static v_result
v_participantIgnorePublicationInfo (
    v_participant _this,
    const struct v_publicationInfo *info);

typedef struct ignoreArg_s {
    v_gid gid;
    v_participant participant;
} *ignoreArg;

static os_boolean
ignorePublications(
    const v_message publication,
    c_voidp arg)
{
    const struct v_publicationInfo *info = (struct v_publicationInfo *)(publication + 1);
    ignoreArg a = (ignoreArg)arg;

    if (info->participant_key.systemId == a->gid.systemId &&
        info->participant_key.localId == a->gid.localId)
    {
        TRACE_IGNORE("Participant::ignore_participant found a publication for topic \"%s\"\n", info->topic_name);

        v_participantIgnorePublicationInfo(a->participant, info);
    }
    return OS_TRUE;
}

static os_boolean
ignoreSubscriptions(
    const v_message subscription,
    c_voidp arg)
{
    const struct v_subscriptionInfo *info = (struct v_subscriptionInfo *)(subscription + 1);
    ignoreArg a = (ignoreArg)arg;

    if (info->participant_key.systemId == a->gid.systemId &&
        info->participant_key.localId == a->gid.localId)
    {
        TRACE_IGNORE("Participant::ignore_participant found a subscription for topic \"%s\"\n", info->topic_name);

        v_participantIgnoreSubscription(a->participant, info->key);
    }
    return OS_TRUE;
}

v_result
v_participantIgnoreParticipant (
    v_participant _this,
    v_gid gid)
{
    v_result result = V_RESULT_OK;
    v_kernel kernel;
    v_ignoreAdmin ignore;
    struct ignoreArg_s a;

    TRACE_IGNORE("Participant::ignore_participant called for participant GID: {%d, %d, %d}\n",
                 gid.systemId, gid.localId, gid.serial);

    OSPL_LOCK(_this);
    ignore = getIgnoreAdmin(_this);
    ignoreGID(ignore, &ignore->participants, gid);
    OSPL_UNLOCK(_this);

    /* Now lookup all publications and subscriptions associated to this participant and add these to the
     * ignore publication and ignore subscription list
     */

    a.gid = gid;
    a.participant = _this;
    kernel = v_objectKernel(_this);
    result = v_kernelWalkPublications(kernel, ignorePublications, &a);
    assert(result == V_RESULT_OK);
    result = v_kernelWalkSubscriptions(kernel, ignoreSubscriptions, &a);
    assert(result == V_RESULT_OK);

    return result;
}

v_result
v_participantIgnoreTopic (
    v_participant _this,
    v_gid gid)
{
    v_result result = V_RESULT_UNSUPPORTED;
    v_ignoreAdmin ignore;

    OS_REPORT(OS_ERROR, "Kernel IgnoreTopic", result, "This operation is currently unsupported.");

    TRACE_IGNORE("v_participantIgnoreTopic called\n");

    OSPL_LOCK(_this);
    ignore = getIgnoreAdmin(_this);
    ignoreGID(ignore, &ignore->topics, gid);
    OSPL_UNLOCK(_this);

    /* Now lookup all publications and subscriptions associated to this topic and add these to the
     * ignore publication and ignore subscription list
     */

    return result;
}

static c_bool
ignoreOnReaders(
    c_object o,
    c_voidp publicationInfo)
{
    v_object vObject = v_object(o);

    if (v_objectKind(vObject) == K_DATAREADER) {
        v_dataReaderIgnore(v_dataReader(o), publicationInfo, TRUE);
    }
    return TRUE;
}

static c_bool
ignoreOnSubscribers(
    c_object o,
    c_voidp publicationInfo)
{
    v_object vObject = v_object(o);

    if (v_objectKind(vObject) == K_SUBSCRIBER) {
        v_subscriberWalkReaders(v_subscriber(o), ignoreOnReaders, publicationInfo);
    }
    return TRUE;
}

static v_result
v_participantIgnorePublicationInfo (
    v_participant _this,
    const struct v_publicationInfo *info)
{
    v_result result = V_RESULT_OK;
    v_ignoreAdmin ignore;

    TRACE_IGNORE("Participant::ignore_publication called for publication GID: {%d, %d, %d}\n",
                 info->key.systemId, info->key.localId, info->key.serial);

    OSPL_LOCK(_this);
    ignore = getIgnoreAdmin(_this);
    ignoreGID(ignore, &ignore->publications, info->key);

    /* Now visit local DataReaders and add gid to the DataReader ignore publication list */

    (void)c_walk(_this->entities, ignoreOnSubscribers, (void *)info);
    OSPL_UNLOCK(_this);

    return result;
}

v_result
v_participantIgnorePublication (
    v_participant _this,
    v_gid gid)
{
    v_result result = V_RESULT_UNSUPPORTED;
    v_message msg;

    msg = v_kernelLookupPublication(v_objectKernel(_this), gid);
    if (msg) {
        struct v_publicationInfo *info = v_builtinPublicationInfoData(msg);
        result = v_participantIgnorePublicationInfo(_this, info);
        c_free(msg);
    } else {
        OS_REPORT(OS_ERROR, "Ignore Publication", 0, "Lookup builtin Publication data failed");
    }
    return result;
}

v_result
v_participantIgnoreSubscription (
    v_participant _this,
    v_gid gid)
{
    v_result result = V_RESULT_OK;
    v_ignoreAdmin ignore;

    TRACE_IGNORE("Participant::ignore_subscription called for subscription GID: {%d, %d, %d}\n",
                 gid.systemId, gid.localId, gid.serial);

    OSPL_LOCK(_this);
    ignore = getIgnoreAdmin(_this);
    ignoreGID(ignore, &ignore->subscriptions, gid);
    OSPL_UNLOCK(_this);

    /* Now visit local DataWriters and add gid to the DataWriter ignore subscription list
     * This is only required if performance is an issue but for noe this doesn't seem the case.
     */

    return result;
}

static os_boolean
ignoredParticipant(
    v_ignoreAdmin _this,
    v_gid gid)
{
    if (inList(_this->participants, gid)) {
        return TRUE;
    }
    return FALSE;
}

void
v_participantNotifyPublication(
    v_participant _this,
    v_message msg)
{
    struct v_publicationInfo *info;

    OSPL_LOCK(_this);
    if (_this->ignore) {
        info = v_builtinPublicationInfoData(msg);
        if (v_messageStateTest(msg, L_DISPOSED)) {
            disposeGID(_this->ignore, &_this->ignore->publications, info->key);
            TRACE_IGNORE("Participant is notified to remove Publication GID {%u,%u,%u} from the ignore list\n",
                         info->key.systemId, info->key.localId, info->key.serial);
        } else if (ignoredParticipant(_this->ignore, info->participant_key)) {
            TRACE_IGNORE("Participant is notified to add Publication GID {%u,%u,%u} to the ignore lists\n",
                         info->key.systemId, info->key.localId, info->key.serial);
            ignoreGID(_this->ignore, &_this->ignore->publications, info->key);
            /* Now visit local DataReaders and add gid to the DataReader ignore publication list */
            (void)c_walk(_this->entities, ignoreOnSubscribers, (void *)info);
        }
    }
    OSPL_UNLOCK(_this);
}

void
v_participantNotifySubscription(
    v_participant _this,
    v_message msg)
{
    struct v_subscriptionInfo *info;
    c_bool ignore = FALSE;
    OSPL_LOCK(_this);
    if (_this->ignore) {
       info = v_builtinSubscriptionInfoData(msg);
       if (v_messageStateTest(msg, L_DISPOSED)) {
           disposeGID(_this->ignore, &_this->ignore->subscriptions, info->key);
           TRACE_IGNORE("Participant is notified to remove Subscription GID {%u,%u,%u} from the ignore list\n",
                        info->key.systemId, info->key.localId, info->key.serial);
       } else if (ignoredParticipant(_this->ignore, info->participant_key)) {
           TRACE_IGNORE("Participant is notified to add Subscription GID {%u,%u,%u} to the ignore lists\n",
                        info->key.systemId, info->key.localId, info->key.serial);
           ignore = TRUE;
       }
    }
    OSPL_UNLOCK(_this);
    if (ignore) {
    	v_participantIgnoreSubscription(_this, info->key);
    }

}

os_boolean
v_participantCheckPublicationIgnored(
    v_participant _this,
    const struct v_publicationInfo *info)
{
    os_boolean ignored = FALSE;

    OSPL_LOCK(_this);
    if (_this->ignore) {
        ignored = inList(_this->ignore->participants, info->participant_key);
        if (!ignored) {
            ignored = inList(_this->ignore->publications, info->key);
        }
    }
    OSPL_UNLOCK(_this);
    return ignored;
}
