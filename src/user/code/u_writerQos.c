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
#include "u_writerQos.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
u_writerQos
u_writerQosNew(
    u_writerQos tmpl)
{
    u_result result;
    u_writerQos q;

    q = os_malloc(sizeof(C_STRUCT(v_writerQos)));
    if (q != NULL) {
        if (tmpl != NULL) {
            *q = *tmpl;
            q->userData.size = tmpl->userData.size;
            if (tmpl->userData.size > 0) {
                q->userData.value = os_malloc(tmpl->userData.size);
                memcpy(q->userData.value,tmpl->userData.value,tmpl->userData.size);                
            } else {
                q->userData.value = NULL;
            }
        } else {
            result = u_writerQosInit(q);
            if (result != U_RESULT_OK) {
                u_writerQosFree(q);
                q = NULL;
            }
        }
    }

    return q;
}

u_result
u_writerQosInit(
    u_writerQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind                                = V_WRITER_QOS;
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
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_writerQosDeinit(
    u_writerQos q)
{
    if (q != NULL) {
        os_free(q->userData.value);
        q->userData.value = NULL;
    }
}

void
u_writerQosFree(
    u_writerQos q)
{
    if (q != NULL) {
        u_writerQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
