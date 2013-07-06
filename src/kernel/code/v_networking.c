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
#include "v_networking.h"
#include "v_networkingStatistics.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_statistics.h"
#include "v__statCat.h"
#include "v__kernel.h"

#include "os_report.h"

v_networking
v_networkingNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos)
{
    v_kernel k;
    v_networking s;
    v_participantQos q;
    v_networkingStatistics ns;

    assert(C_TYPECHECK(manager, v_serviceManager));
    assert(name != NULL);

    k = v_objectKernel(manager);
    q = v_participantQosNew(k, qos); 
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_networkingNew", 0,
                  "Networking service not created: inconsistent qos");
        s = NULL;
    } else {
        s = v_networking(v_objectNew(k, K_NETWORKING));

        if (v_isEnabledStatistics(k, V_STATCAT_NETWORKING)) {
            ns = v_networkingStatistics(v_networkingStatisticsNew(k));
        } else {
            ns = NULL;
        }

        v_serviceInit(v_service(s),
                      manager,
                      name,
                      extStateName,
                      q,
                      v_statistics(ns));
        c_free(q);
        /* always add, even when s->state==NULL,
         * since v_participantFree always removes the participant.
         */
        v_addParticipant(k, v_participant(s));
        if (v_service(s)->state == NULL) {
            v_serviceFree(v_service(s));
            s = NULL;
        }
    }
    return s;
}

void
v_networkingFree(
    v_networking nw)
{
    assert(C_TYPECHECK(nw, v_networking));
    v_serviceFree(v_service(nw));
}
