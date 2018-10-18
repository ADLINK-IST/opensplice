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
#include "vortex_os.h"
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

#define VSERVICESTATE_NAME        "kernelModuleI::v_serviceState"
#define VSERVICESTATE_NAME_LENGTH 14

v_service
v_serviceNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *extStateName,
    v_serviceType serviceType,
    v_participantQos qos,
    c_bool enable)
{
    v_service s = NULL;
    v_participantQos q;

    assert(C_TYPECHECK(kernel, v_kernel));
    /* Do not C_TYPECHECK qos parameter, since it might be allocated on heap! */
    assert(name != NULL);

    if (v_participantQosCheck(qos) == V_RESULT_OK) {
        q = v_participantQosNew(kernel, (v_participantQos)qos);
        if (q == NULL) {
            OS_REPORT(OS_ERROR, "v_serviceNew", V_RESULT_INTERNAL_ERROR,
                "Creation of service <%s> failed. Cannot create participant QoS.",
                name);
        } else {
            s = v_service(v_objectNew(kernel, K_SERVICE));
            v_serviceInit(s, name, extStateName, serviceType, q, enable);
            c_free(q);
            if (s->state == NULL) {
                v_serviceFree(s);
                s = NULL;
            }
        }
    }

    return s;
}

void
v_serviceInit(
    v_service service,
    const c_char *name,
    const c_char *extStateName,
    v_serviceType serviceType,
    v_participantQos qos,
    c_bool enable)
{
    c_char *typeName;
    os_duration lp = 300*OS_DURATION_SECOND;
    v_kernel kernel;
    v_serviceManager manager;

    assert(service != NULL);
    assert(serviceType != V_SERVICETYPE_NONE);
    assert(C_TYPECHECK(service, v_service));
    assert(C_TYPECHECK(qos, v_participantQos));
    assert(name != NULL);

    kernel = v_objectKernel(service);
    manager = v_getServiceManager(kernel);

    /* v_participantInit writes the DCPSParticipant and CMParticipant topics, but
     * it downcasts to v_service to extract serviceType, and hence needs it available.
     */
    service->serviceType = serviceType;
    v_participantInit(v_participant(service), name, qos);
    if(enable) {
        (void)v_entityEnable(v_entity(service));
    }
    service->state = v_serviceManagerRegister(manager, service, extStateName);
    service->lease = v_leaseMonotonicNew(kernel, lp);
    service->newGroups = NULL;
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
            OS_REPORT(OS_FATAL, "v_service", result,
                "A fatal error was detected when trying to register the liveliness lease "
                "to the liveliness lease manager of the kernel. The result code was %d.", result);
        }
    } else
    {
        OS_REPORT(OS_FATAL, "v_service", V_RESULT_INTERNAL_ERROR,
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
                OS_REPORT(OS_FATAL, "v_service", result,
                    "A fatal error was detected when trying to register the spliced's liveliness lease "
                    "to the lease manager of participant %p (%s). The result code was %d.", (void*)service, name, result);
            }
            c_free(splicedParticipant);
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
            /* Splicedaemon may not observer itself! */
            if (strcmp(name, V_SPLICED_NAME) != 0) {
                v_serviceState splicedState;
                splicedState = v_serviceManagerGetServiceState(manager, V_SPLICED_NAME);
                (void)OSPL_ADD_OBSERVER(splicedState, service, V_EVENT_SERVICESTATE_CHANGED, NULL);
            }
        } else {
            OS_REPORT(OS_ERROR, "v_service",
                V_RESULT_ILL_PARAM, "Requested state type (%s) differs with existing state type (%s)",
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
    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    v_participantDeinit(v_participant(service));
}

void
v_serviceNotify(
    v_service service,
    v_event event,
    c_voidp userData)
{
    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));
    assert(event != NULL);

    if (event->kind == V_EVENT_NEW_GROUP) {
        if (event->data && service->newGroups) {
            ospl_c_insert(service->newGroups, event->data);
        }
    }
    v_participantNotify(v_participant(service), event, userData);
}

const c_char *
v_serviceGetName(
    v_service service)
{
    c_char *name = NULL;

    if ((service != NULL) && (service->state != NULL)) {
        assert(C_TYPECHECK(service, v_service));
        name = service->state->name;
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
                OS_REPORT(OS_INFO, "v_serviceChangeState", 0,
                    "++++++++++++++++++++++++++++++++++++++++++++++++" OS_REPORT_NL
                    "++ The service '%s' is now operational. " OS_REPORT_NL
                    "++++++++++++++++++++++++++++++++++++++++++++++++",
                    v_serviceGetName(service));
                break;
            case STATE_TERMINATED:
                OS_REPORT(OS_INFO, "v_serviceChangeState", 0,
                    "================================================" OS_REPORT_NL
                    "== The service '%s' has now terminated. "OS_REPORT_NL
                    "================================================",
                    v_serviceGetName(service));
                break;
            case STATE_DIED:
                OS_REPORT(OS_INFO, "v_serviceChangeState", 0,
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

v_serviceStateKind
v_serviceGetState(
    v_service service)
{
    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));
    assert(service->state != NULL);

    return v_serviceStateGetKind(service->state);
}

void
v_serviceFillNewGroups(
    v_service service)
{
    c_set newGroups;
    C_STRUCT(v_event) ge;
    v_group g;
    v_kernel kernel;
    c_iter groups = NULL;

    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    kernel = v_objectKernel(service);
    newGroups = (c_voidp)c_setNew(v_kernelType(kernel, K_GROUP));

    if (newGroups != NULL) {
        groups = v_groupSetSelectAll(kernel->groupSet);

        /* Take the first group and at the end notify the service about this new group.
         * But before push all other groups to the servive newGroup set so that only one trigger
         * is required to notify all groups.
         * The first group is automatically added to the newGroup set by the notification.
         * TODO : get rid of this mechanism.
         */
        ge.data = v_group(c_iterTakeFirst(groups));
        if (ge.data) {
            ge.kind = V_EVENT_NEW_GROUP;
            ge.source = v_observable(kernel);
            ospl_c_insert(newGroups, ge.data);
            while ((g = v_group(c_iterTakeFirst(groups))) != NULL) {
                ospl_c_insert(newGroups, g);
                c_free(g);
            }

            OSPL_BLOCK_EVENTS(service);
            c_free(service->newGroups);
            service->newGroups = (c_voidp)newGroups;
            OSPL_UNBLOCK_EVENTS(service);

            OSPL_TRIGGER_EVENT((service), &ge, NULL);
        }
        c_iterFree(groups);
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

    OSPL_BLOCK_EVENTS(service);
    newGroups = service->newGroups;
    if (newGroups != NULL) {
        group = v_group(c_take(newGroups));
        while (group != NULL) {
            c_iterInsert(result, group);
            group = v_group(c_take(newGroups));
        }
    }
    OSPL_UNBLOCK_EVENTS(service);

    return result;
}

void
v_serviceRenewLease(
    v_service service,
    os_duration leasePeriod)
{
    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    OSPL_LOCK(service);
    v_leaseRenew(service->lease, leasePeriod);
    OSPL_UNLOCK(service);
}
