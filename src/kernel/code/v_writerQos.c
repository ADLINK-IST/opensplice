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

#include "v__writerQos.h"
#include "v_kernel.h"
#include "v_qos.h"
#include "v_policy.h"

#include "os_report.h"

static const v_qosChangeMask immutableMask =
    V_POLICY_BIT_DURABILITY  |
    V_POLICY_BIT_LIVELINESS  |
    V_POLICY_BIT_RELIABILITY |
    V_POLICY_BIT_ORDERBY     |
    V_POLICY_BIT_OWNERSHIP   |
    V_POLICY_BIT_HISTORY     |
    V_POLICY_BIT_RESOURCE;

static c_bool
v_writerQosValidValues(
    v_writerQos qos)
{
    c_ulong valuesNok = 0;

    /* no typechecking, since qos might be allocated on heap! */
    if (qos != NULL) {
        /* value checking */
        valuesNok |= (c_ulong) (!v_durabilityPolicyIValid(qos->durability)) << V_DURABILITYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_deadlinePolicyIValid(qos->deadline)) << V_DEADLINEPOLICY_ID;
        valuesNok |= (c_ulong) (!v_latencyPolicyIValid(qos->latency)) << V_LATENCYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_livelinessPolicyIValid(qos->liveliness)) << V_LIVELINESSPOLICY_ID;
        valuesNok |= (c_ulong) (!v_reliabilityPolicyIValid(qos->reliability)) << V_RELIABILITYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_orderbyPolicyIValid(qos->orderby)) << V_ORDERBYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_historyPolicyIValid(qos->history)) << V_HISTORYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_resourcePolicyIValid(qos->resource)) << V_RESOURCEPOLICY_ID;
        valuesNok |= (c_ulong) (!v_transportPolicyIValid(qos->transport)) << V_TRANSPORTPOLICY_ID;
        valuesNok |= (c_ulong) (!v_lifespanPolicyIValid(qos->lifespan)) << V_LIFESPANPOLICY_ID;
        valuesNok |= (c_ulong) (!v_ownershipPolicyIValid(qos->ownership)) << V_OWNERSHIPPOLICY_ID;
        valuesNok |= (c_ulong) (!v_strengthPolicyIValid(qos->strength)) << V_STRENGTHPOLICY_ID;
        valuesNok |= (c_ulong) (!v_writerLifecyclePolicyIValid(qos->lifecycle)) << V_WRITERLIFECYCLEPOLICY_ID;
        valuesNok |= (c_ulong) (!v_userDataPolicyIValid(qos->userData)) << V_USERDATAPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
}

static c_bool
v_writerQosConsistent(
    v_writerQos qos)
{
    c_bool result;

    result = TRUE;
    if (qos != NULL) {
        if ((qos->resource.v.max_samples_per_instance != V_LENGTH_UNLIMITED) &&
            (qos->history.v.kind != V_HISTORY_KEEPALL) &&
            (qos->history.v.depth > qos->resource.v.max_samples_per_instance)) {
            result = FALSE;
            OS_REPORT(OS_ERROR, "v_writerQosConsistent", V_RESULT_ILL_PARAM,
                "History depth (%d) may not exceed max_samples_per_instance resource limit (%d)",
                qos->history.v.depth, qos->resource.v.max_samples_per_instance);
        }
    }

    return result;
}

v_writerQos
v_writerQosNew(
    v_kernel kernel,
    v_writerQos template)
{
    v_writerQos q;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));
    q = v_writerQos(v_qosCreate(base,V_WRITER_QOS));

    if (q != NULL) {
        if (template != NULL) {
            *q = *template;
            q->userData.v.size = template->userData.v.size;
            if (template->userData.v.size > 0) {
                q->userData.v.value = c_arrayNew(c_octet_t(base),(c_ulong)template->userData.v.size);
                memcpy(q->userData.v.value,template->userData.v.value,(c_ulong)template->userData.v.size);
            } else {
                q->userData.v.value = NULL;
            }
        } else {
            q->durability.v.kind                              = V_DURABILITY_VOLATILE;
            q->deadline.v.period                              = OS_DURATION_INFINITE;
            q->latency.v.duration                             = OS_DURATION_ZERO;
            q->liveliness.v.kind                              = V_LIVELINESS_AUTOMATIC;
            q->liveliness.v.lease_duration                    = OS_DURATION_ZERO;
            q->reliability.v.kind                             = V_RELIABILITY_BESTEFFORT;
            q->reliability.v.max_blocking_time                = OS_DURATION_ZERO;
            q->reliability.v.synchronous                      = FALSE;
            q->orderby.v.kind                                 = V_ORDERBY_RECEPTIONTIME;
            q->history.v.kind                                 = V_HISTORY_KEEPLAST;
            q->history.v.depth                                = 1;
            q->resource.v.max_samples                         = V_LENGTH_UNLIMITED;
            q->resource.v.max_instances                       = V_LENGTH_UNLIMITED;
            q->resource.v.max_samples_per_instance            = V_LENGTH_UNLIMITED;
            q->userData.v.size                                = 0;
            q->userData.v.value                               = NULL;
            q->ownership.v.kind                               = V_OWNERSHIP_SHARED;
            q->strength.v.value                               = 0;
            q->lifecycle.v.autodispose_unregistered_instances = TRUE;
            q->lifecycle.v.autopurge_suspended_samples_delay  = OS_DURATION_INFINITE;
            q->lifecycle.v.autounregister_instance_delay      = OS_DURATION_INFINITE;
            q->lifespan.v.duration                            = OS_DURATION_INFINITE;
            q->transport.v.value                              = 0;
        }
    }

    return q;
}

void
v_writerQosFree(
    v_writerQos q)
{
    c_free(q);
}

v_result
v_writerQosCompare(
    v_writerQos q,
    v_writerQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL) && (changeMask != NULL)) {
        /* built change mask */
                if (!v_durabilityPolicyIEqual(q->durability, tmpl->durability)) {
            cm |= V_POLICY_BIT_DURABILITY;
        }
                if (!v_deadlinePolicyIEqual(q->deadline, tmpl->deadline)) {
            cm |= V_POLICY_BIT_DEADLINE;
        }
                if (!v_latencyPolicyIEqual(q->latency, tmpl->latency)) {
            cm |= V_POLICY_BIT_LATENCY;
        }
                if (!v_livelinessPolicyIEqual(q->liveliness, tmpl->liveliness)) {
            cm |= V_POLICY_BIT_LIVELINESS;
        }
                if (!v_reliabilityPolicyIEqual(q->reliability, tmpl->reliability)) {
            cm |= V_POLICY_BIT_RELIABILITY;
        }
                if (!v_orderbyPolicyIEqual(q->orderby, tmpl->orderby)) {
            cm |= V_POLICY_BIT_ORDERBY;
        }
                if (!v_historyPolicyIEqual(q->history, tmpl->history)) {
            cm |= V_POLICY_BIT_HISTORY;
        }
                if (!v_resourcePolicyIEqual(q->resource, tmpl->resource)) {
            cm |= V_POLICY_BIT_RESOURCE;
        }
                if (!v_transportPolicyIEqual(q->transport, tmpl->transport)) {
            cm |= V_POLICY_BIT_TRANSPORT;
        }
                if (!v_lifespanPolicyIEqual(q->lifespan, tmpl->lifespan)) {
            cm |= V_POLICY_BIT_LIFESPAN;
        }
                if (!v_userDataPolicyIEqual(q->userData, tmpl->userData)) {
            cm |= V_POLICY_BIT_USERDATA;
        }
                if (!v_ownershipPolicyIEqual(q->ownership, tmpl->ownership)) {
            cm |= V_POLICY_BIT_OWNERSHIP;
        }
                if (!v_strengthPolicyIEqual(q->strength, tmpl->strength)) {
            cm |= V_POLICY_BIT_STRENGTH;
        }
                if (!v_writerLifecyclePolicyIEqual(q->lifecycle, tmpl->lifecycle)) {
            cm |= V_POLICY_BIT_WRITERLIFECYCLE;
        }

        /* check whether immutable policies are changed */
        if (((cm & immutableMask) != 0) && (enabled)) {
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

v_result
v_writerQosCheck(
    v_writerQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (v_writerQosValidValues(_this)) {
            if (!v_writerQosConsistent(_this)) {
                result = V_RESULT_INCONSISTENT_QOS;
                OS_REPORT(OS_ERROR, "v_writerQosCheck", result,
                    "WriterQoS is inconsistent.");
            }
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_writerQosCheck", result,
                "WriterQoS is invalid.");
        }
    }

    return result;
}
