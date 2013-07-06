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

#include "v__topicQos.h"
#include "v_kernel.h"
#include "v_qos.h"
#include "os_report.h"
#include "v_policy.h"
#include "v_time.h"

static const v_qosChangeMask immutableMask = 
    V_POLICY_BIT_DURABILITY  |
    V_POLICY_BIT_LIVELINESS  |
    V_POLICY_BIT_RELIABILITY |
    V_POLICY_BIT_ORDERBY     |
    V_POLICY_BIT_HISTORY     |
    V_POLICY_BIT_RESOURCE    |
    V_POLICY_BIT_OWNERSHIP   |
    V_POLICY_BIT_DURABILITYSERVICE;

/**************************************************************
 * private functions
 **************************************************************/
static c_bool
v_topicQosValidValues(
    v_topicQos qos)
{
    int valuesOk;
    
    valuesOk = 1;
    /* no typechecking, since qos might be allocated on heap! */
    if (qos != NULL) {
        /* value checking */
        valuesOk = 1;
        valuesOk &= v_durabilityPolicyValid(qos->durability);
        valuesOk &= v_durabilityServicePolicyValid(qos->durabilityService);
        valuesOk &= v_deadlinePolicyValid(qos->deadline);
        valuesOk &= v_latencyPolicyValid(qos->latency);
        valuesOk &= v_livelinessPolicyValid(qos->liveliness);
        valuesOk &= v_reliabilityPolicyValid(qos->reliability);
        valuesOk &= v_orderbyPolicyValid(qos->orderby);
        valuesOk &= v_historyPolicyValid(qos->history);
        valuesOk &= v_resourcePolicyValid(qos->resource);
        valuesOk &= v_transportPolicyValid(qos->transport);
        valuesOk &= v_lifespanPolicyValid(qos->lifespan);
        valuesOk &= v_ownershipPolicyValid(qos->ownership);
        valuesOk &= v_topicDataPolicyValid(qos->topicData);
    }

    return (valuesOk?TRUE:FALSE);
}

static c_bool
v_topicQosConsistent(
    v_topicQos qos)
{
    c_bool result;

    /* no typechecking, since qos might be allocated on heap! */
    result = TRUE;
    if (qos != NULL) {
        if ((qos->resource.max_samples_per_instance != V_LENGTH_UNLIMITED) &&
            (qos->history.kind != V_HISTORY_KEEPALL) &&
            (qos->history.depth > qos->resource.max_samples_per_instance)) {
            result = FALSE;
        }
    }

    return result;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_topicQos
v_topicQosNew(
    v_kernel kernel,
    v_topicQos template)
{
    v_topicQos q;
    c_type type;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    if (v_topicQosValidValues(template) &&
        v_topicQosConsistent(template)) {
        q = v_topicQos(v_qosCreate(kernel,V_TOPIC_QOS));

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

                q->topicData.size = template->topicData.size;
                if (template->topicData.size > 0) {
                    type = c_octet_t(c_getBase(c_object(q)));
                    q->topicData.value = c_arrayNew(type,template->topicData.size);
                    memcpy(q->topicData.value,template->topicData.value,template->topicData.size);
                } else {
                    q->topicData.value = NULL;
                }
            } else {
                q->topicData.size                             = 0;
                q->topicData.value                            = NULL;
                q->durability.kind                            = V_DURABILITY_VOLATILE;
                q->durabilityService.history_kind             = V_HISTORY_KEEPLAST;
                q->durabilityService.history_depth            = 1;
                q->durabilityService.max_samples              = V_LENGTH_UNLIMITED;
                q->durabilityService.max_instances            = V_LENGTH_UNLIMITED;
                q->durabilityService.max_samples_per_instance = V_LENGTH_UNLIMITED;
                q->durabilityService.service_cleanup_delay    = C_TIME_ZERO;
                q->deadline.period                            = C_TIME_INFINITE;
                q->latency.duration                           = C_TIME_ZERO;
                q->liveliness.kind                            = V_LIVELINESS_AUTOMATIC;
                q->liveliness.lease_duration                  = C_TIME_ZERO;
                q->reliability.kind                           = V_RELIABILITY_BESTEFFORT;
                q->reliability.max_blocking_time              = C_TIME_ZERO;
                q->reliability.synchronous                    = FALSE;
                q->orderby.kind                               = V_ORDERBY_RECEPTIONTIME;
                q->history.kind                               = V_HISTORY_KEEPLAST;
                q->history.depth                              = 1;
                q->resource.max_samples                       = V_LENGTH_UNLIMITED;
                q->resource.max_instances                     = V_LENGTH_UNLIMITED;
                q->resource.max_samples_per_instance          = V_LENGTH_UNLIMITED;
                q->transport.value                            = 0;
                q->lifespan.duration                          = C_TIME_INFINITE;
                q->ownership.kind                             = V_OWNERSHIP_SHARED;
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_topicQosNew", 0,
            "TopicQos not created: inconsistent qos");
        q = NULL; 
    }

    return q;
}

void
v_topicQosFree(
    v_topicQos q)
{
    c_free(q);
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_result
v_topicQosSet(
    v_topicQos q,
    v_topicQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;
    c_type type;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL)) {
        if (v_topicQosValidValues(tmpl)) {
            if (v_topicQosConsistent(tmpl)) {
                /* built change mask */
                if (!v_topicDataPolicyEqual(q->topicData, tmpl->topicData)) {
                    cm |= V_POLICY_BIT_TOPICDATA;
                }
                if (!v_durabilityPolicyEqual(q->durability, tmpl->durability)) {
                    cm |= V_POLICY_BIT_DURABILITY;
                }
                if (!v_durabilityServicePolicyEqual(q->durabilityService, tmpl->durabilityService)) {
                    cm |= V_POLICY_BIT_DURABILITYSERVICE;
                }
                if (!v_deadlinePolicyEqual(q->deadline, tmpl->deadline)) {
                    cm |= V_POLICY_BIT_DEADLINE;
                }
                if (!v_latencyPolicyEqual(q->latency, tmpl->latency)) {
                    cm |= V_POLICY_BIT_LATENCY;
                }
                if (!v_livelinessPolicyEqual(q->liveliness, tmpl->liveliness)) {
                    cm |= V_POLICY_BIT_LIVELINESS;
                }
                if (!v_reliabilityPolicyEqual(q->reliability, tmpl->reliability)) {
                    cm |= V_POLICY_BIT_RELIABILITY;
                }
                if (!v_orderbyPolicyEqual(q->orderby, tmpl->orderby)) {
                    cm |= V_POLICY_BIT_ORDERBY;
                }
                if (!v_historyPolicyEqual(q->history, tmpl->history)) {
                    cm |= V_POLICY_BIT_HISTORY;
                }
                if (!v_resourcePolicyEqual(q->resource, tmpl->resource)) {
                    cm |= V_POLICY_BIT_RESOURCE;
                }
                if (!v_transportPolicyEqual(q->transport, tmpl->transport)) {
                    cm |= V_POLICY_BIT_TRANSPORT;
                }
                if (!v_lifespanPolicyEqual(q->lifespan, tmpl->lifespan)) {
                    cm |= V_POLICY_BIT_LIFESPAN;
                }
                if (!v_ownershipPolicyEqual(q->ownership, tmpl->ownership)) {
                    cm |= V_POLICY_BIT_OWNERSHIP;
                }
                /* check whether immutable policies are changed */
                if (((cm & immutableMask) != 0) && enabled) {
                    result = V_RESULT_IMMUTABLE_POLICY;
                } else {
                    /* set new policies */
                    q->durability  = tmpl->durability;
                    q->deadline    = tmpl->deadline;
                    q->latency     = tmpl->latency;
                    q->liveliness  = tmpl->liveliness;
                    q->reliability = tmpl->reliability;
                    q->orderby     = tmpl->orderby;
                    q->history     = tmpl->history;
                    q->resource    = tmpl->resource;
                    q->transport   = tmpl->transport;
                    q->lifespan    = tmpl->lifespan;
                    q->ownership   = tmpl->ownership;
                    if (cm & V_POLICY_BIT_TOPICDATA) {
                        c_free(q->topicData.value);
                        q->topicData.size = tmpl->topicData.size;
                        if (tmpl->topicData.size > 0) {
                            type = c_octet_t(c_getBase(c_object(q)));
                            q->topicData.value = c_arrayNew(type,tmpl->topicData.size);
                            memcpy(q->topicData.value,tmpl->topicData.value,tmpl->topicData.size);
                        } else {
                            q->topicData.value = NULL;
                        }
                    }
                    result = V_RESULT_OK;
                }
            } else {
                result = V_RESULT_INCONSISTENT_QOS;
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
