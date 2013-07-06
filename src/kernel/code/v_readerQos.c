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


#include "v__readerQos.h"
#include "v_qos.h"
#include "v_kernel.h"
#include "v_policy.h"

#include "os_report.h"

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

/**************************************************************
 * private functions
 **************************************************************/
static c_bool
v_readerQosValidValues(
    v_readerQos qos)
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
        valuesOk &= v_ownershipPolicyValid(qos->ownership);
        valuesOk &= v_pacingPolicyValid(qos->pacing);
        valuesOk &= v_readerLifecyclePolicyValid(qos->lifecycle);
        valuesOk &= v_readerLifespanPolicyValid(qos->lifespan);
        valuesOk &= v_userDataPolicyValid(qos->userData);
        valuesOk &= v_userKeyPolicyValid(qos->userKey);
        valuesOk &= v_sharePolicyValid(qos->share);
    }
    return (valuesOk?TRUE:FALSE);
}

static c_bool
v_readerQosConsistent(
    v_readerQos qos)
{
    c_bool result;
    c_equality cmp;

    result = TRUE;
    if (qos != NULL) {
        cmp = c_timeCompare(qos->deadline.period, qos->pacing.minSeperation);
        if (cmp == C_LT) {
            result = FALSE;
        }
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
v_readerQos
v_readerQosNew(
    v_kernel kernel,
    v_readerQos template)
{
    v_readerQos q;
    c_type type;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    if (v_readerQosValidValues(template) &&
        v_readerQosConsistent(template)) {
        base = c_getBase(c_object(kernel));
        q = v_readerQos(v_qosCreate(kernel,V_READER_QOS));

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
                if (q->share.enable) {
                    q->share.name = c_stringNew(base,template->share.name);
                } else {
                    q->share.name = NULL;
                }
                if (q->userKey.enable) {
                    q->userKey.expression = c_stringNew(base,template->userKey.expression);
                } else {
                    q->userKey.expression = NULL;
                }
            } else {
                q->durability.kind                            = V_DURABILITY_VOLATILE;
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
                q->userData.size                              = 0;
                q->userData.value                             = NULL;
                q->ownership.kind                             = V_OWNERSHIP_SHARED;
                q->pacing.minSeperation                       = C_TIME_ZERO;
                q->lifecycle.autopurge_nowriter_samples_delay = C_TIME_INFINITE;
                q->lifecycle.autopurge_disposed_samples_delay = C_TIME_INFINITE;
                q->lifecycle.enable_invalid_samples           = TRUE;
                q->lifespan.used                              = FALSE;
                q->lifespan.duration                          = C_TIME_INFINITE;
                q->share.enable                               = FALSE;
                q->share.name                                 = NULL;
                q->userKey.enable                             = FALSE;
                q->userKey.expression                         = NULL;
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_readerQosNew", 0,
            "ReaderQos not created: inconsistent qos");
        q = NULL;
    }

    return q;
}

void
v_readerQosFree(
    v_readerQos q)
{
    c_free(q);
}
/**************************************************************
 * Protected functions
 **************************************************************/
v_result
v_readerQosSet(
    v_readerQos q,
    v_readerQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;
    c_type type;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL)) {
        if (v_readerQosValidValues(tmpl)) {
            if (v_readerQosConsistent(tmpl)) {
                /* built change mask */
#define _SETMASK_(type,qos,label) if (!v_##type##PolicyEqual(q->qos, tmpl->qos)) { cm |= V_POLICY_BIT_##label; }
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
                if (((cm & immutableMask) != 0) && (enabled)) {
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
                    q->ownership   = tmpl->ownership;
                    q->pacing      = tmpl->pacing;
                    q->lifecycle   = tmpl->lifecycle;
                    q->lifespan    = tmpl->lifespan;

                    /* q->share is immutable,  no copy needed */
                    /* q->userKey is immutable, no copy needed */ 
                    
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
