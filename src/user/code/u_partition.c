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

#include "u_partition.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__domain.h"
#include "u_user.h"

#include "v_kernel.h"
#include "v_partition.h"
#include "v_entity.h"
#include "os_report.h"

u_partition
u_partitionNew(
    u_participant p,
    const c_char *name,
    v_partitionQos qos)
{
    u_partition _this = NULL;
    v_kernel ke = NULL;
    v_partition kd;
    u_result result;

    if (name == NULL) {
        name = "No partition specified";
    }
    if (p != NULL) {
        result = u_entityWriteClaim(u_entity(u_participantDomain(p)),(v_entity*)(&ke));
        if ((result == U_RESULT_OK) && (ke != NULL)) {
            kd = v_partitionNew(ke,name,qos);
            if (kd != NULL) {
                _this = u_entityAlloc(p,u_partition,kd,FALSE);
                if (_this != NULL) {
                    result = u_partitionInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_partitionNew", 0,
                                    "Initialisation failed. "
                                    "For Partition: <%s>.", name);
                        u_partitionFree(_this);
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_partitionNew", 0,
                                "Create proxy failed. "
                                "For Partition: <%s>.", name);
                }
                c_free(kd);
            } else {
                OS_REPORT_1(OS_ERROR, "u_partitionNew", 0,
                            "Create kernel entity failed. "
                            "For Partition: <%s>", name);
            }
            result = u_entityRelease(u_entity(u_participantDomain(p)));
        } else {
            OS_REPORT_1(OS_WARNING, "u_partitionNew", 0,
                        "Claim Participant failed. "
                        "For Partition: <%s>", name);
        }
    } else {
        OS_REPORT_1(OS_ERROR,"u_partitionNew",0,
                    "No Participant specified. "
                    "For Partition: <%s>", name);
    }
    return _this;
}

u_result
u_partitionInit(
    u_partition _this)
{
    u_result result;

    if (_this != NULL) {
        result = U_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,"u_partitionInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_partitionFree(
    u_partition _this)
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
            if (u_entityOwner(u_entity(_this))) {
                result = u_partitionDeinit(_this);
            } else {
                /* This user entity is a proxy, meaning that it is not fully
                 * initialized, therefore only the entity part of the object
                 * can be deinitialized.
                 * It would be better to either introduce a separate proxy
                 * entity for clarity or fully initialize entities and make
                 * them robust against missing information.
                 */
                result = u_entityDeinit(u_entity(_this));
            }
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_partitionFree",0,
                            "Operation u_partitionDeinit failed: "
                            "Partition = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_partitionFree",0,
                    "Operation u_entityLock failed: "
                    "Partition = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_partitionDeinit (
    u_partition _this)
{
    u_result result;

    if (_this != NULL) {
        result = U_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,
                  "u_partitionDeinit",0,
                   "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

