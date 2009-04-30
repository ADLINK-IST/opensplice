/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "u_domainQos.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_domainQos
u_domainQosNew(
    v_domainQos tmpl)
{
    v_domainQos q;
    u_result result;

    q = os_malloc(sizeof(C_STRUCT(v_domainQos)));
    if (q != NULL) {
        if (tmpl != NULL) {
            *q = *tmpl;
            q->userData.size = tmpl->userData.size;
            if (q->userData.size > 0) {
                q->userData.value = os_malloc(tmpl->userData.size);
                memcpy(q->userData.value, tmpl->userData.value, tmpl->userData.size);
            } else {
                q->userData.value = NULL;
            }
        } else {
            result = u_domainQosInit(q);
            if (result != U_RESULT_OK) {
                u_domainQosFree(q);
                q = NULL;
            }
        }
    }

    return q;
}

u_result
u_domainQosInit(
    v_domainQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind = V_DOMAIN_QOS;
        q->userData.size  = 0;
        q->userData.value = NULL;

        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void 
u_domainQosDeinit(
    v_domainQos q)
{
    if (q != NULL) {
        os_free(q->userData.value);
        q->userData.value = NULL;
    }
}

void
u_domainQosFree(
    v_domainQos q)
{
    if (q != NULL) {
        u_domainQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
