/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "v_rnr.h"
#include "v__statCat.h"
#include "v_kernel.h"
#include "v_service.h"
#include "v_participant.h"
#include "v_statistics.h"
#include "v_rnrStatistics.h"

#include "os_report.h"

v_rnr
v_rnrNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    c_bool enable)
{
    v_rnr _this;
    v_participantQos q;

    assert(C_TYPECHECK(kernel, v_kernel));
    assert(name != NULL);

    q = v_participantQosNew(kernel, qos);
    if (q == NULL) {
        OS_REPORT(OS_ERROR, "v_rnrNew", V_RESULT_ILL_PARAM,
                  "Record and Replay service not created: inconsistent qos");
        _this = NULL;
    } else {
        _this = v_rnr(v_objectNew(kernel, K_RNR));
        _this->statistics = v_rnrStatisticsNew (kernel, name);
        v_serviceInit(v_service(_this), name, extStateName, V_SERVICETYPE_RECORD_REPLAY, q, enable);
        c_free(q);
        /* always add, even when s->state==NULL, since v_participantFree always
           removes the participant.*/
        v_addParticipant(kernel, v_participant(_this));
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
