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

#include "u__topic.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__domain.h"
#include "u__participant.h"
#include "u__user.h"

#include "v_kernel.h"
#include "v_topic.h"
#include "v_topicAdapter.h"
#include "v_topicQos.h"
#include "v_entity.h"
#include "v_filter.h"
#include "v_observable.h"
#include "v_observer.h"
#include "os_report.h"

#define u_topicReadClaim(_this, topic, claim) \
        u_observableReadClaim(u_observable(_this), (v_public *)(topic), claim)

#define u_topicWriteClaim(_this, topic, claim) \
        u_observableWriteClaim(u_observable(_this), (v_public *)(topic), claim)

#define u_topicRelease(_this, claim) \
        u_observableRelease(u_observable(_this), claim)

u_result
u__topicDeinitW(
    void *_this)
{
    u_result result;
    u_topic topic;
    u_participant p;
    v_participant kp;
    v_topic kt;

    assert(_this != NULL);

    topic = u_topic(_this);
    p = topic->participant;
    result = u_observableWriteClaim(u_observable(p), (v_public *)(&kp), C_MM_RESERVATION_NO_CHECK);
    if (result == U_RESULT_OK) {
        assert(kp);
        result = u_observableWriteClaim(u_observable(_this), (v_public *)(&kt), C_MM_RESERVATION_NO_CHECK);
        if (result == U_RESULT_OK) {
            assert(kt);
            (void)v_observableRemoveObserver(v_observable(kt), v_observer(kp), NULL);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_NO_CHECK);
        } else {
            OS_REPORT(OS_WARNING, "u__topicDeinitW", result,
                        "Claim topic failed. "
                        "Topic = 0x%"PA_PRIxADDR"", (os_address)_this);
        }
        u_observableRelease(u_observable(p), C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_WARNING, "u__topicDeinitW", result,
                    "Claim participant failed. "
                    "Topic = 0x%"PA_PRIxADDR"", (os_address)_this);
    }

    if ((result == U_RESULT_OK) ||
        (result == U_RESULT_ALREADY_DELETED) ||
        (result == U_RESULT_OUT_OF_MEMORY)) {
        result = u__entityDeinitW(_this);
        if (result == U_RESULT_OK) {
            if (topic->name) {
                os_free(topic->name);
                topic->name = NULL;
            }
        } else {
            OS_REPORT(OS_WARNING,
                        "u__topicDeinitW", result,
                        "Operation u__topicDeinitW failed. "
                        "Topic = 0x%"PA_PRIxADDR"",
                        (os_address)_this);
        }
    }

    return result;
}

void
u__topicFreeW(
    void *_vthis)
{
    u__entityFreeW(_vthis);
}

u_topic
u_topicNewFromTopicInfo(
    u_participant p,
    const struct v_topicInfo *info,
    c_bool announce)
{
    u_topic _this = NULL;
    v_topicAdapter kt;
    v_participant kp;
    u_result result;

    assert (p);
    assert (info);

    result = u_observableWriteClaim(u_observable(p),(v_public*)(&kp), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(kp);
        kt = v_topicAdapterNewFromTopicInfo(kp,info,announce);
        if (kt != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_TOPIC, u__topicDeinitW, u__topicFreeW);
            if (_this != NULL) {
                result = u_topicInit(_this,info->name,kt,p);
                if (result == U_RESULT_OK) {
                    result = u_observableWriteClaim(u_observable(p), (v_public *)(&kp), C_MM_RESERVATION_ZERO);
                    if (result == U_RESULT_OK) {
                        assert(kp);
                        v_observableAddObserver(v_observable(kt), v_observer(kp), NULL);
                        u_observableRelease(u_observable(p), C_MM_RESERVATION_ZERO);
                    }
                } else {
                    OS_REPORT(OS_ERROR, "u_topicNew", result,
                                "Initialisation failed. "
                                "For Topic: <%s>", info->name);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_topicNew", U_RESULT_OUT_OF_MEMORY,
                            "Create user proxy failed. "
                            "For Topic: <%s>", info->name);
            }
            c_free(kt);
        } else {
            OS_REPORT(OS_WARNING, "u_topicNewFromTopicInfo", U_RESULT_OUT_OF_MEMORY,
                      "Create kernel entity failed. "
                      "For Topic: <%s>", info->name);
        }
        u_observableRelease(u_observable(p),C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_topicNewFromTopicInfo", U_RESULT_INTERNAL_ERROR,
                  "Claim Kernel failed. "
                  "For Topic: <%s>", info->name);
    }
    return _this;
}

u_result
u_topicInit(
    const u_topic _this,
    const os_char *name,
    const v_topicAdapter kt,
    const u_participant p)
{
    u_result result;

    assert(_this != NULL);
    assert(name != NULL);

    result = u_entityInit(u_entity(_this),v_entity(kt), u_observableDomain(u_observable(p)));
    if (result == U_RESULT_OK) {
        _this->name = os_strdup(name);
        _this->participant = p;
    }
    return result;
}

u_topic
u_topicNew(
    const u_participant p,
    const os_char *name,
    const os_char *typeName,
    const os_char *keyList,
    u_topicQos qos)
{
    u_topic _this = NULL;
    v_topicAdapter kt;
    v_participant kp;
    u_result result;

    assert(name != NULL);
    assert(typeName != NULL);
    assert(p != NULL);

    result = u_observableWriteClaim(u_observable(p),(v_public *)(&kp), C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        assert(kp);
        kt = v_topicAdapterNew(kp,name,typeName,keyList,qos);
        if (kt != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_TOPIC, u__topicDeinitW, u__topicFreeW);
            if (_this != NULL) {
                result = u_topicInit(_this,name,kt,p);
                if (result == U_RESULT_OK) {
                    result = u_observableWriteClaim(u_observable(p), (v_public *)(&kp), C_MM_RESERVATION_LOW);
                    if (result == U_RESULT_OK) {
                        assert(kp);
                        v_observableAddObserver(v_observable(kt), v_observer(kp), NULL);
                        u_observableRelease(u_observable(p), C_MM_RESERVATION_LOW);
                    }
                } else {
                    OS_REPORT(OS_ERROR, "u_topicNew", result,
                                "Initialisation failed. "
                                "For Topic: <%s>", name);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_topicNew", U_RESULT_OUT_OF_MEMORY,
                            "Create user proxy failed. "
                            "For Topic: <%s>", name);
            }
            c_free(kt);
        } else {
            OS_REPORT(OS_WARNING, "u_topicNew", U_RESULT_OUT_OF_MEMORY,
                        "Create kernel entity failed. "
                        "For Topic: <%s>", name);
        }
        u_observableRelease(u_observable(p), C_MM_RESERVATION_HIGH);
    } else {
        OS_REPORT(OS_WARNING, "u_topicNew", U_RESULT_INTERNAL_ERROR,
                    "Claim Kernel failed. "
                    "For Topic: <%s>", name);
    }
    return _this;
}

u_result
u_topicGetQos (
    const u_topic _this,
    u_topicQos *qos)
{
    u_result result;
    v_topic vTopic;
    v_topicQos vQos;

    assert(_this);
    assert(qos);

    result = u_topicReadClaim(_this, &vTopic, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_topicGetQos(vTopic);
        *qos = u_topicQosNew(vQos);
        c_free(vQos);
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_topicSetQos (
    const u_topic _this,
    const u_topicQos qos)
{
    u_result result;
    v_topic vTopic;

    assert(_this);
    assert(qos);

    result = u_topicReadClaim(_this, &vTopic, C_MM_RESERVATION_HIGH);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_topicSetQos(vTopic, qos));
        u_topicRelease(_this, C_MM_RESERVATION_HIGH);
    }
    return result;
}

os_char *
u_topicName(
    const u_topic _this)
{
    assert(_this);

    return os_strdup(_this->name);
}

os_char *
u_topicTypeName(
    const u_topic _this)
{
    v_topic kt;
    u_result r;
    os_char *name;

    assert(_this);

    r = u_topicReadClaim(_this, &kt, C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(kt);
        name = (c_char *)c_metaScopedName(c_metaObject(v_topicDataType(kt)));
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_topicTypeName", r,
                  "Could not claim topic.");
        name = NULL;
    }
    return name;
}

os_uint32
u_topicTypeSize(
    const u_topic _this)
{
    v_topic kt;
    u_result r;
    os_uint32 size = 0;

    assert(_this);

    r = u_topicReadClaim(_this, &kt, C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(kt);
        size = (os_uint32)c_typeSize(v_topicDataType(kt));
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_topicTypeSize", r, "Could not claim topic.");
    }
    return size;
}

os_char *
u_topicKeyExpr(
    const u_topic _this)
{
    v_topic kt;
    u_result r;
    os_char *keyExpr;

    assert(_this);

    r = u_topicReadClaim(_this, &kt, C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(kt);
        keyExpr = os_strdup(v_topicKeyExpr(kt));
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_topicKeyExpr", r,
                  "Could not claim topic.");
        keyExpr = NULL;
    }
    return keyExpr;
}

u_result
u_topicGetInconsistentTopicStatus (
    const u_topic _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_topic topic;
    u_result result;
    v_result r;

    assert(_this);
    assert(action);

    result = u_topicReadClaim(_this, &topic,C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(topic);
        r = v_topicGetInconsistentTopicStatus(topic,reset,action,arg);
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
        result = u_resultFromKernel(r);
    }
    return result;
}

u_result
u_topicGetAllDataDisposedStatus (
    const u_topic _this,
    u_bool reset,
    u_statusAction action,
    void *arg)
{
    v_topic topic;
    u_result result;
    v_result r;

    assert(_this);
    assert(action);

    result = u_topicReadClaim(_this, &topic,C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(topic);
        r = v_topicGetAllDataDisposedStatus(topic,reset,action,arg);
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
        result = u_resultFromKernel(r);
    }
    return result;
}

u_result
u_topicDisposeAllData (
    const u_topic _this)
{
    v_topic topic;
    u_result result;
    v_result r;

    assert(_this);

    result = u_topicReadClaim(_this, &topic, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(topic);
        r = v_topicDisposeAllData(topic);
        u_topicRelease(_this, C_MM_RESERVATION_LOW);
        result = u_resultFromKernel(r);
    }
    return result;
}

u_bool
u_topicContentFilterValidate (
    const u_topic _this,
    const q_expr expr,
    const c_value params[])
{
    v_topic topic;
    u_bool result;
    q_expr subexpr, term;
    int i;
    v_filter filter;
    u_result uResult;

    assert(_this);
    assert(expr);

    result = FALSE;
    filter = NULL;
    uResult = u_topicReadClaim(_this, &topic, C_MM_RESERVATION_LOW);
    if (uResult == U_RESULT_OK) {
        assert(topic);
        i = 0;
        subexpr = q_getPar(expr, i); /* get rid of Q_EXPR_PROGRAM */
        while ((term = q_getPar(subexpr, i++)) != NULL) {
            if (q_getTag(term) == Q_EXPR_WHERE) {
                filter = v_filterNew(topic, term, params);
            }
        }
        u_topicRelease(_this, C_MM_RESERVATION_LOW);
    }
    if (filter != NULL) {
        result = TRUE;
        c_free(filter);
    }
    return result;
}

u_bool
u_topicContentFilterValidate2 (
    const u_topic _this,
    const q_expr expr,
    const c_value params[])
{
    v_topic topic;
    u_bool result;
    v_filter filter;
    u_result uResult;

    assert(_this);
    assert(expr);

    result = FALSE;
    filter = NULL;
    uResult = u_topicReadClaim(_this, &topic, C_MM_RESERVATION_LOW);
    if (uResult == U_RESULT_OK) {
        assert(topic);
        filter = v_filterNew(topic, expr, params);
        u_topicRelease(_this, C_MM_RESERVATION_LOW);
    }
    if (filter != NULL) {
        result = TRUE;
        c_free(filter);
    }
    return result;
}

os_char *
u_topicMetaDescriptor(
    const u_topic _this)
{
    v_topic kt;
    u_result r;
    os_char *descriptor;

    assert(_this);

    r = u_topicReadClaim(_this, &kt, C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(kt);
        descriptor = v_topicMetaDescriptor(kt);
        u_topicRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_topicMetaDescriptor", r,
                  "Could not claim topic.");
        descriptor = NULL;
    }
    return descriptor;
}

