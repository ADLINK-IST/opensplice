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

/* Interface */
#include "u_group.h"

/* Implementation */
#include "u_user.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"

#include "v_entity.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_participant.h"
#include "v_partition.h"
#include "v_kernel.h"

#include "os_report.h"

static u_result
u__groupDeinitW(
    void *_this)
{
    return u__observableDeinitW(_this);
}

static void
u__groupFreeW(
    void *_this)
{
    u__observableFreeW(_this);
}

static u_result
u_groupInit(
    u_group _this,
    v_group group,
    u_domain domain)
{
    return u_observableInit(u_observable(_this), v_public(group), domain);
}

/* -------------------------- "Normal" functions ---------------------------- */
/* These functions can be called by any application having a u_group entity.
 * The functions themselves do the protecting
 */

u_group
u_groupNew(
    const u_participant participant,
    const os_char *partitionName,
    const os_char *topicName,
    os_duration timeout)
{
    u_result r;
    v_participant kparticipant;
    v_kernel kernel;
    v_topic ktopic;
    v_partition kpartition;
    v_group kgroup;
    c_iter topics;
    u_group group = NULL;

    assert(participant != NULL);
    assert(partitionName != NULL);
    assert(topicName != NULL);

    r = u_observableWriteClaim(u_observable(participant), (v_public *)(&kparticipant), C_MM_RESERVATION_HIGH);
    if (r == U_RESULT_OK){
        assert(kparticipant);
        kernel = v_objectKernel(kparticipant);
        topics = v_resolveTopics(kernel,topicName);
        if (c_iterLength(topics) == 0) {
            c_iterFree(topics);
            ospl_os_sleep(timeout);
            topics = v_resolveTopics(v_objectKernel(kparticipant),
                                     topicName);
        }
        if (c_iterLength(topics) > 1) {
            OS_REPORT(OS_WARNING, "u_groupNew", U_RESULT_INTERNAL_ERROR,
                        "Internal error: "
                        "Multiple topics found with name = <%s>.",
                        topicName);
        }

        ktopic = c_iterTakeFirst(topics);

        /* If ktopic == NULL, the topic definition is unknown.
         * This is not critical since it may become known in the near future.
         * In that case the caller is responsible for retrying to create this group,
         * and log something if, eventually, the group still cannot be created.
         */
        if (ktopic != NULL) {
            kpartition = v_partitionNew(kernel, partitionName, NULL);
            if (kpartition != NULL) {
                kgroup = v_groupSetCreate(kernel->groupSet, kpartition, ktopic);
                if (kgroup != NULL) {
                    group = u_objectAlloc(sizeof(*group), U_GROUP, u__groupDeinitW, u__groupFreeW);
                    if (group != NULL) {
                        u_domain domain = u_observableDomain(u_observable(participant));
                        r = u_groupInit(group, kgroup, domain);
                        if (r != U_RESULT_OK) {
                            OS_REPORT(OS_ERROR, "u_groupNew", r,
                                      "Group initialization failed. "
                                      "For Partition <%s> and Topic <%s>.",
                                      partitionName, topicName);
                            u_objectFree (u_object (group));
                            group = NULL;
                        }
                    } else {
                        OS_REPORT(OS_ERROR,"u_groupNew", U_RESULT_INTERNAL_ERROR,
                                    "Create proxy failed. "
                                    "For Partition <%s> and Topic <%s>.",
                                    partitionName, topicName);
                    }
                    c_free(kgroup);
                } else {
                    OS_REPORT(OS_ERROR,"u_groupNew", U_RESULT_INTERNAL_ERROR,
                                "Create kernel entity failed. "
                                "For Partition <%s> and Topic <%s>.",
                                partitionName, topicName);
                }
                c_free(kpartition);
            } else {
                OS_REPORT(OS_ERROR,"u_groupNew", U_RESULT_INTERNAL_ERROR,
                            "Failed to create partition. "
                            "For Partition <%s> and Topic <%s>.",
                            partitionName, topicName);
            }
            c_free(ktopic);
        }
        ktopic = c_iterTakeFirst(topics);
        while (ktopic != NULL) {
            c_free(ktopic);
            ktopic = c_iterTakeFirst(topics);
        }
        c_iterFree(topics);
        u_observableRelease(u_observable(participant), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_ERROR,"u_groupNew", r,
                    "Claim kernel participant failed."
                    "For Partition <%s> and Topic <%s>.",
                    partitionName, topicName);
    }
    return group;
}

u_result
u_groupSetRoutingEnabled(
    u_group group,
    c_bool routingEnabled,
    c_bool *oldRoutingEnabled)
{
    v_group kgroup;
    u_result result;

    result = u_observableReadClaim(u_observable(group), (v_public *)(&kgroup), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        c_bool old;
        old = v_groupSetRoutingEnabled(kgroup, routingEnabled);
        u_observableRelease(u_observable(group), C_MM_RESERVATION_ZERO);
        if (oldRoutingEnabled) {
            *oldRoutingEnabled = old;
        }
    } else {
        OS_REPORT(OS_ERROR, "u_groupSetRoutingEnabled", result, "Could not claim group.");
    }
    return result;
}

#if defined (__cplusplus)
}
#endif
