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

#include "u__types.h"
#include "u__entity.h"
#include "u__subscriber.h"
#include "u_user.h"
#include "u_groupQueue.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_groupQueue.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_entity.h"
#include "os_report.h"

u_groupQueue
u_groupQueueNew(
    u_subscriber s,
    const c_char *name,
    c_ulong queueSize,
    v_readerQos qos,
    c_iter expr)
{
    u_participant p;
    u_groupQueue _this = NULL;
    v_subscriber ks = NULL;
    v_groupQueue kn;
    u_result result;

    if (name != NULL) {
        if (s != NULL) {
            result = u_entityWriteClaim(u_entity(s),(v_entity*)(&ks));
            if (result == U_RESULT_OK) {
                assert(ks);
                kn = v_groupQueueNew(ks,name,queueSize,qos, expr);
                if (kn != NULL) {
                    p = u_entityParticipant(u_entity(s));
                    _this = u_entityAlloc(p,u_groupQueue,kn,TRUE);
                    if (_this != NULL) {
                        result = u_groupQueueInit(_this,s);
                        if (result != U_RESULT_OK) {
                            OS_REPORT_1(OS_ERROR, "u_groupQueueNew", 0,
                                        "Initialisation failed. "
                                        "For groupQueue: <%s>.", name);
                            u_groupQueueFree(_this);
                            _this = NULL;
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "u_groupQueueNew", 0,
                                    "Create proxy failed. "
                                    "For groupQueue: <%s>.", name);
                    }
                    c_free(kn);
                } else {
                    OS_REPORT_1(OS_ERROR, "u_groupQueueNew", 0,
                                "Create kernel entity failed. "
                                "For groupQueue: <%s>.", name);
                }
                result = u_entityRelease(u_entity(s));
            } else {
                OS_REPORT_2(OS_WARNING, "u_groupQueueNew", 0,
                            "Claim Subscriber (0x%x) failed. "
                            "For groupQueue: <%s>.", s, name);
            }
        } else {
            OS_REPORT_1(OS_ERROR,"u_groupQueueNew",0,
                        "No Subscriber specified. "
                        "For groupQueue: <%s>", name);
        }
    } else {
        OS_REPORT(OS_ERROR,"u_groupQueueNew",0,
                  "No name specified.");
    }
    return _this;
}

u_result
u_groupQueueInit(
    u_groupQueue _this,
    u_subscriber s)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerInit(u_reader(_this));
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_groupQueueInit", 0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_groupQueueFree(
    u_groupQueue _this)
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
                result = u_groupQueueDeinit(_this);
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
                            "u_groupQueueFree",0,
                            "Operation u_groupQueueDeinit failed: "
                            "GroupQueue = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_groupQueueFree",0,
                    "Operation u_entityLock failed: "
                    "GroupQueue = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_groupQueueDeinit(
    u_groupQueue _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_readerDeinit(u_reader(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_groupQueueDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}
