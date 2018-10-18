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

#include "v__topicQos.h"
#include "v_kernel.h"
#include "v_qos.h"
#include "os_report.h"
#include "v_policy.h"

static const v_qosChangeMask immutableMask =
    V_POLICY_BIT_DURABILITY  |
    V_POLICY_BIT_LIVELINESS  |
    V_POLICY_BIT_RELIABILITY |
    V_POLICY_BIT_ORDERBY     |
    V_POLICY_BIT_HISTORY     |
    V_POLICY_BIT_RESOURCE    |
    V_POLICY_BIT_OWNERSHIP   |
    V_POLICY_BIT_DURABILITYSERVICE;

static const os_duration MAX_BLOCKING_TIME = OS_DURATION_INIT(0, 100000000);

static c_bool
v_topicQosValidValues(
    v_topicQos qos)
{
    c_ulong valuesNok;

    valuesNok = 0;
    /* no typechecking, since qos might be allocated on heap! */
    if (qos != NULL) {
        /* value checking */
        valuesNok |= (c_ulong) (!v_durabilityPolicyIValid(qos->durability)) << V_DURABILITYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_durabilityServicePolicyIValid(qos->durabilityService)) << V_DURABILITYSERVICEPOLICY_ID;
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
        valuesNok |= (c_ulong) (!v_topicDataPolicyIValid(qos->topicData)) << V_TOPICDATAPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
}

static c_bool
v_topicQosConsistent(
    v_topicQos qos)
{
    c_bool result;

    /* no typechecking, since qos might be allocated on heap! */
    result = TRUE;
    if (qos != NULL) {
        if ((qos->resource.v.max_samples_per_instance != V_LENGTH_UNLIMITED) &&
            (qos->history.v.kind != V_HISTORY_KEEPALL) &&
            (qos->history.v.depth > qos->resource.v.max_samples_per_instance)) {
            result = FALSE;
            OS_REPORT(OS_ERROR, "v_topicQosConsistent", V_RESULT_ILL_PARAM,
                "History depth (%d) may not exceed max_samples_per_instance resource limit (%d)",
                qos->history.v.depth, qos->resource.v.max_samples_per_instance);
        }
    }

    return result;
}

v_topicQos
v_topicQosNew(
    v_kernel kernel,
    v_topicQos template)
{
    v_topicQos q = NULL;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    q = v_topicQos(v_qosCreate(c_getBase(kernel),V_TOPIC_QOS));

    if (q != NULL) {
        if (template != NULL) {

            q->durability         = template->durability;
            q->durabilityService  = template->durabilityService;
            q->deadline           = template->deadline;
            q->latency            = template->latency;
            q->liveliness         = template->liveliness;
            q->reliability        = template->reliability;
            q->orderby            = template->orderby;
            q->history            = template->history;
            q->resource           = template->resource;
            q->ownership          = template->ownership;
            q->transport          = template->transport;
            q->lifespan           = template->lifespan;

            q->topicData.v.size = template->topicData.v.size;
            if (template->topicData.v.size > 0) {
                q->topicData.v.value = c_arrayNew_s(c_octet_t(c_getBase(c_object(q))),(c_ulong)template->topicData.v.size);
                if (q->topicData.v.value) {
                    memcpy(q->topicData.v.value,template->topicData.v.value,(c_ulong)template->topicData.v.size);
                } else {
                    OS_REPORT(OS_ERROR, "v_topicQosNew", V_RESULT_OUT_OF_MEMORY,
                              "Failed to allocate topic_data policy of topic QoS.");
                    c_free(q);
                    return NULL;
                }
            } else {
                q->topicData.v.value = NULL;
            }
        } else {
            q->topicData.v.size                             = 0;
            q->topicData.v.value                            = NULL;
            q->durability.v.kind                            = V_DURABILITY_VOLATILE;
            q->durabilityService.v.history_kind             = V_HISTORY_KEEPLAST;
            q->durabilityService.v.history_depth            = 1;
            q->durabilityService.v.max_samples              = V_LENGTH_UNLIMITED;
            q->durabilityService.v.max_instances            = V_LENGTH_UNLIMITED;
            q->durabilityService.v.max_samples_per_instance = V_LENGTH_UNLIMITED;
            q->durabilityService.v.service_cleanup_delay    = OS_DURATION_ZERO;
            q->deadline.v.period                            = OS_DURATION_INFINITE;
            q->latency.v.duration                           = OS_DURATION_ZERO;
            q->liveliness.v.kind                            = V_LIVELINESS_AUTOMATIC;
            q->liveliness.v.lease_duration                  = OS_DURATION_INFINITE;
            q->reliability.v.kind                           = V_RELIABILITY_BESTEFFORT;
            q->reliability.v.max_blocking_time              = MAX_BLOCKING_TIME;
            q->reliability.v.synchronous                    = FALSE;
            q->orderby.v.kind                               = V_ORDERBY_RECEPTIONTIME;
            q->history.v.kind                               = V_HISTORY_KEEPLAST;
            q->history.v.depth                              = 1;
            q->resource.v.max_samples                       = V_LENGTH_UNLIMITED;
            q->resource.v.max_instances                     = V_LENGTH_UNLIMITED;
            q->resource.v.max_samples_per_instance          = V_LENGTH_UNLIMITED;
            q->transport.v.value                            = 0;
            q->lifespan.v.duration                          = OS_DURATION_INFINITE;
            q->ownership.v.kind                             = V_OWNERSHIP_SHARED;
        }
    }

    return q;
}

void
v_topicQosFree(
    v_topicQos q)
{
    c_free(q);
}

v_result
v_topicQosCompare(
    v_topicQos q,
    v_topicQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL) && (changeMask != NULL)) {
                /* built change mask */
                if (!v_topicDataPolicyIEqual(q->topicData, tmpl->topicData)) {
            cm |= V_POLICY_BIT_TOPICDATA;
        }
                if (!v_durabilityPolicyIEqual(q->durability, tmpl->durability)) {
            cm |= V_POLICY_BIT_DURABILITY;
        }
                if (!v_durabilityServicePolicyIEqual(q->durabilityService, tmpl->durabilityService)) {
            cm |= V_POLICY_BIT_DURABILITYSERVICE;
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
                if (!v_ownershipPolicyIEqual(q->ownership, tmpl->ownership)) {
            cm |= V_POLICY_BIT_OWNERSHIP;
        }

        /* check whether immutable policies are changed */
        if (((cm & immutableMask) != 0) && enabled) {
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
v_topicQosCheck(
    v_topicQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (v_topicQosValidValues(_this)) {
            if (!v_topicQosConsistent(_this)) {
                result = V_RESULT_INCONSISTENT_QOS;
                OS_REPORT(OS_ERROR, "v_topicQosCheck", result,
                    "TopicQoS is inconsistent.");
            }
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_topicQosCheck", result,
                "TopicQoS is invalid.");
        }
    }

    return result;
}

v_topicQos
v_topicQosFromTopicInfo(
    c_base base,
    const struct v_topicInfo *info)
{
    v_topicQos qos;
    v_result result;
    if ((qos = v_topicQos(v_qosCreate (base, V_TOPIC_QOS))) == NULL) {
        return NULL;
    }
    v_policyConvToInt_durability (&qos->durability, &info->durability);
    v_policyConvToInt_durability_service (&qos->durabilityService, &info->durabilityService);
    v_policyConvToInt_deadline (&qos->deadline, &info->deadline);
    v_policyConvToInt_latency_budget (&qos->latency, &info->latency_budget);
    v_policyConvToInt_liveliness (&qos->liveliness, &info->liveliness);
    v_policyConvToInt_reliability (&qos->reliability, &info->reliability);
    v_policyConvToInt_transport_priority (&qos->transport, &info->transport_priority);
    v_policyConvToInt_lifespan (&qos->lifespan, &info->lifespan);
    v_policyConvToInt_destination_order (&qos->orderby, &info->destination_order);
    v_policyConvToInt_history (&qos->history, &info->history);
    v_policyConvToInt_resource_limits (&qos->resource, &info->resource_limits);
    v_policyConvToInt_ownership (&qos->ownership, &info->ownership);
    if ((result = v_policyConvToInt_topic_data (base, &qos->topicData, &info->topic_data)) != V_RESULT_OK) {
        c_free (qos);
        return NULL;
    }
    return qos; /* transfer refcount to caller */
}

v_result
v_topicQosFillTopicInfo (
    struct v_topicInfo *info,
    const struct v_topicQos_s *qos)
{
    c_base base = c_getBase (c_object (qos));
    v_result result;
    v_policyConvToExt_durability (&info->durability, &qos->durability);
    v_policyConvToExt_durability_service (&info->durabilityService, &qos->durabilityService);
    v_policyConvToExt_deadline (&info->deadline, &qos->deadline);
    v_policyConvToExt_latency_budget (&info->latency_budget, &qos->latency);
    v_policyConvToExt_liveliness (&info->liveliness, &qos->liveliness);
    v_policyConvToExt_reliability (&info->reliability, &qos->reliability);
    v_policyConvToExt_transport_priority (&info->transport_priority, &qos->transport);
    v_policyConvToExt_lifespan (&info->lifespan, &qos->lifespan);
    v_policyConvToExt_destination_order (&info->destination_order, &qos->orderby);
    v_policyConvToExt_history (&info->history, &qos->history);
    v_policyConvToExt_resource_limits (&info->resource_limits, &qos->resource);
    v_policyConvToExt_ownership (&info->ownership, &qos->ownership);
    if ((result = v_policyConvToExt_topic_data (base, &info->topic_data, &qos->topicData)) != V_RESULT_OK) {
        return result;
    }
    return V_RESULT_OK;
}
