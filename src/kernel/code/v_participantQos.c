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

#include "v__participantQos.h"
#include "v_kernel.h"
#include "v_qos.h"
#include "v_policy.h"

#include "os_report.h"

static const v_qosChangeMask immutableMask =
    V_POLICY_BIT_SCHEDULING;

/**************************************************************
 * private functions
 **************************************************************/
static c_bool
v_participantQosValidValues(
    v_participantQos qos)
{
    int valuesOk;

    /* no typechecking, since qos might be allocated on heap! */
    valuesOk = 1;
    if (qos != NULL) {
        valuesOk &= v_entityFactoryPolicyValid(qos->entityFactory);
        valuesOk &= v_userDataPolicyValid(qos->userData);
        valuesOk &= v_schedulingPolicyValid(qos->watchdogScheduling);
    }
    return (valuesOk?TRUE:FALSE);
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_participantQos
v_participantQosNew(
    v_kernel kernel,
    v_participantQos template)
{
    v_participantQos q;
    c_type type;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    if (v_participantQosValidValues(template)) {
        base = c_getBase(c_object(kernel));
        q = v_participantQos(v_qosCreate(kernel,V_PARTICIPANT_QOS));
        if (q != NULL) {
            if (template != NULL) {

                q->userData.size = template->userData.size;
                if (template->userData.size > 0) {
                    type = c_octet_t(base);
                    q->userData.value = c_arrayNew(type,template->userData.size);
                    c_free(type);
                    memcpy(q->userData.value,template->userData.value,template->userData.size);
                } else {
                    q->userData.value = NULL;
                }
                q->entityFactory = template->entityFactory;
                q->watchdogScheduling.kind = template->watchdogScheduling.kind;
                q->watchdogScheduling.priorityKind = template->watchdogScheduling.priorityKind;
                q->watchdogScheduling.priority = template->watchdogScheduling.priority;
            } else {
                q->userData.value                            = NULL;
                q->userData.size                             = 0;
                q->entityFactory.autoenable_created_entities = TRUE;
		q->watchdogScheduling.kind = V_SCHED_DEFAULT;
                q->watchdogScheduling.priorityKind = V_SCHED_PRIO_RELATIVE;
		q->watchdogScheduling.priority = 0;
;
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_participantQosNew", 0,
            "ParticipantQos not create: inconsistent qos");
        q = NULL;
    }

    return q;
}

void
v_participantQosFree(
    v_participantQos q)
{
    c_free(q);
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_result
v_participantQosSet(
    v_participantQos q,
    v_participantQos tmpl,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;
    c_type type;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL)) {
        /* no consistency checking needed */
        if (v_participantQosValidValues(tmpl)) {
            /* built change mask */
            if (!v_entityFactoryPolicyEqual(q->entityFactory, tmpl->entityFactory)) {
                cm |= V_POLICY_BIT_ENTITYFACTORY;
            }
            if (!v_userDataPolicyEqual(q->userData, tmpl->userData)) {
                cm |= V_POLICY_BIT_USERDATA;
            }
            if (!v_schedulingPolicyEqual(q->watchdogScheduling, tmpl->watchdogScheduling)) {
                cm |= V_POLICY_BIT_SCHEDULING;
            }
            /* check whether immutable policies are changed */
            if (cm & immutableMask) {
                result = V_RESULT_IMMUTABLE_POLICY;
            } else {
                /* set new policies */
                q->entityFactory = tmpl->entityFactory;
                if (cm & V_POLICY_BIT_USERDATA) {
                    c_free(q->userData.value);
                    q->userData.size = tmpl->userData.size;
                    if (tmpl->userData.size > 0) {
                        type = c_octet_t(c_getBase(c_object(q)));
                        q->userData.value = c_arrayNew(type,tmpl->userData.size);
                        memcpy(q->userData.value,tmpl->userData.value,tmpl->userData.size);                
                    } else {
                        q->userData.value = NULL;
                    }
                }
                result = V_RESULT_OK;
            }
        } else {
            result = V_RESULT_ILL_PARAM;
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }

    if (changeMask != NULL) {
        *changeMask = cm;
    }

    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/
