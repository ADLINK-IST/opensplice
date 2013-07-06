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
#include "u_dataViewQos.h"
#include "os_stdlib.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_dataViewQos
u_dataViewQosNew(
    v_dataViewQos tmpl)
{
    v_dataViewQos q;
    u_result result;
    int len;

    q = os_malloc(sizeof(C_STRUCT(v_dataViewQos)));
    if (q != NULL) {
        if (tmpl != NULL) {
            /* Copy non-reference fields */
            *q = *tmpl;
            /* Copy reference fields */
            
            if (tmpl->userKey.enable){
                if (tmpl->userKey.expression != NULL) {
                    len = strlen(tmpl->userKey.expression);
                    q->userKey.expression = os_malloc(len+1);
                    os_strncpy(q->userKey.expression, tmpl->userKey.expression, len);
                    q->userKey.expression[len] = 0;
                } else {
                    q->userKey.expression = NULL;
                }
            }
        } else {
            result = u_dataViewQosInit(q);
            if (result != U_RESULT_OK) {
                u_dataViewQosFree(q);
                q = NULL;
            }
        }
    }
    return q;
}

u_result
u_dataViewQosInit(
    v_dataViewQos q)
{
    u_result result;

    if (q != NULL) {
        ((v_qos)q)->kind = V_DATAVIEW_QOS;
        q->userKey.enable = FALSE;
        q->userKey.expression = NULL;
        result = U_RESULT_OK;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

void
u_dataViewQosDeinit(
    v_dataViewQos q)
{
    if (q != NULL) {
        if (q->userKey.enable) {
            os_free(q->userKey.expression);
        }
        q->userKey.enable = FALSE;
        q->userKey.expression = NULL;
    }
}

void
u_dataViewQosFree(
    v_dataViewQos q)
{
    if (q != NULL) {
        u_dataViewQosDeinit(q);
        os_free(q);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
 
