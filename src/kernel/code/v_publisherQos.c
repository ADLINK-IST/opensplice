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
    int valuesOk;

    /* no typechecking, since qos might be allocated on heap! */
    valuesOk = 1;
    if (qos != NULL) {
        /* value checking */
        valuesOk &= v_presentationPolicyValid(qos->presentation);
        valuesOk &= v_entityFactoryPolicyValid(qos->entityFactory);
        valuesOk &= v_groupDataPolicyValid(qos->groupData);
    }

    return (valuesOk?TRUE:FALSE);
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
    c_type type;
    c_base base;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    if (v_publisherQosValidValues(template)) {
        base = c_getBase(c_object(kernel));
        q = v_publisherQos(v_qosCreate(kernel,V_PUBLISHER_QOS));
        if (q != NULL) {
            if (template != NULL) {

                q->groupData.size = template->groupData.size;
                if (template->groupData.size > 0) {
                    type = c_octet_t(base);
                    q->groupData.value = c_arrayNew(type,template->groupData.size);
                    c_free(type);
                    memcpy(q->groupData.value,template->groupData.value,template->groupData.size);
                } else {
                    q->groupData.value = NULL;
                }
                q->partition     = c_stringNew(base,template->partition);
                q->presentation  = template->presentation;
                q->entityFactory = template->entityFactory;
            } else {
                q->groupData.value                           = NULL;
                q->groupData.size                            = 0;
                q->presentation.access_scope                 = V_PRESENTATION_INSTANCE;
                q->presentation.coherent_access              = FALSE;
                q->presentation.ordered_access               = FALSE;
                q->partition                                 = c_stringNew(base, "");
                q->entityFactory.autoenable_created_entities = TRUE;
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_publisherQosNew", 0,
            "PublisherQos not created: inconsistent qos");
        q = NULL;
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
v_publisherQosSet(
    v_publisherQos q,
    v_publisherQos tmpl,
    c_bool enable,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;
    c_type type;
    c_base base;

    base = c_getBase(c_object(q));
    cm = 0;
    if ((q != NULL) && (tmpl != NULL)) {
        if (v_publisherQosValidValues(tmpl)) {
            /* no consistency check needed */
            /* built change mask */
            if (!v_presentationPolicyEqual(q->presentation, tmpl->presentation)) {
                cm |= V_POLICY_BIT_PRESENTATION;
            }
            if (!v_partitionPolicyEqual(q->partition, tmpl->partition)) {
                cm |= V_POLICY_BIT_PARTITION;
            }
            if (!v_groupDataPolicyEqual(q->groupData, tmpl->groupData)) {
                cm |= V_POLICY_BIT_GROUPDATA;
            }
            if (!v_entityFactoryPolicyEqual(q->entityFactory, tmpl->entityFactory)) {
                cm |= V_POLICY_BIT_ENTITYFACTORY;
            }
            /* check whether immutable policies are changed */
            if (((cm & immutableMask) != 0) && enable) {
                result = V_RESULT_IMMUTABLE_POLICY;
            } else {
                /* set new policies */
                q->presentation = tmpl->presentation;
                q->entityFactory = tmpl->entityFactory;
                q->partition = c_stringNew(base, tmpl->partition);
                if (cm & V_POLICY_BIT_GROUPDATA) {
                    c_free(q->groupData.value);
                    q->groupData.size = tmpl->groupData.size;
                    if (tmpl->groupData.size > 0) {
                        type = c_octet_t(base);
                        q->groupData.value = c_arrayNew(type,tmpl->groupData.size);
                        c_free(type);
                        memcpy(q->groupData.value,tmpl->groupData.value,tmpl->groupData.size);
                    } else {
                        q->groupData.value = NULL;
                    }
                }
                result = V_RESULT_OK;
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
