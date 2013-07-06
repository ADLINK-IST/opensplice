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
#include "v_rnr.h"
#include "v_kernel.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_statistics.h"
#include "v_rnrStatistics.h"

#include "os_report.h"

v_rnr
v_rnrNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos)
{
    v_kernel k;
    v_rnr _this;
    v_participantQos q;

    assert(C_TYPECHECK(manager, v_serviceManager));
    assert(name != NULL);

    k = v_objectKernel(manager);
    q = v_participantQosNew(k, qos);
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_rnrNew", 0,
                  "Record and Replay service not created: inconsistent qos");
        _this = NULL;
    } else {
        _this = v_rnr(v_objectNew(k, K_RNR));
        v_serviceInit(v_service(_this), manager, name, extStateName, q, v_statistics(v_rnrStatisticsNew(k, name)));
        c_free(q);
        /* always add, even when s->state==NULL, since v_participantFree always
           removes the participant.*/
        v_addParticipant(k, v_participant(_this));
        if (v_service(_this)->state == NULL) {
            v_serviceFree(v_service(_this));
            _this = NULL;
        }
    }
    return _this;
}

void
v_rnrFree(
    v_rnr _this)
{
    assert(C_TYPECHECK(_this, v_rnr));
    v_serviceFree(v_service(_this));
}
