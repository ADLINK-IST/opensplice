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

#include "u_participantQos.h"
#include "v_scheduler.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_participantQos
u_participantQosNew(
    v_participantQos tmpl)
{
    v_participantQos q;

    q = os_malloc(sizeof(C_STRUCT(v_participantQos)));
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
            u_participantQosInit(q);
        }
    }

    return q;
}

u_result
u_participantQosInit(
    v_participantQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind                             = V_PARTICIPANT_QOS;
        q->userData.value                            = NULL;
        q->userData.size                             = 0;
        q->entityFactory.autoenable_created_entities = TRUE;
	q->watchdogScheduling.kind = V_SCHED_DEFAULT;
	q->watchdogScheduling.priorityKind = V_SCHED_PRIO_RELATIVE;
	q->watchdogScheduling.priority = 0;
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_participantQosDeinit(
    v_participantQos q)
{
    if (q != NULL) {
        os_free(q->userData.value);
        q->userData.value = NULL;
    }
}

void
u_participantQosFree(
    v_participantQos q)
{
    if (q != NULL) {
        u_participantQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
