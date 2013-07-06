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
#include "v_cmsoap.h"
#include "v_kernel.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_statistics.h"
#include "v_cmsoapStatistics.h"

#include "os_report.h"

v_cmsoap
v_cmsoapNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos)
{
    v_kernel k;
    v_cmsoap s;
    v_participantQos q;

    assert(C_TYPECHECK(manager, v_serviceManager));
    assert(name != NULL);

    k = v_objectKernel(manager);
    q = v_participantQosNew(k, qos); 
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_cmsoapNew", 0,
                  "CMSoap service not created: inconsistent qos");
        s = NULL;
    } else {
        s = v_cmsoap(v_objectNew(k, K_CMSOAP));
        v_serviceInit(v_service(s), manager, name, extStateName, q, v_statistics(v_cmsoapStatisticsNew(k)));
        c_free(q);
        /* always add, even when s->state==NULL, since v_participantFree always
           removes the participant.*/
        v_addParticipant(k, v_participant(s));
        if (v_service(s)->state == NULL) {
            v_serviceFree(v_service(s));
            s = NULL;
        }
    }
    return s;
}

void
v_cmsoapFree(
    v_cmsoap cms)
{
    assert(C_TYPECHECK(cms, v_cmsoap));
    v_serviceFree(v_service(cms));
}
