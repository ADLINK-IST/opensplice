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

#include "v__subscriberQos.h"
#include "v_kernel.h"
#include "v_qos.h"
#include "v_policy.h"
#include "os_report.h"


static const v_qosChangeMask immutableMask = V_POLICY_BIT_PRESENTATION;

static c_bool
v_subscriberQosValidValues(
    v_subscriberQos qos)
{
    c_ulong valuesNok;

    /* no typechecking, since qos might be allocated on heap! */
    valuesNok = 0;
    if (qos != NULL) {
        /* value checking */
        valuesNok |= (c_ulong) (!v_presentationPolicyIValid(qos->presentation)) << V_PRESENTATIONPOLICY_ID;
        valuesNok |= (c_ulong) (!v_entityFactoryPolicyIValid(qos->entityFactory)) << V_ENTITYFACTORYPOLICY_ID;
        valuesNok |= (c_ulong) (!v_groupDataPolicyIValid(qos->groupData)) << V_GROUPDATAPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
}

_Check_return_
_When_(template, _Ret_maybenull_)
v_subscriberQos
v_subscriberQosNew(
    _In_ v_kernel kernel,
    _In_opt_ v_subscriberQos template)
{
    v_subscriberQos q;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));
    q = v_subscriberQos(v_qosCreate(base,V_SUBSCRIBER_QOS));

    if (template != NULL) {
        q->groupData.v.size = template->groupData.v.size;
        if (template->groupData.v.size > 0) {
            q->groupData.v.value = c_arrayNew_s(c_octet_t(base),(c_ulong)template->groupData.v.size);
            if (q->groupData.v.value) {
                memcpy(q->groupData.v.value,template->groupData.v.value,(c_ulong)template->groupData.v.size);
            } else {
                OS_REPORT(OS_ERROR, "v_subscriberQosNew", V_RESULT_OUT_OF_MEMORY,
                          "Failed to allocate group_data policy of subscriber QoS.");
                c_free(q);
                return NULL;
            }
        } else {
            q->groupData.v.value = NULL;
        }
        q->partition.v = c_stringNew(base,template->partition.v);
        q->presentation = template->presentation;
        q->share.v.enable = template->share.v.enable;
        q->share.v.name = c_stringNew(base,template->share.v.name);
        q->entityFactory = template->entityFactory;
    } else {
        q->groupData.v.value              = NULL;
        q->groupData.v.size               = 0;
        q->presentation.v.access_scope    = V_PRESENTATION_INSTANCE;
        q->presentation.v.coherent_access = FALSE;
        q->presentation.v.ordered_access  = FALSE;
        q->partition.v                    = c_stringNew(base, "");
        q->share.v.enable                 = FALSE;
        q->share.v.name                   = NULL;
        q->entityFactory.v.autoenable_created_entities = TRUE;
    }

    return q;
}

void
v_subscriberQosFree(
        _In_opt_ _Post_invalid_ v_subscriberQos q)
{
    c_free(q);
}

_Success_(return == V_RESULT_OK)
v_result
v_subscriberQosCompare(
    _In_ v_subscriberQos q,
    _In_ v_subscriberQos tmpl,
    _In_ c_bool enabled,
    _Out_ v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm, immutable;
    v_result result;

    cm = 0;
    immutable = immutableMask;
    if(enabled && v_subscriberQosIsGroupCoherent(q)) {
        /* A group-coherent subscriber cannot be changed at all after it has
         * been enabled.
         */
        immutable |= V_POLICY_BIT_PARTITION;
        /* Changing V_POLICY_BIT_GROUPDATA  shouldn't matter, since it doesn't
         * influence the middleware at all, but for now we block all changes.
         */
        immutable |= V_POLICY_BIT_GROUPDATA;
        /* Changing V_POLICY_BIT_ENTITYFACTORY  shouldn't matter, since it is
         * ignored anyway for a group-coherent subscriber.
         */
        immutable |= V_POLICY_BIT_ENTITYFACTORY;
    }

    /* built change mask */
        if (!v_presentationPolicyIEqual(q->presentation, tmpl->presentation)) {
        cm |= V_POLICY_BIT_PRESENTATION;
    }
        if (!v_partitionPolicyIEqual(q->partition, tmpl->partition)) {
        cm |= V_POLICY_BIT_PARTITION;
    }
        if (!v_groupDataPolicyIEqual(q->groupData, tmpl->groupData)) {
        cm |= V_POLICY_BIT_GROUPDATA;
    }
        if (!v_entityFactoryPolicyIEqual(q->entityFactory, tmpl->entityFactory)) {
        cm |= V_POLICY_BIT_ENTITYFACTORY;
    }
    /* check whether immutable policies are changed */
    if (((cm & immutable) != 0) && enabled) {
        v_policyReportImmutable(cm, immutable);
        result = V_RESULT_IMMUTABLE_POLICY;
    } else {
        *changeMask = cm;
        result = V_RESULT_OK;
    }

    return result;
}

c_bool
v_subscriberQosIsGroupCoherent (
    _In_ _Const_ v_subscriberQos _this)
{
    return _this->presentation.v.coherent_access && _this->presentation.v.access_scope == V_PRESENTATION_GROUP;
}

v_result
v_subscriberQosCheck(
    _In_opt_ v_subscriberQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (!v_subscriberQosValidValues(_this)) {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_subscriberQosCheck", result,
                "SubscriberQoS is invalid.");
        }
    }

    return result;
}
