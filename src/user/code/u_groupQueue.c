/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

u_result
u_groupQueueClaim(
    u_groupQueue _this,
    v_groupQueue *groupQueue)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (groupQueue != NULL)) {
        *groupQueue = v_groupQueue(u_entityClaim(u_entity(_this)));
        if (*groupQueue == NULL) {
            OS_REPORT_2(OS_WARNING, "u_groupQueueClaim", 0,
                        "groupQueue could not be claimed. "
                        "<_this = 0x%x, groupQueue = 0x%x>.",
                         _this, groupQueue);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_groupQueueClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, groupQueue = 0x%x>.",
                    _this, groupQueue);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_groupQueueRelease(
    u_groupQueue _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_groupQueueRelease",0,
                   "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_groupQueue
u_groupQueueNew(
    u_subscriber s,
    const c_char *name,
    c_ulong queueSize,
    v_readerQos qos)
{
    u_participant p;
    u_groupQueue _this = NULL;
    v_subscriber ks = NULL;
    v_groupQueue kn;
    u_result result;

    if (name != NULL) {
        if (s != NULL) {
            result = u_subscriberClaim(s,&ks);
            if ((result == U_RESULT_OK) && (ks != NULL)) {
                kn = v_groupQueueNew(ks,name,queueSize,qos);
                if (kn != NULL) {
                    p = u_entityParticipant(u_entity(s));
                    _this = u_entityAlloc(p,u_groupQueue,kn,TRUE);
                    if (_this != NULL) {
                        result = u_groupQueueInit(_this);
                        if (result != U_RESULT_OK) {
                            OS_REPORT_1(OS_ERROR, "u_groupQueueNew", 0,
                                        "Initialisation failed. "
                                        "For groupQueue: <%s>.", name);
                            u_entityFree(u_entity(_this));
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
                result = u_subscriberRelease(s);
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
    u_groupQueue _this)
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

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_groupQueueDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        OS_REPORT(OS_ERROR,"u_groupQueueFree",0,
                  "The specified Group Queue = NIL.");
        result = U_RESULT_OK;
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
