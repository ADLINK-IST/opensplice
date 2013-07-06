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

#include "u_topicQos.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_topicQos
u_topicQosNew(
    v_topicQos tmpl)
{
    v_topicQos q;
    u_result result;

    q = os_malloc(sizeof(C_STRUCT(v_topicQos)));
    if (q != NULL) {
        if (tmpl != NULL) {
            *q = *tmpl;
            q->topicData.size = tmpl->topicData.size;
            if (tmpl->topicData.size > 0) {
                q->topicData.value = os_malloc(tmpl->topicData.size);
                memcpy(q->topicData.value,tmpl->topicData.value,tmpl->topicData.size);                
            } else {
                q->topicData.value = NULL;
            }
        } else {
            result = u_topicQosInit(q);
            if (result != U_RESULT_OK) {
                u_topicQosFree(q);
                q = NULL;
            }
        }
    }

    return q;
}

u_result
u_topicQosInit(
    v_topicQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind                              = V_TOPIC_QOS;
        q->topicData.size                             = 0;
        q->topicData.value                            = NULL;
        q->durability.kind                            = V_DURABILITY_VOLATILE;
        q->durabilityService.service_cleanup_delay    = C_TIME_ZERO;
        q->durabilityService.history_kind             = V_HISTORY_KEEPLAST;
        q->durabilityService.history_depth            = 1;
        q->durabilityService.max_samples              = V_LENGTH_UNLIMITED;
        q->durabilityService.max_instances            = V_LENGTH_UNLIMITED;
        q->durabilityService.max_samples_per_instance = V_LENGTH_UNLIMITED;
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
        q->transport.value                            = 0;
        q->lifespan.duration                          = C_TIME_INFINITE;
        q->ownership.kind                             = V_OWNERSHIP_SHARED;
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_topicQosDeinit(
    v_topicQos q)
{
    if (q != NULL) {
        os_free(q->topicData.value);
        q->topicData.value = NULL;
    }
}

void
u_topicQosFree(
    v_topicQos q)
{
    if (q != NULL) {
        u_topicQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
