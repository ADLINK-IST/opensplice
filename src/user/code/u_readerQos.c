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
#include "u_readerQos.h"
#include "os_stdlib.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
u_readerQos
u_readerQosNew(
    u_readerQos tmpl)
{
    u_result result;
    u_readerQos q;

    q = os_malloc(sizeof(C_STRUCT(v_readerQos)));
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
            q->share.enable = tmpl->share.enable;
            if (tmpl->share.enable) {
                q->share.name = os_strdup(tmpl->share.name);
            } else {
                q->share.name = NULL;
            }
            q->userKey.enable = tmpl->userKey.enable;
            if (tmpl->userKey.enable) {
                q->userKey.expression = os_strdup(tmpl->userKey.expression);
            } else {
                q->userKey.expression = NULL;
            }
        } else {
            result = u_readerQosInit(q);
            if (result != U_RESULT_OK) {
                u_readerQosFree(q);
                q = NULL;
            }
        }
    }
    return q;
}

u_result
u_readerQosInit(
    u_readerQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind                              = V_READER_QOS;
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
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_readerQosDeinit(
    u_readerQos q)
{
    if (q != NULL) {
        os_free(q->userData.value);
        q->userData.value = NULL;
        if (q->share.enable && q->share.name) {
            os_free(q->share.name);
        }
        if(q->userKey.expression) {
            os_free(q->userKey.expression);
        }
    }
}

void
u_readerQosFree(
    u_readerQos q)
{
    if (q != NULL) {
        u_readerQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
