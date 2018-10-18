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
#include "u__domain.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__service.h"
#include "u_durability.h"

#include "v_service.h"
#include "v_durability.h"
#include "v_configuration.h"
#include "os_report.h"
#include "os_atomics.h"

static void *wrapper(v_kernel kernel, const c_char *name, const c_char *extStateName, v_participantQos qos, c_bool enable, void *varg)
{
    OS_UNUSED_ARG(varg);
    return v_durabilityNew(kernel, name, extStateName, qos, enable);
}

u_service
u_durabilityNew (
    const os_char *uri,
    const u_domainId_t id,
    const os_int32 timeout, /* in seconds */
    const os_char *name,
    const u_participantQos qos,
    c_bool enable)
{
    return u_serviceNewSpecialized(wrapper, "DurabilityService", uri, id, timeout, name, qos, enable, NULL);
}

static void
u__durabilityDecreasePurgeSuppression(
    v_public _this,
    void *varg)
{
    v_kernel k;
    os_uint32 *purgeSuppressionCount = (os_uint32 *)varg;
    os_uint32 psc;

    k = v_objectKernel(_this);
    assert(k != NULL);
    assert(v_kernelHasDurabilityService(k));

    /* Decrease the purgeSuppressCount to signal that durability processed (an
     * update to) a DCPSHeartbeart.
     */
    psc = pa_dec32_nv(&k->purgeSuppressCount);

    if(purgeSuppressionCount) {
        *purgeSuppressionCount = psc;
    }
}

u_result
u_durabilityHeartbeatProcessed (
        _In_ u_service durability,
        _Out_opt_ os_uint32 *purgeSuppressionCount )
{
    u_result result;

    result = u_observableAction (u_observable (durability), &u__durabilityDecreasePurgeSuppression, purgeSuppressionCount);

    return result;
}

