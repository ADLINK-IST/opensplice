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
#include "os.h"
#include "os_report.h"

#include "v__serviceManager.h"
#include "v__service.h"
#include "v__serviceState.h"
#include "v__lease.h"
#include "v_participant.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_entity.h"
#include "v_event.h"
#include "v_groupSet.h"
#include "v_group.h"
#include "v_event.h"
#include "v_kernel.h"

#include "v__leaseManager.h"
#include "v_public.h"

#define VSERVICESTATE_NAME        "kernelModule::v_serviceState"
#define VSERVICESTATE_NAME_LENGTH 14

/**************************************************************
 * private functions
 **************************************************************/
static void
v_serviceWatchSplicedaemon(
    v_service service)
{
    v_kernel k;
    v_serviceManager m;
    v_serviceState splicedState;

    k = v_objectKernel(service);
    m = v_getServiceManager(k);
    splicedState = v_serviceManagerGetServiceState(m, V_SPLICED_NAME);
    v_observableAddObserver(v_observable(splicedState), v_observer(service), NULL);
}


static void
addAllGroups(
    c_set newGroups,
    v_groupSet groupSet)
{
    c_iter groups = NULL;
    v_group g;

    assert(C_TYPECHECK(groupSet, v_groupSet));

    groups = v_groupSetSelectAll(groupSet);
    g = v_group(c_iterTakeFirst(groups));
    while (g != NULL) {
        c_insert(newGroups, g);
        c_free(g);
        g = v_group(c_iterTakeFirst(groups));
    }
    c_iterFree(groups);
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_service
v_serviceNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    v_statistics stats)
{
    v_kernel k;
    v_service s;
    v_participantQos q;

    assert(C_TYPECHECK(manager, v_serviceManager));
    /* Do not C_TYPECHECK qos parameter, since it might be allocated on heap! */
    assert(name != NULL);

    k = v_objectKernel(manager);
    /* do no use cast method on qos parameter,
     * it is most likely allocated on heap! */
    q = v_participantQosNew(k, (v_participantQos)qos);
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_serviceNew", 0,
                  "Service not created: inconsistent qos");
        s = NULL;
    } else {
        s = v_service(v_objectNew(k, K_SERVICE));
        v_serviceInit(s, manager, name, extStateName, q, stats);
        c_free(q);
        /* always add, even when s->state==NULL, since v_participantFree always
           removes the participant.*/
        v_addParticipant(k, v_participant(s));
        if (s->state == NULL) {
            v_serviceFree(s);
            s = NULL;
        }
    }

    return s;
}

void
v_serviceInit(
    v_service service,
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    v_statistics stats)
{
    c_char *typeName;
    v_duration lp = {300, 0};
    v_kernel kernel;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));
    assert(C_TYPECHECK(qos, v_participantQos));
    assert(name != NULL);

    kernel = v_objectKernel(service);
    v_participantInit(v_participant(service), name, qos, stats, TRUE);
    service->state = v_serviceManagerRegister(manager, service, extStateName);
    service->lease = v_leaseNew(kernel, lp);
    if(service->lease)
    {
        v_result result;

        result = v_leaseManagerRegister(
            kernel->livelinessLM,
            service->lease,
            V_LEASEACTION_SERVICESTATE_EXPIRED,
            v_public(service->state),
            FALSE/*do not repeat */);
        if(result != V_RESULT_OK)
        {
            c_free(service->lease);
            service->lease = NULL;
            OS_REPORT_1(OS_ERROR, "v_service", 0,
                "A fatal error was detected when trying to register the liveliness lease "
                "to the liveliness lease manager of the kernel. The result code was %d.", result);
        }
    } else
    {
        OS_REPORT(OS_ERROR, "v_service", 0,
            "Unable to create a liveliness lease! Most likely not enough shared "
            "memory available to complete the operation.");
    }
    if(service->lease)/* aka everything is ok so far */
    {
        v_result result;
        c_iter participants;
        v_participant splicedParticipant;


        participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
        assert(c_iterLength(participants) == 1 || 0 == strcmp(name, V_SPLICED_NAME));
        splicedParticipant = v_participant(c_iterTakeFirst(participants));
        if(splicedParticipant)
        {
            result = v_leaseManagerRegister(
                v_participant(service)->leaseManager,
                v_service(splicedParticipant)->lease,
                V_LEASEACTION_SERVICESTATE_EXPIRED,
                v_public(v_service(splicedParticipant)->state),
                FALSE /* only observing, do not repeat */);
            if(result != V_RESULT_OK)
            {
                c_free(service->lease);
                service->lease = NULL;
                OS_REPORT_3(OS_ERROR, "v_service", 0,
                    "A fatal error was detected when trying to register the spliced's liveliness lease "
                    "to the lease manager of participant %p (%s). The result code was %d.", service, name, result);
            }
        }
        c_iterFree(participants);
    }

    if (service->state != NULL) {
      /* check if state has correct type */
        typeName = c_metaScopedName(c_metaObject(c_getType(c_object(service->state))));
        if (extStateName == NULL) {
            extStateName = VSERVICESTATE_NAME;
        }
        if (strcmp(typeName, extStateName) == 0) {
            if (strcmp(name, V_SPLICED_NAME) != 0) {
                /* Splicedaemon may not observer itself! */
                v_serviceWatchSplicedaemon(service);
            }
        } else {
            OS_REPORT_2(OS_ERROR, "v_service",
                0, "Requested state type (%s) differs with existing state type (%s)",
                extStateName, typeName);
            c_free(service->state);
            service->state = NULL;
        }
        os_free(typeName);
    }
}


void
v_serviceFree(
    v_service service)
{
    assert(C_TYPECHECK(service, v_service));

    v_participantFree(v_participant(service));
}

void
v_serviceDeinit(
    v_service service)
{
    v_kernel kernel;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    c_free(service->state);
    service->state = NULL;
    c_free((c_object)v_observer(service)->eventData);
    kernel = v_objectKernel(service);
    v_leaseManagerDeregister(kernel->livelinessLM, service->lease);
    c_free(service->lease);
    service->lease = NULL;
    v_participantDeinit(v_participant(service));
}

/**************************************************************
 * Protected functions
 **************************************************************/
void
v_serviceNotify(
    v_service service,
    v_event event,
    c_voidp userData)
{
    v_group group;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    if (event != NULL) {
        switch (event->kind) {
        case V_EVENT_SERVICESTATE_CHANGED:
        break;
        case V_EVENT_NEW_GROUP:
            group = v_group(event->userData);

            if ((group != NULL) && (v_observer(service)->eventData != NULL)) {
                /* Update new group admin */
                c_insert((c_set)v_observer(service)->eventData, group);
            }
            /* This allows receiving the event by means of a waitset.*/
            v_observableNotify(v_observable(service), event);
        break;
        case V_EVENT_HISTORY_DELETE:
            /* This allows receiving the event by means of a waitset.*/
            v_observableNotify(v_observable(service), event);
        break;
        case V_EVENT_HISTORY_REQUEST:
            /* This allows receiving the event by means of a waitset.*/
            v_observableNotify(v_observable(service), event);
        break;
        case V_EVENT_CONNECT_WRITER:
        case V_EVENT_PERSISTENT_SNAPSHOT:
            /* This allows receiving the event by means of a waitset.*/
            v_observableNotify(v_observable(service), event);
        break;
        default:
        break;
        }
    }
    v_participantNotify(v_participant(service), event, userData);
}

/**************************************************************
 * Public functions
 **************************************************************/
const c_char *
v_serviceGetName(
    v_service service)
{
    c_char *name = NULL;

    if ((service != NULL) && (service->state != NULL)) {
        assert(C_TYPECHECK(service, v_service));
        name = v_entity(service->state)->name;
    }
    return (const c_char *)name;
}

c_bool
v_serviceChangeState(
    v_service service,
    v_serviceStateKind newState)
{
    c_bool result;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));
    assert(service->state != NULL);
    assert(C_TYPECHECK(service->state, v_serviceState));

    result = v_serviceStateChangeState(service->state, newState);
    if(result)
    {
        switch(newState)
        {
            case STATE_OPERATIONAL:
                OS_REPORT_1(OS_INFO, "v_serviceChangeState", 0,
                    "++++++++++++++++++++++++++++++++++++++++++++++++" OS_REPORT_NL
                    "++ The service '%s' is now operational. " OS_REPORT_NL
                    "++++++++++++++++++++++++++++++++++++++++++++++++",
                    v_serviceGetName(service));
                break;
            case STATE_TERMINATED:
                OS_REPORT_1(OS_INFO, "v_serviceChangeState", 0,
                    "================================================" OS_REPORT_NL
                    "== The service '%s' has now terminated. "OS_REPORT_NL
                    "================================================",
                    v_serviceGetName(service));
                break;
            case STATE_DIED:
                OS_REPORT_1(OS_INFO, "v_serviceChangeState", 0,
                    "================================================" OS_REPORT_NL
                    "== The service '%s' has died. "OS_REPORT_NL
                    "================================================",
                    v_serviceGetName(service));
                break;
            case STATE_NONE:
            case STATE_INITIALISING:
            case STATE_TERMINATING:
            case STATE_INCOMPATIBLE_CONFIGURATION:
            default:
                /* ignore */
                break;
        }
    }
    return result;
}

void
v_serviceFillNewGroups(
    v_service service)
{
    c_set newGroups;
    C_STRUCT(v_event) ge;
    v_group g, oldGroup;
    c_iter oldGroups;
    v_kernel kernel;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    kernel = v_objectKernel(service);
    newGroups = (c_voidp)c_setNew(v_kernelType(kernel, K_GROUP));

    if (newGroups != NULL) {
        addAllGroups(newGroups, kernel->groupSet);
        v_observerLock(v_observer(service));
        g = v_group(c_read(newGroups)); /* need a group for the event */

        if(v_observer(service)->eventData != NULL){
            oldGroups = ospl_c_select((c_set)v_observer(service)->eventData, 0);
            oldGroup = v_group(c_iterTakeFirst(oldGroups));

            while(oldGroup){
                newGroups = c_setInsert(newGroups, oldGroup);
                c_free(oldGroup);
                oldGroup = v_group(c_iterTakeFirst(oldGroups));
            }
            c_iterFree(oldGroups);
        }
        /* just for safety, when assertion are compiled out, free the prev set */
        c_free((c_object)v_observer(service)->eventData);
        v_observer(service)->eventData = (c_voidp)newGroups;

        ge.kind = V_EVENT_NEW_GROUP;
        ge.source = v_publicHandle(v_public(kernel));
        ge.userData = g;
        v_observerNotify(v_observer(service), &ge, NULL);
        v_observerUnlock(v_observer(service));
        c_free(g);
    }
}

c_iter
v_serviceTakeNewGroups(
    v_service service)
{
    c_iter result;
    v_group group;
    c_set newGroups;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    result = c_iterNew(NULL);

    v_observerLock(v_observer(service));
    newGroups = (c_set)v_observer(service)->eventData;
    if (newGroups != NULL) {
        group = v_group(c_take(newGroups));
        while (group != NULL) {
            c_iterInsert(result, group);
            group = v_group(c_take(newGroups));
        }
    }
    v_observerUnlock(v_observer(service));

    return result;
}

void
v_serviceRenewLease(
    v_service service,
    v_duration leasePeriod)
{
    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    v_observerLock(v_observer(service));
    v_leaseRenew(service->lease, &leasePeriod);
    v_observerUnlock(v_observer(service));
}
