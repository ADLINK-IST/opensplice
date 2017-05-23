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

#include "u_partition.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__domain.h"
#include "u_user.h"

#include "v_kernel.h"
#include "v_partition.h"
#include "v_entity.h"
#include "os_report.h"

static u_result u__partitionDeinitW(void *_this)
{
    return u__observableProxyDeinitW(_this);
}

static void u__partitionFreeW(void *_this)
{
    u__observableProxyFreeW(_this);
}

u_partition
u_partitionNew(
    u_participant p,
    const os_char *name,
    u_partitionQos qos)
{
    u_domain domain;
    u_partition _this = NULL;
    v_kernel ke = NULL;
    v_partition kd;
    u_result result;

    assert(p != NULL);

    if (name == NULL) {
        name = "No partition specified";
    }
    domain = u_observableDomain(u_observable(p));
    result = u_observableWriteClaim(u_observable(domain),(v_public *)(&ke), C_MM_RESERVATION_LOW);
    if ((result == U_RESULT_OK) && (ke != NULL)) {
        kd = v_partitionNew(ke,name,qos);
        if (kd != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_PARTITION, u__partitionDeinitW, u__partitionFreeW);
            if (_this != NULL) {
                result = u_observableInit(u_observable(_this), v_public(kd), domain);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_partitionNew", result,
                              "Initialisation failed. For Partition: <%s>.", name);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_partitionNew", U_RESULT_INTERNAL_ERROR,
                          "Create proxy failed. For Partition: <%s>.", name);
            }
            c_free(kd);
        } else {
            OS_REPORT(OS_ERROR, "u_partitionNew", U_RESULT_INTERNAL_ERROR,
                      "Create kernel entity failed. For Partition: <%s>", name);
        }
        u_observableRelease(u_observable(domain), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_partitionNew", U_RESULT_INTERNAL_ERROR,
                    "Claim Participant failed. "
                    "For Partition: <%s>", name);
    }
    return _this;
}

