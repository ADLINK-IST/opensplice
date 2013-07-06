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

#include "u_partitionQos.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_partitionQos
u_partitionQosNew(
    v_partitionQos tmpl)
{
    v_partitionQos q;
    u_result result;

    q = os_malloc(sizeof(C_STRUCT(v_partitionQos)));
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
            result = u_partitionQosInit(q);
            if (result != U_RESULT_OK) {
                u_partitionQosFree(q);
                q = NULL;
            }
        }
    }

    return q;
}

u_result
u_partitionQosInit(
    v_partitionQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind = V_PARTITION_QOS;
        q->userData.size  = 0;
        q->userData.value = NULL;

        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void 
u_partitionQosDeinit(
    v_partitionQos q)
{
    if (q != NULL) {
        os_free(q->userData.value);
        q->userData.value = NULL;
    }
}

void
u_partitionQosFree(
    v_partitionQos q)
{
    if (q != NULL) {
        u_partitionQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
