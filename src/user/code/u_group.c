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

/* Interface */
#include "u_group.h"

/* Implementation */
#include "u_user.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__participant.h"

#include "v_entity.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_participant.h"
#include "v_partition.h"
#include "v_kernel.h"

#include "os_report.h"

/* To be called from protected threads only */
u_group
u_groupCreate(
    v_group group,
    u_participant participant)
{
    u_group _this;

    if (group != NULL) {
        /* Create u_entity. We are not the owner of the handle */
        _this = u_entityAlloc(participant, u_group, group, FALSE);
    } else {
        _this = NULL;
    }

    return _this;
}

u_result
u_groupFree(
    u_group _this)
{
    u_result result;
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            /* This user entity is a proxy, meaning that it is not fully
             * initialized, therefore only the entity part of the object
             * can be deinitialized.
             * It would be better to either introduce a separate proxy
             * entity for clarity or fully initialize entities and make
             * them robust against missing information.
             */
            result = u_entityDeinit(u_entity(_this));

            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_groupFree",0,
                            "Operation u_groupDeinit failed: "
                            "Waitset = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_groupFree",0,
                    "Operation u_entityLock failed: "
                    "Waitset = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

/* -------------------------- "Normal" functions ---------------------------- */
/* These functions can be called by any application having a u_group entity.
 * The functions themselves do the protecting */

u_group
u_groupNew(
    u_participant participant,
    const c_char *partitionName,
    const c_char *topicName,
    v_duration timeout)
{
    u_result r;
    v_participant kparticipant;
    v_kernel kernel;
    v_topic ktopic;
    v_partition kpartition;
    v_group kgroup;
    c_iter topics;
    os_time delay;
    u_group group = NULL;

    if ((partitionName != NULL) && (topicName != NULL)) {
        if (participant != NULL) {
            r = u_entityWriteClaim(u_entity(participant), (v_entity*)(&kparticipant));
            if (r == U_RESULT_OK){
                assert(kparticipant);
                kernel = v_objectKernel(kparticipant);
                topics = v_resolveTopics(kernel,topicName);
                if (c_iterLength(topics) == 0) {
                    c_iterFree(topics);
                    delay.tv_sec = timeout.seconds;
                    delay.tv_nsec = timeout.nanoseconds;
                    os_nanoSleep(delay);
                    topics = v_resolveTopics(v_objectKernel(kparticipant),
                                             topicName);
                }
                if (c_iterLength(topics) > 1) {
                    OS_REPORT_1(OS_WARNING, "u_groupNew", 0,
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
                            group = u_groupCreate(kgroup, participant);
                            if (group == NULL) {
                                OS_REPORT_2(OS_ERROR,"u_groupNew",0,
                                            "Create proxy failed. "
                                            "For Partition <%s> and Topic <%s>.",
                                            partitionName, topicName);
                            }
                            c_free(kgroup);
                        } else {
                            OS_REPORT_2(OS_ERROR,"u_groupNew",0,
                                        "Create kernel entity failed. "
                                        "For Partition <%s> and Topic <%s>.",
                                        partitionName, topicName);
                        }
                        c_free(kpartition);
                    } else {
                        OS_REPORT_2(OS_ERROR,"u_groupNew", 0,
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
                r = u_entityRelease(u_entity(participant));
            } else {
                OS_REPORT_2(OS_ERROR,"u_groupNew",0,
                            "Claim kernel participant failed."
                            "For Partition <%s> and Topic <%s>.",
                            partitionName, topicName);
            }
        } else {
            OS_REPORT_2(OS_ERROR,"u_groupNew",0,
                        "No participant specified. "
                        "For Partition <%s> and Topic <%s>.",
                        partitionName, topicName);
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_groupNew",0,
                    "Illegal parameter."
                    "partitionName = <0x%x>, topicName = <0x%x>.",
                    partitionName, topicName);
    }
    return group;
}

u_result
u_groupFlush(
    u_group group)
{
    v_group kgroup;
    u_result result;

    result = u_entityReadClaim(u_entity(group), (v_entity*)(&kgroup));
    if (result == U_RESULT_OK) {
        v_groupFlush(kgroup);
        u_entityRelease(u_entity(group));
    } else {
        OS_REPORT(OS_ERROR, "u_groupFlush", 0, "Could not claim group.");
    }
    return result;
}

#if defined (__cplusplus)
}
#endif
