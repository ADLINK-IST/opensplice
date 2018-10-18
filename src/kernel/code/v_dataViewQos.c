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

/* Interface */
#include "v_dataViewQos.h"

/* Implementation */
#include "v_qos.h"
#include "v_policy.h"
#include "os_report.h"

static const v_qosChangeMask immutableMask = V_POLICY_BIT_USERKEY;

static c_bool
v_dataViewQosValidValues(
    v_dataViewQos qos)
{
    c_ulong valuesNok = 0;

    /* no typechecking, since qos might be allocated on heap! */
    if (qos != NULL) {
        /* value checking */
        valuesNok |= (c_ulong) (!v_userKeyPolicyIValid(qos->userKey)) << V_USERKEYPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
}

v_dataViewQos
v_dataViewQosNew(
    v_kernel kernel,
    v_dataViewQos template)
{
    v_dataViewQos q;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));
    q = v_dataViewQos(v_qosCreate(base,V_DATAVIEW_QOS));

    if (q != NULL) {
        if (template != NULL) {
            q->userKey.v.enable = template->userKey.v.enable;
            if (q->userKey.v.enable) {
                q->userKey.v.expression = c_stringNew(base,template->userKey.v.expression);
            } else {
                q->userKey.v.expression = NULL;
            }
        }
    }
    return q;
}

void
v_dataViewQosFree(
    v_dataViewQos q)
{
    c_free(q);
}

v_result
v_dataViewQosCompare(
    v_dataViewQos q,
    v_dataViewQos tmpl,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL) && (changeMask != NULL)) {
        if (!v_userKeyPolicyIEqual(q->userKey, tmpl->userKey)) {
            cm |= V_POLICY_BIT_USERKEY;
        }
        /* check whether immutable policies are changed */
        if (cm & immutableMask) {
            v_policyReportImmutable(cm, immutableMask);
            result = V_RESULT_IMMUTABLE_POLICY;
        } else {
            /* set new policies */
            /* currently no mutable policies... */
            *changeMask = cm;
            result = V_RESULT_OK;
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

v_result
v_dataViewQosCheck(
    v_dataViewQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (!v_dataViewQosValidValues(_this)) {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_dataviewQosCheck", result,
                "DataViewQoS is invalid.");
        }
    }

    return result;
}
