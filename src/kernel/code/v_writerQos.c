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

/**************************************************************
 * private functions
 **************************************************************/
static c_bool
v_writerQosValidValues(
    v_writerQos qos)
{
    int valuesOk;

    /* no typechecking, since qos might be allocated on heap! */
    valuesOk = 1;
    if (qos != NULL) {
        /* value checking */
        valuesOk &= v_durabilityPolicyValid(qos->durability);
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
        valuesOk &= v_strengthPolicyValid(qos->strength);
        valuesOk &= v_writerLifecyclePolicyValid(qos->lifecycle);
        valuesOk &= v_userDataPolicyValid(qos->userData);
    }

    return (valuesOk?TRUE:FALSE);
}

static c_bool
v_writerQosConsistent(
    v_writerQos qos)
{
    c_bool result;

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
v_writerQos
v_writerQosNew(
    v_kernel kernel,
    v_writerQos template)
{
    v_writerQos q;
    c_type type;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    if (v_writerQosValidValues(template) &&
        v_writerQosConsistent(template)) {
        base = c_getBase(c_object(kernel));
        q = v_writerQos(v_qosCreate(kernel,V_WRITER_QOS));

        if (q != NULL) {
            if (template != NULL) {

                *q = *template;
                type = c_octet_t(base);
                q->userData.size = template->userData.size;
                if (template->userData.size > 0) {
                    q->userData.value = c_arrayNew(type,template->userData.size);
                    memcpy(q->userData.value,template->userData.value,template->userData.size);
                } else {
                    q->userData.value = NULL;
	        }
            } else {
                q->durability.kind                              = V_DURABILITY_VOLATILE;
                q->deadline.period                              = C_TIME_INFINITE;
                q->latency.duration                             = C_TIME_ZERO;
                q->liveliness.kind                              = V_LIVELINESS_AUTOMATIC;
                q->liveliness.lease_duration                    = C_TIME_ZERO;
                q->reliability.kind                             = V_RELIABILITY_BESTEFFORT;
                q->reliability.max_blocking_time                = C_TIME_ZERO;
                q->reliability.synchronous                      = FALSE;
                q->orderby.kind                                 = V_ORDERBY_RECEPTIONTIME;
                q->history.kind                                 = V_HISTORY_KEEPLAST;
                q->history.depth                                = 1;
                q->resource.max_samples                         = V_LENGTH_UNLIMITED;
                q->resource.max_instances                       = V_LENGTH_UNLIMITED;
                q->resource.max_samples_per_instance            = V_LENGTH_UNLIMITED;
                q->userData.size                                = 0;
                q->userData.value                               = NULL;
                q->ownership.kind                               = V_OWNERSHIP_SHARED; 
                q->strength.value                               = 0;
                q->lifecycle.autodispose_unregistered_instances = TRUE;
                q->lifecycle.autopurge_suspended_samples_delay  = C_TIME_INFINITE;
                q->lifecycle.autounregister_instance_delay      = C_TIME_INFINITE;
                q->lifespan.duration                            = C_TIME_INFINITE;
                q->transport.value                              = 0;
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_writerQosNew", 0,
            "WriterQos not created: inconsistent qos");
        q = NULL;
    }

    return q;
}

void
v_writerQosFree(
    v_writerQos q)
{
    c_free(q);
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_result
v_writerQosSet(
    v_writerQos q,
    v_writerQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;
    c_type type;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL)) {
        if (v_writerQosValidValues(tmpl)) {
            if (v_writerQosConsistent(tmpl)) {
                /* built change mask */
                if (!v_durabilityPolicyEqual(q->durability, tmpl->durability)) {
                    cm |= V_POLICY_BIT_DURABILITY;
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
                if (!v_userDataPolicyEqual(q->userData, tmpl->userData)) {
                    cm |= V_POLICY_BIT_USERDATA;
                }
                if (!v_ownershipPolicyEqual(q->ownership, tmpl->ownership)) {
                    cm |= V_POLICY_BIT_OWNERSHIP;
                }
                if (!v_strengthPolicyEqual(q->strength, tmpl->strength)) {
                    cm |= V_POLICY_BIT_STRENGTH;
                }
                if (!v_writerLifecyclePolicyEqual(q->lifecycle, tmpl->lifecycle)) {
                    cm |= V_POLICY_BIT_WRITERLIFECYCLE;
		}
                /* check whether immutable policies are changed */
                if (((cm & immutableMask) != 0) && (enabled)) {
                    result = V_RESULT_IMMUTABLE_POLICY;
                } else {
                    /* set new policies */
                    q->durability                                   = tmpl->durability;
                    q->deadline                                     = tmpl->deadline;
                    q->latency                                      = tmpl->latency;
                    q->liveliness                                   = tmpl->liveliness;
                    q->reliability                                  = tmpl->reliability;
                    q->orderby                                      = tmpl->orderby;
                    q->resource                                     = tmpl->resource;
                    q->ownership                                    = tmpl->ownership;
                    q->strength                                     = tmpl->strength;
                    q->lifecycle                                    = tmpl->lifecycle;
                    q->lifespan                                     = tmpl->lifespan;
                    q->transport                                    = tmpl->transport;
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
