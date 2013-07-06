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
#include "u_subscriberQos.h"
#include "os_stdlib.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_subscriberQos
u_subscriberQosNew(
    v_subscriberQos tmpl)
{
    u_result result;
    v_subscriberQos q;

    q = os_malloc(sizeof(C_STRUCT(v_subscriberQos)));
    if (q != NULL) {
        if (tmpl != NULL) {
            *q = *tmpl;
            q->groupData.size = tmpl->groupData.size;
            if (tmpl->groupData.size > 0) {
                q->groupData.value = os_malloc(tmpl->groupData.size);
                memcpy(q->groupData.value,tmpl->groupData.value,tmpl->groupData.size);                
            } else {
                q->groupData.value = NULL;
            }
            if (tmpl->partition != NULL) {
                q->partition = os_strdup(tmpl->partition);
            } else {
                q->partition = NULL;
            }
            q->share.enable = tmpl->share.enable;
            if (tmpl->share.enable) {
                q->share.name = os_strdup(tmpl->share.name);
            } else {
                q->share.name = NULL;
            }
        } else {
            result = u_subscriberQosInit(q);
            if (result != U_RESULT_OK) {
                u_subscriberQosFree(q);
                q = NULL;
	    }
        }
    }

    return q;
}

u_result
u_subscriberQosInit(
    v_subscriberQos q)
{
    u_result result;

    result = U_RESULT_OK;
    if (q != NULL) {
        ((v_qos)q)->kind                             = V_SUBSCRIBER_QOS;
        q->groupData.value                           = NULL;
        q->groupData.size                            = 0;
        q->presentation.access_scope                 = V_PRESENTATION_INSTANCE;
        q->presentation.coherent_access              = FALSE;
        q->presentation.ordered_access               = FALSE;
#if 1
        q->partition                                 = NULL;
#else
/* this part is obsolete */
        q->partition                                 = os_malloc(1);
        if (q->partition != NULL) {
            q->partition[0]                          = '\0';
        } else {
            result = U_RESULT_OUT_OF_MEMORY;
        }
#endif
        q->entityFactory.autoenable_created_entities = TRUE;
        q->share.name = NULL;
        q->share.enable = FALSE;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_subscriberQosDeinit(
    v_subscriberQos q)
{
    if (q != NULL) {
        os_free(q->groupData.value);
        q->groupData.value = NULL;
        os_free(q->partition);
        q->partition = NULL;
        if (q->share.enable && q->share.name) {
            os_free(q->share.name);
            q->share.name = NULL;
            q->share.enable = FALSE;
        }
    }
}

void
u_subscriberQosFree(
    v_subscriberQos q)
{
    if ( q!= NULL) {
        u_subscriberQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
