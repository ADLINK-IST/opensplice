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
#include "u_networkReader.h"

#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__reader.h"
#include "u_subscriber.h"
#include "u__user.h"

#include "v_subscriber.h"
#include "v_topic.h"
#include "v_networkReader.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_entity.h"

#include "os_report.h"

#define TIME_CONVERSION_REQUIRED

static u_result
u_networkReaderInit(
    const u_networkReader _this,
    const v_networkReader reader,
    const u_subscriber subscriber)
{
    return u_readerInit(u_reader(_this),v_reader(reader),subscriber);
}

static u_result
u__networkReaderDeinitW(
    void *_this)
{
    return u__readerDeinitW(_this);
}

static void
u__networkReaderFreeW(
    void *_this)
{
    u__readerFreeW(_this);
}

u_networkReader
u_networkReaderNew(
    const u_subscriber s,
    const os_char *name,
    const u_readerQos qos,
    u_bool ignoreReliabilityQoS)
{
    u_networkReader _this = NULL;
    v_subscriber ks = NULL;
    v_networkReader kn;
    u_result result;

    assert(s);

    if (name == NULL) {
        name = "No name specified";
    }

    result = u_observableWriteClaim(u_observable(s), (v_public *)(&ks), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(ks);
        kn = v_networkReaderNew(ks, name, qos, ignoreReliabilityQoS);
        if (kn != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_NETWORKREADER, u__networkReaderDeinitW, u__networkReaderFreeW);
            if (_this != NULL) {
                result = u_networkReaderInit(_this,kn,s);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_networkReaderNew", result,
                                "Initialisation failed. "
                                "For networkReader: <%s>.", name);
                    u_objectFree(u_object(_this));
                    _this = NULL;
                }
            } else {
                result = U_RESULT_INTERNAL_ERROR;
                OS_REPORT(OS_ERROR, "u_networkReaderNew", result,
                            "Create user proxy fialed. "
                            "For networkReader: <%s>.", name);
            }
            c_free(kn);
        } else {
            OS_REPORT(OS_ERROR, "u_networkReaderNew", U_RESULT_INTERNAL_ERROR,
                        "Create kernel entity failed. "
                        "For networkReader: <%s>.", name);
        }
        u_observableRelease(u_observable(s), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_networkReaderNew", result,
                    "Claim Subscriber failed. "
                    "For networkReader: <%s>.", name);
    }
    return _this;
}

u_result
u_networkReaderCreateQueue(
    const u_networkReader _this,
    os_uint32 queueSize,
    os_uint32 priority,
    u_bool reliable,
    u_bool P2P,
    os_duration resolution,
    u_bool useAsDefault,
    os_uint32 *queueId,
    const os_char *name)
{
    v_networkReader kn;
    u_result result = U_RESULT_OK;

    assert(_this);
    assert(queueId);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&kn), C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        assert(kn);
        *queueId = v_networkReaderCreateQueue(kn,queueSize, priority,
                                              reliable, P2P,
                                              resolution, useAsDefault, name);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_WARNING, "u_networkReaderCreateQueue", result,
                  "Claim networkReader failed.");
    }
    return result;
}

u_result
u_networkReaderTrigger(
    const u_networkReader _this,
    os_uint32 queueId)
{
    v_networkReader kn;
    u_result result = U_RESULT_OK;

    assert(_this);

    if (queueId != 0) {
        result = u_observableReadClaim(u_observable(_this), (v_public *)(&kn), C_MM_RESERVATION_ZERO);
        if (result == U_RESULT_OK) {
            assert(kn);
            v_networkReaderTrigger(kn,queueId);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderTrigger", result,
                      "Claim networkReader failed.");
        }
    } else {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_networkReaderTrigger", result,
                  "Illegal parameter.");
    }
    return result;
}

u_result
u_networkReaderRemoteActivityDetected(
    const u_networkReader _this)
{
    v_networkReader kn;
    u_result result;

    assert(_this);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&kn), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(kn);
        v_networkReaderRemoteActivityDetected(kn);
        u_observableRelease(u_observable(_this),C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_networkReaderRemoteActivityDetected", result,
                  "Claim networkReader failed.");
    }
    return result;
}

u_result
u_networkReaderRemoteActivityLost(
    const u_networkReader _this)
{
    v_networkReader kn;
    u_result result;

    if (_this != NULL) {
        result = u_observableReadClaim(u_observable(_this), (v_public *)(&kn), C_MM_RESERVATION_ZERO);
        if(result == U_RESULT_OK)
        {
            assert(kn);
            v_networkReaderRemoteActivityLost(kn);
            u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
        } else {
            OS_REPORT(OS_WARNING, "u_networkReaderRemoteActivityLost", result,
                  "Claim networkReader failed.");
        }
    } else {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_networkReaderRemoteActivityLost", result,
          "Illegal parameter.");
    }
    return result;
}

