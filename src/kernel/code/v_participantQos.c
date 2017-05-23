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
    c_ulong valuesNok;

    /* no typechecking, since qos might be allocated on heap! */
    valuesNok = 0;
    if (qos != NULL) {
        valuesNok |= (c_ulong) (!v_entityFactoryPolicyIValid(qos->entityFactory)) << V_ENTITYFACTORYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_userDataPolicyIValid(qos->userData)) << V_USERDATAPOLICY_ID;
        valuesNok |= (c_ulong) (!v_schedulingPolicyIValid(qos->watchdogScheduling)) << V_SCHEDULINGPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
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
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));
    q = v_participantQos(v_qosCreate(base,V_PARTICIPANT_QOS));
    if (q != NULL) {
        if (template != NULL) {
            q->userData.v.size = template->userData.v.size;
            if (template->userData.v.size > 0) {
                q->userData.v.value = c_arrayNew_s(c_octet_t(base),(c_ulong)template->userData.v.size);
                if (q->userData.v.value) {
                    memcpy(q->userData.v.value,template->userData.v.value,(c_ulong)template->userData.v.size);
                } else {
                    OS_REPORT(OS_ERROR, "v_participantQosNew", V_RESULT_OUT_OF_MEMORY,
                              "Failed to allocate user_data policy of participant QoS.");
                    c_free(q);
                    return NULL;
                }
            } else {
                q->userData.v.value = NULL;
            }
            q->entityFactory = template->entityFactory;
            q->watchdogScheduling = template->watchdogScheduling;
        } else {
            q->userData.v.value                            = NULL;
            q->userData.v.size                             = 0;
            q->entityFactory.v.autoenable_created_entities = TRUE;
            q->watchdogScheduling.v.kind = V_SCHED_DEFAULT;
            q->watchdogScheduling.v.priorityKind = V_SCHED_PRIO_RELATIVE;
            q->watchdogScheduling.v.priority = 0;
        }
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
v_participantQosCompare(
    v_participantQos q,
    v_participantQos tmpl,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL) && (changeMask != NULL)) {
        /* built change mask */
            if (!v_entityFactoryPolicyIEqual(q->entityFactory, tmpl->entityFactory)) {
            cm |= V_POLICY_BIT_ENTITYFACTORY;
        }
            if (!v_userDataPolicyIEqual(q->userData, tmpl->userData)) {
            cm |= V_POLICY_BIT_USERDATA;
        }
            if (!v_schedulingPolicyIEqual(q->watchdogScheduling, tmpl->watchdogScheduling)) {
            cm |= V_POLICY_BIT_SCHEDULING;
        }
        /* check whether immutable policies are changed */
        if (cm & immutableMask) {
            v_policyReportImmutable(cm, immutableMask);
            result = V_RESULT_IMMUTABLE_POLICY;
        } else {
            *changeMask = cm;
            result = V_RESULT_OK;
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/
v_result
v_participantQosCheck(
    v_participantQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (!v_participantQosValidValues(_this)) {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_participantQosCheck", result,
                "ParticipantQoS is invalid.");
        }
    }

    return result;
}
