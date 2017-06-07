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

#include "v__publisherQos.h"
#include "v_kernel.h"
#include "v_qos.h"
#include "v_policy.h"

#include "os_report.h"

static const v_qosChangeMask immutableMask = V_POLICY_BIT_PRESENTATION;

/**************************************************************
 * private functions
 **************************************************************/
static c_bool
v_publisherQosValidValues(
    v_publisherQos qos)
{
    c_ulong valuesNok;

    /* no typechecking, since qos might be allocated on heap! */
    valuesNok = 0;
    if (qos != NULL) {
        /* value checking */
        valuesNok |= (c_ulong) (!v_presentationPolicyIValid(qos->presentation)) << V_PRESENTATIONPOLICY_ID;
        valuesNok |= (c_ulong)(!v_entityFactoryPolicyIValid(qos->entityFactory)) << V_ENTITYFACTORYPOLICY_ID;
        valuesNok |= (c_ulong)(!v_groupDataPolicyIValid(qos->groupData)) << V_GROUPDATAPOLICY_ID;
    }

    if (valuesNok) {
        v_policyReportInvalid(valuesNok);
    }

    return (valuesNok) ? FALSE : TRUE;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_publisherQos
v_publisherQosNew(
    v_kernel kernel,
    v_publisherQos template)
{
    v_publisherQos q;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(c_object(kernel));
        q = v_publisherQos(v_qosCreate(base,V_PUBLISHER_QOS));
    if (q != NULL) {
        if (template != NULL) {
            q->groupData.v.size = template->groupData.v.size;
            if (template->groupData.v.size > 0) {
                q->groupData.v.value = c_arrayNew_s(c_octet_t(base),(c_ulong)template->groupData.v.size);
                if (q->groupData.v.value) {
                    memcpy(q->groupData.v.value,template->groupData.v.value,(c_ulong)template->groupData.v.size);
                } else {
                    OS_REPORT(OS_ERROR, "v_publisherQosNew", V_RESULT_OUT_OF_MEMORY,
                              "Failed to allocate group_data policy of publisher QoS.");
                    c_free(q);
                    return NULL;
                }
            } else {
                q->groupData.v.value = NULL;
            }
            q->partition.v   = c_stringNew(base,template->partition.v);
            q->presentation  = template->presentation;
            q->entityFactory = template->entityFactory;
        } else {
            q->groupData.v.value                           = NULL;
            q->groupData.v.size                            = 0;
            q->presentation.v.access_scope                 = V_PRESENTATION_INSTANCE;
            q->presentation.v.coherent_access              = FALSE;
            q->presentation.v.ordered_access               = FALSE;
            q->partition.v                                 = c_stringNew(base, "");
            q->entityFactory.v.autoenable_created_entities = TRUE;
        }
    }

    return q;
}

void
v_publisherQosFree(
    v_publisherQos q)
{
    c_free(q);
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_result
v_publisherQosCompare (
    v_publisherQos _this,
    v_publisherQos tmpl,
    c_bool enable,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;

    cm = 0;
    if ((_this != NULL) && (tmpl != NULL) && (changeMask != NULL)) {
        if (v_publisherQosValidValues(tmpl)) {
            if (!v_presentationPolicyIEqual(_this->presentation, tmpl->presentation)) {
                cm |= V_POLICY_BIT_PRESENTATION;
            }
            if (!v_partitionPolicyIEqual(_this->partition, tmpl->partition)) {
                cm |= V_POLICY_BIT_PARTITION;
            }
            if (!v_groupDataPolicyIEqual(_this->groupData, tmpl->groupData)) {
                cm |= V_POLICY_BIT_GROUPDATA;
            }
            if (!v_entityFactoryPolicyIEqual(_this->entityFactory, tmpl->entityFactory)) {
                cm |= V_POLICY_BIT_ENTITYFACTORY;
            }
            /* check whether immutable policies are changed */
            if (((cm & immutableMask) != 0) && enable) {
                v_policyReportImmutable(cm, immutableMask);
                result = V_RESULT_IMMUTABLE_POLICY;
            } else {
                *changeMask = cm;
                result = V_RESULT_OK;
            }
        } else {
            result = V_RESULT_ILL_PARAM;
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
v_publisherQosCheck(
    v_publisherQos _this)
{
    v_result result = V_RESULT_OK;

    if (_this) {
        if (!v_publisherQosValidValues(_this)) {
            result = V_RESULT_ILL_PARAM;
            OS_REPORT(OS_ERROR, "v_publisherQosCheck", result,
                "PublisherQoS is invalid.");
        }
    }

    return result;
}
