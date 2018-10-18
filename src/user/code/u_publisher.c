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

#include "u_publisher.h"
#include "u_writer.h"
#include "u__topic.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__user.h"

#include "v_participant.h"
#include "v_publisher.h"
#include "v_publisherQos.h"
#include "v_group.h"

#include "os_report.h"

#define u_publisherReadClaim(_this, pub, claim) \
        u_observableReadClaim(u_observable(_this), (v_public *)(pub), claim)

#define u_publisherWriteClaim(_this, pub, claim) \
        u_observableWriteClaim(u_observable(_this), (v_public *)(pub), claim)

#define u_publisherRelease(_this, claim) \
        u_observableRelease(u_observable(_this), claim)

static u_result
u__publisherDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

static void
u__publisherFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

static u_result
u_publisherInit(
    const u_publisher _this,
    const v_publisher publisher,
    const u_participant participant)
{
    u_result result;

    if (_this) {
        result = u_entityInit(u_entity(_this), v_entity(publisher), u_observableDomain(u_observable(participant)));
    } else {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR,
                    "u_publisherInit", result,
                    "Illegal parameter: _this = 0x%"PA_PRIxADDR".",
                    (os_address)_this);
    }
    return result;
}

u_publisher
u_publisherNew(
    const u_participant participant,
    const os_char *name,
    const u_publisherQos qos,
    u_bool enable)
{
    u_publisher _this = NULL;
    v_publisher o;
    v_participant kp = NULL;
    u_result result;

    assert(participant);

    if (name == NULL) {
        name = "No name specified";
    }
    if (participant!= NULL) {
        result = u_observableWriteClaim(u_observable(participant),(v_public*)(&kp), C_MM_RESERVATION_LOW);
        if (result == U_RESULT_OK) {
            assert(kp);
            o = v_publisherNew(kp, name, qos, enable);
            if (o != NULL) {
                _this = u_objectAlloc(sizeof(*_this), U_PUBLISHER, u__publisherDeinitW, u__publisherFreeW);
                if (_this != NULL) {
                    result = u_publisherInit(_this, o, participant);
                    if (result != U_RESULT_OK) {
                        OS_REPORT(OS_ERROR, "u_publisherNew", result,
                                    "Initialisation failed. "
                                    "For Publisher: <%s>.", name);
                        u_objectFree (u_object (_this));
                        _this = NULL;
                    }
                } else {
                    OS_REPORT(OS_ERROR, "u_publisherNew", U_RESULT_INTERNAL_ERROR,
                                "Create proxy failed. "
                                "For Publisher: <%s>.", name);
                }
                c_free(o);
            } else {
                OS_REPORT(OS_ERROR, "u_publisherNew", U_RESULT_INTERNAL_ERROR,
                            "Create kernel entity failed. "
                            "For Publisher: <%s>.", name);
            }
            u_observableRelease(u_observable(participant), C_MM_RESERVATION_LOW);
        } else {
            OS_REPORT(OS_WARNING, "u_publisherNew", result,
                        "Claim Participant (0x%"PA_PRIxADDR") failed. "
                        "For Publisher: <%s>.", (os_address)participant, name);
        }
    } else {
        OS_REPORT(OS_ERROR,"u_publisherNew", U_RESULT_ILL_PARAM,
                    "No Participant specified. "
                    "For Publisher: <%s>", name);
    }
    return _this;
}

u_result
u_publisherGetQos (
    const u_publisher _this,
    u_publisherQos *qos)
{
    u_result result;
    v_publisher vPublisher;
    v_publisherQos vQos;

    assert(_this);
    assert(qos);

    result = u_publisherReadClaim(_this, &vPublisher, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        vQos = v_publisherGetQos(vPublisher);
        *qos = u_publisherQosNew(vQos);
        c_free(vQos);
        u_publisherRelease(_this, C_MM_RESERVATION_ZERO);
    }
    // Coverity false positive: leaking qos: vQos is freed locally, 
    // qos is passed to caller and is freed there
    // coverity [leaked_storage : FALSE]
    return result;
}

u_result
u_publisherSetQos (
    const u_publisher _this,
    const u_publisherQos qos)
{
    u_result result;
    v_publisher vPublisher;

    assert(_this);
    assert(qos);

    result = u_publisherReadClaim(_this, &vPublisher, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        result = u_resultFromKernel(v_publisherSetQos(vPublisher, qos));
        u_publisherRelease(_this, C_MM_RESERVATION_LOW);
    }
    return result;
}

u_result
u_publisherSuspend(
    const u_publisher _this)
{
    v_publisher kp;
    u_result result;

    assert(_this);

    result = u_publisherReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        assert(kp);
        v_publisherSuspend(kp);
        u_publisherRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_publisherSuspend", result,
                    "Claim Publisher (0x%"PA_PRIxADDR") failed.", (os_address)_this);
     }
    return result;
}

u_result
u_publisherResume(
    const u_publisher _this)
{
    v_publisher kp;
    u_result result;
    u_bool resumed;

    assert(_this);

    result = u_publisherReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        assert(kp);
        resumed = v_publisherResume(kp);
        u_publisherRelease(_this, C_MM_RESERVATION_ZERO);
        if (resumed == FALSE) {
            result = U_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR, "u_publisherResume", result,
                        "Resume Publisher (0x%"PA_PRIxADDR") failed.", (os_address)_this);
        }
    } else {
        OS_REPORT(OS_WARNING, "u_publisherResume", result,
                    "Claim Publisher (0x%"PA_PRIxADDR") failed.", (os_address)_this);
    }
    return result;
}

u_result
u_publisherCoherentBegin(
    const u_publisher _this)
{
    v_publisher kp;
    u_result result;

    assert(_this);

    result = u_publisherReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        assert(kp);
        result = u_resultFromKernel(v_publisherCoherentBegin(kp));
        u_publisherRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_publisherCoherentBegin", result,
                    "Claim Publisher (0x%"PA_PRIxADDR") failed.", (os_address)_this);
    }
    return result;
}

u_result
u_publisherCoherentEnd(
    const u_publisher _this)
{
    v_publisher kp;
    u_result result;

    assert(_this);

    result = u_publisherReadClaim(_this, &kp, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK){
        assert(kp);
        result = u_resultFromKernel(v_publisherCoherentEnd(kp));
        u_publisherRelease(_this, C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_publisherCoherentEnd", result,
                    "Claim Publisher (0x%"PA_PRIxADDR") failed.", (os_address)_this);
    }
    return result;
}

