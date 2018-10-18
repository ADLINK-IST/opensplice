/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v_networking.h"
#include "v_networkingStatistics.h"
#include "v_service.h"
#include "v_participant.h"
#include "v__observable.h"
#include "v__statCat.h"
#include "v__kernel.h"

#include "os_report.h"
#include "os_atomics.h"

v_networking
v_networkingNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *extStateName,
    v_serviceType serviceType,
    v_participantQos qos,
    c_bool enable)
{
    v_networking s;
    v_participantQos q;

    assert(C_TYPECHECK(kernel, v_kernel));
    assert(name != NULL);

    q = v_participantQosNew(kernel, qos); 
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_networkingNew", 0,
                  "Networking service not created: inconsistent qos");
        s = NULL;
    } else {
        s = v_networking(v_objectNew(kernel, K_NETWORKING));

        if (v_isEnabledStatistics(kernel, V_STATCAT_NETWORKING)) {
            s->statistics = v_networkingStatisticsNew(kernel);
        } else {
            s->statistics = NULL;
        }
        v_serviceInit(v_service(s), name, extStateName, serviceType, q, enable);
        c_free(q);
        if (v_service(s)->state == NULL) {
            v_serviceFree(v_service(s));
            s = NULL;
        } else {
            OSPL_ADD_OBSERVER(kernel, s, V_EVENT_NEW_GROUP, NULL);
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
