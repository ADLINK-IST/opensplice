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

/* Interface */
#include "v_dataViewQos.h"

/* Implementation */
#include "v_qos.h"
#include "v_policy.h"
#include "os_report.h"

static const v_qosChangeMask immutableMask = V_POLICY_BIT_USERKEY;

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
    q = v_dataViewQos(v_qosCreate(kernel,V_DATAVIEW_QOS));

    if (q != NULL) {
        if (template != NULL) {
            if (((v_qos)template)->kind != V_DATAVIEW_QOS) {
                OS_REPORT_1(OS_ERROR,
                            "v_dataViewQos::New",0,
                            "Illegal Qos kind specified (%s)",
                            v_qosKindImage(v_qos(template)->kind));
                c_free(q);
                return NULL;
            } else {
                q->userKey.enable = template->userKey.enable;
                if (q->userKey.enable) {
                    q->userKey.expression = c_stringNew(base,template->userKey.expression);
                } else {
                    q->userKey.expression = NULL;
                }
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



/**************************************************************
 * Protected functions
 **************************************************************/
v_result
v_dataViewQosSet(
    v_dataViewQos q,
    v_dataViewQos tmpl,
    v_qosChangeMask *changeMask)
{
    v_qosChangeMask cm;
    v_result result;

    cm = 0;
    if ((q != NULL) && (tmpl != NULL)) {
        if (!v_userKeyPolicyEqual(q->userKey, tmpl->userKey)) {
            cm |= V_POLICY_BIT_USERKEY;
        }
        /* check whether immutable policies are changed */
        if (cm & immutableMask) {
            result = V_RESULT_IMMUTABLE_POLICY;
        } else {
            /* set new policies */
            /* currently no mutable policies... */
            result = V_RESULT_OK;
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }

    if (changeMask != NULL) {
        *changeMask = cm;
    }

    return result;
}
