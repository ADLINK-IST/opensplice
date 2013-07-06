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
#include "v_durability.h"
#include "v__statCat.h"
#include "v__statisticsInterface.h"
#include "v_statistics.h"
#include "v_durabilityStatistics.h"
#include "v_service.h"
#include "v__groupInstance.h"
#include "v_group.h"
#include "v__kernel.h"
#include "v_participant.h"
#include "os_report.h"

#include "os_report.h"

v_durability
v_durabilityNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos)
{
    v_kernel k;
    v_durability s;
    v_participantQos q;
    v_durabilityStatistics dStat;

    assert(C_TYPECHECK(manager, v_serviceManager));
    assert(name != NULL);

    k = v_objectKernel(manager);

    q = v_participantQosNew(k, qos);
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_durabilityNew", 0,
                  "Durability service not created: inconsistent qos");
        s = NULL;
    } else {
    	s = v_durability(v_objectNew(k, K_DURABILITY));

    	if (v_isEnabledStatistics(k, V_STATCAT_DURABILITY)) {
            dStat = v_durabilityStatisticsNew(k);
        } else {
            dStat = NULL;
        }
    	v_serviceInit(v_service(s), manager, name, extStateName, q, v_statistics(dStat));
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
v_durabilityFree(
    v_durability du)
{
    assert(C_TYPECHECK(du, v_durability));
    v_serviceFree(v_service(du));
}

v_groupSample
v_durabilityGroupInstanceHead(
    v_groupInstance instance)
{
    return v_groupInstanceHead(instance);
}

void
v_durabilityGroupInstanceSetHead(
    v_groupInstance instance,
    v_groupSample sample)
{
	v_groupInstanceSetHead(instance, sample);
}

v_groupSample
v_durabilityGroupInstanceTail(
    v_groupInstance instance)
{
	return v_groupInstanceTail(instance);
}

void
v_durabilityGroupInstanceSetTail(
    v_groupInstance instance,
    v_groupSample sample)
{
	v_groupInstanceSetTail(instance, sample);
}
