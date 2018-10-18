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


#include "v__readerQos.h"
#include "v__subscriberQos.h"
#include "v_qos.h"
#include "v_kernel.h"
#include "v_policy.h"

#include "os_report.h"
#include "os_abstract.h"

static const v_qosChangeMask immutableMask =
    V_POLICY_BIT_DURABILITY  |
    V_POLICY_BIT_LIVELINESS  |
    V_POLICY_BIT_RELIABILITY |
    V_POLICY_BIT_ORDERBY     |
    V_POLICY_BIT_HISTORY     |
    V_POLICY_BIT_USERKEY     |
    V_POLICY_BIT_SHARE       |
    V_POLICY_BIT_OWNERSHIP   |
    V_POLICY_BIT_RESOURCE;

static c_bool
v_readerQosValidValues(
    v_readerQos qos)
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
        valuesNok |= (c_ulong) (!v_ownershipPolicyIValid(qos->ownership)) << V_OWNERSHIPPOLICY_ID;
        valuesNok |= (c_ulong) (!v_pacingPolicyIValid(qos->pacing)) << V_PACINGPOLICY_ID;
        valuesNok |= (c_ulong) (!v_readerLifecyclePolicyIValid(qos->lifecycle)) << V_READERLIFECYCLEPOLICY_ID;
        valuesNok |= (c_ulong) (!v_readerLifespanPolicyIValid(qos->lifespan)) << V_READERLIFESPANPOLICY_ID;
        valuesNok |= (c_ulong) (!v_userDataPolicyIValid(qos->userData)) << V_USERDATAPOLICY_ID;
        valuesNok |= (c_ulong) (!v_userKeyPolicyIValid(qos->userKey)) << V_USERKEYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_sharePolicyIValid(qos->share)) << V_SHAREPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
}

static c_bool
v_readerQosConsistent(
    v_readerQos qos)
{
    c_bool result;
    os_compare cmp;

    result = TRUE;
    if (qos != NULL) {
        cmp = os_durationCompare(qos->deadline.v.period, qos->pacing.v.minSeperation);
        if (cmp == OS_LESS) {
            result = FALSE;
            OS_REPORT(OS_ERROR, "v_readerQosConsistent", V_RESULT_INCONSISTENT_QOS,
                "Time-based filter period (%"PA_PRIduration"s) may not exceed deadline period (%"PA_PRIduration"s)",
                OS_DURATION_PRINT(qos->pacing.v.minSeperation),
                OS_DURATION_PRINT(qos->deadline.v.period));
        }
        if ((qos->resource.v.max_samples_per_instance != V_LENGTH_UNLIMITED) &&
            (qos->history.v.kind != V_HISTORY_KEEPALL) &&
            (qos->history.v.depth > qos->resource.v.max_samples_per_instance)) {
            result = FALSE;
            OS_REPORT(OS_ERROR, "v_readerQosConsistent", V_RESULT_INCONSISTENT_QOS,
                        "History depth (%d) may not exceed max_samples_per_instance resource limit (%d)",
                        qos->history.v.depth, qos->resource.v.max_samples_per_instance);
        }
    }

    return result;
}

v_readerQos
v_readerQosNew(
    v_kernel kernel,
    v_readerQos template)
{
    v_readerQos q;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));
    q = v_readerQos(v_qosCreate(base,V_READER_QOS));

    if (q != NULL) {
        if (template != NULL) {
            *q = *template;
            q->userData.v.size = template->userData.v.size;
            if (template->userData.v.size > 0) {
                q->userData.v.value = c_arrayNew_s(c_octet_t(base),(c_ulong)template->userData.v.size);
                if (q->userData.v.value) {
                    memcpy(q->userData.v.value,template->userData.v.value,(c_ulong)template->userData.v.size);
                } else {
                    OS_REPORT(OS_ERROR, "v_readerQosNew", V_RESULT_OUT_OF_MEMORY,
                              "Failed to allocate user_data policy of datareader QoS.");
                    c_free(q);
                    return NULL;
                }
            } else {
                q->userData.v.value = NULL;
            }
            if (q->share.v.enable) {
                q->share.v.name = c_stringNew_s(base,template->share.v.name);
                if (!q->share.v.name) {
                    OS_REPORT(OS_ERROR, "v_readerQosNew", V_RESULT_OUT_OF_MEMORY,
                              "Failed to allocate share policy of datareader QoS.");
                    c_free(q);
                    return NULL;
                }
            } else {
                q->share.v.name = NULL;
            }
            if (q->userKey.v.enable) {
                q->userKey.v.expression = c_stringNew_s(base,template->userKey.v.expression);
                if (!q->userKey.v.expression) {
                    OS_REPORT(OS_ERROR, "v_readerQosNew", V_RESULT_OUT_OF_MEMORY,
                              "Failed to allocate user_key policy of datareader QoS.");
                    c_free(q);
                    return NULL;
                }
            } else {
                q->userKey.v.expression = NULL;
            }
        } else {
            q->durability.v.kind                            = V_DURABILITY_VOLATILE;
            q->deadline.v.period                            = OS_DURATION_INFINITE;
            q->latency.v.duration                           = OS_DURATION_ZERO;
            q->liveliness.v.kind                            = V_LIVELINESS_AUTOMATIC;
            q->liveliness.v.lease_duration                  = OS_DURATION_ZERO;
            q->reliability.v.kind                           = V_RELIABILITY_BESTEFFORT;
            q->reliability.v.max_blocking_time              = OS_DURATION_ZERO;
            q->reliability.v.synchronous                    = FALSE;
            q->orderby.v.kind                               = V_ORDERBY_RECEPTIONTIME;
            q->history.v.kind                               = V_HISTORY_KEEPLAST;
            q->history.v.depth                              = 1;
            q->resource.v.max_samples                       = V_LENGTH_UNLIMITED;
            q->resource.v.max_instances                     = V_LENGTH_UNLIMITED;
            q->resource.v.max_samples_per_instance          = V_LENGTH_UNLIMITED;
            q->userData.v.size                              = 0;
            q->userData.v.value                             = NULL;
            q->ownership.v.kind                             = V_OWNERSHIP_SHARED;
            q->pacing.v.minSeperation                       = OS_DURATION_ZERO;
            q->lifecycle.v.autopurge_nowriter_samples_delay = OS_DURATION_INFINITE;
            q->lifecycle.v.autopurge_disposed_samples_delay = OS_DURATION_INFINITE;
            q->lifecycle.v.autopurge_dispose_all            = FALSE;
            q->lifecycle.v.enable_invalid_samples           = TRUE;
            q->lifecycle.v.invalid_sample_visibility        = V_VISIBILITY_MINIMUM_INVALID_SAMPLES;
            q->lifespan.v.used                              = FALSE;
            q->lifespan.v.duration                          = OS_DURATION_INFINITE;
            q->share.v.enable                               = FALSE;
            q->share.v.name                                 = NULL;
            q->userKey.v.enable                             = FALSE;
            q->userKey.v.expression                         = NULL;
        }
    } else {
        OS_REPORT(OS_ERROR, "v_readerQosNew", V_RESULT_OUT_OF_MEMORY,
                  "Out of resources: allocate memory for Qos value failed");
    }

    return q;
}

void
v_readerQosFree(
    v_readerQos q)
{
    c_free(q);
}

_Success_(return == V_RESULT_OK)
v_result
v_readerQosCompare(
    _In_ v_readerQos q,
    _In_ v_readerQos tmpl,
    _In_ c_bool enabled,
    _In_ c_bool groupCoherent,
    _Out_ v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm, immutable;
    v_result result;

    cm = 0;
    immutable = immutableMask;
    if(enabled && groupCoherent) {
        /* A group-coherent reader cannot be changed at all after it has
         * been enabled.
         */
        immutable |= V_POLICY_BIT_DURABILITY;
        immutable |= V_POLICY_BIT_DEADLINE;
        immutable |= V_POLICY_BIT_LATENCY;
        immutable |= V_POLICY_BIT_LIVELINESS;
        immutable |= V_POLICY_BIT_RELIABILITY;
        immutable |= V_POLICY_BIT_ORDERBY;
        immutable |= V_POLICY_BIT_HISTORY;
        immutable |= V_POLICY_BIT_RESOURCE;
        immutable |= V_POLICY_BIT_USERDATA;
        immutable |= V_POLICY_BIT_OWNERSHIP;
        immutable |= V_POLICY_BIT_PACING;
        immutable |= V_POLICY_BIT_READERLIFECYCLE;
        immutable |= V_POLICY_BIT_READERLIFESPAN;
        immutable |= V_POLICY_BIT_SHARE;
        immutable |= V_POLICY_BIT_USERKEY;
    }

    if (v_readerQosValidValues(tmpl)) {
        if (v_readerQosConsistent(tmpl)) {
            /* built change mask */
#define _SETMASK_(type,qos,label) if (!v_##type##PolicyIEqual(q->qos, tmpl->qos)) { cm |= V_POLICY_BIT_##label; }
            _SETMASK_(durability,durability,DURABILITY)
            _SETMASK_(deadline,deadline,DEADLINE)
            _SETMASK_(latency,latency,LATENCY)
            _SETMASK_(liveliness,liveliness,LIVELINESS)
            _SETMASK_(reliability,reliability,RELIABILITY)
            _SETMASK_(orderby,orderby,ORDERBY)
            _SETMASK_(history,history,HISTORY)
            _SETMASK_(resource,resource,RESOURCE)
            _SETMASK_(userData,userData,USERDATA)
            _SETMASK_(ownership, ownership, OWNERSHIP);
            _SETMASK_(pacing,pacing,PACING)
            _SETMASK_(readerLifecycle,lifecycle,READERLIFECYCLE)
            _SETMASK_(readerLifespan,lifespan,READERLIFESPAN)
            _SETMASK_(share,share,SHARE)
            _SETMASK_(userKey,userKey,USERKEY)
#undef _SETMASK_
            /* check whether immutable policies are changed */
            if (((cm & immutable) != 0) && (enabled)) {
                v_policyReportImmutable(cm, immutable);
                result = V_RESULT_IMMUTABLE_POLICY;
                OS_REPORT(OS_ERROR, "v_readerQosCompare", result,
                          "Precondition not met: Immutable Qos policy violation");
            } else {
                *changeMask = cm;
                result = V_RESULT_OK;
            }
        } else {
            result = V_RESULT_INCONSISTENT_QOS;
            OS_REPORT(OS_ERROR, "v_readerQosCompare", result,
                      "Precondition not met: Detected Inconsistent Qos policy");
        }
    } else {
        result = V_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "v_readerQosCompare", result,
                  "Bad parameter: Detected Invalid Qos policy");
    }

    return result;
}

v_result
v_readerQosCheck(
    v_readerQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (v_readerQosValidValues(_this)) {
            if (!v_readerQosConsistent(_this)) {
                result = V_RESULT_INCONSISTENT_QOS;
                OS_REPORT(OS_ERROR, "v_readerQosCheck", result,
                    "ReaderQoS is inconsistent.");
            }
        } else {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_readerQosCheck", result,
                "ReaderQoS is invalid.");
        }
    }

    return result;
}
