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

#include "u__topic.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__kernel.h"
#include "u__participant.h"
#include "u__user.h"

#include "v_kernel.h"
#include "v_topic.h"
#include "v_entity.h"
#include "os_report.h"

u_result
u_topicClaim(
    u_topic _this,
    v_topic *topic)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (topic != NULL)) {
        *topic = v_topic(u_entityClaim(u_entity(_this)));
        if (*topic == NULL) {
            OS_REPORT_2(OS_WARNING, "u_topicClaim", 0,
                        "Claim Topic failed. "
                        "<_this = 0x%x, topic = 0x%x>.",
                         _this, topic);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_topicClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, topic = 0x%x>.",
                    _this, topic);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_topicRelease(
    u_topic _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_topicRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_topic
u_topicNew(
    u_participant p,
    const c_char *name,
    const c_char *typeName,
    const c_char *keyList,
    v_topicQos qos)
{
    u_topic _this = NULL;
    v_topic kt;
    v_kernel kk;
    u_result result;

    if (name != NULL) {
        if (p != NULL) {
            result = u_kernelClaim(p->kernel,&kk);
            if ((result == U_RESULT_OK) && (kk != NULL)) {
                kt = v_topicNew(kk,name,typeName,keyList,qos);
                if (kt != NULL) {
                    _this = u_entityAlloc(p,u_topic,kt,FALSE);
                    if (_this != NULL) {
                        result = u_topicInit(_this);
                        if (result == U_RESULT_OK) {                            
                            v_entitySetUserData(v_entity(kt), _this);
                        } else {
                            OS_REPORT_1(OS_ERROR, "u_topicNew", 0,
                                        "Initialisation failed. "
                                        "For Topic: <%s>", name);
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "u_topicNew", 0,
                                    "Create user proxy failed. "
                                    "For Topic: <%s>", name);
                    }
                    c_free(kt);
                } else {
                    OS_REPORT_1(OS_ERROR, "u_topicNew", 0,
                                "Create kernel entity failed. "
                                "For Topic: <%s>", name);
                }
                result = u_kernelRelease(p->kernel);
            } else {
                OS_REPORT_1(OS_WARNING, "u_topicNew", 0,
                            "Claim Kernel failed. "
                            "For Topic: <%s>", name);
            }
        } else {
            OS_REPORT_1(OS_ERROR,"u_topicNew",0,
                        "No Participant specified. "
                        "For Topic: <%s>", name);
        }
    } else {
        OS_REPORT(OS_ERROR,"u_topicNew",0,
                  "No name specified.");
    }
    return _this;
}

u_result
u_topicInit(
    u_topic _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_topicInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_topicFree(
    u_topic _this)
{
    u_result result;

    if (_this != NULL) {
        if (u_entity(_this)->flags & U_ECREATE_INITIALISED) {
            result = u_topicDeinit(_this);
            os_free(_this);
        } else {
            result = u_entityFree(u_entity(_this));
        }
    } else {
        result = U_RESULT_OK;
    }
    return result;
}

u_result
u_topicDeinit(
    u_topic _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherDeinit(u_dispatcher(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_topicDeinit", 0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_char *
u_topicTypeName(
    u_topic t)
{
    v_topic kt;
    u_result r;
    c_char *name;

    r = u_topicClaim(t,&kt);
    if ((r == U_RESULT_OK) && (kt != NULL)) {
        name = (c_char *)c_metaScopedName(c_metaObject(v_topicDataType(kt)));
        u_topicRelease(t);
    } else {
        OS_REPORT(OS_WARNING, "u_topicTypeName", 0,
                  "Could not claim topic.");
        name = NULL;
    }
    return name;
}

u_result
u_topicGetInconsistentTopicStatus (
    u_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_topic topic;
    u_result result;
    v_result r;

    result = u_topicClaim(_this, &topic);

    if ((result == U_RESULT_OK) && (topic != NULL)) {
        r = v_topicGetInconsistentTopicStatus(topic,reset,action,arg);
        u_topicRelease(_this);
        result = u_resultFromKernel(r);
    }
    return result;
}

