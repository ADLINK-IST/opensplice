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

#include "u_subscriber.h"
#include "u_dataReader.h"
#include "u_networkReader.h"
#include "u_groupQueue.h"
#include "u_dataView.h"
#include "u__types.h"
#include "u__user.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"
#include "v_participant.h"
#include "v_subscriber.h"
#include "v_group.h"
#include "os_report.h"

#define u_subscriberReadClaim(_this, sub, claim) \
        u_observableReadClaim(u_observable(_this), (v_public *)(sub), claim)

#define u_subscriberWriteClaim(_this, sub, claim) \
        u_observableWriteClaim(u_observable(_this), (v_public *)(sub), claim)

#define u_subscriberRelease(_this, claim) \
        u_observableRelease(u_observable(_this), claim)

static u_result
u__subscriberDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

static void
u__subscriberFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

static u_result
u_subscriberInit(
    u_subscriber _this,
    v_subscriber ks,
    const u_participant participant)
{
    assert(_this);
    return u_entityInit(u_entity(_this), v_entity(ks), u_observableDomain(u_observable(participant)));
}

u_subscriber
u_subscriberNew(
    const u_participant participant,
    const os_char *name,
    const u_subscriberQos qos,
    u_bool enable)
{
    u_subscriber _this = NULL;
    v_subscriber ks;
    v_participant kp = NULL;
    u_result result;

    assert(participant);

    if (name == NULL) {
        name = "No name specified";
    }

    result = u_observableWriteClaim(u_observable(participant),(v_public*)(&kp), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(kp);
        ks = v_subscriberNew(kp, name, qos, enable);
        if (ks != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_SUBSCRIBER, u__subscriberDeinitW, u__subscriberFreeW);
            if (_this != NULL) {
                result = u_subscriberInit(_this, ks, participant);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_subscriberNew", result,
                                "Initialisation failed. "
                                "For DataReader: <%s>.", name);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_subscriberNew", result,
                            "Create user proxy failed. "
                            "For Subscriber: <%s>.", name);
            }
            c_free(ks);
        } else {
            OS_REPORT(OS_ERROR, "u_subscriberNew", U_RESULT_OUT_OF_MEMORY,
                        "Create kernel entity failed. "
                        "For Subscriber: <%s>.", name);
        }
        u_observableRelease(u_observable(participant), C_MM_RESERVATION_LOW);
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_WARNING, "u_subscriberNew", result,
                        "Could not release participant."
                        "However subscriber <%s> is created.", name);
        }
    } else {
        OS_REPORT(OS_WARNING, "u_subscriberNew", result,
                    "Claim Participant failed. "
                    "For Subscriber: <%s>.", name);
    }
    return _this;
}

u_result
u_subscriberGetQos (
    const u_subscriber _this,
    u_subscriberQos *qos)
{
    u_result result;
    v_subscriber vSubscriber;
    v_subscriberQos vQos;

    assert(_this);

    result = u_subscriberReadClaim(_this, &vSubscriber, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_subscriberGetQos(vSubscriber);
        *qos = u_subscriberQosNew(vQos);
        c_free(vQos);
        u_subscriberRelease(_this, C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_subscriberSetQos (
    const u_subscriber _this,
    const u_subscriberQos qos)
{
    u_result result;
    v_subscriber vSubscriber;

    assert(_this);
    assert(qos);

    result = u_subscriberReadClaim(_this, &vSubscriber, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_subscriberSetQos(vSubscriber, qos));
        u_subscriberRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_subscriberBeginAccess(
    const u_subscriber _this)
{
    u_result result;
    v_subscriber vSubscriber;

    assert(_this);
    result = u_subscriberReadClaim(_this, &vSubscriber, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = v_subscriberBeginAccess(vSubscriber);
        u_subscriberRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_subscriberEndAccess(
    const u_subscriber _this)
{
    u_result result;
    v_subscriber vSubscriber;

    assert(_this);
    result = u_subscriberReadClaim(_this, &vSubscriber, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = v_subscriberEndAccess(vSubscriber);
        u_subscriberRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_dataReader
u_subscriberCreateDataReader (
    const u_subscriber _this,
    const os_char *name,
    const os_char *expression,
    const c_value params[],
    const u_readerQos qos,
    u_bool enable)
{
    assert(_this);
    return u_dataReaderNew(_this, name, expression, params, qos, enable);
}

static c_bool
getDataReadersAction (
    v_dataReader reader,
    c_voidp arg)
{
    u_dataReader r;
    c_iter list = (c_iter)arg;

    assert(reader);
    assert(list);

    r = u_dataReader(v_entityGetUserData(v_entity(reader)));
    c_iterAppend(list, r);

    return TRUE;
}

u_result
u_subscriberGetDataReaders (
    const u_subscriber _this,
    u_sampleMask mask,
    c_iter *readers)
{
    u_result result;
    v_subscriber subscriber;
    c_iter list;

    assert(_this);
    assert(readers);

    result = u_subscriberReadClaim(_this, &subscriber, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        list = c_iterNew(NULL);
        result = u_resultFromKernel(v_subscriberGetDataReaders(subscriber, mask, getDataReadersAction, list));
        if (result != U_RESULT_OK) {
            c_iterFree(list);
            list = NULL;
        }
        *readers = list;
        u_subscriberRelease(_this, C_MM_RESERVATION_ZERO);
    }

    return result;
}

typedef struct createDataReadersActionArg {
    c_iter list;
    u_domain domain;
} *createDataReadersActionArg;

static c_bool
createDataReadersAction (
    v_dataReader reader,
    c_voidp arg)
{
    u_observable o;
    createDataReadersActionArg a = (createDataReadersActionArg)arg;
    u_result result;

    assert(reader);
    assert(arg);

    o = u_objectAlloc(sizeof(struct u_reader_s), U_READER, u__observableProxyDeinitW, u__observableProxyFreeW);
    if (o) {
        result = u_observableInit(o, v_public(reader), a->domain);
        if (result != U_RESULT_OK) {
            u_objectFree(o);
            o = NULL;
        }
    }
    c_iterAppend(a->list, o);

    return TRUE;
}

u_result
u_subscriberGetDataReaderProxies (
    const u_subscriber _this,
    u_sampleMask mask,
    c_iter *readers)
{
    u_result result;
    v_subscriber subscriber;
    struct createDataReadersActionArg arg;

    assert(_this);
    assert(readers);

    result = u_subscriberReadClaim(_this, &subscriber, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        arg.list = c_iterNew(NULL);
        arg.domain = u_observableDomain(u_observable(_this));
        result = u_resultFromKernel(v_subscriberGetDataReaders(subscriber, mask, createDataReadersAction, &arg));
        if (result != U_RESULT_OK) {
            c_iterFree(arg.list);
            arg.list = NULL;
        }
        *readers = arg.list;
        u_subscriberRelease(_this, C_MM_RESERVATION_ZERO);
    }

    return result;
}
