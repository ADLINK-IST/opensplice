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
#include "u_publisherQos.h"
#include "os_stdlib.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_publisherQos
u_publisherQosNew(
    v_publisherQos tmpl)
{
    u_result result;
    v_publisherQos q;

    q = os_malloc(sizeof(C_STRUCT(v_publisherQos)));
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
        } else {
            result = u_publisherQosInit(q);
            if (result != U_RESULT_OK) {
                u_publisherQosFree(q);
                q = NULL;
            }
        }
    }

    return q;
}

u_result
u_publisherQosInit(
    v_publisherQos q)
{
    u_result result;

    result = U_RESULT_OK;
    if (q != NULL) {
        ((v_qos)q)->kind                             = V_PUBLISHER_QOS;
        q->groupData.value                           = NULL;
        q->groupData.size                            = 0;
        q->presentation.access_scope                 = V_PRESENTATION_INSTANCE;
        q->presentation.coherent_access              = FALSE;
        q->presentation.ordered_access               = FALSE;
#if 1
        q->partition                                 = NULL;
#else
/* Obsolete */
        q->partition                                 = os_malloc(1);
        if (q->partition != NULL) {
            q->partition[0]                              = '\0';
	} else {
            result = U_RESULT_OUT_OF_MEMORY;
	}
#endif
        q->entityFactory.autoenable_created_entities = TRUE;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_publisherQosDeinit(
    v_publisherQos q)
{
    if (q != NULL) {
        os_free(q->groupData.value);
        q->groupData.value = NULL;
        os_free(q->partition);
        q->partition = NULL;
    }
}

void
u_publisherQosFree(
    v_publisherQos q)
{
    if ( q!= NULL) {
        u_publisherQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
