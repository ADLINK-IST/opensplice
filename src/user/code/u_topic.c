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

#include "u__topic.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u__domain.h"
#include "u__participant.h"
#include "u__user.h"

#include "v_kernel.h"
#include "v_topic.h"
#include "v_entity.h"
#include "v_filter.h"
#include "os_report.h"

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
            result = u_entityWriteClaim(u_entity(p->domain),(v_entity*)(&kk));
            if (result == U_RESULT_OK) {
                assert(kk);
                kt = v_topicNew(kk,name,typeName,keyList,qos);
                if (kt != NULL) {
                    _this = u_entityAlloc(p,u_topic,kt,FALSE);
                    if (_this != NULL) {
                      /* This call is moved to u_entityNew to support
                       * proxy Topics. This move does not comply with
                       * the constructor pattern used by all other
                       * Entities, so this need to be fixed.
                       *
                    result = u_topicInit(_this,name,p);
                       */

                        if (result == U_RESULT_OK) {
                            v_entitySetUserData(v_entity(kt), _this);
                        } else {
                            OS_REPORT_1(OS_ERROR, "u_topicNew", 0,
                                        "Initialisation failed. "
                                        "For Topic: <%s>", name);
                            u_topicFree(_this);
                        }
                    } else {
                        OS_REPORT_1(OS_ERROR, "u_topicNew", 0,
                                    "Create user proxy failed. "
                                    "For Topic: <%s>", name);
                    }
                    c_free(kt);
                } else {
                    OS_REPORT_1(OS_WARNING, "u_topicNew", 0,
                                "Create kernel entity failed. "
                                "For Topic: <%s>", name);
                }
                result = u_entityRelease(u_entity(p->domain));
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
    u_topic _this,
    const c_char *name,
    u_participant p)
{
    u_result result;

    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        if (result == U_RESULT_OK) {
            _this->name = os_strdup(name);
            _this->participant = p;
            u_entity(_this)->flags |= U_ECREATE_INITIALISED;
/* Note: redefinitions of the a topic are added to the list. */
            result = u_participantAddTopic(p,_this);
        }
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
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            result = u_topicDeinit(_this);
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_topicFree",0,
                            "Operation u_topicDeinit failed: "
                            "Topic = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_topicFree",0,
                    "Operation u_entityLock failed: "
                    "Topic = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_topicDeinit(
    u_topic _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_participantRemoveTopic(_this->participant, _this);
        if (result == U_RESULT_OK) {
            result = u_dispatcherDeinit(u_dispatcher(_this));
            if (result == U_RESULT_OK) {
                if (_this->name) {
                    os_free(_this->name);
                    _this->name = NULL;
                }
            } else {
                OS_REPORT_1(OS_WARNING,
                            "u_topicDeinit", 0,
                            "Operation u_dispatcherDeinit failed. "
                            "Topic = 0x%x",
                            _this);
            }
        } else {
            OS_REPORT_2(OS_WARNING,
                        "u_topicDeinit", 0,
                        "The Topic (0x%x) could not be removed "
                        "from the Participant (0x%x).",
                        _this, _this->participant);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "u_topicDeinit", 0,
                  "Illegal parameter: Topic == NULL.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

c_char *
u_topicName(
    u_topic t)
{
    c_char *name;

    if (t) {
        name = os_strdup(t->name);
    } else {
        OS_REPORT(OS_WARNING, "u_topicName", 0,
                  "topic == NULL.");
        name = NULL;
    }
    return name;
}

c_char *
u_topicTypeName(
    u_topic t)
{
    v_topic kt;
    u_result r;
    c_char *name;


    r = u_entityReadClaim(u_entity(t),(v_entity*)(&kt));
    if (r == U_RESULT_OK) {
        assert(kt);
        name = (c_char *)c_metaScopedName(c_metaObject(v_topicDataType(kt)));
        u_entityRelease(u_entity(t));
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

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&topic));
    if (result == U_RESULT_OK) {
        assert(topic);
        r = v_topicGetInconsistentTopicStatus(topic,reset,action,arg);
        u_entityRelease(u_entity(_this));
        result = u_resultFromKernel(r);
    }
    return result;
}


u_result
u_topicGetAllDataDisposedStatus (
    u_topic _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_topic topic;
    u_result result;
    v_result r;

    result = u_entityReadClaim(u_entity(_this), (v_entity*)(&topic));
    if (result == U_RESULT_OK) {
        assert(topic);
        r = v_topicGetAllDataDisposedStatus(topic,reset,action,arg);
        u_entityRelease(u_entity(_this));
        result = u_resultFromKernel(r);
    }
    return result;
}

u_result
u_topicDisposeAllData (u_topic _this)
{
    v_topic topic;
    u_result result;
    v_result r;

    result = u_entityWriteClaim(u_entity(_this), (v_entity*)(&topic));
    if (result == U_RESULT_OK) {
        assert(topic);
        r = v_topicDisposeAllData(topic);
        u_entityRelease(u_entity(_this));
        result = u_resultFromKernel(r);
    }
    return result;
}

c_bool
u_topicIsBuiltin (
    u_topic _this)
{
    c_bool result = FALSE;

    if (_this) {
        result = (strncmp(_this->name, "DCPS", 4) == 0);
    }
    return result;
}

u_participant
u_topicParticipant (
    u_topic _this)
{
    return _this->participant;
}

c_bool
u_topicContentFilterValidate (
    u_topic _this,
    q_expr expr,
    c_value params[])
{
    v_topic topic;
    c_type type;
    c_bool result;
    q_expr subexpr, term;
    int i;
    v_filter filter;
    u_result uResult;

    result = FALSE;
    filter = NULL;
    uResult = u_entityReadClaim(u_entity(_this), (v_entity*)(&topic));
    if (uResult == U_RESULT_OK) {
        assert(topic);
        type = v_topicMessageType(topic);
        i = 0;
        subexpr = q_getPar(expr, i); /* get rid of Q_EXPR_PROGRAM */
        while ((term = q_getPar(subexpr, i++)) != NULL) {
            if (q_getTag(term) == Q_EXPR_WHERE) {
                filter = v_filterNew(topic, term, params);
            }
        }
        u_entityRelease(u_entity(_this));
    }
    if (filter != NULL) {
        result = TRUE;
        c_free(filter);
    }
    return result;
}

c_type
u_topicGetUserType (
    u_topic _this)
{
    v_topic topic;
    c_type type = NULL;
    u_result uResult = u_entityReadClaim(u_entity(_this), (v_entity*)(&topic));
    if (uResult == U_RESULT_OK) {
        assert(topic);
        type = v_topicGetUserType(topic);
        c_keep(type);
        u_entityRelease(u_entity(_this));
    }
    return type;
}

c_string
u_topicGetTopicKeys (
    u_topic _this)
{
    v_topic topic;
    c_string keys = NULL;
    u_result uResult = u_entityReadClaim(u_entity(_this), (v_entity*)(&topic));
    if (uResult == U_RESULT_OK) {
        assert(topic);
        keys = v_topicKeyExpr(topic);
        c_keep(keys);
        u_entityRelease(u_entity(_this));
    }
    return keys;
}
